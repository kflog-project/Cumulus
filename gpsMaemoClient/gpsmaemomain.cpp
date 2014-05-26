/***********************************************************************
**
**   gpsmaemomain.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2010-2011 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <iostream>

#include <glib.h>
#include <glib-object.h>

#include <QtCore>

#include "signalhandler.h"
#include "gpsmaemoclient.h"

//----------------------------------------------------
// import global shutdown flag, set by SignalHandler
//----------------------------------------------------
extern bool shutdownState;

// ===========================================================================
// Usage of program
// ===========================================================================
void usage( char * progName )
{
  cerr << "Usage: " << basename(progName) << " options ..." << endl
       << "       -help   (optional)  display this usage" << endl
       << "       -port   (mandatory) socket port for IPC to server" << endl
       << "       -slave  (optional)  process is running as slave" << endl
       << endl;

  exit (2);
}

/**
 * Main of GPS Maemo client program. The program has three options
 *
 * a) -help shows the usage to the caller
 * b) -port Port number of listening end point of Cumulus process
 * c) -slave if this option is set, process will terminate, if parent
 *           has gone down (zombie protection).
 *
 * This module checks the passed options and handles the socket events.
 *
 */
int main( int argc, char* argv[] )
{
  static QString method( "Main(): " );

  unsigned short ipcPort = 0;
  bool           slave   = false;

  if ( argc < 3 )
    {
      cerr << "Missing mandatory arguments" << endl << endl;
      usage(argv[0]);
    }

  // process passed arguments
  int i = 1;
  int res;

  while ( i < argc )
    {
      if ( strcmp( argv[i], "-help" ) == 0 )
        {
          usage(argv[0]);
        }
      else if ( strcmp( argv[i], "-slave" ) == 0 )
        {
          slave = true; // Running as slave
          i++;
        }
      else if ( strcmp( argv[i], "-port" ) == 0 )
        {
          if ( i+1 < argc )
            {
              res = sscanf( argv[i+1], "%hu", &ipcPort );

              if ( res != 1 ) // conversion error occurred
                {
                  cerr << basename(argv[0])
                       << ": wrong -port argument passed!"
                       << endl;
                  usage( argv[0] );
                }

              i += 2;
            }
          else
            {
              cerr << basename(argv[0])
                   << ": missing argument!"
                   << endl;
              usage( argv[0] );
            }
        }
      else
        {
          cerr << basename(argv[0]) << ": unknown option "
               << argv[i] << endl;
          usage( argv[0] );
        }
    } // End of while

  if ( ! ipcPort ) // port is a mandatory argument
    {
      cerr << basename(argv[0]) << ": missing port option!" << endl;
      usage( argv[0] );
    }

  // install signal handler for catching termination requests
  initSignalHandler();

  // Initialize GObject type system
  g_type_init();

  // Initialize the GLib thread system
  g_thread_init( 0 );

  // Setup GLib main loop for signal handling of libLocation.
  GMainLoop *gloop = g_main_loop_new( 0, FALSE );

  // GPS client module, manages the connection between the libLocation and Cumulus.
  GpsMaemoClient *client = new GpsMaemoClient( ipcPort );

  struct timeval timerInterval;

  fd_set *readFds;
  fd_set writeFds;

  // ==========================================================================
  // Main loop of Gps Client process
  // ==========================================================================

  while( true ) // process event loop
    {
      // check, if GpsClient instance has set shutdown flag or if
      // signal handler was called for shutdown
      if ( client->getShutdownFlag() || shutdownState )
        {
          cerr << "Gps client main loop has discovered "
               << "shutdown request" << endl;
          break;
        }

      // Check, if process is running as slave and the parent is
      // alive. When not, initiate shutdown of own process.
      if ( slave && getppid() == 1 )
        {
          cerr << "Cumulus has going down, "
               << "because ppid has changed to 1. Follow it." << endl;
          break;
        }

      // Get read file descriptors to Cumulus and to GPSD (only Maemo 4).
      readFds = client->getReadFdMask();

      FD_ZERO( &writeFds );

      // Get all G-Lib main loop file descriptors.
      gint max_priority = G_PRIORITY_DEFAULT;
      gint gTimeout = 0;  // desired timeout in ms
      gint n_fds = 32;
      GPollFD fds[n_fds];

#ifdef MAEMO5
      // Maemo 4 complains with error to this function call.
      g_main_context_prepare( g_main_context_default(), &max_priority );
#endif

      // Returns the number of current file descriptors in use by g_main_loop.
      // Can be greater as n_fds but 32 should be sufficient!
      gint c_fds = g_main_context_query( g_main_context_default(),
                                         max_priority,
                                         &gTimeout,
                                         &fds[0],
                                         n_fds );
      if( c_fds > n_fds )
        {
          cerr << "GpsMaemoClient: G-MainLoop needs more FDs as are reserved! "
               << c_fds << " > " << n_fds << endl;
        }

      for( int i = 0; i < c_fds && i < n_fds; i++ )
        {
          // Add returned file descriptors to select masks.
          if( fds[i].events & G_IO_IN )
            {
              FD_SET( fds[i].fd, readFds );
            }

          if( fds[i].events & G_IO_OUT )
            {
              FD_SET( fds[i].fd, &writeFds );
            }
        }

      // main loop timeout set to 1 second as default
      timerInterval.tv_sec  =  1;
      timerInterval.tv_usec =  0;

      if( gTimeout > 0 )
        {
          // G-Main loop has provided a timeout value in milliseconds.
          timerInterval.tv_sec  =  gTimeout / 1000;
          timerInterval.tv_usec =  (gTimeout % 1000) * 1000;
        }

      int maxFds = getdtablesize();  // getdtablehi() better but not provided

      // Wait for read events or timeout
      int result = select( maxFds, readFds, &writeFds,
                          (fd_set *) 0, &timerInterval );

      if( result == -1 ) // Select returned with error
        {
          if ( errno == EINTR )
            {
              // Interrupted select call, ignore error.
              continue;
            }
          else
            {
              // Other error occurred, report it.
              cerr << "Fatal Error of select call, errno=" << errno
                   << ", " << strerror(errno)
                   << "\n +++ TERMINATE PROCESS +++" << endl;
              break;
            }
        }

      if( result > 0 ) // read/write event occurred
        {
          // Process pending G-main loop events. Because the function
          // g_main_context_iteration do not dispatch all pending events at once
          // it must be called recursive in a limited loop. In my tests I saw 6
          // loops as maximum. I hope that 50 loops are sufficient.
          int loops = 50;

          while( loops &&
                 g_main_context_iteration( g_main_context_default(), FALSE ) )
            {
              loops--;
            }

          // qDebug() << "G-Main-Loops=" << 50-loops;

          // Call Gps Maemo client for event processing
          client->processEvent( readFds );
        }
      else if ( result == 0 ) // timeout, do low prioritized things
        {
          // Call G-Main loop to handle its expired timers.
          g_main_context_iteration( g_main_context_default(), FALSE );
        }

      // call timeout control at last after all events have been
      // processed
      client->toController();

    } // End of while

  g_main_loop_unref( gloop ); // destroy G-main loop

  delete client; // shutdown clients activities

  return(0);
}
