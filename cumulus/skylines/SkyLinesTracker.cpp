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

SkyLinesTracker::PackageId SkyLinesTracker::m_packetId = 0;

SkyLinesTracker::SkyLinesTracker(QObject *parent) :
  LiveTrackBase(parent),
  m_retryTimer(0),
  m_hostLookupIsRunning(false),
  m_liveTrackingKey(0),
  m_udpSend(0),
  m_udpReceive(0),
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

  if( m_udpSend != 0 )
    {
      m_udpSend->close();
      delete(m_udpSend);
    }

  if( m_udpReceive != 0 )
    {
      m_udpReceive->close();
      delete(m_udpReceive);
    }
}

bool SkyLinesTracker::startTracking()
{
  const char* method = "SkyLinesTracker::startTracking():";

  if( isServiceRequested() == false )
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

  // Check, if user name contains a live tracking key entry.
  GeneralConfig* conf = GeneralConfig::instance();

  m_liveTrackingKeyString = conf->getLiveTrackUserName();

  if( m_liveTrackingKeyString.isEmpty() )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines tracking user key is missing!";
      return false;
    }

  bool ok;
  m_liveTrackingKey = m_liveTrackingKeyString.toULongLong(&ok, 16);

  if( ok == false )
    {
      qWarning() << __LINE__ << method
                 << "SkyLines tracking user key is wrong, no uint64!";
      return false;
    }

  // Get host information.
  slotHostInfoRequest();
  return true;
}

void SkyLinesTracker::slotHostInfoRequest()
{
  if( isServiceRequested() == false || m_hostLookupIsRunning == true )
    {
      return;
    }

  if( m_serverIpAdress.isNull() && m_hostLookupIsRunning == false )
    {
      // We need the IP address of the skyline tracking server.
      m_hostLookupIsRunning = true;

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
      qWarning() << "Lookup failed:" << hostInfo.errorString();

      emit connectionFailed();

      if( isServiceRequested() == true )
        {
          // Request host info again after 60s.
          QTimer::singleShot( 60000, this, SLOT(slotHostInfoRequest()) );
          return;
        }
    }

  // We take the first address only.
  m_serverIpAdress = hostInfo.addresses().first();

  qDebug() << "IP-Address" << m_serverIpAdress.toString();

  // Send a ping to the skyLines server to verify the user key.
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
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true )
    {
      return;
    }

  qDebug() << "slotSendPing() is called";

  if( m_udpSend == static_cast<QUdpSocket *> (0) )
    {
      if( m_udpReceive != static_cast<QUdpSocket *> (0) )
        {
          m_udpReceive->close();
          delete m_udpReceive;
        }

      /*
       * Note! Qt's UdpSocket has problems, if I try to use one UdpSocket
       * for sending and receiving.
       */

      // Setup an UDP socket for data exchange with the SkyLines server
      m_udpSend = new QUdpSocket( this );
      //m_udpSend->joinMulticastGroup( QHostAddress( getMyIpAddress() ));

      // Setup an UDP socket as receiver.
      m_udpReceive = new QUdpSocket( this );
      //m_udpReceive->joinMulticastGroup( QHostAddress( getMyIpAddress() ));

      m_udpSend->connectToHost( m_serverIpAdress,
                                getDefaultPort(),
                                QIODevice::WriteOnly );

      if( m_udpReceive->bind( m_udpSend->localPort(),
                              QUdpSocket::ReuseAddressHint |
                              QUdpSocket::ShareAddress) == false )
        {
          qWarning() << "SkyLinesTracker::slotSendPing: bind to host"
                     << m_serverIpAdress.toString()
                     << "failed";
          qDebug() << m_udpReceive->error() << m_udpReceive->errorString();

          delete m_udpSend; m_udpSend = 0;
          delete m_udpReceive; m_udpReceive = 0;
          return;
        }

      connect( m_udpSend, SIGNAL(bytesWritten(qint64)),
               this, SLOT(slotBytesWritten(qint64)) );

      connect( m_udpReceive, SIGNAL(readyRead()),
               this, SLOT(slotReadPendingDatagrams()));

      slotReadPendingDatagrams();
    }

  // Send a ping packet to the server to verify the connection and the user's
  // key.
  SkyLinesTracking::PingPacket pp;
  pp.header.magic = ToBE32( SkyLinesTracking::MAGIC );
  pp.header.crc = 0;
  pp.header.type = ToBE16( SkyLinesTracking::Type::PING);
  pp.header.key = ToBE32( m_liveTrackingKey );
  pp.id = ToBE16( getId() );
  pp.reserved = 0;
  pp.reserved2 = 0;
  pp.header.crc = ToBE16( UpdateCRC16CCITT( static_cast<const void *>( &pp ),
                                            sizeof(pp), 0) );

  qint64 res = m_udpSend->write( (const char *) &pp,
                                 sizeof(SkyLinesTracking::PingPacket) );

  qDebug() << "Size Ping" << sizeof(SkyLinesTracking::PingPacket)
           << "res=" << res;

  if( res == -1 )
    {
      qDebug() << "slotSendPing(): failed";

      emit connectionFailed();

      // Ping could not be send, try it again after 60s.
      QTimer::singleShot( 60000, this, SLOT(slotSendPing()) );
      return;
    }

  // Ping could be sent...
}

