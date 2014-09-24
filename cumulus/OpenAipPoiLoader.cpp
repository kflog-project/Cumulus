/***********************************************************************
**
**   OpenAipPoiLoader.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "airfield.h"
#include "filetools.h"
#include "generalconfig.h"
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

// Dats stream version to be used for compiled files.
#define Q_DATA_STREAM QDataStream::Qt_4_7

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

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*_wpt.aip" );

      if( readSource == false )
        {
          MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*_wpt.aic" );
        }
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No airfield files found in the map directories!" );
      return loadCounter;
    }

  // source files follows compiled files
  preselect.sort();

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No airfield files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).completeBaseName() + ".aip";

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( !preselect.isEmpty() )
    {
      QString aipName;
      QString aicName;
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      if( preselect.first().endsWith( QString( ".aip" ) ) )
        {
          // There can't be the same name .aic after this .aip. Parse aip file.
          aipName = preselect.first();

          // Remove source file to be read from the list.
          preselect.removeAt(0);

          listBegin = airfieldList.size();

          bool ok = openAip.readAirfields( aipName,
                                           airfieldList,
                                           errorInfo,
                                           true );
          if( ok )
            {
              QString aicName = aipName.replace( aipName.size() - 1, 1 , QChar('c') );
              createCompiledFile( aicName, airfieldList, listBegin );
              loadCounter++;
            }

          continue;
        }

      // At first we found a binary file with the extension aic.
      aicName = preselect.first();
      preselect.removeAt( 0 );
      aipName = aicName;
      aipName.replace( aipName.size() - 1, 1, QChar('p')  );

      if( ! preselect.isEmpty() && aipName == preselect.first() )
        {
          // We found the related source file and will do some checks to
          // decide which type of file will be read in.

          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_AIRFIELD_OAIP_C, FILE_VERSION_AIRFIELD_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
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
              continue;
            }

          if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
            {
              // Home position has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
              continue;
            }

          double filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

          if( m_hd.h_homeRadius != filterRadius )
            {
              // Home radius has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
              continue;
            }

          float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

          if( m_hd.h_runwayLengthFilter != filterRunwayLength )
            {
              // Runway length filter has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Runway length mismatch";
              continue;
            }

          // All checks passed, there is no need to read the source file and
          // we can remove it from the list.
          preselect.removeAt( 0 );
        }

      // We will read the compiled file, because a source file is not to
      // find after it or all checks were successfully passed.
      if( readCompiledFile( aicName, airfieldList ) )
        {
          loadCounter++;
        }
    } // End of While

  qDebug( "OAIP: %d airfield file(s) with %d items loaded in %dms",
          loadCounter, airfieldList.size(), t.elapsed() );

  return loadCounter;
}

int OpenAipPoiLoader::load( QList<RadioPoint>& navAidsList, bool readSource )
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
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*_nav.aip" );

      if( readSource == false )
        {
          MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*_nav.aic" );
        }
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No navAids files found in the map directories!" );
      return loadCounter;
    }

  // source files follows compiled files
  preselect.sort();

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipPoiFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "OAIP: No navAids files defined for loading by the user!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo(preselect.at(i)).completeBaseName() + ".aip";

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( !preselect.isEmpty() )
    {
      QString aipName;
      QString aicName;
      OpenAip openAip;
      QString errorInfo;
      int listBegin;

      if( preselect.first().endsWith( QString( ".aip" ) ) )
        {
          // There can't be the same name .aic after this .aip. Parse aip file.
          aipName = preselect.first();

          // Remove source file to be read from the list.
          preselect.removeAt(0);

          listBegin = navAidsList.size();

          bool ok = openAip.readNavAids( aipName, navAidsList, errorInfo, true );

          if( ok )
            {
              QString aicName = aipName.replace( aipName.size() - 1, 1 , QChar('c') );
              createCompiledFile( aicName, navAidsList, listBegin );
              loadCounter++;
            }

          continue;
        }

      // At first we found a binary file with the extension aic.
      aicName = preselect.first();
      preselect.removeAt( 0 );
      aipName = aicName;
      aipName.replace( aipName.size() - 1, 1, QChar('p')  );

      if( ! preselect.isEmpty() && aipName == preselect.first() )
        {
          // We found the related source file and will do some checks to
          // decide which type of file will be read in.

          // Lets check, if we can read the header of the compiled file
          if( ! getHeaderData( aicName, FILE_TYPE_NAV_AIDS_OAIP_C, FILE_VERSION_NAV_AIDS_C ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
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
              continue;
            }

          if( m_hd.h_homeCoord != _globalMapMatrix->getHomeCoord() )
            {
              // Home position has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
              continue;
            }

          double filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

          if( m_hd.h_homeRadius != filterRadius )
            {
              // Home radius has been changed. That requires a reparse
              // of the source files.
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
              continue;
            }

          // All checks passed, there is no need to read the source file and
          // we can remove it from the list.
          preselect.removeAt( 0 );
        }

      // We will read the compiled file, because a source file is not to
      // find after it or all checks were successfully passed.
      if( readCompiledFile( aicName, navAidsList ) )
        {
          loadCounter++;
        }
    } // End of While

  qDebug( "OAIP: %d navAids file(s) with %d items loaded in %dms",
          loadCounter, navAidsList.size(), t.elapsed() );

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
      qWarning( "OAIP: Can't open airfield file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  double filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

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

  // write number of airfield records to be stored
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
      out << qint16( af.getElevation());

      // frequency is written as e.g. 126.575, is reduced to 16 bits
      if( af.getFrequency() == 0.0 )
        {
          out << quint16(0);
        }
      else
        {
          out << quint16( rint((af.getFrequency() - 100.0) * 1000.0 ));
        }

      // The runway list is saved
      QList<Runway>& rwyList = af.getRunwayList();

      // Number of runways
      out << quint8( rwyList.size() );

      for( int i = 0; i < rwyList.size(); i++ )
       {
         Runway rwy = rwyList.at(i);

         out << rwy.m_length;
         out << rwy.m_width;
         out << quint16( rwy.m_heading );
         out << quint8( rwy.m_surface );
         out << quint8( rwy.m_isOpen );
         out << quint8( rwy.m_isBidirectional );
       }
    }

  file.close();
  return true;
}

bool OpenAipPoiLoader::createCompiledFile( QString& fileName,
                                           QList<RadioPoint>& navAidsList,
                                           int listBegin )
{
  if( navAidsList.size() == 0 || navAidsList.size() < listBegin )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( Q_DATA_STREAM );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open airfield file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  double filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  // NavAids uses not this filter but to have a unique header, it is considered.
  float filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();

  qDebug() << "OAIP: creating navAids file" << QFileInfo(fileName).fileName()
           << "with" << (navAidsList.size() - listBegin) << "elements";

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

  // write number of airfield records to be stored
  out << quint32( navAidsList.size() - listBegin );

  // Storing starts at the given index.
  for( int i = listBegin; i < navAidsList.size(); i++ )
    {
      RadioPoint& rp = navAidsList[i];

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
      out << qint16( rp.getElevation());

      // frequency is written as e.g. 126.575, is reduced to 16 bits
      if( rp.getFrequency() == 0.0 )
        {
          out << quint16(0);
        }
      else
        {
          out << quint16( rint((rp.getFrequency() - 100.0) * 1000.0 ));
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

  quint8 afType;
  QByteArray utf8_temp;
  WGSPoint wgsPos;
  QPoint position;
  qint16 elevation;
  quint16 inFrequency;

  uint counter = 0;

  while( ! in.atEnd() )
    {
      counter++;

      Airfield af;

      in >> afType; af.setTypeID( static_cast<BaseMapElement::objectType>(afType) );

      // read long name
      ShortLoad(in, utf8_temp);
      af.setName(QString::fromUtf8(utf8_temp));

      // read short name
      ShortLoad(in, utf8_temp);
      af.setWPName(QString::fromUtf8(utf8_temp));

      // read the 2 letter country code
      ShortLoad(in, utf8_temp);
      af.setCountry(QString::fromUtf8(utf8_temp));

      // read ICAO
      ShortLoad(in, utf8_temp);
      af.setICAO(QString::fromUtf8(utf8_temp));

      // read comment
      ShortLoad(in, utf8_temp);
      af.setComment(QString::fromUtf8(utf8_temp));

      in >> wgsPos; af.setWGSPosition(wgsPos);

      in >> position; af.setPosition(position);

      in >> elevation; af.setElevation(elevation);

      in >> inFrequency;

      if( inFrequency == 0 )
        {
          af.setFrequency( 0.0 );
        }
      else
        {
          af.setFrequency((((float) inFrequency) / 1000.0) + 100.);
        }

      // The runway list has to be read
      quint8 listSize; in >> listSize;

      for( short i = 0; i < (short) listSize; i++ )
        {
          float length;
          float width;
          quint16 heading;
          quint8 surface;
          quint8 isOpen;
          quint8 isBidirectional;

          in >> length;
          in >> width;
          in >> heading;
          in >> surface;
          in >> isOpen;
          in >> isBidirectional;

          Runway rwy( length, heading, surface, isOpen, isBidirectional, width );

          af.addRunway( rwy );
        }

      // Add the airfield site to the list.
      airfieldList.append( af );
    }

  inFile.close();

  qDebug( "OAIP: %d airfields read from %s in %dms",
          counter, fileName.toLatin1().data(), t.elapsed() );

  return true;
}

bool OpenAipPoiLoader::readCompiledFile( QString &fileName,
                                         QList<RadioPoint>& navAidsList )
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
      navAidsList.reserve( navAidsList.size() + lrn );
    }

  quint8 type;
  QByteArray utf8_temp;
  WGSPoint wgsPos;
  QPoint position;
  qint16 elevation;
  quint16 inFrequency;
  float range;
  float declination;
  quint8 isAligned2TrueNorth;

  uint counter = 0;

  while( ! in.atEnd() )
    {
      counter++;

      RadioPoint rp;

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

      in >> inFrequency;

      if( inFrequency == 0 )
        {
          rp.setFrequency( 0.0 );
        }
      else
        {
          rp.setFrequency((((float) inFrequency) / 1000.0) + 100.);
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
      navAidsList.append( rp );
    }

  inFile.close();

  qDebug( "OAIP: %d navAids read from %s in %dms",
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
