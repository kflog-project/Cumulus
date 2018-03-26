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
#include <unistd.h>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include <QtGlobal>
#include <QtNetwork>

#include "calculator.h"
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

SkyLinesTracker::PackageId SkyLinesTracker::m_packetId = 0;

SkyLinesTracker::SkyLinesTracker(QObject *parent, bool testing) :
  LiveTrackBase(parent),
  m_testing(testing),
  m_retryTimer(0),
  m_hostLookupIsRunning(false),
  m_liveTrackingKey(0),
  m_udp(0),
  m_lastPingAnswer(999),
  m_startDay(0),
  m_sentPackages(0)
{
  m_retryTimer = new QTimer( this );
  m_retryTimer->setInterval( 30000 ); // Timeout is 30s
  m_retryTimer->setSingleShot( true );

  connect( m_retryTimer, SIGNAL(timeout()), this, SLOT(slotRetry()) );
}

SkyLinesTracker::~SkyLinesTracker()
{
  qDebug() << "SkyLinesTracker::~SkyLinesTracker()";

  m_retryTimer->stop();

  if( m_udp != 0 )
    {
      m_udp->closeSocket();
      m_udp->deleteLater();
    }
}

bool SkyLinesTracker::startTracking()
{
  const char* method = "SkyLinesTracker::startTracking():";

  if( m_testing == false && isServiceRequested() == false )
    {
      return false;
    }

  // Reset sent package counter
  m_sentPackages = 0;

  // All previous cached data are cleared because there was no login to the
  // server in the past and in this case only the current session data
  // are stored.
  m_fixPacketQueue.clear();

  // Calculate begin of day in milli seconds UTC and save it
  m_startDay = QDateTime::currentMSecsSinceEpoch() / (24 * 3600000);
  m_startDay *= (24 * 3600000);

  // Check, if user name contains a live tracking key entry.
  GeneralConfig* conf = GeneralConfig::instance();

  m_liveTrackingKeyString = conf->getLiveTrackUserName();

  if( m_liveTrackingKeyString.isEmpty() )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines tracking user key is missing!";
      return false;
    }

  // Check live tracking key, must be a hex string.
  bool ok;
  m_liveTrackingKey = m_liveTrackingKeyString.toULongLong(&ok, 16);

  if( ok == false )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines tracking user key is wrong, no uint64!";
      return false;
    }

  // Get host information, IP address of skylines.aero server.
  slotHostInfoRequest();
  return true;
}

void SkyLinesTracker::slotHostInfoRequest()
{
  if( ( m_testing == false && isServiceRequested() == false ) ||
        m_hostLookupIsRunning == true )
    {
      return;
    }

  if( m_serverIpAdress.isNull() && m_hostLookupIsRunning == false )
    {
      // We need the IP address of the skyline tracking server.
      m_hostLookupIsRunning = true;

      qDebug() << "QHostInfo::lookupHost";

      QHostInfo::lookupHost( getServerName(),
                             this, SLOT(slotHostInfoResponse(QHostInfo)) );
    }
}

void SkyLinesTracker::slotHostInfoResponse( QHostInfo hostInfo)
{
  qDebug() << "slotHostInfoResponse";

  m_hostLookupIsRunning = false;

  if( hostInfo.error() != QHostInfo::NoError )
    {
      qWarning() << "SkyLinesTracker lookup failed:"
                 << hostInfo.errorString();

      emit connectionFailed();

      if( isServiceRequested() == true )
        {
          // Request host info again after 60s.
          QTimer::singleShot( 60000, this, SLOT(slotHostInfoRequest()) );
          return;
        }
    }

  // We take the first IP address only.
  m_serverIpAdress = hostInfo.addresses().first();

  qDebug() << "IP-Address" << m_serverIpAdress.toString();

  // Send a ping to the skyLines server to verify the user's live tracking key.
  slotSendPing();
}

QString SkyLinesTracker::getMyIpAddress()
{
  QString ipAddress;

  QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();

  // use the first non-localhost IPv4 address
  for (int i = 0; i < ipAddressesList.size(); ++i)
    {
      if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
          ipAddressesList.at(i).toIPv4Address())
        {
          ipAddress = ipAddressesList.at(i).toString();
          break;
        }
    }
  // if we did not find one, use IPv4 localhost
  if (ipAddress.isEmpty())
    {
      ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

      qWarning() << "SkyLinesTracker::getMyIpAddress():"
                 << "Returning LocalHost as IP address.";
    }

  qDebug() << "MyIpAddress:" << ipAddress;
  return ipAddress;
}

