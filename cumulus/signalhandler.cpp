/***********************************************************************
**
**   signalhandler.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2009 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#include <stdio.h>
#include <signal.h>

// Global flags used by other methods to get the related
// info. Shutdown can be initiated by catching signal SIGHUP, SIGINT,
// SIGTERM

bool shutdownState  = false;
bool childDeadState = false;
bool brockenPipe    = false;

void signalHandler( int signal );


/**
 * Signal handler routines
 */
void initSignalHandler()
{
  sigset_t sigset;

  sigemptyset( &sigset );  // set all signals in sigset

  sigaddset( &sigset, SIGHUP );
  sigaddset( &sigset, SIGINT );
  sigaddset( &sigset, SIGTERM );
  sigaddset( &sigset, SIGCHLD );
  sigaddset( &sigset, SIGPIPE );

  // activate desired signals

  sigprocmask( SIG_UNBLOCK, &sigset, 0 );

  // set up signal handler for activated signals

  struct sigaction act;

  act.sa_handler = signalHandler; // assign signal handler routine

  // We don't want to block any other signals
  sigemptyset(&act.sa_mask);

  // We're only interested in children that have terminated, not ones
  // which have been stopped (eg user pressing control-Z at terminal)

  act.sa_flags = SA_NOCLDSTOP | SA_RESTART;

  if ( sigaction(SIGHUP, &act, 0) < 0 )
    {
      fprintf( stderr, "sigaction for signal SIGHUP failed\n" );
    }

  if ( sigaction(SIGINT, &act, 0) < 0 )
    {
      fprintf( stderr, "sigaction for signal SIGINT failed\n" );
    }

  if ( sigaction(SIGTERM, &act, 0) < 0 )
    {
      fprintf( stderr, "sigaction for signal SIGTERM failed\n" );
    }

  if ( sigaction(SIGCHLD, &act, 0) < 0 )
    {
      fprintf( stderr, "sigaction for signal SIGCHLD failed\n" );
    }

  if ( sigaction(SIGPIPE, &act, 0) < 0 )
    {
      fprintf( stderr, "sigaction for signal SIGPIPE failed\n" );
    }

  return;
}


/** Signal handler */
void signalHandler( int sig )
{
  const char* signal = "";

  switch( sig )
  {
    case SIGCHLD:
      signal = "SIGCHLD";
      // change in status of child
      childDeadState = true;
      break;

    case SIGHUP:
      signal = "SIGHUP";
      // shutdown requested
      shutdownState = true;
      break;

    case SIGTERM:
      signal = "SIGTERM";
      // shutdown requested
      shutdownState = true;
      break;

    case SIGINT:
      signal = "SIGINT";
      // shutdown requested
      shutdownState = true;
      break;

    case SIGPIPE:
      signal = "SIGPIPE";
      // shutdown requested
      brockenPipe = true;
      break;

    default:
      signal = "Unknown";
  }

  fprintf( stderr, "Signal %d, %s caught\n", sig, signal );
}
