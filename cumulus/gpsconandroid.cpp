/***********************************************************************
 **
 **   gpsconandroid.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2012-2025 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 ***********************************************************************/

#include <signal.h>
#include <unistd.h>

#include <QtCore>

#include "androidevents.h"
#include "generalconfig.h"
#include "gpsconandroid.h"
#include "gpsnmea.h"
#include "jnisupport.h"
#include "MainWindow.h"
#include "TcpSocket.h"
#include "KRT2.h"

#ifdef FLARM
#include "flarmbase.h"
#include "flarmbincomandroid.h"
#endif

// static members
GpsConAndroid* GpsConAndroid::instance1 = 0;

QByteArray GpsConAndroid::rcvBuffer;
QMutex     GpsConAndroid::mutexRead;
QMutex     GpsConAndroid::mutexWrite;
QMutex     GpsConAndroid::mutexAction;

TcpSocket* GpsConAndroid::m_xcvario = 0;
QString    GpsConAndroid::m_xcvarioIp;
QString    GpsConAndroid::m_xcvarioPort;
bool       GpsConAndroid::m_xcvarioActive = false;

TcpSocket* GpsConAndroid::m_xcgps = 0;
QString    GpsConAndroid::m_xcgpsIp;
QString    GpsConAndroid::m_xcgpsPort;
bool       GpsConAndroid::m_xcgpsActive = false;

KRT2*      GpsConAndroid::m_krt2 = 0;
QString    GpsConAndroid::m_krt2Ip;
QString    GpsConAndroid::m_krt2Port;
bool       GpsConAndroid::m_krt2Active = false;
bool       GpsConAndroid::m_wiFiRequest = false;

GpsConAndroid::GpsConAndroid(QObject* parent) : QObject(parent)
{
  static short instances = 0;

  setObjectName( "GpsConAndroid" );

  if( instances == 0 )
    {
      instances++;
      instance1 = this;
    }
}

GpsConAndroid::~GpsConAndroid()
{
  // qDebug() << "~GpsConAndroid()";
}

bool GpsConAndroid::event(QEvent *event)
{
  // Handles WiFi request from the Android system
  if( event->type() == QEvent::User + 8 )
    {
      qDebug() << "GpsConAndroid: WiFi event received";
      WifiEvent *wifiEvent = static_cast<WifiEvent *>(event);
      handleWiFiRequest( wifiEvent->requestInfo() );
      return true;
    }

  // Calls the default event processing.
  return QObject::event(event);
}

void GpsConAndroid::handleWiFiRequest( int request )
{
  qDebug() << "GpsConAndroid::handleWiFiRequest() request=" << request;
  m_wiFiRequest = request;
  slot_configChanged();
}

/**
 * This slot is called, to handle configuration changes.
 */
void GpsConAndroid::slot_configChanged()
{
  slot_xcvario();
  slot_xcgps();
  slot_krt2();
}

/**
 * This method is called to activate/deactivate the XCVario TCP connection interface.
 */
