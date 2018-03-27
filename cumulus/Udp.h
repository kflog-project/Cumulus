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

class QSocketNotifier;

/**
 * \class Udp
 *
 * \author Axel pauli
 *
 * \brief UDP client class.
 *
 * This class provides a simple UDP client for sending and receiving
 * UDP datagrams. Could not get running the qUdp class for that purpose.
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

  /**
   * Send a datagram to the server.
   */
  bool sendDatagram( QByteArray& datagram );

  /**
   * Returns the next datagram from the receiver list, if the list is not empty.
   * Otherwise a QByteArray with none content is returned.
   */
  QByteArray readDatagram();

  /**
   * Checks the receiver list for pending diagrams.
   */
  bool hasPendingDatagrams()
  {
    return (m_readDatagrams.size() > 0);
  }

  /**
   * Closes the datagram socket.
   */
  void closeSocket()
  {
    if( m_socket != 0 )
      {
        close(m_socket);
      }
  }

signals:

  /**
   * Emitted, if a datagram has been received.
   */
  void readyRead();

  /**
   * This signal is emitted every time a payload of data has been written to
   * the device.
   */
  void bytesWritten();

private slots:

  /**
   * Called, if reading data are available in the socket buffer.
   */
  void slotReadEvent(int socket);

private:

  /**
   * Called to read datagrams from the UDP socket.
   */
  void receiveFromServer();

  /** IP V4 server address in dotted notation. */
  QString m_ipAddress;

  /**
   * Server port.
   */
  unsigned short m_port;

  /*
   * UDP socket used for the communication.
   */
  int m_socket;

  /**
   * Socket address and port parameters.
   */
  struct sockaddr_in m_sockaddr;

  /**
   * Storage for received datagrams.
   */
  QList<QByteArray> m_readDatagrams;

  /**
   * Socket notifier. Calls method receiveFromServer(), if data for read are
   * available.
   */
  QSocketNotifier *m_readNotifier;
};

#endif
