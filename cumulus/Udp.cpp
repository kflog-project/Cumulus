/***********************************************************************
 **
 **   Udp.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2018 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 ***********************************************************************/

using namespace std;

#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <QtCore>

#include "Udp.h"

/**
 * Constructor
 */
Udp::Udp(QObject *parent, QString serverIpAddress, ushort port ) :
    QObject(parent),
    m_ipAddress(serverIpAddress),
    m_port(port),
    m_socket(0)
{
  m_socket = socket( AF_INET, SOCK_DGRAM, 0 );

  if ( m_socket == -1 )
    {
      qWarning() << "Udp:"
                 << "Cannot create socket: errno="
                 << errno
                 << "," << strerror(errno);
      return;
    }

  int opt = 1;

  setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (int *)&opt, sizeof(opt));
  setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (int *)&opt, sizeof(opt));

  memset( &m_sockaddr, 0, sizeof( sockaddr_in ) );

  m_sockaddr.sin_family = AF_INET;

  // Internet host address in numbers-and-dots notation
  if ( m_ipAddress.size() == 0 )
    {
      m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
  else
    {
      if( inet_aton( m_ipAddress.toLatin1(), &m_sockaddr.sin_addr ) == 0 )
        {
          qWarning() << "Udp"
                     << "inet_aton: invalid IP address format ("
                     << serverIpAddress << ") errno="
                     << errno
                     << "," << strerror(errno);
          return;
        }
    }

  if ( port > 0 )
    {
      m_sockaddr.sin_port = htons(port);
    }
  else
    {
      // OS assigns a free port number, if 0 is used.
      m_sockaddr.sin_port = 0;
    }
}

/**
 * Destructor
 */
Udp::~Udp()
{
  if( m_socket != 0 )
    {
      close(m_socket);
    }
}

void Udp::sendDatagram( QByteArray& datagram )
{
  m_sendDatagrams.append(datagram);
}

QByteArray Udp::readDatagram()
{
  if( m_readDatagrams.size() > 0 )
    {
      return m_readDatagrams.takeFirst();
    }

  return QByteArray();
}

void Udp::send2Server()
{
  /* send the next message to the server */
  if( m_sendDatagrams.size() == 0 )
    {
      return;
    }

  QByteArray& next = m_readDatagrams.first();

  int result = sendto( m_socket,
                       static_cast<const void *>(next.data()),
                       static_cast<uint>(next.size()),
                       0,
                       (const sockaddr*) &m_sockaddr,
                       sizeof(m_sockaddr) );
  if( result < 0)
    {
      qWarning() << "Error in sendto"
                 << "errno="
                 << errno
                 << "," << strerror(errno);
      return;
    }

  m_readDatagrams.removeFirst();
}

void Udp::receiveFromServer()
{
  /* gets the server's reply */

  char data[1024];
  memset( data, 0, sizeof(data) ); // clear data buffer

  socklen_t addrlen = sizeof(m_sockaddr);

  int result = recvfrom( m_socket,
                         static_cast<void *>(data),
                         sizeof(data),
                         0,
                         (struct sockaddr *) &m_sockaddr,
                         &addrlen );
  if( result < 0)
    {
      qWarning() << "Error in recvfrom"
                 << "errno="
                 << errno
                 << "," << strerror(errno);
    }

  m_readDatagrams.append( QByteArray(data) );

  // TODO send Signal
}


