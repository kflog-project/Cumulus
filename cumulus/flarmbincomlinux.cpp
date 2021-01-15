/***********************************************************************
**
**   flarmbincomlinux.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012-2021 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

#include <cerrno>
#include <unistd.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <QtGui>

#include "flarmbincomlinux.h"

// #define DEBUG 1

FlarmBinComLinux::FlarmBinComLinux( int socket ) :
  FlarmBinCom(),
  m_Socket(socket)
{
}

FlarmBinComLinux::~FlarmBinComLinux()
{
}

int FlarmBinComLinux::writeChar(const unsigned char c)
{
  int done = -1;

#ifdef DEBUG
  qDebug("%02X ", c);
#endif

  while(true)
    {
      done = write( m_Socket, &c, sizeof(c) );

      if( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          qDebug() << "FlarmBinComLinux::writeCharErr" << errno << strerror(errno);
        }

      break;
    }

  return done;
}

int FlarmBinComLinux::readChar(unsigned char* b, const int timeout)
{
  // Note, non blocking IO is set on our file descriptor.
  int done = read( m_Socket, b, sizeof(unsigned char) );

  if( done > 0 )
    {
#ifdef DEBUG
      qDebug("%02X ", *b);
#endif
      return true;
    }

  if( done == 0 || (done == -1 && errno != EWOULDBLOCK) )
    {
      qDebug() << "FlarmBinComLinux::readCharErr" << errno << strerror(errno);
      return false;
    }

  // No data available, wait for it until timeout
  int maxFds = getdtablesize();

  fd_set readFds;
  FD_ZERO( &readFds );
  FD_SET( m_Socket, &readFds );

  struct timeval timerInterval;
  timerInterval.tv_sec  = timeout / 1000;
  timerInterval.tv_usec = (timeout % 1000) * 1000;

  done = select( maxFds, &readFds, (fd_set *) 0, (fd_set *) 0, &timerInterval );

  if( done == 0 )
    {
      qDebug() << "FlarmBinComLinux::readChar: select() Timeout" << timeout/1000 << "s";
      // done = 0  -> Timeout
      return done;
    }

  if( done < 0 )
    {
      qWarning() << "FlarmBinComLinux::readChar: select() Err" << errno << strerror(errno);
      // done = -1 -> Error
      return done;
    }

  done = read( m_Socket, b, sizeof(unsigned char) );

#ifdef DEBUG
  qDebug("%02X ", *b);
#endif

  return done;
}
