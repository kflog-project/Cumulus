/***********************************************************************
 **
 **  Udp.cpp
 **
 **  This file is part of Cumulus
 **
 ************************************************************************
 **
 **  Copyright (c): 2018 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation; either version 2 of the License, or
 **  (at your option) any later version.
 **
 ***********************************************************************/

#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <QtCore>
#include <QSocketNotifier>

#include "Udp.h"

/**
 * Constructor
 */
Udp::Udp(QObject *parent, QString serverIpAddress, ushort port ) :
    QObject(parent),
    m_ipAddress(serverIpAddress),
    m_port(port),
    m_socket(0),
    m_readNotifier(0)
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

  // None blocking IO is requested.
  fcntl(m_socket, F_SETFL, FNDELAY);

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

  // Setup a socket notifier to get read events signaled.
  m_readNotifier = new QSocketNotifier( m_socket,
                                        QSocketNotifier::Read,
                                        this );

  connect( m_readNotifier, SIGNAL(activated(int)),
           this, SLOT(slotReadEvent(int)) );
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

  if( m_readNotifier != 0 )
    {
      m_readNotifier->setEnabled( false );
      delete m_readNotifier;
    }
}

bool Udp::sendDatagram( QByteArray& datagram )
{
  int result = sendto( m_socket,
                       static_cast<const void *>(datagram.data()),
                       static_cast<size_t>(datagram.size()),
                       0,
                       (const sockaddr*) &m_sockaddr,
                       sizeof(m_sockaddr) );
  if( result < 0 )
    {
      if( errno == EWOULDBLOCK )
        {
          // The write call would block because the transfer queue is full.
          // In this case we discard the message.
          qWarning() << "sendDatagram(): Write would block!";
          return false;
        }

      qWarning() << "sendDatagram(): Error in sendto"
                 << "errno="
                 << errno
                 << "," << strerror(errno);

      return false;
    }

  // A signal is emitted via a timer call to break the call chain between
  // signals and slots. Otherwise you can get unwanted recursive calls.
  QTimer::singleShot( 10, this, SIGNAL(bytesWritten()) );
  return true;
}

QByteArray Udp::readDatagram()
{
  if( m_readDatagrams.size() > 0 )
    {
      return m_readDatagrams.takeFirst();
    }

  return QByteArray();
}

void Udp::slotReadEvent(int socket)
{
  Q_UNUSED( socket )

  // Data for reading are available
  m_readNotifier->setEnabled( false );
  receiveFromServer();
  m_readNotifier->setEnabled( true );
}

void Udp::receiveFromServer()
{
#if 0
  /* gets the server's reply */
  int size = 0;

  // Get the size of bytes to be available in the socket receiver buffer.
  if( ioctl(m_socket, FIONREAD, &size) == -1 || size == 0 )
    {
      return;
    }

  qDebug() << "ioctl sagt es hat" << size << "bytes";
#endif

  // Buffer size should be sufficient for SkyLines answers.
  char data[128];

  socklen_t addrlen = sizeof(m_sockaddr);

  int result = recvfrom( m_socket,
                         static_cast<void *>(data),
                         sizeof(data),
                         0,
                         (struct sockaddr *) &m_sockaddr,
                         &addrlen );

  if( result == 0 || (result == -1 && errno != EWOULDBLOCK) )
    {
      qWarning() << "Udp::receiveFromServer(): recvfrom error"
              << errno << ","
              << strerror(errno);
      return;
    }

  QByteArray ba;
  ba.append( (const char *) data, result );

  // Store received datagram in the list.
  m_readDatagrams.append( ba );
  emit readyRead();
}
