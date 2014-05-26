/***********************************************************************
**
**   flarmbincomlinux.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
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

  // qDebug("%02X ", c);

  while(true)
    {
      done = write( m_Socket, &c, sizeof(c) );

      if( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          qDebug() << "writeCharErr" << errno << strerror(errno);
        }

      break;
    }

  return done;
}

int FlarmBinComLinux::readChar(unsigned char* b)
{
  // Note, non blocking IO is set on our file descriptor.
  int done = read( m_Socket, b, sizeof(unsigned char) );

  if( done > 0 )
    {
      // qDebug("%02X ", *b);
      return true;
    }

  if( done == 0 || (done == -1 && errno != EWOULDBLOCK) )
    {
      qDebug() << "readCharErr" << errno << strerror(errno);
      return false;
    }

  // No data available, wait for it until timeout
  int maxFds = getdtablesize();

  fd_set readFds;
  FD_ZERO( &readFds );
  FD_SET( m_Socket, &readFds );

  struct timeval timerInterval;
  timerInterval.tv_sec  =  3; // 3s timeout
  timerInterval.tv_usec =  0;

  done = select( maxFds, &readFds, (fd_set *) 0, (fd_set *) 0, &timerInterval );

  if( done == 0 )
    {
      qDebug() << "select() Timeout";
      // done = 0  -> Timeout
      return done;
    }

  if( done < 0 )
    {
      qWarning() << "select() Err" << errno << strerror(errno);
      // done = -1 -> Error
      return done;
    }

  done = read( m_Socket, b, sizeof(unsigned char) );

  // qDebug("%02X ", *b);
  return done;
}
