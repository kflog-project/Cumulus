/***********************************************************************
 **
 **   KRT2.cpp
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
#include <cmath>

#include <QtCore>

#include "generalconfig.h"
#include "KRT2Constants.h"
#include "KRT2.h"

/**
 * KRT2 device class.
 *
 * This class provides the interface to communicate with the KRT-2 radio.
 */

KRT2::KRT2( QObject *parent, QString ip, QString port ) :
  QObject( parent ),
  m_ip(ip),
  m_port(port),
  m_connected(false),
  m_sychronized(false),
  m_socket(nullptr)
{
  setObjectName( "KRT2" );
  slotConnect();
}

KRT2::~KRT2()
{
  qDebug() << "~KRT2() is called.";
  close();
}

/**
 * Close the socket connection.
 */
void KRT2::close()
{
  qDebug() << "KRT2::close() is called";

  if( m_socket != nullptr )
    {
      if( m_socket->isOpen() )
        {
          qDebug() << "KRT2::close(): Stop running KRT2 connection";
          m_socket->flush();
          m_socket->close();
        }
    }
}

void KRT2::slotConnect()
{
  qDebug() << "KRT2::connect() is called: " << QThread::currentThreadId();

  m_socket = new QTcpSocket( this );
  m_socket->connect( m_socket, SIGNAL(disconnected()), this, SLOT(slotDisconnected()) );
  m_socket->setSocketOption( QAbstractSocket::LowDelayOption, QVariant( 1 ).toInt() );
  m_socket->connectToHost( m_ip, m_port.toUShort() );
  QObject::connect( m_socket, SIGNAL(readyRead()), this, SLOT( slotHandleRxData()) );

  if( m_socket->waitForConnected( 2000 ) == false )
    {
      // We wait 2s for the connection success
      qCritical( ) << "KRT2::connect(): connection error"
                   << m_ip << ":" << m_port << m_socket->error()
                   << m_socket->errorString();

      forwardDeviceError( QObject::tr("Cannot open device") + " " +
                          m_ip + ":" + m_port + ", " +
                          m_socket->errorString() );
      m_socket->close();
      delete m_socket;
      m_socket = nullptr;
      m_connected = false;

      // Start retry timer for connection retry after 5s.
      QTimer::singleShot( 5000, this, SLOT(slotConnect()));
      return;
    }

  m_connected = true;
  QByteArray data;
  data.append( "!krt2" );
  send( data );
  qDebug() << "KRT2::slotConnect(): sending !krt2";
}

/**
 * Handle disconnected signal.
 */
void KRT2::slotDisconnected()
{
  close();
  m_socket->deleteLater();
}

/**
 * Send the passed data to the KRT-2 device.
 *
 * @param data
 * @return true in case of success otherwise false.
 */