void SkyLinesTracker::slotSendPing()
{
  if( ( m_testing == false && isServiceRequested() == false ) ||
        m_serverIpAdress.isNull() == true )
    {
      return;
    }

  qDebug() << "slotSendPing() is called";

  if( m_udp == static_cast<Udp *> (0) )
    {
      // Setup an UDP socket for data exchange with the SkyLines.aero server
      m_udp = new Udp( this,
                       m_serverIpAdress.toString(),
                       getDefaultPort() );

      connect( m_udp, SIGNAL(readyRead()),
               this, SLOT(slotReadPendingDatagrams()));

      connect( m_udp, SIGNAL(bytesWritten()),
               this, SLOT(slotBytesWritten()) );
    }

  // Send a ping packet to the server to verify the connection and the user's
  // live tracking key.
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

  QByteArray ba;
  ba.append( (const char *) &pp, sizeof(SkyLinesTracking::PingPacket) );

  bool result = m_udp->sendDatagram(ba);

  if( result == false )
    {
      qDebug() << "SkyLinesTracker::slotSendPing(): ping sending failed";
      emit connectionFailed();

      // Ping could not be send, try it again after 30s.
      QTimer::singleShot( 30000, this, SLOT(slotSendPing()) );
      return;
    }

  // Ping could be sent...
}

void SkyLinesTracker::slotReadPendingDatagrams()
{
  // UDP datagrams maybe available. Read in and process each datagram.
  while( m_udp->hasPendingDatagrams() )
    {
      QByteArray dg = m_udp->readDatagram();
      processDatagram( dg );
    }
}

void SkyLinesTracker::processDatagram( QByteArray& datagram )
{
  // Check header size.
  if( static_cast<uint>(datagram.size()) < sizeof(SkyLinesTracking::Header) )
    {
      qWarning() << "SkyLinesTracker::processDatagram(): Header to short!";
      return;
    }

  for( int i = 0; i < datagram.size(); i++ )
    {
      printf( "%02x ", (uchar) datagram.at(i) );
    }

    printf("\n");

  // extract header from diagram
  SkyLinesTracking::Header header;

  char* dst = (char *) &header;
  char* src = datagram.data();

  for( uint i = 0; i < sizeof(SkyLinesTracking::Header); i++ )
    {
      *dst = *src;
      src++;
      dst++;
    }

  const quint16 received_crc = FromBE16( header.crc );
  header.crc = 0;

  // Set CRC field in datagram to zero before crc calculation.
  datagram[4] = 0;
  datagram[5] = 0;

  const quint16 calculated_crc = UpdateCRC16CCITT( datagram.data(),
                                                   datagram.size(),
                                                   0 );
  // Check the CRCs, received and self calculated.
  if (received_crc != calculated_crc)
    {
      qWarning() << "SkyLinesTracker::processDatagram(): CRC error!";
      return;
    }

  // The possible response packages at the moment from the server.
  SkyLinesTracking::ACKPacket ack;

  // currently not used
  //SkyLinesTracking::TrafficResponsePacket traffic;
  //SkyLinesTracking::UserNameResponsePacket userName;
  //SkyLinesTracking::WaveResponsePacket wave;
  //SkyLinesTracking::ThermalResponsePacket thermal;

  // extract received package
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
    // That are all client packages, should not come from a server.
    break;

  case SkyLinesTracking::ACK:
    {
      // acknowledge packet
      if( static_cast<uint>(datagram.size()) < sizeof(SkyLinesTracking::ACK) )
        {
          qWarning() << "SkyLinesTracker::processDatagram(): ACK package to short!";
          return;
        }

      // Extract complete ack packet from datagram
      char* dst = (char *) &ack;
      char* src = datagram.data();

      for( uint i = 0; i < sizeof(SkyLinesTracking::ACKPacket); i++ )
        {
          *dst = *src;
          src++;
          dst++;
        }

      // Response id from request ping. We ignaore that.
      // quint16 id = FromBE16( ack.id );

      // Response flags. If set to 1 the ack is negativ.
      m_lastPingAnswer = FromBE32( ack.flags );

      emit pingResult( m_lastPingAnswer );

      if( m_lastPingAnswer == 1 )
        {
          // We got an negative answer from the server. User key seems to be invalid.
          qDebug() << "Got negative ping answer from SkyLines.aero server";
          stopLiveTracking();
        }

      break;
    }

  case SkyLinesTracking::TRAFFIC_RESPONSE:
    break;

  case SkyLinesTracking::USER_NAME_RESPONSE:
    break;

  case SkyLinesTracking::WAVE_RESPONSE:
    break;

  case SkyLinesTracking::THERMAL_RESPONSE:
     break;

  default:
    break;
  }
}

