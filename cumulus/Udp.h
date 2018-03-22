/***********************************************************************
**
**   Udp.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2018 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

#ifndef Udp_h_
#define Udp_h_ 1

#include <unistd.h>
#include <netinet/in.h>

#include <QList>
#include <QObject>
#include <QByteArray>

#define IPC_IP "127.0.0.1"

/**
 * \class Udp
 *
 * \author Axel pauli
 *
 * \brief UDP client class.
 *
 * This class provides a simple UDP client for sending and receiving
 * UDP datagrams.
 *
 * \date 2018
 */
class Udp : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( Udp )

public:

  /**
   * Class constructor. This class is derived from QObject.
   */
  Udp( QObject* parent, QString serverIpAddress, ushort port );

  virtual ~Udp();

  void sendDatagram( QByteArray& datagram );

  QByteArray readDatagram();

  bool hasPendingDatagrams()
  {
    return (m_readDatagrams.size() > 0);
  }

  void closeSocket()
  {
    if( m_socket != 0 )
      {
        close(m_socket);
      }
  }

private:

  void send2Server();
  void receiveFromServer();

  QByteArray m_host;
  QString m_ipAddress;
  unsigned short m_port;
  int m_socket;
  struct sockaddr_in m_sockaddr;
  QList<QByteArray> m_sendDatagrams;
  QList<QByteArray> m_readDatagrams;
};

#endif
