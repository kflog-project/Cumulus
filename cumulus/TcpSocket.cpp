/***********************************************************************
 **
 **   TcpSocket.cpp
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

#include <QtCore>

#include "TcpSocket.h"

/**
 * TcpSocket device class.
 *
 * This class provides the interface to a TCP socket device..
 */

TcpSocket::TcpSocket( QObject *parent, QString ip, QString port, short channel ) :
  QObject( parent ),
  m_ip(ip),
  m_port(port),
  m_channel(channel),
  m_connected(false),
  m_sychronized(false),
  m_socket(0)
{
  qDebug() << "TcpSocket::TcpSocket() IP=" << ip << "Port=" << port << channel;
  setObjectName( "TcpSocket" );
}

TcpSocket::~TcpSocket()
{
  qDebug() << "~TcpSocket() is called." << m_channel;
}

/**
 * Closes the socket connection.
 */
void TcpSocket::close()
{
  qDebug() << "TcpSocket::close() is called" << m_channel;

  if( m_socket != 0 )
    {
      if( m_socket->isOpen() )
        {
          qDebug() << "TcpSocket::close(): Stop running TcpSocket connection";
          m_socket->flush();
          m_socket->close();
        }
    }
}

void TcpSocket::slotConnect()
{
  qDebug() << "TcpSocket::connect() is called: " << m_channel;

  m_socket = new QTcpSocket( this );
  m_socket->connect( m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
  m_socket->setSocketOption( QAbstractSocket::LowDelayOption, QVariant( 1 ).toInt() );
  m_socket->connectToHost( m_ip, m_port.toUShort() );
  QObject::connect( m_socket, SIGNAL(readyRead()), this, SLOT( slotHandleRxData()) );

  if( m_socket->waitForConnected( 2000 ) == false )
    {
      // We wait 2s for the connection success
      qCritical( ) << "TcpSocket::connect(): connection error"
                   << m_ip << ":" << m_port << m_socket->error()
                   << m_socket->errorString();

      forwardDeviceError( QObject::tr("Cannot open device") + " " +
                          m_ip + ":" + m_port + ", " +
                          m_socket->errorString(), false );
      m_socket->close();
      delete m_socket;
      m_socket = 0;
      m_connected = false;

      // Start retry timer for connection retry after 10s.
      QTimer::singleShot( 10000, this, SLOT(slotConnect()));
      return;
    }

  m_connected = true;
}

/**
 * Handle disconnected signal.
 */
void TcpSocket::slotDisconnected()
{
  qDebug() << "TcpSocket::slotDisconnect() is called: " << m_channel;
  close();
  m_socket->deleteLater();
}

/**
 * Send the passed data to the connected TCP device.
 *
 * @param data
 *
 * @return true in case of success otherwise false.
 */
bool TcpSocket::send( QByteArray& data )
{
  qDebug() << "TcpSocket::send() is called, m_connected=" << m_connected << data.toHex();

  QMutexLocker locker( &mutex );

  if( m_connected == false )
    {
      return false;
    }

  int bytes = m_socket->write( data.data(), data.size() );
  m_socket->flush();

  // qDebug() << "Bytes" << bytes << "written";

  if( bytes == data.size() )
    {
      return true;
    }

  return false;
}

/**
 * Handle RX data.
 */
void TcpSocket::slotHandleRxData()
{
  qDebug() << "TcpSocket::handleRxData() is called: " << m_channel;

  char buffer[128];

  while( true )
    {
      // read message data
      qint64 read = m_socket->read( buffer, sizeof( buffer ) );

      if( read == 0 )
        {
          qDebug() << "TcpSocket::handleRxData(): read " << read << " bytes.";
          break;
        }
      else if( read == -1 )
        {
          qDebug() << "TcpSocket::handleRxData(): returned -1 -> Error";
          break;
        }

      rxBuffer.append( buffer, read );
      // qDebug() << "TcpSocket::handleRxData():" << rxBuffer.toHex();
   }

  if( rxBuffer.size() > 0 )
    {
      emit rxData( rxBuffer, m_channel );
      rxBuffer.clear();
    }
}
