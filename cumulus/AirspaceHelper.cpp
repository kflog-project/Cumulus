/***********************************************************************
**
**   AirspaceHelper.cpp
**
**   Created on: 03.02.2014 by Axel Pauli
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2022 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "AirspaceHelper.h"
#include "filetools.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "OpenAip.h"
#include "openairparser.h"
#include "projectionbase.h"
#include "resource.h"

extern MapMatrix* _globalMapMatrix;

QMap<QString, BaseMapElement::objectType> AirspaceHelper::m_airspaceTypeMap;

QSet<QString> AirspaceHelper::m_airspaceDictionary;

QMap<quint16, BaseMapElement::objectType> AirspaceHelper::m_icaoClassMap;

QMutex AirspaceHelper::m_mutex;

int AirspaceHelper::loadAirspaces( QList<Airspace*>& list, bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QElapsedTimer t; t.start();
  uint loadCounter = 0; // number of successfully loaded files

  m_airspaceDictionary.clear();

  // Check, which files the user wants to load.
  QStringList& files = GeneralConfig::instance()->getAirspaceFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "ASH: No Airspace files defined for loading!";
      return loadCounter;
    }

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  // Setup a filter for the supported file extensions.
  QString filter = "*.txt *.TXT *_asp.json";

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airspaces", filter );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "ASH: No Airspace files found in the map directories!" );
      return loadCounter;
    }

  // First check, if we have found an openair file name in upper letters. May
  // be true, if a file was downloaded from the Internet. We will convert
  // such a file name to lower cases and replace it in the file list.
  for( int i = 0; i < preselect.size(); ++i )
    {
      if( preselect.at( i ).endsWith( ".TXT" ) )
        {
          QFileInfo fInfo = preselect.at( i );
          QString path = fInfo.absolutePath();
          QString fn = fInfo.fileName().toLower();
          QString newFn = path + "/" + fn;

          // Do not use QFile::remove and QFile::rename, that will result in a
          // remove of the source and destination file on FAT32 file systems.
          rename( preselect.at(i).toLatin1().data(), newFn.toLatin1().data() );
          preselect[i] = newFn;
        }
    }

  if( files.first() != "All" )
    {
      // Tidy up the preselection list, if not all found files shall be loaded.
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QString file = QFileInfo( preselect.at(i) ).fileName();

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  while( ! preselect.isEmpty() )
    {
      OpenAirParser oap;
      OpenAip oaip;
      QString errorInfo;

      // Check, if a compiled file exists. In this case we try to read it as first.
      QString aipName = preselect.first();
      QString aicName = preselect.first() + "c";

      if( QFile::exists( aicName ) == true && readSource == false )
        {
          QDateTime h_creationDateTime;
          ProjectionBase* h_projection = 0;

          // Lets check, if we can read the header of the compiled file
          if( ! readHeaderData( aicName, h_creationDateTime, &h_projection ) )
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

          if ( h_creationDateTime < lastModTxt )
            {
              // Modification date-time of source is younger as from
              // compiled file. Therefore we require a reparsing of the
              // source files.
              qDebug() << "ASH:" << QFileInfo(aicName).fileName() << "Time mismatch";
              QFile::remove( aicName );
              continue;
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

              qDebug() << "AHS:" << QFileInfo(aicName).fileName() << "Projection mismatch";
              QFile::remove( aicName );
              continue;
            }

          if( AirspaceHelper::readCompiledFile( aicName, list ) )
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

          bool ok = false;

          if( aipName.endsWith( ".txt") )
            {
              // read open air file
              ok = oap.parse( aipName, list, true );
            }
          else if( aipName.endsWith( ".json") )
            {
              // read open aip file
              ok = oaip.readAirspaces( aipName, list, errorInfo, true);
            }

          if( ok )
            {
              loadCounter++;
            }
         }
    } // End of While

  qDebug( "ASH: %d Airspace file(s) with %d items loaded in %lldms",
          loadCounter, list.size(), t.elapsed() );

//    for(int i=0; i < list.size(); i++ )
//      {
//        list.at(i)->debug();
//      }

  return loadCounter;
}

bool AirspaceHelper::createCompiledFile( QString& fileName,
                                         QList<Airspace*>& airspaceList,
                                         int airspaceListStart )
{
  if( airspaceList.size() == 0 || airspaceList.size() < airspaceListStart )
    {
      return false;
    }

  QFile file( fileName );
  QDataStream out( &file );

  out.setVersion( QDataStream::Qt_4_7 );

  if( file.open(QIODevice::WriteOnly) == false )
    {
      qWarning( "ASH: Can't open airspace file %s for writing!"
                " Aborting ...",
                fileName.toLatin1().data() );

      return false;
    }

  // create compiled binary version
  out << quint32( KFLOG_FILE_MAGIC );
  out << QByteArray( FILE_TYPE_AIRSPACE_C );
  out << quint16( FILE_VERSION_AIRSPACE_C );
  out << QDateTime::currentDateTime();
  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of airspace records to be stored
  out << quint32( airspaceList.size() - airspaceListStart );

  // Storing starts at the given index.
  for( int i = airspaceListStart; i < airspaceList.size(); i++ )
    {
      Airspace* as = airspaceList[i];

      const char* country = "\0\0";

      if( as->getCountry().left(2).size() == 2 )
        {
          country = as->getCountry().left(2).toLatin1().data();
        }

      // Normalize Flight Level altitudes to its original value before storing.
      float uAlt = as->getUpperAltitude().getFeet();
      float lAlt = as->getLowerAltitude().getFeet();

      if( as->getUpperT() == Airspace::FL )
        {
          uAlt /= 100.0;
        }

      if( as->getLowerT() == Airspace::FL )
        {
          lAlt /= 100.0;
        }

      ShortSave( out, as->getName() );
      out.writeRawData( country, 2 );
      out << quint8( as->getIcaoClass() );
      out << quint8( as->getActivity() );
      out << as->isByNotam();
      out << quint8( as->getTypeID() );
      out << quint8( as->getLowerT() );
      out << float( lAlt );
      out << quint8( as->getUpperT() );
      out << float( uAlt );
      ShortSave( out, as->getProjectedPolygon() );
    }

  file.close();
  return true;
}

/**
 * Read the content of a compiled file and put it into the passed list.
 *
 * @param path Full name with path of OpenAir binary file
 * @param list All airspace objects have to be stored in this list
 * @returns true (success) or false (error occurred)
 */
