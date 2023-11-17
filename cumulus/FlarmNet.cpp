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
**   Copyright (c):  2023 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <FlarmNet.h>
#include <QtCore>

#include "generalconfig.h"

QHash<uint, QString> FlarmNet::m_datamap;
QMutex FlarmNet::m_mutex;

void FlarmNet::unloadData()
{
  QMutexLocker locker( &m_mutex );
  m_datamap.clear();
}

int FlarmNet::loadData()
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  uint items = 0; // number of successfully loaded items

  m_datamap.clear();

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
  QString fpath = dir.absolutePath() + "/flarmNet/" + fname;
  QFile file( fpath);

  if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open DB file %s for writing!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return items;
    }

  QString fs = GeneralConfig::instance()->getFlarmNetFilter().toUpper();
  QStringList filterList = fs.split( QRegExp("[\\s,]+"), QString::SkipEmptyParts);

  QTextStream in( &file );
  in.setCodec( "ISO 8859-15" );

  while( ! in.atEnd() )
    {
      QString line = in.readLine();

      if( items == 0 )
        {
          // Magic id is expected
          qDebug() << "FlarmNet magic" << line;
          items++;
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

      if( filterList.size() > 0 )
        {
          // Check filter for taking
          bool takeit = false;

          for( int i=0; i < filterList.size(); i++ )
            {
              if( kz.startsWith( filterList.at(i) ) == true )
                {
                  takeit = true;
                  break;
                }
            }

          if( takeit == false )
            {
              continue;
            }
        }

      items++;
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 48, 21 ).trimmed() ); // Flugzeug Type
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 76, 3 ).trimmed() ); // WKZ
      fdata.append( "|" ); // Trenner
      fdata.append( ba.mid( 79, 7 ).trimmed() ); // Frequenz
      m_datamap.insert( fid, fdata );
      // qDebug() << fid << fdata;
    } // End of While

  qDebug( "FlarmNet: %d items loaded in %lldms", items-1, t.elapsed() );
  return items-1;
}

int FlarmNet::applyFilter( QString filter )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  uint items = 0; // number of count items

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
  QString fpath = dir.absolutePath() + "/flarmNet/" + fname;
  QFile file( fpath);

  if( file.open( QIODevice::ReadOnly | QIODevice::Text) == false )
    {
      qWarning( "FlarmNet: Can't open DB file %s for writing!"
                " Aborting ...",
                fpath.toLatin1().data() );

      return items;
    }

  QStringList filterList = filter.split( QRegExp("[\\s,]+"), Qt::SkipEmptyParts);

  QTextStream in( &file );
  in.setCodec( "ISO 8859-15" );

  while( ! in.atEnd() )
    {
      QString line = in.readLine();

      if( items == 0 )
        {
          // Magic id is expected
          qDebug() << "FlarmNet magic" << line;
          items++;
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

      if( filterList.size() > 0 )
        {
          // Check filter for taking
          bool takeit = false;

          for( int i=0; i < filterList.size(); i++ )
            {
              if( kz.startsWith( filterList.at(i) ) == true )
                {
                  takeit = true;
                  break;
                }
            }

          if( takeit == false )
            {
              continue;
            }
        }

      items++;
    } // End of While

  qDebug( "FlarmNet: %d items filtered and count in %lldms", items-1, t.elapsed() );
  return items-1;
}

bool FlarmNet::getData( int id, QStringList &data )
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

/*---------------------- FlarmNetThread --------------------------------*/

#include <csignal>

FlarmNetThread::FlarmNetThread( QObject *parent ) :
  QThread( parent )
{
  setObjectName( "FlarmNetThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

FlarmNetThread::~FlarmNetThread()
{
}

void FlarmNetThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  int records = FlarmNet::loadData();
  emit loadedRecords( records );
}
