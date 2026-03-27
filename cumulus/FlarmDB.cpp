/***********************************************************************
**
**   FlarmNet.cpp
**
**   Created on: 12.10.2023 by Axel Pauli
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2023-2026 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <FlarmDB.h>
#include <QtCore>

#include "generalconfig.h"

QHash<uint, QString> FlarmDB::m_datamap;
QMutex FlarmDB::m_mutex;
QStringList FlarmDB::filterList;

void FlarmDB::unloadData()
{
  QMutexLocker locker( &m_mutex );
  m_datamap.clear();
}

int FlarmDB::loadFNData( QHash<uint, QString>& dict )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  uint items = 0; // number of loaded items
  bool start = true;

  // Check, which file the user wants to load.
  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  QString url = GeneralConfig::instance()->getFlarmNetUrl();

  if( url.isEmpty() )
    {
      // No file shall be loaded
      qWarning() << "FlarmNet: No DB file defined for loading!";
      return items;
    }

  QString fname = QFileInfo( url.mid( 6 ) ).fileName();
  QString fpath = dir.absolutePath() + "/flarmDB/" + fname;
  QFile file( fpath);

  if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open FlarmNet DB file %s for reading!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return items;
    }

  QString fs = GeneralConfig::instance()->getFlarmDBFilter().toUpper();
  filterList = fs.split( QRegExp("[\\s,]+"), QString::SkipEmptyParts);

  QTextStream in( &file );
  in.setCodec( "ISO 8859-15" );

  while( ! in.atEnd() )
    {
      QString line = in.readLine();

      if( start == true )
        {
          // Magic id is expected
          qDebug() << "FlarmNet magic" << line;
          start = false;
          continue;
        }

      if( line.size() < 172 )
        {
          // ignore short line.
          qDebug() << "FlarmNet: line shorter than 172:" << line.size();
          continue;
        }

      QByteArray ba = QByteArray::fromHex( line.toLatin1() );

      for( int i=0; i < ba.size(); i++ )
        {
          if( ba[i] > (char) 0x7e || ba[i] < ' ' )
            {
              ba[i] = '?';
            }
        }

      bool ok;
      uint fid = ba.mid( 0, 6 ).toUInt( &ok, 16);

      if( ! ok )
        {
          continue;
        }

      /**
        bytearray content
                  1         2         3         4         5         6         7         8
        01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
        3e7c78                     D-KMII               Ventus bT            D-KMII V  123.005
        ba, 6, record.flarm_id;
        ba + 6, 21, record.pilot;
        ba + 27, 21, record.airfield;
        ba + 48, 21, record.plane_type;
        ba + 69, 7, record.registration;
        ba + 76, 3, record.callsign;
        ba + 79, 7, record.frequency;
       */
      QByteArray fdata = ba.mid( 69, 7 ).trimmed().toUpper(); // KZ
      QString kz = QString( fdata ).toUpper();

      if( checkFilter( kz ) == false )
        {
          continue;
        }

      items++;
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 48, 21 ).trimmed() ); // Flugzeug Type
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 76, 3 ).trimmed() ); // WKZ
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 79, 7 ).trimmed() ); // Frequenz
      dict.insert( fid, fdata );
      // qDebug() << fid << fdata;
    } // End of While

  file.close();

  qDebug( "FlarmNet: %d items loaded in %lldms", items, t.elapsed() );
  return items;
}

int FlarmDB::loadOGNData( QHash<uint, QString>& dict )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  uint items = 0; // number of loaded items
  bool start = true;

  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  QString fpath = dir.absolutePath() + "/flarmDB/ogn.csv";
  QFile file( fpath);

  if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open OGN DB file %s for reading!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return items;
    }

  QString fs = GeneralConfig::instance()->getFlarmDBFilter().toUpper();
  filterList = fs.split( QRegExp("[\\s,]+"), QString::SkipEmptyParts);

  QTextStream in( &file );
  in.setCodec( "ISO 8859-15" );

  while( ! in.atEnd() )
    {
      QString line = in.readLine().trimmed();

      if( start == true && line.startsWith( "#" ) )
        {
          // headline is read
          // #DEVICE_TYPE,DEVICE_ID,AIRCRAFT_MODEL,REGISTRATION,CN,TRACKED,IDENTIFIED,AIRCRAFT_TYPE
          // 'F','000000','SZD-41 Jantar Std','HA-4403','J','Y','Y','1'
          qDebug() << "OGN header:" << line;
          start = false;
          continue;
        }

      if( line.startsWith( "#" ) == true )
        {
          // ignore comment line.
          continue;
        }

      // remove ' characters from the string
      line = line.remove( QChar( '\'' ) );

      QStringList qsl = line.split( ",", QString::KeepEmptyParts );

      if( qsl.size() < 8 )
        {
          // to less parameters
          continue;
        }

      if( qsl.at(0) == "O" )
        {
          // Read only Flarm or ICAO entries
          continue;
        }

      bool ok;
      // Device ID (Flarm, ICAO)
      uint did = qsl.at(1).toUInt( &ok, 16);

      if( ! ok )
        {
          continue;
        }

      // Registration sign (Kennzeichen)
      QString kz = qsl.at(3).trimmed().toUpper();

      if( checkFilter( kz ) == false )
        {
          continue;
        }

      items++;

      QString entry = kz;
      entry.append( "|" ); // Trenner
      entry.append( qsl.at(2).trimmed() ); // Flugzeug Type
      entry.append( "|" ); // Trenner
      entry.append( qsl.at(4).trimmed() ); // WKZ
      entry.append( "|" ); // Trenner

      QString oldEntry;

      if( dict.contains( did ) == true )
        {
          // Check for an old entry, if frequency is defined
          oldEntry = m_datamap[ did ];

          QStringList qslOld = oldEntry.split( "|", QString::KeepEmptyParts );

          if( qslOld.size() >= 4 && qslOld.at(3).isEmpty() == false )
            {
              // Take frequency over as 4th argument
              entry.append( qslOld.at(3) );
           }
        }

      dict.insert( did, entry );
    } // End of While

  file.close();

  qDebug( "OGN: %d items loaded in %lldms", items, t.elapsed() );
  return items;
}

