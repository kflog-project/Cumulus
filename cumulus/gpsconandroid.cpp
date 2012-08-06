/***********************************************************************
 **
 **   gpsconandroid.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2012 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <signal.h>
#include <unistd.h>

#include <QtGui>

#include "androidevents.h"
#include "generalconfig.h"
#include "gpsconandroid.h"
#include "gpsnmea.h"
#include "jnisupport.h"

#ifdef FLARM
#include "flarm.h"
#include "flarmbincomandroid.h"
#endif

// static members
QByteArray GpsConAndroid::rcvBuffer;
QMutex     GpsConAndroid::mutexRead;
QMutex     GpsConAndroid::mutexWrite;
QMutex     GpsConAndroid::mutexAction;

GpsConAndroid::GpsConAndroid(QObject* parent) : QObject(parent)
{
  setObjectName( "GpsConAndroid" );
}

GpsConAndroid::~GpsConAndroid()
{
  qDebug() << "~GpsConAndroid()";
}

bool GpsConAndroid::sndByte( const char byte )
{
  // Called to transfer a byte to the GPS port of the java part.
  QMutexLocker locker(&mutexWrite)
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

  if( Flarm::getProtocolMode() == Flarm::text && byte == '\n' )
    {
      // Flarm works in text mode and the complete GPS sentence must be
      // forwarded to GpsNmea.
      QString ns( rcvBuffer.data() );
      forwardNmea( ns );
      rcvBuffer.clear();
      return;
    }

  // Flarm works in binary mode, do nothing more as to store the byte.
  // Another thread will read it.
  return;
}

bool GpsConAndroid::getByte( unsigned char* b )
{
  // Called to read out a byte from the byte buffer.
  int loop = 60; // Timeout is 3s

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
      usleep( 50 * 1000 ); // Wait 50ms
    }

  qWarning() << "GpsConAndroid::getByte(): Timeout!";
  return false;
}

void GpsConAndroid::forwardNmea( QString& qnmea )
{
  static QHash<QString, short> gpsKeys;
  static GeneralConfig* gci = 0;
  static bool init = false;

  if( init == false )
    {
      GpsNmea::getGpsMessageKeys( gpsKeys );
      gci = GeneralConfig::instance();
      init = true;
    }

  if( verifyCheckSum( qnmea.toAscii().data() ) == false )
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
  qDebug() << "GpsConAndroid::flarmBinMode()";

  // Binary switch command for Flarm interface
  QByteArray pflax = QByteArray("$PFLAX\n");

  FlarmBinComAndroid fbc;

  // Precondition is that the NMEA output of the Flarm device was disabled by
  // the calling method before!

  // qDebug() << "Switch Flarm to binary mode";

  // I made the experience, that the Flarm device did not answer to the first
  // binary transfer switch. Therefore I make several tries. Flarm tool makes
  // the same, as I could observe with a RS232 port sniffer.
  bool pingOk = false;
  int loop = 5;

  while( loop-- )
    {
      // Switch connection to binary mode.
      if( sndBytes(pflax) == false )
        {
          // write failed
          break;
        }

      // Check connection with a ping command.
      if( fbc.ping() == true )
        {
          Flarm::setProtocolMode( Flarm::binary );
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
  qDebug() << "GpsConAndroid::getFlarmFlightList()";

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
  QStringList flights;

  while( true )
    {
      if( fbc.selectRecord( recNo ) == true )
        {
          recNo++;

          if( fbc.getRecordInfo( buffer ) )
            {
              flights << QString( buffer );
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

  // Send back flight headers to application
  QString list;

  if( flights.size() )
    {
      list = flights.join("\n");
    }
  else
    {
      list = "Empty";
    }

  // Hand over the flight list data as event to the GUI thread.
  FlarmFlightListEvent *event = new FlarmFlightListEvent(list);
  QCoreApplication::postEvent( GpsNmea::gps, event, Qt::HighEventPriority );
  return;
}

// This action must be executed in a thread.
void GpsConAndroid::getFlarmIgcFiles(QString& args)
{
  qDebug() << "GpsConAndroid::getFlarmIgcFiles()" << args;

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
  uint progress = 0;

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

  for( int idx = 0; idx < idxList.size(); idx++ )
    {
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

          uint lastProgress = -1;
          bool eof = false;

          while( fbc.getIGDData(buffer, &progress) )
            {
              if( lastProgress != progress )
                {
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
  qDebug() << "GpsConAndroid::flarmReset()";

  if( ! flarmBinMode() )
    {
     return false;
    }

  FlarmBinComAndroid fbc;
  bool res = fbc.exit();

  // Enable NMEA output of Flarm after 60 seconds. Flarm needs some time to
  // coming up again after a reset.
  QTimer::singleShot( 60000, this, SLOT(slot_FlarmTextMode()) );
  return res;
}

void GpsConAndroid::slot_FlarmTextMode()
{
  // Enable NMEA output of Flarm device.
  if( GpsNmea::gps->sendSentence( QString("$PFLAC,S,NMEAOUT,1") ) == false )
    {
      qWarning() << "GpsConAndroid::slot_FlarmTextMode(): enable NMEA failed!";
    }
}

/**
 * Starts a thread which gets the Flarm flight list.
 */
void GpsConAndroid::startGetFlarmFlightList()
{
  qDebug() << "GpsConAndroid::startGetFlarmFlightList";

  FlarmFlightListThread* thread = new FlarmFlightListThread(this);
  thread->start();
}

/**
 * Starts a thread which executes the Flarm flight IGC downloads.
 */
void GpsConAndroid::startGetFlarmIgcFiles( QString& flightData )
{
  qDebug() << "GpsConAndroid::FlarmFlightListThread";

  FlarmIgcFilesThread* thread = new FlarmIgcFilesThread( this, flightData );
  thread->start();
}

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
  qDebug() << "~FlarmFlightListThread()";
}

void FlarmFlightListThread::run()
{
  qDebug() << "FlarmFlightListThread::run()";

  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  GpsConAndroid gca;
  gca.getFlarmFlightList();
}

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
  qDebug() << "~FlarmIgcFilesThread()";
}

void FlarmIgcFilesThread::run()
{
  qDebug() << "FlarmIgcFilesThread::run()";

  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  GpsConAndroid gca;
  gca.startGetFlarmIgcFiles( m_flightData );
}

#endif
