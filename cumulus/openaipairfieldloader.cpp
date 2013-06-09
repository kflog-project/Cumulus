/***********************************************************************
**
**   openaipairfieldloader.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "airfield.h"
#include "filetools.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "openaip.h"
#include "openaipairfieldloader.h"

// Type definition for openAIP compiled airfield files. OAIP__AF
#define FILE_TYPE_AIRFIELD_C 0x4f4149505f5f4146

// Version used for compiled airfield files created from openAIP airfield data
#define FILE_VERSION_AIRFIELD_C 0

#ifdef BOUNDING_BOX
extern MapContents*  _globalMapContents;
#endif

extern MapMatrix* _globalMapMatrix;

// set static member variable
QMutex OpenAipAirfieldLoader::m_mutex;

OpenAipAirfieldLoader::OpenAipAirfieldLoader() :
  h_magic(0),
  h_fileType(0),
  h_fileVersion(0),
  h_homeRadius(0),
  h_projection(static_cast<ProjectionBase *> (0)),
  h_headerIsValid(false)
{
}

OpenAipAirfieldLoader::~OpenAipAirfieldLoader()
{
  if( h_projection )
    {
      delete h_projection;
    }
}

int OpenAipAirfieldLoader::load( QList<Airfield>& airfieldList, bool readSource )
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
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*.aip" );
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*.AIP" );

      if( readSource == false )
        {
          MapContents::addDir( preselect, mapDirs.at( i ) + "/airfields", "*.aic" );
        }
    }

  if( preselect.count() == 0 )
    {
      qWarning( "OAIP: No airfield files found in the map directories!" );
      return loadCounter;
    }

  // First check, if we have found a file name in upper letters. That may
  // be true, if a file was downloaded from the Internet. We will convert
  // such a file name to lower cases and replace it in the file list.
  for( int i = 0; i < preselect.size(); i++ )
    {
      if( preselect.at( i ).endsWith( ".AIP" ) )
        {
          QFileInfo fInfo = preselect.at( i );
          QString path = fInfo.absolutePath();
          QString fn = fInfo.fileName().toLower();
          QString newFn = path + "/" + fn;
          QFile::rename( preselect.at( i ), newFn );
          preselect[i] = newFn;
        }
    }

  // source files follows compiled files
  preselect.sort();

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getOpenAipAirfieldFileList();

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
      int afListStart;

      if( preselect.first().endsWith( QString( ".aip" ) ) )
        {
          // There can't be the same name aic after this aip.
          // Parse found aip file
          aipName = preselect.first();

          afListStart = airfieldList.size();

          bool ok = openAip.readAirfields( aipName,
                                           airfieldList,
                                           errorInfo,
                                           true );
          if( ok )
            {
              QString aicName = aipName.replace( aipName.size() - 1, 1 , QChar('c') );
              createCompiledFile( aicName, airfieldList, afListStart );
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

      // At first we found a binary file with the extension aic.
      aicName = preselect.first();
      preselect.removeAt( 0 );
      aipName = aicName;
      aipName.replace( aipName.size() - 1, 1, QChar('p')  );
      bool readSource = false;

      if ( ! preselect.isEmpty() && aipName == preselect.first() )
        {
          preselect.removeAt( 0 );
          // We found the related source file and will do some checks to
          // decide which type of file will be read in.

          // Lets check, if we can read the header of the compiled file
          if ( ! setHeaderData( aicName ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and require a reparsing of source files.
              readSource = true;
            }

          // Do a date-time check. If the source file is younger in its
          // modification time as the compiled file, a new compilation
          // must be forced.
          QFileInfo fi(aipName);
          QDateTime lastModTxt = fi.lastModified();

          if ( h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              readSource = true;
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Time mismatch";
            }

          // Check, if the projection has been changed in the meantime
          ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

          if( ! MapContents::compareProjections( h_projection, currentProjection ) )
            {
              // Projection has changed in the meantime. That requires a reparse
              // of the source files.
              if( h_projection )
                {
                  delete h_projection;
                  h_projection = 0;
                }

              readSource = true;
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Projection mismatch";
            }

          if( h_homeCoord != _globalMapMatrix->getHomeCoord() )
            {
              // Home position has been changed. That requires a reparse
              // of the source files.
              readSource = true;
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Home mismatch";
            }

          int iRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

          // We must look, which unit the user has chosen. This unit must
          // be considered during load of data items.
          double filterRadius = Distance::convertToMeters( iRadius );

          if( h_homeRadius != filterRadius )
            {
              // Home radius has been changed. That requires a reparse
              // of the source files.
              readSource = true;
              qDebug() << "OAIP:" << QFileInfo(aicName).fileName() << "Radius mismatch";
            }

          if( readSource == true )
            {
              QFile::remove( aicName );
              afListStart = airfieldList.size();

              bool ok = openAip.readAirfields( aipName,
                                               airfieldList,
                                               errorInfo,
                                               true );
              if( ok )
                {
                  createCompiledFile( aicName, airfieldList, afListStart );
                  loadCounter++;
                }

              continue;
            }
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

bool OpenAipAirfieldLoader::createCompiledFile( QString& fileName,
                                                QList<Airfield>& airfieldList,
                                                int airfieldListStart )
{
  if( airfieldList.size() == 0 || airfieldList.size() < airfieldListStart )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( QDataStream::Qt_4_7 );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "OAIP: Can't open airfield file %s for writing!"
               " Aborting ...",
               fileName.toLatin1().data() );

      return false;
    }

  int iRadius = GeneralConfig::instance()->getAirfieldHomeRadius();

  // We must look, which unit the user has chosen. This unit must
  // be considered during load of data items.
  double filterRadius = Distance::convertToMeters( iRadius );

  qDebug() << "OAIP: creating file" << QFileInfo(fileName).fileName();

  out << quint32( KFLOG_FILE_MAGIC );
  out << quint64( FILE_TYPE_AIRFIELD_C );
  out << quint8( FILE_VERSION_AIRFIELD_C );
  out << QDateTime::currentDateTime();
  out << filterRadius;
  out << QPoint( _globalMapMatrix->getHomeCoord() );

#ifdef BOUNDING_BOX
  // boundingbox is never used during read in, we don't need to write out it
  // qDebug("Bounding box is: (%d, %d),(%d, %d)",
  // boundingBox.left(), boundingBox.top(), boundingBox.right(), boundingBox.bottom());
  out << boundingBox;
#endif

  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of airfield records to be stored
  out << quint32( airfieldList.size() - airfieldListStart );

  // Storing starts at the given index.
  for( int i = airfieldListStart; i < airfieldList.size(); i++ )
    {
      Airfield& af = airfieldList[i];

      // airfield type
      out << quint8( af.getTypeID() );
      // airfield name
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

         out << quint16( rwy.length );
         out << quint16( rwy.width );
         out << quint16( rwy.heading );
         out << quint8( rwy.surface );
         out << quint8( rwy.isOpen );
         out << quint8( rwy.isBidirectional );
       }
    }

  file.close();
  return true;
}

bool OpenAipAirfieldLoader::readCompiledFile( QString &fileName,
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
  in.setVersion( QDataStream::Qt_4_7 );

  quint32 magic;
  quint64 fileType;
  quint8 fileVersion;
  QDateTime creationDateTime;
  QStringList countryList;
  double homeRadius;
  QPoint homeCoord;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  ProjectionBase *projectionFromFile;

  // airfield record number
  quint32 afrn;

  quint8 afType;
  QByteArray utf8_temp;
  WGSPoint wgsPos;
  QPoint position;
  qint16 elevation;
  quint16 inFrequency;

  in >> magic;

  if( magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "OAIP: wrong magic key %x read! Aborting ...", magic );
      inFile.close();
      return false;
    }

  in >> fileType;

  if( fileType != FILE_TYPE_AIRFIELD_C )
    {
      qWarning( "OAIP: wrong file type %llx read! Aborting ...", fileType );
      inFile.close();
      return false;
    }

  in >> fileVersion;

  if( fileVersion != FILE_VERSION_AIRFIELD_C )
    {
      qWarning( "OAIP: wrong file version %hhx read! Aborting ...", fileVersion );
      inFile.close();
      return false;
    }

  in >> creationDateTime;
  in >> homeRadius;
  in >> homeCoord;

#ifdef BOUNDING_BOX
  in >> boundingBox;
#endif

  projectionFromFile = LoadProjection(in);

  // projectionFromFile is allocated dynamically, we don't need it
  // here. Therefore it is immediately deleted to avoid memory leaks.
  delete projectionFromFile;
  projectionFromFile = 0;

  // read number of records in the file
  in >> afrn;

  // Preallocate list memory for new elements to be added.
  if( afrn )
    {
      airfieldList.reserve( airfieldList.size() + afrn );
    }

  uint counter = 0;

  while( ! in.atEnd() )
    {
      counter++;

      Airfield af;

      in >> afType;

      af.setTypeID( static_cast<BaseMapElement::objectType>(afType) );

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
          quint16 length;
          quint16 width;
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

/**
 * Reads the header data of a compiled file and put it in our class
 * variables.
 */