bool KRT2::send( QByteArray& data )
{
  qDebug() << "KRT2::send() is called, m_connected=" << m_connected << data.toHex();

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

bool KRT2::setActiveFrequency( const float frequency,
                               const QString& name )
{
  return sendFrequency( ACTIVE_FREQUENCY, frequency, name );
}

bool KRT2::setStandbyFrequency( const float frequency,
                                const QString& name )
{
  return sendFrequency( STANDBY_FREQUENCY, frequency, name );
}


bool KRT2::sendFrequency( const uint8_t cmd,
                          const float frequency,
                          const QString& name )
{
  QByteArray msg;
  uint8_t mhz;
  uint8_t channel;

  if( splitFreqency( frequency, mhz, channel ) == false )
    {
      // Check and split frequency
      return false;
    }

  msg.append( STX );
  msg.append( cmd );
  msg.append( mhz );
  msg.append( channel );

  // KRT2 can only handle ASCII characters
  QString checked = replaceUmlauts( name );

  if( checked.size() <= MAX_NAME_LENGTH )
    {
      msg.append( checked.toLatin1() );

      for( int i=checked.size(); i < MAX_NAME_LENGTH; i++ )
        {
          // Channel name is always 8 characters long
          {
            msg.append( ' ' );
          }
        }
    }
  else
    {
      msg.append( checked.left( MAX_NAME_LENGTH ).toLatin1() );
    }

  msg.append( mhz ^ channel );
  send( msg );
  return true;
}

/** Activate dual mode */
void KRT2::activateDualMode()
{
  QByteArray msg;
  msg.append( STX );
  msg.append( DUAL_ON );
  send( msg );
}

/** Deactivate dual mode */
void KRT2::deactivateDualMode()
{
  QByteArray msg;
  msg.append( STX );
  msg.append( DUAL_OFF );
  send( msg );
}

QString KRT2::replaceUmlauts( QString string )
{
  string = string.replace( Qt::Key_Adiaeresis, "Ae" );
  string = string.replace( Qt::Key_Odiaeresis, "Oe" );
  string = string.replace( Qt::Key_Udiaeresis, "Ue" );
  string = string.replace( Qt::Key_Adiaeresis + 0x20, "ae" );
  string = string.replace( Qt::Key_Odiaeresis + 0x20, "oe" );
  string = string.replace( Qt::Key_Udiaeresis + 0x20, "ue" );
  string = string.replace( 0xdf, "ss" );

  // Check for other non ASCII characters and replace them by ?
  for( int i=0; i < string.size(); i++ )
    {
      QChar qc = string[i];

      if( qc.toLatin1() == 0 )
        {
          // Convert unknown ASCII character to ?
          string[i] = QChar( '?' );
        }
    }

  return string;
}

/**
 * Called, to exchangeFrequency the frequencies active/standby on the KRT2 radio.
 */
void KRT2::exchangeFrequency()
{
  QByteArray msg;

  msg.append( STX );
  msg.append( EXCHANGE_FREQUENCIES );
  send( msg );
}

/**
 * Handle KRT message.
 */
void KRT2::slotHandleRxData()
{
  qDebug() << "KRT2::handleRxData() is called: " << QThread::currentThreadId();

  char buffer[128];

  while( true )
    {
      // read message data
      qint64 read = m_socket->read( buffer, sizeof( buffer-1 ) );

      if( read == 0 )
        {
          qDebug() << "KRT2::handleRxData(): read " << read << " bytes.";
          return;
        }
      else if( read == -1 )
        {
          qDebug() << "KRT2::handleRxData(): returned -1 -> Error";
          return;
        }

      rxBuffer.append( buffer, read );

      QString msg = QString("0x%1").arg( rxBuffer.at(0), 2, 16, QChar('0') );
      qDebug() << "KRT2::handleRxData():" << msg << " : " << rxBuffer.toHex();

      // Handle commands
      while( rxBuffer.size() > 0 )
        {
          switch( rxBuffer.at(0) )
            {
              case 0x01:
                {
                  // Response from KRT2 to 'S' ping.
                  qDebug() << "Received 0x01 from KRT2 to my ping";
                  m_sychronized = true;
                  rxBuffer.remove( 0 , 1 );
                  break;
                }
              case RCQ:
                {
                  // Respond to connection query.
                  QByteArray answer( 1, 0x1 );
                  bool res = send( answer );
                  qDebug() << "sending 0x01 to KRT2, result=" << res;
                  rxBuffer.remove( 0 , 1 );
                  m_sychronized = true;
                  break;
                }

              case ACK:
                // Received a response to a normal user command (STX)
                qDebug() << "Received ACK" << rxBuffer.at(0);
                rxBuffer.remove( 0 , 1 );
                break;

              case NAK:
                // Received a response to a normal user command (STX)
                qDebug() << "Received NAK" << rxBuffer.at(0);
                rxBuffer.remove( 0 , 1 );
                break;

              case STX:
                // Received a command from the radio (STX). Handle what we know.
                if( handleSTX() == false )
                  {
                    // Wait for more data.
                    return;
                  }

                break;

              case '\n':
        	// Alive from XCVario, can be ignored
                rxBuffer.remove( 0 , 1 );
        	break;

              default:
                // Unknown rx data, clear the rx buffer
                qDebug() << "KRT2::handleRxData(): unknown command " << rxBuffer.toHex() << "received";
                rxBuffer.remove( 0 , 1 );
                // rxBuffer.clear();
                break;
            }
        }
    }
}

/**
 * Handle STX message from the KRT2 device
 */
bool KRT2::handleSTX()
{
  qDebug() << "KRT2::handleSTX():" << rxBuffer.toHex();

  if( rxBuffer.size() < 2 )
    {
     return false;
    }

  switch( rxBuffer[1] )
    {
      case ACTIVE_FREQUENCY:
      case STANDBY_FREQUENCY:
        if( rxBuffer.size() < 13 )
          {
           return false;
          }

        // HandleFrequency(*(const struct stx_msg *)src.data(), info);
        rxBuffer.remove( 0 , 13 );
        return true;

      case SET_FREQUENCY:
        if( rxBuffer.size() < 14 )
          {
           return false;
          }

        rxBuffer.remove( 0 , 14 );
        return true;

      case SET_VOLUME:
        if( rxBuffer.size() < 6 )
          {
           return false;
          }

        rxBuffer.remove( 0 , 6 );
        return true;

      case EXCHANGE_FREQUENCIES:
        rxBuffer.remove( 0 , 2 );
        return true;

      case UNKNOWN1:
      case LOW_BATTERY:
      case NO_LOW_BATTERY:
      case PLL_ERROR:
      case PLL_ERROR2:
      case NO_PLL_ERROR:
      case RX:
      case NO_RX:
      case TX:
      case TE:
      case NO_TX_RX:
      case DUAL_ON:
      case DUAL_OFF:
      case RX_ON_ACTIVE_FREQUENCY:
      case NO_RX_ON_ACTIVE_FREQUENCY:

        rxBuffer.remove( 0 , 2 );
        return true;

      default:
        // Received unknown STX code
        rxBuffer.remove( 0 , 2 );
        return true;
    }
}

/**
 *
 * @param fin Frequency in MHz
 * @param mhz Megahertz part of the frequency
 * @param channel KRT Channel part of the frequency
 * @return
 *
 * VHF Voice channels range from 118000 kHz up to not including 137000 kHz
 * Valid 8.33 kHz channels must be a multiple of 5 kHz
 * Due to rounding from 8.33 kHz to multiples of 5 (for displaying), some
 * channels are invalid. These are matched by (value % 25) == 20.
 */
bool KRT2::splitFreqency( const float fin, uint8_t& mhz, uint8_t& channel )
{
  if( fin < 118.0 || fin >= 137.0 )
    {
      return false;
    }

   QString sn = QString("%1").arg( fin, 0, 'f', 3 );

   QStringList parts = sn.split(".");

   mhz = parts.at(0).toShort();

   short khz = parts.at(1).toShort();

   if( (khz % 5) == 0 && (khz % 25 != 20) )
     {
       channel = khz / 5;
       return true;
     }

   return false;
}