void SkyLinesTracker::slotBytesWritten()
{
  qDebug() << "SkyLinesTracker::slotBytesWritten()";

  m_sentPackages++;
  sendNextFixpoint();
}

bool SkyLinesTracker::routeTracking( const QPoint& position,
                                     const int altitude,
                                     const uint groundSpeed,
                                     const uint course,
                                     const double vario,
                                     qint64 utcTimeStamp )
{
  qDebug() << "SkyLinesTracker::routeTracking()";
#if 0
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true )
    {
      return false;
    }
#endif
  // Prepare a fix packet
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

  quint32 time = static_cast<quint32>( (utcTimeStamp * 1000) - m_startDay);
  fp.time = ToBE32(time);
  fp.reserved = 0;

  qint32 latitude = static_cast<qint32>(rint((double(position.x()) / 6.) * 10.));
  qint32 longitude = static_cast<qint32>(rint((double(position.y()) / 6.) * 10.));
  fp.location.latitude = ToBE32(latitude);
  fp.location.longitude = ToBE32(longitude);

  quint16 track = static_cast<quint16>(course);
  fp.track = ToBE16(track);

  quint16 gs = static_cast<quint16>((groundSpeed * 1000.0 / 3600.0) * 16);
  fp.ground_speed = ToBE16(gs);
  fp.airspeed = 0;
  fp.altitude = ToBE16(static_cast<qint16>(altitude));

  qint16 vs = static_cast<qint16>(vario * 256);
  fp.vario = ToBE16(vs);
  fp.engine_noise_level = 0;

  fp.header.crc = ToBE16( UpdateCRC16CCITT( static_cast<const void *>( &fp ),
                                            sizeof(fp), 0) );
  // Create datagram
  QByteArray ba;
  ba.append( (const char *) &fp, sizeof(SkyLinesTracking::FixPacket) );
  enqueueRequest( ba );
  return true;
}

bool SkyLinesTracker::endTracking()
{
  qDebug() << "SkyLinesTracker::endTracking()";

  m_fixPacketQueue.clear();
  m_retryTimer->stop();
  return true;
}

void SkyLinesTracker::slotRetry()
{
  qDebug() << "SkyLinesTracker::slotRetry()";

  // Data sending was not possible and the retry timer has expired.
  // As next we check if LiveTracking is still switched on.
  // If not, we stop here the data sending and clear the request queue.
  if( isServiceRequested() == false )
    {
      m_fixPacketQueue.clear();
      return;
    }

  // Take the next fix point from the queue and try to send it to the server.
  sendNextFixpoint();
}

bool SkyLinesTracker::enqueueRequest( QByteArray& fixPaket )
{
  qDebug() << "SkyLinesTracker::enqueueRequest()";

  checkQueueLimit();
  m_fixPacketQueue.enqueue( fixPaket );
  return sendNextFixpoint();
}

bool SkyLinesTracker::sendNextFixpoint()
{
  qDebug() << "SkyLinesTracker::sendNextFixpoint()";

  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true ||
      m_lastPingAnswer == 1 )
    {
      // Note! A ping command can also call this method.
      return false;
    }

  // A new fix packet is only send, if the ping command has received a
  // positive response. That guaranteed, that the user key is valid.
  if( m_fixPacketQueue.size() > 0 )
    {
      QByteArray& ba = m_fixPacketQueue.head();

      bool result = m_udp->sendDatagram(ba);

      if( result == false )
        {
          // Fix packet could not be send, try it again after 30s.
          m_retryTimer->start();
          return false;
        }

      // Remove last sent fix packet from the queue in case of success.
      m_fixPacketQueue.removeFirst();
    }

  return true;
}

void SkyLinesTracker::checkQueueLimit()
{
  qDebug() << "SkyLinesTracker::checkQueueLimit()";

  while( m_fixPacketQueue.size() > MaxQueueLen )
    {
      qDebug() << "QSize=" << m_fixPacketQueue.size() << "hau was wech";
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
  QString msg = QString(tr("<html>Your SkyLines user key is invalid!<br><br>Switching off service.</html>"));

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
