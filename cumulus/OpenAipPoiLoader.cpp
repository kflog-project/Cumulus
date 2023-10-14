/***********************************************************************
**
**   OpenAipPoiLoader.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-20223 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "airfield.h"
#include "filetools.h"
#include "generalconfig.h"
#include "Frequency.h"
#include "mapcontents.h"
#include "OpenAip.h"
#include "OpenAipPoiLoader.h"
#include "resource.h"

#ifdef BOUNDING_BOX
extern MapContents*  _globalMapContents;
#endif

extern MapMatrix* _globalMapMatrix;

// set static member variable
QMutex OpenAipPoiLoader::m_mutex;

// Data stream version to be used for compiled files.
#define Q_DATA_STREAM QDataStream::Qt_4_8

OpenAipPoiLoader::OpenAipPoiLoader()
{
}

OpenAipPoiLoader::~OpenAipPoiLoader()
{
}

int OpenAipPoiLoader::load( QList<Airfield>& airfieldList, bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  // Check, which files the user wants to load.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No airfield files defined for loading by the user!";
      return loadCounter;
    }

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      // search for source files
      MapContents::addDir( preselect, mapDirs.at( i ) + "/points", "*_apt.json" );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No airfield files found in the map directories!" );
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  // Load desired files
  while( !preselect.isEmpty() )
    {
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      // Check, if a compiled file exists. In this case we try to read it as first.
      QString aipName = preselect.first();
      QString aicName = preselect.first() + "c";

      if( QFile::exists( aicName ) == true && readSource == false )
        {
          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_AIRFIELD_OAIP_C, FILE_VERSION_AIRFIELD_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
              QFile::remove( aicName );
              continue;
            }

          // Do a date-time check. If the source file is younger in its
          // modification time as the compiled file, a new compilation
          // must be forced. Remove the compiled file.
          QFileInfo fi( aipName );
          QDateTime lastModTxt = fi.lastModified();

          if ( m_hd.h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Time mismatch";
              QFile::remove( aicName );
              continue;
            }

            // Check, if the projection has been changed in the meantime
            ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

            if( ! MapContents::compareProjections( m_hd.h_projection, currentProjection ) )
              {
                // Projection has changed in the meantime. That requires a reparse
                // of the source files.
                if( m_hd.h_projection )
                  {
                    delete m_hd.h_projection;
                    m_hd.h_projection = 0;
                  }

                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Projection mismatch";
                QFile::remove( aicName );
                continue;
              }

            if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
              {
                // Home position has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
                QFile::remove( aicName );
                continue;
              }

            float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

            if( m_hd.h_homeRadius != filterRadius )
              {
                // Home radius has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
                QFile::remove( aicName );
                continue;
              }

          // All checks passed, we will read the compiled file
          if( readCompiledFile( aicName, airfieldList ) )
            {
              loadCounter++;
              // Remove the source file from the list, if we had read the compiled file.
              preselect.removeAt( 0 );
            }
          else
            {
              // Compiled file could not be read, remove it and try to read
              // the source file
              QFile::remove( aicName );
            }
        }
      else
        {
          // Source has to be read, remove source file from the list.
          preselect.removeAt(0);
          listBegin = airfieldList.size();

          bool ok = openAip.readAirfields( aipName, airfieldList, errorInfo, true );

          if( ok )
            {
              createCompiledFile( aicName, airfieldList, listBegin );
              loadCounter++;
            }
         }
      } // End of While

  qDebug( "OAIP: %d airfield file(s) with %d items loaded in %dms",
          loadCounter, airfieldList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QList<RadioPoint>& navAidList, bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      // search for source files
      MapContents::addDir( preselect, mapDirs.at( i ) + "/points", "*_nav.json" );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No navAid files found in the map directories!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No navAid files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  // Load desired files
  while( !preselect.isEmpty() )
    {
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      // Check, if a compiled file exists. In this case we try to read it as first.
      QString aipName = preselect.first();
      QString aicName = preselect.first() + "c";

      if( QFile::exists( aicName ) == true && readSource == false )
        {
          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_NAV_AIDS_OAIP_C, FILE_VERSION_NAV_AIDS_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
              QFile::remove( aicName );
              continue;
            }

          // Do a date-time check. If the source file is younger in its
          // modification time as the compiled file, a new compilation
          // must be forced.
          QFileInfo fi(aipName);
          QDateTime lastModTxt = fi.lastModified();

          if ( m_hd.h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Time mismatch";
              QFile::remove( aicName );
              continue;
            }

          // Check, if the projection has been changed in the meantime
          ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

          if( ! MapContents::compareProjections( m_hd.h_projection, currentProjection ) )
            {
              // Projection has changed in the meantime. That requires a reparse
              // of the source files.
              if( m_hd.h_projection )
                {
                  delete m_hd.h_projection;
                  m_hd.h_projection = 0;
                }

              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Projection mismatch";
              QFile::remove( aicName );
              continue;
            }

          if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
            {
              // Home position has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
              QFile::remove( aicName );
              continue;
            }

          float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

          if( m_hd.h_homeRadius != filterRadius )
            {
              // Home radius has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
              QFile::remove( aicName );
              continue;
            }

          // All checks passed, we will read the compiled file
          if( readCompiledFile( aicName, navAidList ) )
            {
              // Remove the source file from the list, if we had read the compiled file.
              preselect.removeAt( 0 );
              loadCounter++;
            }
          else
            {
              // Compiled file could not be read, remove it and try to read
              // the source file
              QFile::remove( aicName );
            }
        }
      else
        {
         // Source has to be read, remove source file from the list.
          preselect.removeAt(0);
          listBegin = navAidList.size();

          bool ok = openAip.readNavAids( aipName, navAidList, errorInfo, true );

          if( ok )
            {
              createCompiledFile( aicName, navAidList, listBegin );
              loadCounter++;
            }
          }
    } // End of While

  qDebug( "OAIP: %d navAid file(s) with %d items loaded in %dms",
          loadCounter, navAidList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QString filter,
                            int type,
                            QList<SinglePoint>& spList,
                            bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      // search for source files
      MapContents::addDir( preselect, mapDirs.at( i ) + "/points", filter );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No single point files found in the map directories!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No single point files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  // Load desired files
  while( !preselect.isEmpty() )
    {
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      // Check, if a compiled file exists. In this case we try to read it as first.
      QString aipName = preselect.first();
      QString aicName = preselect.first() + "c";

      if( QFile::exists( aicName ) == true && readSource == false )
        {
          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_SINGLE_POINT_OAIP_C, FILE_VERSION_SINGLE_POINT_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
              QFile::remove( aicName );
              continue;
            }

          // Do a date-time check. If the source file is younger in its
          // modification time as the compiled file, a new compilation
          // must be forced. Remove the compiled file.
          QFileInfo fi( aipName );
          QDateTime lastModTxt = fi.lastModified();

          if ( m_hd.h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Time mismatch";
              QFile::remove( aicName );
              continue;
            }

            // Check, if the projection has been changed in the meantime
            ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

            if( ! MapContents::compareProjections( m_hd.h_projection, currentProjection ) )
              {
                // Projection has changed in the meantime. That requires a reparse
                // of the source files.
                if( m_hd.h_projection )
                  {
                    delete m_hd.h_projection;
                    m_hd.h_projection = 0;
                  }

                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Projection mismatch";
                QFile::remove( aicName );
                continue;
              }

            if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
              {
                // Home position has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
                QFile::remove( aicName );
                continue;
              }

            float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

            if( m_hd.h_homeRadius != filterRadius )
              {
                // Home radius has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
                QFile::remove( aicName );
                continue;
              }

          // All checks passed, we will read the compiled file
          if( readCompiledFile( aicName, spList ) )
            {
              loadCounter++;
              // Remove the source file from the list, if we had read the compiled file.
              preselect.removeAt( 0 );
            }
          else
            {
              // Compiled file could not be read, remove it and try to read
              // the source file
              QFile::remove( aicName );
            }
        }
      else
        {
          // Source has to be read, remove source file from the list.
          preselect.removeAt(0);
          listBegin = spList.size();

          bool ok = openAip.readSinglePoints( aipName,
                                              type,
                                              spList,
                                              errorInfo,
                                              true );

          if( ok )
            {
              createCompiledFile( aicName, spList, listBegin );
              loadCounter++;
            }
         }
      } // End of While

  qDebug( "OAIP: %d single point file(s) with %d items loaded in %dms",
          loadCounter, spList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QList<ThermalPoint>& tpList, bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );

  QTime t;
  t.start();
  int loadCounter = 0; // number of successfully loaded files

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      // search for source files
      MapContents::addDir( preselect, mapDirs.at( i ) + "/points", "*_hot.json" );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No single point files found in the map directories!" );
      return loadCounter;
    }

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No single point files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  // Load desired files
  while( !preselect.isEmpty() )
    {
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      // Check, if a compiled file exists. In this case we try to read it as first.
      QString aipName = preselect.first();
      QString aicName = preselect.first() + "c";

      if( QFile::exists( aicName ) == true && readSource == false )
        {
          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_HOTSPOT_OAIP_C, FILE_VERSION_HOTSPOT_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
              QFile::remove( aicName );
              continue;
            }

          // Do a date-time check. If the source file is younger in its
          // modification time as the compiled file, a new compilation
          // must be forced. Remove the compiled file.
          QFileInfo fi( aipName );
          QDateTime lastModTxt = fi.lastModified();

          if ( m_hd.h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Time mismatch";
              QFile::remove( aicName );
              continue;
            }

            // Check, if the projection has been changed in the meantime
            ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

            if( ! MapContents::compareProjections( m_hd.h_projection, currentProjection ) )
              {
                // Projection has changed in the meantime. That requires a reparse
                // of the source files.
                if( m_hd.h_projection )
                  {
                    delete m_hd.h_projection;
                    m_hd.h_projection = 0;
                  }

                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Projection mismatch";
                QFile::remove( aicName );
                continue;
              }

            if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
              {
                // Home position has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
                QFile::remove( aicName );
                continue;
              }

            float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

            if( m_hd.h_homeRadius != filterRadius )
              {
                // Home radius has been changed. That requires a reparse
                // of the source files.
                qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
                QFile::remove( aicName );
                continue;
              }

          // All checks passed, we will read the compiled file
          if( readCompiledFile( aicName, tpList ) )
            {
              loadCounter++;
              // Remove the source file from the list, if we had read the compiled file.
              preselect.removeAt( 0 );
            }
          else
            {
              // Compiled file could not be read, remove it and try to read
              // the source file
              QFile::remove( aicName );
            }
        }
      else
        {
          // Source has to be read, remove source file from the list.
          preselect.removeAt(0);
          listBegin = tpList.size();

          bool ok = openAip.readHotspots( aipName, tpList, errorInfo, true );

          if( ok )
            {
              createCompiledFile( aicName, tpList, listBegin );
              loadCounter++;
            }
         }
      } // End of While

  qDebug( "OAIP: %d thermal point file(s) with %d items loaded in %dms",
          loadCounter, tpList.size(), t.elapsed() );

  return loadCounter;
}

bool OpenAipPoiLoader::createCompiledFile( QString& fileName,
                                           QList<Airfield>& airfieldList,
                                           int listBegin )
{
  if( airfieldList.size() == 0 || airfieldList.size() < listBegin )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( Q_DATA_STREAM );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "OAIP: creating airfield file" << QFileInfo(fileName).fileName()
           << "with" << (airfieldList.size() - listBegin) << "elements";

  out << quint32( KFLOG_FILE_MAGIC );
  out << QByteArray( FILE_TYPE_AIRFIELD_OAIP_C );
  out << quint8( FILE_VERSION_AIRFIELD_C );
  out << QDateTime::currentDateTime();
  out << filterRadius;
  out << filterRunwayLength;
  out << QPoint( _globalMapMatrix->getHomeCoord() );

#ifdef BOUNDING_BOX
  // boundingbox is never used during read in, we don't need to write out it
  // qDebug("Bounding box is: (%d, %d),(%d, %d)",
  // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
  out << boundingBox;
#endif

  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of records to be stored
  out << quint32( airfieldList.size() - listBegin );

  // Storing starts at the given index.
  for( int i = listBegin; i < airfieldList.size(); i++ )
    {
      Airfield& af = airfieldList[i];

      // element type
      out << quint8( af.getTypeID() );
      // element name
      ShortSave(out, af.getName().toUtf8());
      // short waypoint name
      ShortSave(out, af.getWPName().toUtf8());
      // country
      ShortSave(out, af.getCountry().toUtf8());
      // icao
      ShortSave(out, af.getICAO().toUtf8());
      // comment
      ShortSave(out, af.getComment().toUtf8());
      // WGS84 coordinates
      out << af.getWGSPosition();
      // projected WGS84 coordinates
      out << af.getPosition();
      // elevation in meters
      out << af.getElevation();
      // has winch
      out << af.hasWinch();
      // has towing
      out << af.hasTowing();
      // is PPR
      out << af.isPPR();
      // is private
      out << af.isPrivate();
      // has sky diving
      out << af.hasSkyDiving();
      // is landable
      out << af.isLandable();

      // The frequency list is saved.
      Frequency::saveFrequencies( out, af.getFrequencyList() );

      // The runway list is saved
      Runway::saveRunways( out, af.getRunwayList() );
    }

  file.close();
  return true;
}

bool OpenAipPoiLoader::createCompiledFile( QString& fileName,
                                           QList<RadioPoint>& navAidList,
                                           int listBegin )
{
  if( navAidList.size() == 0 || navAidList.size() < listBegin )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( Q_DATA_STREAM );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  // Navaid uses not this filter but to have a unique header, it is considered.
  float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "OAIP: creating navAid file" << QFileInfo(fileName).fileName()
           << "with" << (navAidList.size() - listBegin) << "elements";

  out << quint32( KFLOG_FILE_MAGIC );
  out << QByteArray( FILE_TYPE_NAV_AIDS_OAIP_C );
  out << quint8( FILE_VERSION_NAV_AIDS_C );
  out << QDateTime::currentDateTime();
  out << filterRadius;
  out << filterRunwayLength;
  out << QPoint( _globalMapMatrix->getHomeCoord() );

#ifdef BOUNDING_BOX
  // boundingbox is never used during read in, we don't need to write out it
  // qDebug("Bounding box is: (%d, %d),(%d, %d)",
  // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
  out << boundingBox;
#endif

  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of records to be stored
  out << quint32( navAidList.size() - listBegin );

  // Storing starts at the given index.
  for( int i = listBegin; i < navAidList.size(); i++ )
    {
      RadioPoint& rp = navAidList[i];

      // element type
      out << quint8( rp.getTypeID() );
      // element name
      ShortSave(out, rp.getName().toUtf8());
      // short waypoint name
      ShortSave(out, rp.getWPName().toUtf8());
      // country
      ShortSave(out, rp.getCountry().toUtf8());
      // icao
      ShortSave(out, rp.getICAO().toUtf8());
      // comment
      ShortSave(out, rp.getComment().toUtf8());
      // WGS84 coordinates
      out << rp.getWGSPosition();
      // projected WGS84 coordinates
      out << rp.getPosition();
      // elevation in meters
      out << rp.getElevation();

      // The frequency list is saved.
      QList<Frequency>& fList = rp.getFrequencyList();

      // Number of Frequencies
      out << quint8( fList.size() );

      for( int i = 0; i < fList.size (); i++ )
        {
          Frequency freq = fList.at (i);

          // frequency value
          out << freq.getValue();
          // frequency unit
          out << freq.getUnit();
          // frequency type
          out << freq.getType();
        }

      // Channel info
      ShortSave(out, rp.getChannel().toUtf8());
      // Service range as float
      out << rp.getRange();
      // Declination
      out << rp.getDeclination();
      // Aligned2TrueNorth
      out << quint8( rp.isAligned2TrueNorth() );
    }

  file.close();
  return true;
}


bool OpenAipPoiLoader::createCompiledFile( QString& fileName,
                                           QList<ThermalPoint>& thermalList,
                                           int listBegin )
{
  if( thermalList.size() == 0 || thermalList.size() < listBegin )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( Q_DATA_STREAM );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  // Hotspot uses not this filter but to have a unique header, it is considered.
  float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "OAIP: creating hotspot point file" << QFileInfo(fileName).fileName()
           << "with" << (thermalList.size() - listBegin) << "elements";

  out << quint32( KFLOG_FILE_MAGIC );
  out << QByteArray( FILE_TYPE_HOTSPOT_OAIP_C );
  out << quint8( FILE_VERSION_HOTSPOT_C );
  out << QDateTime::currentDateTime();
  out << filterRadius;
  out << filterRunwayLength;
  out << QPoint( _globalMapMatrix->getHomeCoord() );

#ifdef BOUNDING_BOX
  // boundingbox is never used during read in, we don't need to write out it
  // qDebug("Bounding box is: (%d, %d),(%d, %d)",
  // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
  out << boundingBox;
#endif

  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of records to be stored
  out << quint32( thermalList.size() - listBegin );

  // Storing starts at the given index.
  for( int i = listBegin; i < thermalList.size(); i++ )
    {
      ThermalPoint& tp = thermalList[i];

      // element type
      out << quint8( tp.getTypeID() );
      // element name
      ShortSave(out, tp.getName().toUtf8());
      // element short name
      ShortSave(out, tp.getWPName().toUtf8());
      // country
      ShortSave(out, tp.getCountry().toUtf8());
      // comment
      ShortSave(out, tp.getComment().toUtf8());
      // WGS84 coordinates
      out << tp.getWGSPosition();
      // projected WGS84 coordinates
      out << tp.getPosition();
      // elevation in meters
      out << tp.getElevation();
      // type
      out << quint8( tp.getType() );
      // reliability
      out << quint8( tp.getReliability() );
      // occurrence
      out << quint8( tp.getOccurrence() );
      // m_category
      out << quint8( tp.getCategory() );
    }

  file.close();
  return true;
}

bool OpenAipPoiLoader::createCompiledFile( QString& fileName,
                                           QList<SinglePoint>& spList,
                                           int listBegin )
{
  if( spList.size() == 0 || spList.size() < listBegin )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( Q_DATA_STREAM );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  float filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  // Hotspot uses not this filter but to have a unique header, it is considered.
  float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "OAIP: creating single point file" << QFileInfo(fileName).fileName()
           << "with" << (spList.size() - listBegin) << "elements";

  out << quint32( KFLOG_FILE_MAGIC );
  out << QByteArray( FILE_TYPE_SINGLE_POINT_OAIP_C );
  out << quint8( FILE_VERSION_SINGLE_POINT_C );
  out << QDateTime::currentDateTime();
  out << filterRadius;
  out << filterRunwayLength;
  out << QPoint( _globalMapMatrix->getHomeCoord() );

#ifdef BOUNDING_BOX
  // boundingbox is never used during read in, we don't need to write out it
  // qDebug("Bounding box is: (%d, %d),(%d, %d)",
  // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
  out << boundingBox;
#endif

  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of records to be stored
  out << quint32( spList.size() - listBegin );

  // Storing starts at the given index.
  for( int i = listBegin; i < spList.size(); i++ )
    {
      SinglePoint& sp = spList[i];

      // element type
      out << quint8( sp.getTypeID() );
      // element name
      ShortSave(out, sp.getName().toUtf8());
      // element short name
      ShortSave(out, sp.getWPName().toUtf8());
      // country
      ShortSave(out, sp.getCountry().toUtf8());
      // comment
      ShortSave(out, sp.getComment().toUtf8());
      // WGS84 coordinates
      out << sp.getWGSPosition();
      // projected WGS84 coordinates
      out << sp.getPosition();
      // elevation in meters
      out << sp.getElevation();
    }

  file.close();
  return true;
}

bool OpenAipPoiLoader::readCompiledFile( QString &fileName,
                                         QList<Airfield>& airfieldList )
{
  QTime t;
  t.start();

  QFile inFile( fileName );

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OAIP: Cannot open airfield file %s!", fileName.toLatin1().data());
      return false;
    }

  QDataStream in( &inFile );
  in.setVersion( Q_DATA_STREAM );

  bool ok = readHeaderData( in, FILE_TYPE_AIRFIELD_OAIP_C, FILE_VERSION_AIRFIELD_C );

  if( ok == false )
    {
      inFile.close();
      return false;
    }

  // read number of records in the file
  quint32 lrn; in >> lrn;

  // Preallocate list memory for new elements to be added.
  if( lrn )
    {
      airfieldList.reserve( airfieldList.size() + lrn );
    }

  uint counter = 0;

  while( ! in.atEnd() )
    {
      quint8 afType;
      QString name;
      QString wpName;
      QString country;
      QString icao;
      QString comment;
      WGSPoint wgsPos;
      QPoint position;
      float elevation;
      bool hasWinch;
      bool hasTowing;
      bool isPPR;
      bool isPrivate;
      bool hasSkyDiving;
      bool isLandable;
      Airfield af;

      counter++;

      // read type
      in >> afType;
      af.setTypeID( static_cast<BaseMapElement::objectType>(afType) );

      // read long name
      ShortLoad( in, name ); af.setName( name );

      // read short name
      ShortLoad(in, wpName ); af.setWPName( wpName );

      // read the 2 letter country code
      ShortLoad( in, country ); af.setCountry( country );

      // read ICAO
      ShortLoad( in, icao ); af.setICAO( icao );

      // read comment
      ShortLoad( in, comment ); af.setComment( comment );

      in >> wgsPos; af.setWGSPosition(wgsPos);
      in >> position; af.setPosition(position);
      in >> elevation; af.setElevation(elevation);
      in >> hasWinch; af.setWinch( hasWinch );
      in >> hasTowing; af.setTowing( hasTowing );
      in >> isPPR; af.setPPR( isPPR );
      in >> isPrivate; af.setPrivate( isPrivate );
      in >> hasSkyDiving; af.setSkyDiving( hasSkyDiving );
      in >> isLandable; af.setLandable( isLandable );

      // The frequency list is loaded.
      Frequency::loadFrequencies( in, af.getFrequencyList() );

      // The runway list is loaded
      Runway::loadRunways( in, af.getRunwayList() );

      // Add the airfield site to the list.
      airfieldList.append( af );
    }

  inFile.close();

  qDebug( "OAIP: %d airfields read from %s in %dms",
          counter, fileName.toLatin1().data(), t.elapsed() );

  return true;
}

bool OpenAipPoiLoader::readCompiledFile( QString &fileName,
                                         QList<ThermalPoint>& hotspotList )
{
  QTime t;
  t.start();

  QFile inFile( fileName );

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OAIP: Cannot open hotspot file %s!", fileName.toLatin1().data());
      return false;
    }

  QDataStream in( &inFile );
  in.setVersion( Q_DATA_STREAM );

  bool ok = readHeaderData( in, FILE_TYPE_HOTSPOT_OAIP_C, FILE_VERSION_HOTSPOT_C );

  if( ok == false )
    {
      inFile.close();
      return false;
    }

  // read number of records in the file
  quint32 lrn; in >> lrn;

  // Preallocate list memory for new elements to be added.
  if( lrn )
    {
      hotspotList.reserve( hotspotList.size() + lrn );
    }

  uint counter = 0;

  while( ! in.atEnd() )
    {
      quint8 type;
      QByteArray utf8_temp;
      WGSPoint wgsPos;
      QPoint position;
      float elevation;
      quint8 enumeration;
      ThermalPoint tp;

      counter++;

      in >> type; tp.setTypeID( static_cast<BaseMapElement::objectType>(type) );

      // read long name
      ShortLoad(in, utf8_temp);
      tp.setName(QString::fromUtf8(utf8_temp));

      // read short name
      ShortLoad(in, utf8_temp);
      tp.setWPName(QString::fromUtf8(utf8_temp));

      // read the 2 letter country code
      ShortLoad(in, utf8_temp);
      tp.setCountry(QString::fromUtf8(utf8_temp));

      // read comment
      ShortLoad(in, utf8_temp);
      tp.setComment(QString::fromUtf8(utf8_temp));

      in >> wgsPos; tp.setWGSPosition(wgsPos);
      in >> position; tp.setPosition(position);
      in >> elevation; tp.setElevation(elevation);

      in >> enumeration; tp.setType(enumeration);
      in >> enumeration; tp.setReliability(enumeration);
      in >> enumeration; tp.setOccurrence(enumeration);
      in >> enumeration; tp.setCategory(enumeration);

      // Add the thermal point element to the list.
      hotspotList.append( tp );
    }

  inFile.close();

  qDebug( "OAIP: %d hotspots read from %s in %dms",
          counter, fileName.toLatin1().data(), t.elapsed() );

  return true;
}

bool OpenAipPoiLoader::readCompiledFile( QString &fileName,
                                         QList<RadioPoint>& navAidList )
{
  QTime t;
  t.start();

  QFile inFile( fileName );

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OAIP: Cannot open navAid file %s!", fileName.toLatin1().data());
      return false;
    }

  QDataStream in( &inFile );
  in.setVersion( Q_DATA_STREAM );

  bool ok = readHeaderData( in, FILE_TYPE_NAV_AIDS_OAIP_C, FILE_VERSION_NAV_AIDS_C );

  if( ok == false )
    {
      inFile.close();
      return false;
    }

  // read number of records in the file
  quint32 lrn; in >> lrn;

  // Preallocate list memory for new elements to be added.
  if( lrn )
    {
      navAidList.reserve( navAidList.size() + lrn );
    }

  uint counter = 0;

  while( ! in.atEnd() )
    {
      quint8 type;
      QByteArray utf8_temp;
      WGSPoint wgsPos;
      QPoint position;
      float elevation;
      float range;
      float declination;
      quint8 isAligned2TrueNorth;
      quint8 listSize;
      RadioPoint rp;

      counter++;

      in >> type; rp.setTypeID( static_cast<BaseMapElement::objectType>(type) );

      // read long name
      ShortLoad(in, utf8_temp);
      rp.setName(QString::fromUtf8(utf8_temp));

      // read short name
      ShortLoad(in, utf8_temp);
      rp.setWPName(QString::fromUtf8(utf8_temp));

      // read the 2 letter country code
      ShortLoad(in, utf8_temp);
      rp.setCountry(QString::fromUtf8(utf8_temp));

      // read ICAO
      ShortLoad(in, utf8_temp);
      rp.setICAO(QString::fromUtf8(utf8_temp));

      // read comment
      ShortLoad(in, utf8_temp);
      rp.setComment(QString::fromUtf8(utf8_temp));

      in >> wgsPos; rp.setWGSPosition(wgsPos);
      in >> position; rp.setPosition(position);
      in >> elevation; rp.setElevation(elevation);

      // The frequency list has to be read
      in >> listSize;

      for( short i = 0; i < (short) listSize; i++ )
        {
          Frequency fqy;
          float value;
          quint8 unit;
          quint8 type;

          // Frequency value, unit and type
          in >> value;
          in >> unit;
          in >> type;

          fqy.setValue( value );
          fqy.setUnit( unit );
          fqy.setType( type );
          rp.addFrequency( fqy );
        }

      // Channel info
      ShortLoad(in, utf8_temp);
      rp.setChannel(QString::fromUtf8(utf8_temp));

      // Service range as float
      in >> range; rp.setRange(range);

      // Declination
      in >> declination; rp.setDeclination(declination);

      // Aligned2TrueNorth
      in >> isAligned2TrueNorth; rp.setAligned2TrueNorth(isAligned2TrueNorth);

      // Add the radio point element to the list.
      navAidList.append( rp );
    }

  inFile.close();

  qDebug( "OAIP: %d navAids read from %s in %dms",
          counter, fileName.toLatin1().data(), t.elapsed() );

  return true;
}

bool OpenAipPoiLoader::readCompiledFile( QString &fileName,
                                         QList<SinglePoint>& spList )
{
  QTime t;
  t.start();

  QFile inFile( fileName );

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OAIP: Cannot open single point file %s!", fileName.toLatin1().data());
      return false;
    }

  QDataStream in( &inFile );
  in.setVersion( Q_DATA_STREAM );

  bool ok = readHeaderData( in, FILE_TYPE_SINGLE_POINT_OAIP_C,
                            FILE_VERSION_SINGLE_POINT_C );

  if( ok == false )
    {
      inFile.close();
      return false;
    }

  // read number of records in the file
  quint32 lrn; in >> lrn;

  // Preallocate list memory for new elements to be added.
  if( lrn )
    {
      spList.reserve( spList.size() + lrn );
    }

  uint counter = 0;

  while( ! in.atEnd() )
    {
      quint8 type;
      QByteArray utf8_temp;
      WGSPoint wgsPos;
      QPoint position;
      float elevation;
      SinglePoint sp;

      counter++;

      in >> type; sp.setTypeID( static_cast<BaseMapElement::objectType>(type) );

      // read long name
      ShortLoad(in, utf8_temp);
      sp.setName(QString::fromUtf8(utf8_temp));

      // read short name
      ShortLoad(in, utf8_temp);
      sp.setWPName(QString::fromUtf8(utf8_temp));

      // read the 2 letter country code
      ShortLoad(in, utf8_temp);
      sp.setCountry(QString::fromUtf8(utf8_temp));

      // read comment
      ShortLoad(in, utf8_temp);
      sp.setComment(QString::fromUtf8(utf8_temp));

      in >> wgsPos;    sp.setWGSPosition(wgsPos);
      in >> position;  sp.setPosition(position);
      in >> elevation; sp.setElevation(elevation);

      // Add the single point element to the list.
      spList.append( sp );
    }

  inFile.close();

  qDebug( "OAIP: %d single points read from %s in %dms",
          counter, fileName.toLatin1().data(), t.elapsed() );

  return true;
}

bool OpenAipPoiLoader::getHeaderData( QString &path,
                                      QString fileType,
                                      int fileVersion  )
{
  QFile inFile(path);

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OpenAip: Cannot open file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( Q_DATA_STREAM );

  bool ok = readHeaderData( in, fileType, fileVersion );

  inFile.close();
  return ok;
}

bool OpenAipPoiLoader::readHeaderData( QDataStream& dataStream,
                                       QString fileType,
                                       int fileVersion  )
{
  m_hd.h_headerIsValid      = false;
  m_hd.h_magic              = 0;
  m_hd.h_fileType.clear();
  m_hd.h_fileVersion        = 0;
  m_hd.h_homeRadius         = 0.0;
  m_hd.h_runwayLengthFilter = 0.0;
  m_hd.h_homeCoord.setX(0);
  m_hd.h_homeCoord.setY(0);

  if( m_hd.h_projection )
    {
      // delete an older projection object
      delete  m_hd.h_projection;
      m_hd.h_projection = 0;
    }

  dataStream >> m_hd.h_magic;

  if( m_hd.h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "OAIP: wrong magic key %x read! Aborting ...", m_hd.h_magic );
      return false;
    }

  dataStream >> m_hd.h_fileType;

  if( m_hd.h_fileType != fileType )
    {
      qWarning( "OAIP: wrong file type %s read! Aborting ...", m_hd.h_fileType.data() );
      return false;
    }

  dataStream >> m_hd.h_fileVersion;

  if( m_hd.h_fileVersion != fileVersion )
    {
      qWarning( "OAIP: wrong file version %hhx read! Aborting ...", m_hd.h_fileVersion );
      return false;
    }

  dataStream >> m_hd.h_creationDateTime;
  dataStream >> m_hd.h_homeRadius;
  dataStream >> m_hd.h_runwayLengthFilter;
  dataStream >> m_hd.h_homeCoord;

#ifdef BOUNDING_BOX
  dataStream >> m_hd.h_boundingBox;
#endif

  m_hd.h_projection = LoadProjection(dataStream);
  m_hd.h_headerIsValid = true;
  return true;
}