void GpsConAndroid::slot_xcvario()
{
  qDebug() << "GpsConAndroid::slot_xcvario()";

  GeneralConfig *conf = GeneralConfig::instance();

  QString ip = conf->getGpsWlanIp1();
  QString port = conf->getGpsWlanPort1();
  bool active = GeneralConfig::instance()->getGpsWlanCB1();
  const short channel = 0;

  if( active == true && m_wiFiRequest == 1 )
    {
      if( m_xcvario == 0 )
        {
          // No socket is active
          m_xcvario = new TcpSocket( 0, ip, port, channel );
          connect( m_xcvario, SIGNAL( rxData(QByteArray, const short)),
                   this, SLOT( slot_extractNmea( QByteArray, const short )) );

          connect( m_xcvario, SIGNAL( forwardDeviceError( const QString& error, const bool sound ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }
      else if( m_xcvarioIp != ip || m_xcvarioPort != port )
        {
          // IP data was changed
          m_xcvario->close();
          m_xcvario->deleteLater();
          m_xcvario = new TcpSocket( 0, ip, port, channel );
          connect( m_xcvario, SIGNAL( rxData(QByteArray, const short)),
                   this, SLOT( slot_extractNmea( QByteArray, const short )) );

          connect( m_xcvario, SIGNAL( forwardDeviceError( const QString& error, const bool sound ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }

      m_xcvario->slotConnect();
    }
  else // active is false
    {
      if( m_xcvario != 0 )
        {
          m_xcvario->close();
          m_xcvario->deleteLater();
          m_xcvario = 0;
        }
    }

  // store last configuration
  m_xcvarioIp = ip;
  m_xcvarioPort = port;
  m_xcvarioActive = active;
}

/**
 * This method is called to activate/deactivate the XCVario GPS TCP connection interface.
 */
void GpsConAndroid::slot_xcgps()
{
  qDebug() << "GpsConAndroid::slot_xcgps()";

  GeneralConfig *conf = GeneralConfig::instance();

  QString ip = conf->getGpsWlanIp2();
  QString port = conf->getGpsWlanPort2();
  bool active = GeneralConfig::instance()->getGpsWlanCB2();
  const short channel = 1;

  if( active == true && m_wiFiRequest == 1 )
    {
      if( m_xcgps == 0 )
        {
          // No socket is active
          m_xcgps = new TcpSocket( 0, ip, port, channel );
          connect( m_xcgps, SIGNAL( rxData(QByteArray, const short)),
                   this, SLOT( slot_extractNmea( QByteArray, const short )) );

          connect( m_xcgps, SIGNAL( forwardDeviceError( const QString&, const bool ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }
      else if( m_xcgpsIp != ip || m_xcgpsPort != port )
        {
          // IP data was changed
          m_xcgps->close();
          m_xcgps->deleteLater();
          m_xcgps = new TcpSocket( 0, ip, port, channel );
          connect( m_xcgps, SIGNAL( rxData(QByteArray, const short)),
                   this, SLOT( slot_extractNmea( QByteArray, const short )) );

          connect( m_xcgps, SIGNAL( forwardDeviceError( const QString&, const bool ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }

      m_xcgps->slotConnect();
    }
  else // active is false
    {
      if( m_xcgps != 0 )
        {
          m_xcgps->close();
          m_xcgps->deleteLater();
          m_xcgps = 0;
        }
    }

  // store last configuration
  m_xcgpsIp = ip;
  m_xcgpsPort = port;
  m_xcgpsActive = active;
}

/**
 * This slot is called to handle the KRT-2 connection interface.
 */
void GpsConAndroid::slot_krt2()
{
  qDebug() << "GpsConAndroid::slot_krt2()";

  GeneralConfig *conf = GeneralConfig::instance();

  QString ip = conf->getGpsWlanIp3();
  QString port = conf->getGpsWlanPort3();
  bool active = GeneralConfig::instance()->getGpsWlanCB3();

  if( active == true && m_wiFiRequest == 1 )
    {
      if( m_krt2 == 0 )
        {
          // No KRT2 is active
          m_krt2 = new KRT2( 0, ip, port );

          connect( m_krt2, SIGNAL( forwardDeviceError( const QString&, const bool ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }
      else if( m_krt2Ip != ip || m_krt2Port != port )
        {
          // IP data was changed
          m_krt2->close();
          m_krt2->deleteLater();
          m_krt2 = new KRT2( 0, ip, port );

          connect( m_krt2, SIGNAL( forwardDeviceError( const QString&, const bool ) ),
                   MainWindow::mainWindow(), SLOT( slotNotification( const QString&, const bool ) ) );
        }

      m_krt2->slotConnect();
    }
  else // active is false
    {
      if( m_krt2 != 0 )
        {
          m_krt2->close();
          m_krt2->deleteLater();
          m_krt2 = 0;
        }
    }

  // store last configuration
  m_krt2Ip = ip;
  m_krt2Port = port;
  m_krt2Active = active;
}

bool GpsConAndroid::sndByte( const char byte )
{
  // Called to transfer a byte to the GPS port of the java part.
  QMutexLocker locker(&mutexWrite);
  return jniByte2Gps( byte );
}

bool GpsConAndroid::sndBytes( QByteArray& bytes )
{
  // Called to transfer a byte stream to the GPS port of the java part.
  QMutexLocker locker(&mutexWrite);

  bool ok = true;

  for( int i = 0; i < bytes.size(); i++ )
    {
      ok &= jniByte2Gps( bytes.at(i) );
    }

  return ok;
}

void GpsConAndroid::rcvByte( const char byte )
{
  // Called if a byte is read from the GPS port of the java part.
  QMutexLocker locker(&mutexRead);

  rcvBuffer.append( byte );

  if( FlarmBase::getProtocolMode() == FlarmBase::text && byte == '\n' )
    {
      // Check if Flarm has answered to the $PFLAX command
      if( rcvBuffer.contains( "$PFLAX,A") )
        {
          // This answer is expected by the flarmBinMode() method.
          return;
        }

      // Flarm works in text mode and the complete GPS sentence must be
      // forwarded to GpsNmea.
      QString ns( rcvBuffer.data() );
      forwardNmea( ns );
      rcvBuffer.clear();
      return;
    }

  // We check the buffer size to avoid a blocking case.
  if( rcvBuffer.size() > 8*1024 )
    {
      // More than 8KB received, we will clear the buffer.
      rcvBuffer.clear();
      qWarning() << "GpsConAndroid::rcvByte: 8KB reached, rcvBuffer is cleared!";
    }

  // Flarm works in binary mode, do nothing more as to store the byte.
  // Another thread will read it.
  return;
}

bool GpsConAndroid::getByte( unsigned char* b, const int timeout )
{
  // Called to read out a byte from the byte buffer. Sometimes the Flarm needs
  // a longer time to provide the data. Therefore a timeout of 10s can be necessary.
  int loop = timeout / 10;

  if( loop == 0 )
    {
      // Set default timeout to 10 seconds.
      loop = 10000 / 10;
    }

  while( loop-- )
    {
      mutexRead.lock();

      if( rcvBuffer.size() > 0 )
        {
          *b = rcvBuffer.at(0);
          rcvBuffer.remove( 0, 1);
          mutexRead.unlock();
          return true;
        }

      mutexRead.unlock();
      usleep( 10 * 1000 ); // Wait 10ms
    }

  qWarning() << "GpsConAndroid::getByte(): Timeout after" << timeout << "ms!";
  return false;
}

/**
 * Extract a NMEA sentence from the data stream. Two rxBuffers (0, 1) are
 * exist, to handle 2 streams separately.
 */
void GpsConAndroid::slot_extractNmea( QByteArray stream, const short rxBufIdx )
{
  // qDebug() << "GpsConAndroid::slot_extractNmea(): IDX=" << rxBufIdx;

  static QByteArray srxBuffer[2];

  if( rxBufIdx < 0 || rxBufIdx > 1 )
    {
      qDebug() << "GpsConAndroid::slot_extractNmea(): RX buffer index out of range";
      return;
    }

  srxBuffer[rxBufIdx].append( stream );

  while( srxBuffer[rxBufIdx].size() > 0 && srxBuffer[rxBufIdx].contains( "\n") == true )
    {
      QString s = srxBuffer[rxBufIdx].left( srxBuffer[rxBufIdx].indexOf( '\n' ) + 1 );

      if( s.size() > 1 || ( s.size() == 1 && s.at(0) != QChar('\n') ) )
        {
          // XCVario send NL as alive sign. That must be filtered out.
          forwardNmea( s );
        }

      srxBuffer[rxBufIdx].remove( 0, s.size() );
    }
}

void GpsConAndroid::forwardNmea( QString& qnmea )
{
  // qDebug() << "GpsConAndroid::forwardNmea():" << qnmea;
  static QHash<QString, short> gpsKeys;
  static GeneralConfig* gci = 0;
  static bool init = false;

  if( init == false )
    {
      GpsNmea::getGpsMessageKeys( gpsKeys );
      gci = GeneralConfig::instance();
      init = true;
    }

  if( verifyCheckSum( qnmea.toLatin1().data() ) == false )
    {
      return;
    }

  if( gci->getGpsNmeaLogState() == false )
    {
      // Check, if sentence is of interest for us.
      QString item = qnmea.mid( 0, qnmea.indexOf( QChar(',') ) );

      if( gpsKeys.contains(item) == false )
        {
          // Ignore undesired sentences for performance reasons. They are
          // only forwarded, if data file logging is switched on.
          return;
        }
    }

  // Hand over the GPS data as event to the GUI thread.
  GpsNmeaEvent *ne = new GpsNmeaEvent(qnmea);
  QCoreApplication::postEvent( GpsNmea::gps, ne, Qt::HighEventPriority );
}

/**
 * Verify the checksum of the passed sentences.
 *
 * @returns true (success) or false (error occurred)
 */
bool GpsConAndroid::verifyCheckSum( const char *sentence )
{
  // Filter out wrong data messages read in from the GPS port. Known messages
  // do start with a dollar sign or an exclamation mark.
  if( sentence[0] != '$' && sentence[0] != '!' )
    {
      qWarning() << "GpsConAndroid::CheckSumError:" << sentence;
    }

  for( int i = strlen(sentence) - 1; i >= 0; i-- )
    {
      if( sentence[i] == '*' )
        {
          if( (strlen(sentence) - 1 - i) < 2 )
            {
              // too less characters
              return false;
            }

          char checkBytes[3];
          checkBytes[0] = sentence[i+1];
          checkBytes[1] = sentence[i+2];
          checkBytes[2] = '\0';

          bool ok = false;
          uchar checkSum = (uchar) QString( checkBytes ).toUShort( &ok, 16 );

          if( ok && checkSum == GpsNmea::calcCheckSum( sentence ) )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  return false;
}

#ifdef FLARM

bool GpsConAndroid::flarmBinMode()
{
  if( FlarmBase::getProtocolMode() == FlarmBase::binary )
    {
      qDebug() << "GpsConAndroid::flarmBinMode() is binary";
      return true;
    }

  // Binary switch command for Flarm interface
  QByteArray pflax("$PFLAX\r\n");

  FlarmBinComAndroid fbc;

  qDebug() << "GpsConAndroid::flarmBinMode(): Request Flarm to switch to binary mode";

  // Switch connection to binary mode.
  if( sndBytes( pflax ) == false )
    {
      // write failed
      qWarning() << "GpsConAndroid::flarmBinMode(): Sending of $PFLAX failed!";
      return false;
    }

  // Read out the Flarm answer. Flarm sent $PFLAX,A*2E\r\n in good case.
  // Note that other sentences can be sent before the answer $PFLAX is sent.
  const char* ok = "$PFLAX,A*2E";
  const char* error = "$PFLAX,A,ERROR";
  QByteArray buffer;
  int loops = 512;
  unsigned char c;
  bool readAnswer = false;

  while( loops-- )
    {
      bool res = getByte( &c, 0 );

      if( res == true )
        {
          buffer.append( c );
        }
      else
        {
          // timeout
          qWarning() << "GpsConAndroid::flarmBinMode(): Timeout!";
          return false;
        }

      if( c == '\n' )
        {
          if( buffer.contains( ok ) == true )
            {
              qDebug() << "GpsConAndroid::flarmBinMode(): $PFLAX switch to binary mode Ok!";
              FlarmBase::setProtocolMode( FlarmBase::binary );
              readAnswer = true;
              break;
            }
          else if( buffer.contains( error ) == true )
            {
              qWarning() << "GpsConAndroid::flarmBinMode(): $PFLAX Error!";
              return false;
            }
          else
            {
              // Expected answer was not received, clear buffer for next sentence.
              buffer.clear();
            }
        }
    }

  if( readAnswer == false )
    {
      qWarning() << "GpsConAndroid::flarmBinMode(): $PFLAX no answer!";
      return false;
    }

  // I made the experience, that the Flarm device did not answer to the first
  // binary transfer switch. Therefore I make several tries. Flarm tool makes
  // the same, as I could observe with a RS232 port sniffer.
  bool pingOk = false;
  int loop = 5;

  while( loop-- )
    {
      // Check connection with a ping command.
      if( fbc.ping() == true )
        {
          pingOk = true;
          break;
        }
    }

  if( pingOk == false )
    {
      // Switch to binary mode failed
      qWarning() << "GpsConAndroid::flarmBinMode(): Switch failed!";
    }

  return pingOk;
}

// This action must be executed in a thread.
void GpsConAndroid::getFlarmFlightList()
{
  QMutexLocker locker(&mutexAction);
  FlarmBinComAndroid fbc;

  if( flarmBinMode() == false )
    {
       // Hand over the flight list data as event to the GUI thread.
      FlarmFlightListEvent *event = new FlarmFlightListEvent("Error");
      QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
      return;
    }

  // read out flight header records
  int recNo = 0;
  char buffer[MAXSIZE];
  int flights = 0;

  while( true )
    {
      if( fbc.selectRecord( recNo ) == true )
        {
          recNo++;

          if( fbc.getRecordInfo( buffer ) )
            {
              QString qbuffer( buffer );
              FlarmFlightListEvent *event = new FlarmFlightListEvent(qbuffer);
              QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
              flights++;
            }
          else
            {
              qWarning() << "GpsConAndroid::getFlarmFlightList(): GetRecordInfo("
                          << (recNo - 1)
                          << ") failed!";
              break;
            }
        }
      else
        {
          // No more records available
          break;
        }
    }

  // Send back flight headers answer to application
  QString answer;

  if( flights == 0 )
    {
      answer = "Empty";
    }
  else
    {
      answer = "End";
    }

  // Hand over the flight list data as event to the GUI thread.
  FlarmFlightListEvent *event = new FlarmFlightListEvent(answer);
  QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
  return;
}

// This action must be executed in a thread.
void GpsConAndroid::getFlarmIgcFiles(QString& args)
{
  // qDebug() << "GpsConAndroid::getFlarmIgcFiles()" << args;
  QMutexLocker locker(&mutexAction);

  // The argument string contains at the first position the destination directory
  // for the files and then the indexes of the flights separated by vertical tabs.
  QStringList idxList = args.split("\v");

  if( idxList.size() < 2 )
    {
      return;
    }

  FlarmBinComAndroid fbc;

  if( flarmBinMode() == false )
    {
      flarmFlightDowloadInfo( "Error" );
      return;
    }

  // read out flights
  char buffer[MAXSIZE];
  int progress = 0;

  // Check, if the download directory exists. Here we take the directory element
  // from the list.
  QDir igcDir( idxList.takeFirst() );

  if( ! igcDir.exists() )
    {
      if( ! igcDir.mkpath( igcDir.absolutePath() ) )
        {
          flarmFlightDowloadInfo( "Error create directory" );
          return;
        }
    }

  QTime dlTime;

  for( int idx = 0; idx < idxList.size(); idx++ )
    {
      dlTime.start();
      downloadTimeControl.start();

      // Select the flight to be downloaded
      int recNo = idxList.at(idx).toInt();
      QStringList flightData;

      if( fbc.selectRecord(recNo ) == true )
        {
          // read flight header data
          if( fbc.getRecordInfo( buffer ) )
            {
              flightData = QString( buffer ).split("|");
            }
          else
            {
              // Entry not available, although select answered positive!
              // Not conform to the specification.
              flarmFlightDowloadInfo( "Error" );
              return;
            }

          // Open an IGC file for writing download data.
          QFile f( igcDir.absolutePath() + "/" + flightData.at(0) );

          if( ! f.open( QIODevice::WriteOnly ) )
            {
              // could not open file ...
              qWarning() << "Cannot open file: " << f.fileName();
              flarmFlightDowloadInfo( "Error open file" );
              return;
            }

          int lastProgress = -1;
          bool eof = false;

          while( fbc.getIGCData(buffer, &progress) )
            {
              if( lastProgress != progress || downloadTimeControl.elapsed() >= 10000 )
                {
                  // After a certain time a progress must be reported otherwise
                  // the GUI thread runs in a timeout.
                  downloadTimeControl.start();

                  // That eliminates a lot of intermediate steps
                  flarmFlightDowloadProgress(recNo, progress);
                  lastProgress = progress;
                }

              if( buffer[strlen(buffer) - 1] == 0x1A )
                {
                  // EOF was send by the Flarm, remove it from the data stream.
                   buffer[strlen(buffer) - 1] = '\0';
                   eof = true;
                 }

              f.write(buffer);

              if( eof )
                {
                  break;
                }
            }

          f.close();

          if( eof == false )
            {
              // Abort downloads due to timeout error
              flarmFlightDowloadInfo( "Error" );
              return;
            }

          qDebug() << flightData.at(0) << "downloaded in"
                   << (dlTime.elapsed() / 1000.0) << "s";
        }
     }


  flarmFlightDowloadInfo( "Finished" );
}

void GpsConAndroid::flarmFlightDowloadInfo( QString info )
{
  // Hand over the flight download info as event to the GUI thread.
  FlarmFlightDownloadInfoEvent *event = new FlarmFlightDownloadInfoEvent(info);
  QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
}

/** Reports the flight download progress to the calling application. */
void GpsConAndroid::flarmFlightDowloadProgress( const int idx, const int progress )
{
  // Hand over the flight download progress data as event to the GUI thread.
  FlarmFlightDownloadProgressEvent *event = new FlarmFlightDownloadProgressEvent( idx, progress );
  QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
}

bool GpsConAndroid::flarmReset()
{
  // qDebug() << "GpsConAndroid::flarmReset()";

  if( ! flarmBinMode() )
    {
     return false;
    }

  FlarmBinComAndroid fbc;
  bool res = fbc.exit();

  // Switch Flarm back to text mode.
  FlarmBase::setProtocolMode( FlarmBase::text );
  return res;
}

/**
 * Starts a thread which gets the Flarm flight list.
 */
void GpsConAndroid::startGetFlarmFlightList()
{
  // qDebug() << "GpsConAndroid::startGetFlarmFlightList";

  FlarmFlightListThread* thread = new FlarmFlightListThread(this);
  thread->start();
}

/**
 * Starts a thread which executes the Flarm flight IGC downloads.
 */
void GpsConAndroid::startGetFlarmIgcFiles( QString& flightData )
{
  // qDebug() << "GpsConAndroid::FlarmFlightListThread";

  FlarmIgcFilesThread* thread = new FlarmIgcFilesThread( this, flightData );
  thread->start();
}

//#############################################################################

// A better approach would be:
// http://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
FlarmFlightListThread::FlarmFlightListThread( QObject *parent ) : QThread( parent )
{
  setObjectName( "FlarmFlightListThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

FlarmFlightListThread::~FlarmFlightListThread()
{
  // qDebug() << "~FlarmFlightListThread()";
}

void FlarmFlightListThread::run()
{
  // qDebug() << "FlarmFlightListThread::run()";

  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  GpsConAndroid::instance()->getFlarmFlightList();
}

//#############################################################################

FlarmIgcFilesThread::FlarmIgcFilesThread( QObject *parent, QString& flightData ) :
  QThread( parent ),
  m_flightData( flightData )
{
  setObjectName( "FlarmIgcFilesThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

FlarmIgcFilesThread::~FlarmIgcFilesThread()
{
  // qDebug() << "~FlarmIgcFilesThread()";
}

void FlarmIgcFilesThread::run()
{
  // qDebug() << "FlarmIgcFilesThread::run():" << m_flightData;

  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  GpsConAndroid::instance()->getFlarmIgcFiles( m_flightData );
}

#endif
