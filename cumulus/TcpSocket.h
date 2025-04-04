/***********************************************************************
 **
 **   TcpSocket.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2025 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 ***********************************************************************/

#pragma once

#include <QtCore>
#include <QTcpSocket>

class TcpSocket : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( TcpSocket )

 public:

  TcpSocket( QObject *parent, QString ip, QString port, short channel );

  virtual ~TcpSocket();

  /**
   * Returns the connection flag.
   *
   * @return true in case of connected otherwise false.
   */
  bool connected()
    {
      return m_connected;
    }

  /**
   * Send the passed data to the connected TcpSocket device.
   *
   * @param data
   * @return true in case of success otherwise false.
   */
  bool send( QByteArray& data );

  /**
   * Close the socket connection.
   */
  void close();

 public slots:

  /**
  * Try to establish the connection to the TcpSocket device.
  */
  void slotConnect();

 private slots:

  /**
   * Called by the socket, when data are received from the TcpSocket device.
   */
  void slotHandleRxData();

  /**
   * Handle disconnected signal.
   */
  void slotDisconnected();

 signals:

   void forwardDeviceError( const QString& error, const bool sound );

   /**
    * Signal, that RX data has been read.
    *
    * @param rxData read data.
    *
    * @param channel additional info for the connected slot.
    */
   void rxData( QByteArray rxData, const short channel );

  private:

  // Tcp data
  QString m_ip;
  QString m_port;
  short m_channel;
  bool m_connected;
  QTcpSocket *m_socket;

  QQueue<QByteArray> m_txQueue;
  QByteArray rxBuffer;
  QMutex mutex;
};