bool OpenAipAirfieldLoader::setHeaderData( QString &path )
{
  h_headerIsValid = false;
  h_magic         = 0;
  h_fileType      = 0;
  h_fileVersion   = 0;
  h_homeRadius    = 0.0;
  h_homeCoord.setX(0);
  h_homeCoord.setY(0);

  if( h_projection )
    {
      // delete an older projection object
      delete  h_projection;
      h_projection = 0;
    }

  QFile inFile(path);

  if( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("OpenAip: Cannot open airfield file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  in >> h_magic;

  if( h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "OAIP: wrong magic key %x read! Aborting ...", h_magic );
      inFile.close();
      return false;
    }

  in >> h_fileType;

  if( h_fileType != FILE_TYPE_AIRFIELD_C )
    {
      qWarning( "OAIP: wrong file type %llx read! Aborting ...", h_fileType );
      inFile.close();
      return false;
    }

  in >> h_fileVersion;

  if( h_fileVersion != FILE_VERSION_AIRFIELD_C )
    {
      qWarning( "OAIP: wrong file version %hhx read! Aborting ...", h_fileVersion );
      inFile.close();
      return false;
    }

  in >> h_creationDateTime;
  in >> h_homeRadius;
  in >> h_homeCoord;

#ifdef BOUNDING_BOX
  in >> h_boundingBox;
#endif

  h_projection = LoadProjection(in);

  inFile.close();
  h_headerIsValid = true;
  return true;
}

/*------------------------- OpenAipThread ------------------------------------*/

#include <signal.h>

OpenAipThread::OpenAipThread( QObject *parent, bool readSource ) :
  QThread( parent ),
  m_readSource(readSource)
{
  setObjectName( "OpenAipThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

OpenAipThread::~OpenAipThread()
{
}

void OpenAipThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  QList<Airfield>* airfieldList = new QList<Airfield>;

  OpenAipAirfieldLoader oaipl;

  bool ok = oaipl.load( *airfieldList, m_readSource );

  /* It is expected that a receiver slot is connected to this signal. The
   * receiver is responsible to delete the passed lists. Otherwise a big
   * memory leak will occur.
   */
  emit loadedList( ok, airfieldList );

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( loadedList( bool, QList<Airfield>* )) ) == 0 )
    {
      qWarning() << "OpenAipThread: No Slot connection to Signal loadedList!";

      delete airfieldList;
    }
}