void SkyLinesTracker::slotReadPendingDatagrams()
{
  // An UDP diagram is available. Read in each datagram.
  while( m_udpReceive->hasPendingDatagrams() )
    {
      QByteArray datagram;
      datagram.resize( m_udpReceive->pendingDatagramSize() );
      QHostAddress sender;
      quint16 senderPort;

      m_udpReceive->readDatagram( datagram.data(), datagram.size(),
                                  &sender, &senderPort );

      processDatagram( datagram );
    }
}

void SkyLinesTracker::processDatagram( QByteArray& datagram )
{
  if( static_cast<uint>(datagram.size()) < sizeof(SkyLinesTracking::Header) )
    {
      qWarning() << "SkyLinesTracker::processDatagram: Header to short!";
      return;
    }

  // extract header
  SkyLinesTracking::Header header;

  qstrncpy( (char *) &header,
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

  // possible response packages from the server
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
          qWarning() << "SkyLinesTracker::processDatagram: ACK package to short!";
          return;
        }

      qstrncpy( (char *) &ack,
                datagram.data(),
                sizeof(SkyLinesTracking::ACKPacket) );

      // Response id for request id
      // quint16 id = FromBE16( ack.id );

      // Response flags.
      quint32 flags = FromBE32( ack.flags );
      m_lastPingAnswer = static_cast<qint32>(flags);

      emit pingResult( m_lastPingAnswer );

      if( m_lastPingAnswer != 0 )
        {
          // We got an negative answer from the server. User key seems to be invalid.
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

void SkyLinesTracker::slotBytesWritten( qint64 bytes )
{
  Q_UNUSED(bytes);

  qDebug() << "slotBytesWritten" << bytes;

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
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true )
    {
      return false;
    }

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

  quint32 time = static_cast<quint32>(utcTimeStamp - m_startDay);

  fp.time = ToBE32(time);
  fp.reserved = 0;

  SkyLinesTracking::GeoPoint geoPoint;
  geoPoint.latitude = static_cast<qint32>(rint((double(position.x()) / 6.) * 100.));
  geoPoint.longitude = static_cast<qint32>(rint((double(position.y()) / 6.) * 100.));
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
  enqueueRequest( fp );
  return true;
}

bool SkyLinesTracker::endTracking()
{
  m_fixPacketQueue.clear();
  m_retryTimer->stop();
  return true;
}

void SkyLinesTracker::slotRetry()
{
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

bool SkyLinesTracker::enqueueRequest( SkyLinesTracking::FixPacket fixPaket )
{
  checkQueueLimit();
  m_fixPacketQueue.enqueue( fixPaket );
  return sendNextFixpoint();
}

bool SkyLinesTracker::sendNextFixpoint()
{
  if( isServiceRequested() == false || m_serverIpAdress.isNull() == true ||
      m_lastPingAnswer != 0 )
    {
      // Note! A ping command can also call this method.
      return false;
    }

  // A new fix packet is only send, if the ping command has received a
  // positive response. That guaranteed, that the user key is valid.
  if( ! m_fixPacketQueue.isEmpty() )
    {
      SkyLinesTracking::FixPacket fp = m_fixPacketQueue.head();

      qint64 res = m_udpSend->write( (const char *) &fp,
                                     sizeof(SkyLinesTracking::FixPacket) );

      if( res == -1 )
        {
          // Fix packet could not be send, try it again after 60s.
          m_retryTimer->start();
          return false;
        }

      // Remove last sent fix packet from the queue in case of success.
      fp = m_fixPacketQueue.dequeue();
    }

  return true;
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
