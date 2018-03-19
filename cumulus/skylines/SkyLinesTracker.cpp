/***********************************************************************
**
 **   SkyLinesTracker.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
 **   Copyright (c): 2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

// See https://skylines.aero/tracking/info

#include <cstdlib>
#include <ctime>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include <QtGlobal>
#include <QHostInfo>

#include "calculator.h"
#include "hwinfo.h"
#include "mainwindow.h"
#include "SkyLinesTracker.h"

#include "byteOrder.h"
#include "crc.h"


#ifdef ANDROID
#include "jnisupport.h"
#endif

// Define a maximum queue length to limit the data amount. That limit is reached
// after an hour, if all 5s an entry is made.
#define MaxQueueLen 750

SkyLinesTracker::SkyLinesTracker(QObject *parent) :
  LiveTrackBase(parent),
  m_retryTimer(0),
  m_hostLookupIsRunning(false),
  m_liveTrackingKey(0),
  m_udp(0),
  m_packetId(0),
  m_lastPingAnswer(-1),
  m_startDay(0),
  m_sentPackages(0)
{
  m_retryTimer = new QTimer( this );
  m_retryTimer->setInterval( 60000 ); // Timeout is 60s
  m_retryTimer->setSingleShot( true );

  connect( m_retryTimer, SIGNAL(timeout()), this, SLOT(slotRetry()) );
}

SkyLinesTracker::~SkyLinesTracker()
{
  m_retryTimer->stop();
}

bool SkyLinesTracker::startTracking()
{
  const char* method = "SkyLinesTracker::startTracking():";

  if( isServiceRequested() == false )
    {
      return false;
    }

  // Reset package counter
  m_sentPackages = 0;

  // All previous cached data are cleared because there was no login to the
  // server in the past and in this case only the current session data
  // are stored.
  m_fixPacketQueue.clear();

  // Calculate begin of day in milli seconds UTC and save it
  m_startDay = QDateTime::currentMSecsSinceEpoch() / 24 * 3600000;

  // Check, if user name contains a live tracking key entry.
  GeneralConfig* conf = GeneralConfig::instance();

  m_liveTrackingKeyString = conf->getLiveTrackUserName();

  if( m_liveTrackingKeyString.isEmpty() )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines Tracking user name is missing!";
      return false;
    }

  bool ok;
  m_liveTrackingKey = m_liveTrackingKeyString.toULongLong(&ok, 16);

  if( ok == false )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines Tracking user name is wrong, no uint64!";
      return false;
    }

  slotHostInfoRequest();
  return true;
}

void SkyLinesTracker::slotHostInfoResponse( QHostInfo hostInfo)
{
  m_hostLookupIsRunning = false;

  if( hostInfo.error() != QHostInfo::NoError )
    {
      qWarning() << "Lookup failed:" << hostInfo.errorString();

      if( isServiceRequested() == true )
        {
          // Request host info again after 60s.
          QTimer::singleShot( 60000, this, SLOT(slotHostInfoRequest()) );
          return;
        }
    }

  // We take the first address only.
  m_serverIpAdress = hostInfo.addresses().first();

  // Send a ping to the skyLines server to verify the user key.
  slotSendPing();
}

void SkyLinesTracker::slotHostInfoRequest()
{
  if( isServiceRequested() == false )
    {
      return;
    }

  if( m_serverIpAdress.isNull() && m_hostLookupIsRunning == false )
    {
      // We need the IP address of the skyline tracking server.
      m_hostLookupIsRunning = true;

      QHostInfo::lookupHost( getServerName(),
                             this, SLOT(lookedUp(QHostInfo)) );
    }
}

void SkyLinesTracker::slotSendPing()
{
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true )
    {
      return;
    }

  if( m_udp == static_cast<QUdpSocket *> (0) )
    {
      m_udp = new QUdpSocket( this );
      m_udp->bind( m_serverIpAdress, getDefaultPort() );

      connect( m_udp, SIGNAL(readyRead()),
               this, SLOT(slotReadPendingDatagrams()));
    }

  SkyLinesTracking::PingPacket pp;
  pp.header.magic = ToBE32( SkyLinesTracking::MAGIC );
  pp.header.crc = 0;
  pp.header.type = ToBE16( SkyLinesTracking::Type::PING);
  pp.header.key = ToBE64( m_liveTrackingKey );
  pp.id = ToBE16( getId() );
  pp.reserved = 0;
  pp.reserved2 = 0;
  pp.header.crc = ToBE16( UpdateCRC16CCITT( static_cast<const void *>( &pp ),
                                            sizeof(pp), 0) );

  qint64 res = m_udp->writeDatagram( static_cast<const char *>( &pp ),
                                     sizeof(SkyLinesTracking::PingPacket),
                                     m_serverIpAdress,
                                     getDefaultPort() );

  if( res == -1 )
    {
      // Ping could not be send, try it again after 60s.
      QTimer::singleShot( 60000, this, SLOT(slotSendPing()) );
      return;
    }

  // Ping could be sent...
}

void SkyLinesTracker::slotReadPendingDatagrams()
{
  // An UDP diagram is available. Read in each datagram.
  while( m_udp->hasPendingDatagrams() )
    {
      QByteArray datagram;
      datagram.resize( m_udp->pendingDatagramSize() );
      QHostAddress sender;
      quint16 senderPort;

      m_udp->readDatagram( datagram.data(), datagram.size(),
                           &sender, &senderPort );

      processDatagram( datagram );
    }
}

void SkyLinesTracker::processDatagram( QByteArray& datagram )
{
  if( datagram.size() < sizeof(SkyLinesTracking::Header) )
    {
      qWarning() << "SkyLinesTracker::processDatagram: Header to short!";
      return;
    }

  // extract header
  SkyLinesTracking::Header header;

  qstrncpy( static_cast<char *>(&header),
            datagram.data(),
            sizeof(SkyLinesTracking::Header) );


  const quint16 received_crc = FromBE16( header.crc );
  header.crc = 0;

  const quint16 calculated_crc = UpdateCRC16CCITT( datagram.data(),
                                                   datagram.size(),
                                                   0 );
  if (received_crc != calculated_crc)
    {
      qWarning() << "SkyLinesTracker::processDatagram: CRC error!";
      return;
    }

  // extract package
  SkyLinesTracking::ACKPacket ack;
  SkyLinesTracking::TrafficResponsePacket traffic;
  SkyLinesTracking::UserNameResponsePacket userName;
  SkyLinesTracking::WaveResponsePacket wave;
  SkyLinesTracking::ThermalResponsePacket thermal;

  switch( static_cast<SkyLinesTracking::Type>(FromBE16(header.type)) )
  {
  case SkyLinesTracking::PING:
  case SkyLinesTracking::FIX:
  case SkyLinesTracking::TRAFFIC_REQUEST:
  case SkyLinesTracking::USER_NAME_REQUEST:
  case SkyLinesTracking::WAVE_SUBMIT:
  case SkyLinesTracking::WAVE_REQUEST:
  case SkyLinesTracking::THERMAL_SUBMIT:
  case SkyLinesTracking::THERMAL_REQUEST:
    break;

  case SkyLinesTracking::ACK:

    if( datagram.size() < sizeof(SkyLinesTracking::ACK) )
      {
        qWarning() << "SkyLinesTracker::processDatagram: ACK package to short!";
        return;
      }

    qstrncpy( static_cast<char *>(&ack),
              datagram.data(),
              sizeof(SkyLinesTracking::ACKPacket) );

    // Response id for request id
    quint16 id = FromBE16( ack.id );

    // Response flags.
    quint32 flags = FromBE32( ack.flags );
    m_lastPingAnswer = static_cast<qint32>(flags);
    break;

  case SkyLinesTracking::TRAFFIC_RESPONSE:
    break;

  case SkyLinesTracking::USER_NAME_RESPONSE:
    break;

  case SkyLinesTracking::WAVE_RESPONSE:
    break;

  case SkyLinesTracking::THERMAL_RESPONSE:
     break;
  }

}

bool SkyLinesTracker::routeTracking( const QPoint& position,
                                     const int altitude,
                                     const uint groundSpeed,
                                     const uint course,
                                     const double vario,
                                     qint64 utcTimeStamp )
{
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true )
    {
      return false;
    }

  SkyLinesTracking::FixPacket fp;

  fp.header.magic = ToBE32( SkyLinesTracking::MAGIC );
  fp.header.crc = 0;
  fp.header.type = ToBE16( SkyLinesTracking::Type::FIX);
  fp.header.key = ToBE64( m_liveTrackingKey );

  quint32 flags = 0;
  flags = SkyLinesTracking::FixPacket::FLAG_LOCATION |
          SkyLinesTracking::FixPacket::FLAG_TRACK |
          SkyLinesTracking::FixPacket::FLAG_GROUND_SPEED |
          SkyLinesTracking::FixPacket::FLAG_ALTITUDE |
          SkyLinesTracking::FixPacket::FLAG_VARIO;

  fp.flags = ToBE32(flags);

  quint32 time = static_cast<quint32>(utcTimeStamp - m_startDay);

  fp.time = ToBE32(time);
  fp.reserved = 0;

  SkyLinesTracking::GeoPoint geoPoint;
  geoPoint.latitude = static_cast<qint32>(rint(double(position.x()) / 600000.0 * 1000000.0));
  geoPoint.longitude = static_cast<qint32>(rint(double(position.y()) / 600000.0 * 1000000.0));
  geoPoint.latitude = ToBE32(geoPoint.latitude);
  geoPoint.longitude = ToBE32(geoPoint.longitude);

  fp.location = geoPoint;
  fp.track = ToBE16(static_cast<quint16>(course));
  fp.ground_speed = ToBE16(static_cast<quint16>((groundSpeed * 1000.0 / 3600.0) * 16));
  fp.airspeed = 0;
  fp.altitude = ToBE16(static_cast<qint16>(altitude));
  fp.vario = ToBE16(static_cast<qint16>(vario * 256));
  fp.engine_noise_level = 0;

  fp.header.crc = ToBE16( UpdateCRC16CCITT( static_cast<const void *>( &fp ),
                                            sizeof(fp), 0) );
  queueRequest( fp );
  return true;
}

bool SkyLinesTracker::endTracking()
{
  if( isServiceRequested() == false )
    {
      return true;
    }

  return true;
}

void SkyLinesTracker::slotRetry()
{
  // Data sending was not possible and the retry timer has expired.
  // As next we check if LiveTracking is still switched on.
  // If not, we stop here the data sending and clear the request queue.
  if( GeneralConfig::instance()->isLiveTrackOnOff() == false )
    {
      m_fixPacketQueue.clear();
      return;
    }

  // Take the next request from the queue and try to send it to the server.
  sendHttpRequest();
}

bool SkyLinesTracker::queueRequest( SkyLinesTracking::FixPacket fixPaket )
{
  checkQueueLimit();
  m_fixPacketQueue.enqueue( fixPaket );
  return sendHttpRequest();
}

void SkyLinesTracker::checkQueueLimit()
{
  while( m_fixPacketQueue.size() > MaxQueueLen )
    {
      // hau wech
      m_fixPacketQueue.removeAt( 0 );
    }
}

void SkyLinesTracker::stopLiveTracking()
{
  // Login failed. We disable further live tracking.
  GeneralConfig::instance()->setLiveTrackOnOff( false );
  m_retryTimer->stop();
  m_fixPacketQueue.clear();

  // Inform the user about our decision.
  QString msg = QString(tr("<html>SkyLines login failed!<br><br>Switching off service.</html>"));

  QMessageBox mb( QMessageBox::Critical,
                  tr("Login Error"),
                  msg,
                  QMessageBox::Ok,
                  MainWindow::mainWindow() );

#ifdef ANDROID

  mb.show();
  QPoint pos = MainWindow::mainWindow()->mapToGlobal(QPoint( MainWindow::mainWindow()->width()/2  - mb.width()/2,
                                                             MainWindow::mainWindow()->height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  mb.exec();
}