bool AirspaceHelper::readCompiledFile( QString &path, QList<Airspace*>& list )
{
  QElapsedTimer t;
  t.start();

  QFile inFile(path);

  if ( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("ASH: Cannot open airspace file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  quint32 magic;
  QByteArray fileType;
  quint16 fileVersion;
  QDateTime creationDateTime;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  ProjectionBase *projectionFromFile;
  quint32 noOfAirspaces;

  in >> magic;

  if ( magic != KFLOG_FILE_MAGIC )
    {
      qWarning() << "ASH: wrong magic key" << magic << "read! Aborting ...";
      inFile.close();
      return false;
    }

  in >> fileType;

  if ( fileType != FILE_TYPE_AIRSPACE_C )
    {
      qWarning() <<  "ASH: Wrong file type" <<  fileType << "read! Aborting ...";
      inFile.close();
      return false;
    }

  in >> fileVersion;

  if ( fileVersion != FILE_VERSION_AIRSPACE_C )
    {
      qWarning( "ASH: Wrong file version %x read! Aborting ...", fileVersion );
      inFile.close();
      return false;
    }

  in >> creationDateTime;

  projectionFromFile = LoadProjection(in);

  // projectionFromFile is allocated dynamically, we don't need it
  // here. Therefore it is immediately deleted to avoid memory leaks.
  delete projectionFromFile;
  projectionFromFile = 0;

#ifdef BOUNDING_BOX

  in >> boundingBox;
#endif

  // Read at first the number of airspaces contained in the compiled file.
  // We have to read it and make a preallocation in the airspace list.
  in >> noOfAirspaces;

  list.reserve( list.size() + noOfAirspaces );

  uint counter = 0;

  QString name;
  quint8 icaoClass;
  quint8 activity;
  bool byNotam;
  quint8 type;
  quint8 lowerType;
  float lower;
  quint8 upperType;
  float upper;
  QPolygon pa;
  QByteArray utf8_temp;
  char country[3] = { 0, 0, 0 };

  while ( ! in.atEnd() )
    {
      pa.resize(0);

      ShortLoad( in, name );
      in.readRawData( country, 2 );
      in >> icaoClass;
      in >> activity;
      in >> byNotam;
      in >> type;
      in >> lowerType;
      in >> lower;
      in >> upperType;
      in >> upper;
      ShortLoad( in, pa );

      Airspace *a = new Airspace( name,
                                  (BaseMapElement::objectType) type,
                                  pa,
                                  upper, (BaseMapElement::elevationType) upperType,
                                  lower, (BaseMapElement::elevationType) lowerType,
                                  icaoClass,
                                  QString(country),
                                  activity,
                                  byNotam );
      list.append(a);
      counter++;
    }

  inFile.close();

  QFileInfo fi( path );

  qDebug( "ASH: %d airspace objects read from file %s in %lldms",
          counter, fi.fileName().toLatin1().data(), t.elapsed() );

  return true;
}

/**
 * Get the header data of a compiled file and put it in the class
 * variables.
 *
 * \param path Full name with path of OpenAir binary file
 *
 * \param creationDateTime Date and time of file creation
 *
 * \param ProjectionBase stored projection type
 *
 * \returns true (success) or false (error occurred)
 */
bool AirspaceHelper::readHeaderData( QString &path,
                                     QDateTime& creationDateTime,
                                     ProjectionBase** projection )
{
  quint32 h_magic = 0;
  QByteArray h_fileType;
  quint16 h_fileVersion = 0;

  QFile inFile(path);

  if ( !inFile.open( QIODevice::ReadOnly) )
    {
      qWarning("ASH: Cannot open airspace file %s!", path.toLatin1().data());
      return false;
    }

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  in >> h_magic;

  if ( h_magic != KFLOG_FILE_MAGIC )
    {
      qWarning( "ASH: wrong magic key %x read! Aborting ...", h_magic );
      inFile.close();
      return false;
    }

  in >> h_fileType;

  if ( h_fileType != FILE_TYPE_AIRSPACE_C )
    {
      qWarning( "ASH: wrong file type '%s' read! Aborting ...", h_fileType.data() );
      inFile.close();
      return false;
    }

  in >> h_fileVersion;

  if ( h_fileVersion != FILE_VERSION_AIRSPACE_C )
    {
      qWarning( "ASH: wrong file version %x read! Aborting ...", h_fileVersion );
      inFile.close();
      return false;
    }

  in >> creationDateTime;

  *projection = LoadProjection(in);

#ifdef BOUNDING_BOX
  in >> h_boundingBox;
#endif

  inFile.close();
  return true;
}

void AirspaceHelper::loadAirspaceTypeMapping()
{
  // Creates a mapping from a string representation of the supported
  // airspace types in Cumulus to their integer codes
  m_airspaceTypeMap.clear();
  m_airspaceTypeMap.insert("AirA", BaseMapElement::AirA);
  m_airspaceTypeMap.insert("AirB", BaseMapElement::AirB);
  m_airspaceTypeMap.insert("AirC", BaseMapElement::AirC);
  m_airspaceTypeMap.insert("AirD", BaseMapElement::AirD);
  m_airspaceTypeMap.insert("AirE", BaseMapElement::AirE);
  m_airspaceTypeMap.insert("AirG", BaseMapElement::AirG);
  m_airspaceTypeMap.insert("WaveWindow", BaseMapElement::WaveWindow);
  m_airspaceTypeMap.insert("AirF", BaseMapElement::AirF);
  m_airspaceTypeMap.insert("ControlC", BaseMapElement::ControlC);
  m_airspaceTypeMap.insert("ControlD", BaseMapElement::ControlD);
  m_airspaceTypeMap.insert("Ctr", BaseMapElement::Ctr);
  m_airspaceTypeMap.insert("Danger", BaseMapElement::Danger);
  m_airspaceTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_airspaceTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_airspaceTypeMap.insert("Sua", BaseMapElement::Sua);
  m_airspaceTypeMap.insert("Rmz", BaseMapElement::Rmz);
  m_airspaceTypeMap.insert("Tmz", BaseMapElement::Tmz);
  m_airspaceTypeMap.insert("GliderSector", BaseMapElement::GliderSector);
  m_airspaceTypeMap.insert("AirUkn", BaseMapElement::AirUkn);
}

QMap<QString, BaseMapElement::objectType>
AirspaceHelper::initializeAirspaceTypeMapping(const QString& mapFilePath)
{
  QMap<QString, BaseMapElement::objectType> typeMap;

  QFileInfo fi(mapFilePath);

  if( fi.suffix().toLower() == "txt" )
    {
      // OpenAir default airspace mapping
      typeMap.insert("A", BaseMapElement::AirA);
      typeMap.insert("B", BaseMapElement::AirB);
      typeMap.insert("C", BaseMapElement::AirC);
      typeMap.insert("D", BaseMapElement::AirD);
      typeMap.insert("E", BaseMapElement::AirE);
      typeMap.insert("F", BaseMapElement::AirF);
      typeMap.insert("G", BaseMapElement::AirG);
      typeMap.insert("UKN", BaseMapElement::AirUkn);
      typeMap.insert("GP", BaseMapElement::Restricted);
      typeMap.insert("R", BaseMapElement::Restricted);
      typeMap.insert("P", BaseMapElement::Prohibited);
      typeMap.insert("TRA", BaseMapElement::Restricted);
      typeMap.insert("Q", BaseMapElement::Danger);
      typeMap.insert("CTR", BaseMapElement::Ctr);
      typeMap.insert("RMZ", BaseMapElement::Rmz);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("W", BaseMapElement::WaveWindow);
      typeMap.insert("GSEC", BaseMapElement::GliderSector);
    }
  else if( fi.suffix().toLower() == "json" )
    {
      // OpenAip default airspace mapping
      typeMap.insert("0", BaseMapElement::AirUkn);
      typeMap.insert("1", BaseMapElement::Restricted);
      typeMap.insert("2", BaseMapElement::Danger);
      typeMap.insert("3", BaseMapElement::Prohibited);
      typeMap.insert("4", BaseMapElement::Ctr);
      typeMap.insert("5", BaseMapElement::Tmz);
      typeMap.insert("6", BaseMapElement::Rmz);
      typeMap.insert("7", BaseMapElement::Sua); // TMA
      typeMap.insert("8", BaseMapElement::Sua); // TRA
      typeMap.insert("9", BaseMapElement::Sua); // TSA
      typeMap.insert("10", BaseMapElement::AirFir);
      typeMap.insert("11", BaseMapElement::Sua); // UIR
      typeMap.insert("12", BaseMapElement::Sua); // ADIZ
      typeMap.insert("13", BaseMapElement::Sua); // ATZ
      typeMap.insert("14", BaseMapElement::Sua); // Alert Area
      typeMap.insert("15", BaseMapElement::Sua); // Airway
      typeMap.insert("16", BaseMapElement::Sua); // Military Training Route (MTR)
      typeMap.insert("17", BaseMapElement::Sua); // Alert Area
      typeMap.insert("18", BaseMapElement::Sua); // Warning Area
      typeMap.insert("19", BaseMapElement::Restricted); // Protected Area
      typeMap.insert("20", BaseMapElement::Sua); // Helicopter Traffic Zone (HTZ)
      typeMap.insert("21", BaseMapElement::GliderSector);
      typeMap.insert("22", BaseMapElement::Tmz); // Transponder Setting (TRP)
      typeMap.insert("23", BaseMapElement::AirFir); // Traffic Information Zone (TIZ)
      typeMap.insert("24", BaseMapElement::AirFir); // Traffic Information Area (TIA)
      typeMap.insert("25", BaseMapElement::Sua); // Military Training Area (MTA)
      typeMap.insert("26", BaseMapElement::Ctr); // Controlled Area (CTA)
      typeMap.insert("27", BaseMapElement::Sua); // ACC Sector (ACC)
      typeMap.insert("28", BaseMapElement::Sua); // Aerial Sporting Or Recreational Activity
      typeMap.insert("29", BaseMapElement::Sua); // Low Altitude Overflight Restriction
    }
  else
    {
      qWarning() << "ASH::initializeAirspaceTypeMapping failed, unknown file suffix"
                 << fi.suffix();
      return typeMap;
    }

  QStringList fList;

  // Default user airspace type mapping file
  fList << fi.path() + "/airspace_mappings.conf";

  // User airspace type mapping file only for the current airspace file
  fList << fi.path() + "/" + fi.baseName() + "_mappings.conf";

  for (int i = 0; i < fList.size(); i++)
    {
      fi.setFile(fList.at(i));

      if (fi.exists() && fi.isFile() && fi.isReadable())
        {
          QFile f(fList.at(i));

          if (!f.open(QIODevice::ReadOnly))
            {
              qWarning() << "ASH: Cannot open airspace mapping file" << fList.at(i)
                         << "!";
              return typeMap;
            }

          QTextStream in(&f);
          qDebug() << "ASH: Parsing mapping file" << fList.at(i);

          // start parsing
          while (!in.atEnd())
            {
              QString line = in.readLine().simplified();

              if (line.startsWith("*") || line.startsWith("#")
                  || line.isEmpty())
                {
                  continue;
                }

              int pos = line.indexOf("=");

              if (pos > 1 && pos < line.length() - 1)
                {
                  QString key = line.left(pos).simplified();
                  QString value = line.mid(pos + 1).simplified();

                  if (key.isEmpty() || value.isEmpty())
                    {
                      continue;
                    }

                  if (isAirspaceBaseTypeKnown(value) == false)
                    {
                      continue;
                    }

                  qDebug() << "ASH: Airspace type mapping changed by user:"
                           << key << "-->" << value;

                  typeMap.remove(key);
                  typeMap.insert(key, mapAirspaceBaseType(value));
                }
            }

          f.close();
        }
    }

  return typeMap;
}

/**
 * Initialize a mapping from an openAIP ICAO class integer to the related
 * Cumulus integer type.
 */
void AirspaceHelper::loadIcaoClassMapping()
{
  m_icaoClassMap.clear();
  m_icaoClassMap.insert( 0, BaseMapElement::AirA );
  m_icaoClassMap.insert( 1, BaseMapElement::AirB );
  m_icaoClassMap.insert( 2, BaseMapElement::AirC );
  m_icaoClassMap.insert( 3, BaseMapElement::AirD );
  m_icaoClassMap.insert( 4, BaseMapElement::AirE );
  m_icaoClassMap.insert( 5, BaseMapElement::AirF );
  m_icaoClassMap.insert( 6, BaseMapElement::AirG );
  m_icaoClassMap.insert( 8, BaseMapElement::Sua );
}


/*---------------------- AirspaceHelperThread --------------------------------*/

#include <csignal>

AirspaceHelperThread::AirspaceHelperThread( QObject *parent, bool readSource ) :
  QThread( parent ),
  m_readSource(readSource)
{
  setObjectName( "AirspaceHelperThread" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(deleteLater()) );
}

AirspaceHelperThread::~AirspaceHelperThread()
{
}


void AirspaceHelperThread::run()
{
  sigset_t sigset;
  sigfillset( &sigset );

  // deactivate all signals in this thread
  pthread_sigmask( SIG_SETMASK, &sigset, 0 );

  // Check is signal is connected to a slot.
  if( receivers( SIGNAL( loadedList( int, SortableAirspaceList* )) ) == 0 )
    {
      qWarning() << "AirspaceHelperThread: No Slot connection to Signal loadedList!";
      return;
    }

  SortableAirspaceList* airspaceList = new SortableAirspaceList;

  int ok = AirspaceHelper::loadAirspaces( *airspaceList, m_readSource );

  /* It is expected that a receiver slot is connected to this signal. The
   * receiver is responsible to delete the passed lists. Otherwise a big
   * memory leak will occur.
   */
  emit loadedList( ok, airspaceList );
}
