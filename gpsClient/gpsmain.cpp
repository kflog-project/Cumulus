/***********************************************************************
**
**   gpsmain.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2016 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

using namespace std;

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <libgen.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <iostream>

#include <QtCore>

#include "signalhandler.h"
#include "gpsclient.h"
#include "gpscon.h"

//----------------------------------------------------
// import global sutdown flag, set by SignalHandler
//----------------------------------------------------

extern bool shutdownState;

// ===========================================================================
// Usage of programm
// ===========================================================================
void usage( char * progName )
{
  cerr << "Usage: " << basename(progName) << " options ..." << endl
       << "       -help   (optional)  display this usage" << endl
       << "       -port   (mandatory) socket port for IPC to server" << endl
       << "       -slave  (optional)  process is running as slave" << endl
       << endl;

  exit(2);
}

/**
 * Main of GPS client program. The program has three options
 *
 * a) -help shows the usage to the caller
 * b) -port Port number of listening end point of cumulus process
 * c) -slave if this option is set, process will terminate, if parent
 *           has gone down (zombie protection).
 *
 * This module checks the passed options and handles the socket events.
 *
 */
int main( int argc, char* argv[] )
{
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

#ifdef QT_5
  // install message handler
  qInstallMessageHandler(0);
#else
  qInstallMsgHandler(0);
#endif

  // install signal handler for catching termination requests
  initSignalHandler();

  // GPS client module, manages the connection to the GPS and to cumulus
  GpsClient *client = new GpsClient( ipcPort );

  struct timeval timerInterval;

  // ==========================================================================
  // main loop of Gps Client process
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

      int maxFds = getdtablesize();  // getdtablehi() better but not provided

      // get all session file descriptors
      fd_set *readFds = client->getReadFdMask();

      // main loop timeout set to one second
      timerInterval.tv_sec  =  1;
      timerInterval.tv_usec =  0;

      // Wait for read events or timeout

      int result = select( maxFds, readFds, (fd_set *) 0,
                          (fd_set *) 0, &timerInterval );

      if( result == -1 ) // Select returned with error
        {
          if( errno == EINTR )
            {
              continue; // interrupted select call, ignore it
            }
          else
            {
              // other error occurred, report it
              cerr << "Fatal Error of select call, errno=" << errno
                   << ", " << strerror(errno)
                   << "\n +++ TERMINATE PROCESS +++" << endl;
              break;
            }
        }

      else if( result > 0 ) // read event occurred
        {
          // call gps client for event processing
          client->processEvent( readFds );
        }
      else if( result == 0 ) // timeout, do low prioritized things
        {
        }

      // call timeout control at last after all events have been
      // processed
      client->toController();

    } // End of while

  if( !strcmp(client->getDevice(), NMEASIM_DEVICE) )
    {
      // delete fifo: we leave the system as we found;
      // if start first cumulus and after that nmea simulator,
      // it takes up to 30 seconds until connection is established
      // if we would not delete it, we would get nearly immediate connection
      unlink(NMEASIM_DEVICE);
    }

  delete client; // shutdown clients activities

  return 0;
} // End of main
