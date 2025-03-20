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
  m_active(false),
  krt2(nullptr)
{
  qDebug() << "KRT2::KRT2() is called: " << QThread::currentThreadId();
  setObjectName( "KRT2" );

  // create TX/RX thread for communication to KRT2 devive
  krt2 = new KRT2Thread( this, m_ip, m_port );

  // Activate self destroy after finish signal has been caught.
  connect( krt2, SIGNAL(finished()), krt2, SLOT(deleteLater()) );

  // Activate thread
  krt2->start();
}

KRT2::~KRT2()
{
  qDebug() << "~KRT2() is called.";

  if( krt2 != nullptr && krt2->isRunning() == true )
    {
      qDebug() << "~KRT2(): Stop running KRT2 Thread";
      // Stop running thread
      krt2->quit();
      krt2->wait();
      qDebug() << "~KRT2(): KRT2 Thread finished";
    }
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

  msg.setNum( STX, 16 );
  msg.setNum( cmd, 16 );
  msg.setNum( mhz, 16 );
  msg.setNum( channel, 16 );

  if( name.size() <= MAX_NAME_LENGTH )
    {
      msg.append( name.toLatin1() );

      // Channel name is always 8 characters long
      while( name.size() < MAX_NAME_LENGTH )
        {
          msg.append( ' ' );
        }
    }
  else
    {
      msg.append( name.left( MAX_NAME_LENGTH ).toLatin1() );
    }

  msg.setNum( mhz ^ channel, 16 );

  krt2->send( msg );
  return true;
}

/**
 * Handle KRT message.
 */
void KRT2::handleKRTMessage( QByteArray& data )
{
  Q_UNUSED(data)
  // TODO...
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

   mhz = static_cast<uint8_t> (floor( fin ));

   uint16_t khz = static_cast<uint16_t> (floor( (fin - floor( fin )) * 1000.0 ));

   if( (khz % 5) == 0 && (khz % 25 != 20) )
     {
       channel = khz / 5;

       return true;
     }

   return false;
}

/*-------------------------------- KRT2Thread --------------------------------*/

KRT2Thread::KRT2Thread( QObject *parent, QString ip, QString port ) :
  QThread( parent ),
  m_ip( ip ),
  m_port( port ),
  m_connected( false ),
  m_socket( nullptr ),
  m_snRX( nullptr ),
  m_snExcept( nullptr )
{
  qDebug() << "KRT2Thread() is called: " << QThread::currentThreadId();
  setObjectName( "KRT2Thread" );
}

KRT2Thread::~KRT2Thread()
{
  qDebug() << "~KRT2Thread() is called: " << QThread::currentThreadId();

  slotFinished();
}

void KRT2Thread::slotFinished()
{
  qDebug() << "KRT2Thread::slotFinished() is called: " << QThread::currentThreadId();

  if( m_snRX != nullptr )
    {
      m_snRX->setEnabled( false );
      delete m_snRX;
    }

  if( m_snExcept != nullptr )
    {
      m_snExcept->setEnabled( false );
      delete m_snExcept;
    }

  if( m_socket != nullptr )
    {
      m_socket->close();
      delete m_socket;
    }
}

void KRT2Thread::run()
{
  qDebug() << "KRT2Thread::run() is called: " << QThread::currentThreadId();
  connect();
  qDebug() << "KRT2Thread::run() calls exec().";

  exec();

  qDebug() << "KRT2Thread::run() returns from exec() call." << QThread::currentThreadId();
}

bool KRT2Thread::connect()
{
  qDebug() << "KRT2Thread::connect() is called: " << QThread::currentThreadId();

  m_socket = new QTcpSocket;
  m_socket->setSocketOption( QAbstractSocket::LowDelayOption, QVariant( 1 ).toInt() );
  m_socket->connectToHost( m_ip, m_port.toUShort() );

  if( m_socket->waitForConnected( 2000 ) == false )
    {
      // We wait 2s for the connection success
      qCritical( ) << "KRT2Thread::connect(): connection error"
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
      QTimer::singleShot( 5000, this, SLOT(slotRetry()));
      return false;
    }

  m_connected = true;
  // Setup RX notifier
  m_snRX = new QSocketNotifier( m_socket->socketDescriptor(), QSocketNotifier::Read );
  m_snRX->connect( m_snRX, SIGNAL(activated(int)), this, SLOT(handleRxData(int)) );

  m_snExcept = new QSocketNotifier( m_socket->socketDescriptor(), QSocketNotifier::Exception );
  m_snExcept->connect( m_snExcept, SIGNAL(activated(int)), this, SLOT(handleException(int)) );

 return true;
}

/**
 * Called by the socket notifier, when an exception is received.
 */
void KRT2Thread::handleException( int type )
{
  Q_UNUSED(type)
  qDebug() << "KRT2Thread::handleException() is called.";
}

void KRT2Thread::handleRxData( int type )
{
  qDebug() << "KRT2Thread::handleRxData() is called: " << QThread::currentThreadId();

  Q_UNUSED(type)

  char buffer[128];

  if( m_socket->bytesAvailable() == 0 )
    {
      return;
    }

  // read message data
  quint64 read = m_socket->read( buffer, 127 );

  qDebug() << "RT2Thread::handleRxData(): read " << read << " Bytes";

  buffer[read] = '\0';
  rxBuffer.append( buffer );

  QString msg = QString("0x%1").arg( rxBuffer.at(0), 2, 16, QChar('0') );
  qDebug() << "KRT2Thread::handleRxData():" << msg << " : " << rxBuffer.toHex();

  // Handle commands
  while( rxBuffer.size() > 0 )
    {
      switch( rxBuffer.at(0) )
        {
          case RCQ:
            {
              // Respond to connection query.
              char answer = 0x01;
              mutex.lock();
              m_socket->write( &answer, 1);
              mutex.unlock();
              rxBuffer.remove( 0 , 1 );
              break;
            }

          case ACK:
          case NAK:
            // Received a response to a normal user command (STX)
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

          default:
            // Unknown rx data, clear the rx buffer
            qDebug() << "KRT2Thread::handleRxData(): clearing RX Buffer";
            rxBuffer.clear();
            break;
        }
    }
}

/**
 * Handle STX message from the KRT2 device
 */
bool KRT2Thread::handleSTX()
{
  qDebug() << "KRT2Thread::handleSTX():" << QString("0x%1").arg( rxBuffer[0], 2, 16, QChar( '0') );

  if( rxBuffer.size() < 2 )
    {
     return false;
    }

  switch( rxBuffer[0] )
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
 * Send the passed data to the KRT2 device.
 *
 * @param data
 * @return true in case of success otherwise false.
 */
bool KRT2Thread::send( QByteArray& data )
{
  qDebug() << "KRT2Thread::send() is called.";

  QMutexLocker locker( &mutex );

  if( m_connected == false )
    {
      return false;
    }

  int bytes = m_socket->write( data.data(), data.size() );

  if( bytes == data.size() )
    {
      return true;
    }

  return false;
}