int FlarmDB::applyFilter( QString& filter )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  QSet<uint> dids;
  bool start = true;

  // Check, which file the user wants to load.
  QDir dir( GeneralConfig::instance()->getUserDataDirectory() );
  QString url = GeneralConfig::instance()->getFlarmNetUrl();

  if( url.isEmpty() )
    {
      // No file shall be loaded
      qWarning() << "FlarmDB: No DB file defined for loading!";
      return dids.size();
    }

  QString fname = QFileInfo( url.mid( 6 ) ).fileName();
  QString fpath = dir.absolutePath() + "/flarmDB/" + fname;
  QFile file( fpath);

  if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open DB file %s for reading!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return dids.size();
    }

  filterList = filter.split( QRegExp("[\\s,]+"), QString::SkipEmptyParts);

  QTextStream in( &file );
  in.setCodec( "ISO 8859-15" );

  while( ! in.atEnd() )
    {
      QString line = in.readLine();

      if( start == true )
        {
          start = false;
          continue;
        }

      if( line.size() < 172 )
        {
          // ignore short line.
          qDebug() << "FlarmNet: line shorter than 172:" << line.size();
          continue;
        }

      QByteArray ba = QByteArray::fromHex( line.toLatin1() );

      for( int i=0; i < ba.size(); i++ )
        {
          if( ba[i] > (char) 0x7e || ba[i] < ' ' )
            {
              ba[i] = '?';
            }
        }

      bool ok;
      uint fid = ba.mid( 0, 6 ).toUInt( &ok, 16);

      if( ! ok )
        {
          continue;
        }

      /**
        bytearray content
                  1         2         3         4         5         6         7         8
        01234567890123456789012345678901234567890123456789012345678901234567890123456789012345
        3e7c78                     D-KMII               Ventus bT            D-KMII V  123.005
        ba, 6, record.flarm_id;
        ba + 6, 21, record.pilot;
        ba + 27, 21, record.airfield;
        ba + 48, 21, record.plane_type;
        ba + 69, 7, record.registration;
        ba + 76, 3, record.callsign;
        ba + 79, 7, record.frequency;
       */
      QByteArray fdata = ba.mid( 69, 7 ).trimmed().toUpper(); // KZ
      QString kz = QString( fdata ).toUpper();

      if( checkFilter( kz ) == true )
        {
          dids.insert( fid );
        }
    } // End of While

  file.close();

  // Evaluate OGN Database file
  fpath = dir.absolutePath() + "/flarmDB/ogn.csv";
  QFile file1( fpath);

  if( file1.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open OGN DB file %s for reading!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return dids.size();
    }

  QTextStream in1( &file1 );
  in1.setCodec( "ISO 8859-15" );

  while( ! in1.atEnd() )
    {
      QString line = in1.readLine().trimmed();

      if( line.startsWith( "#" ) == true )
        {
          continue;
        }

      // remove ' characters from the string
      line = line.remove( QChar( '\'' ) );

      QStringList qsl = line.split( ",", QString::KeepEmptyParts );

      if( qsl.size() < 8 )
        {
          // to less parameters
          continue;
        }

      if( qsl.at(0) == "O" )
        {
          // Read only Flarm or ICAO entries
          continue;
        }

      bool ok;
      // Device ID (Flarm, ICAO)
      uint did = qsl.at(1).toUInt( &ok, 16);

      if( ! ok )
        {
          continue;
        }

      if( dids.contains( did ) == true )
        {
          // did already contained in set.
          continue;
        }

      // Registration sign (Kennzeichen)
      QString kz = qsl.at(3).trimmed().toUpper();

      if( checkFilter( kz ) == true )
        {
          dids.insert( did );
        }
    }

  file1.close();
  qDebug( "FlarmDB: %d filtered items extracted in %lldms", dids.size(), t.elapsed() );

  return dids.size();
}

bool FlarmDB::getData( int id, QStringList &data )
{
  QMutexLocker locker( &m_mutex );

  if( m_datamap.contains( id ) )
    {
      // List contains KZ, Type, WKZ, Frequenz. Unknown elements are empty.
      data = m_datamap.value( id ).split( "|", QString::KeepEmptyParts );
      return true;
    }

  return false;
}

/*----------------------------- FlarmDBThread --------------------------------*/

#include <csignal>

FlarmDBThread::FlarmDBThread( QObject *parent ) :
  QThread( parent )
{
  setObjectName( "FlarmDBThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

FlarmDBThread::~FlarmDBThread()
{
}

void FlarmDBThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  // clear dictionary
  FlarmDB::unloadData();
  FlarmDB::loadFNData( FlarmDB::dictionary() );
  FlarmDB::loadOGNData( FlarmDB::dictionary() );

  emit loadedRecords( FlarmDB::dictionary().size() );
}
