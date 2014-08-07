/***********************************************************************
**
**   AirspaceHelper.cpp
**
**   Created on: 03.02.2014
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "AirspaceHelper.h"
#include "filetools.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "openaip.h"
#include "openairparser.h"
#include "projectionbase.h"
#include "resource.h"

extern MapMatrix* _globalMapMatrix;

QMap<QString, BaseMapElement::objectType> AirspaceHelper::m_airspaceTypeMap;

QSet<int> AirspaceHelper::m_airspaceDictionary;

QMutex AirspaceHelper::m_mutex;

int AirspaceHelper::loadAirspaces( QList<Airspace*>& list, bool readSource )
{
  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &m_mutex );
  QTime t; t.start();
  uint loadCounter = 0; // number of successfully loaded files

  m_airspaceDictionary.clear();

  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();
  QStringList preselect;

  // Setup a filter for the wanted file extensions.
  QString filter = "*.txt *.TXT *.txc *.aip *.AIP *.aic";

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      MapContents::addDir( preselect, mapDirs.at( i ) + "/airspaces", filter );
    }

  if( preselect.count() == 0 )
    {
      qWarning( "ASH: No Airspace files found in the map directories!" );
      return loadCounter;
    }

  // First check, if we have found a file name in upper letters. May
  // be true, if a file was downloaded from the Internet. We will convert
  // such a file name to lower cases and replace it in the file list.
  for( int i = 0; i < preselect.size(); ++i )
    {
      if( preselect.at( i ).endsWith( ".TXT" ) ||
          preselect.at( i ).endsWith( ".AIP" ))
        {
          QFileInfo fInfo = preselect.at( i );
          QString path = fInfo.absolutePath();
          QString fn = fInfo.fileName().toLower();
          QString newFn = path + "/" + fn;

          // The destination file may not exist otherwise rename will fail.
          QFile::remove( newFn );
          QFile::rename( preselect.at( i ), newFn );
          preselect[i] = newFn;
        }
    }

  // source files follows compiled files
  preselect.sort();

  // Check, which files shall be loaded.
  QStringList& files = GeneralConfig::instance()->getAirspaceFileList();

  if( files.isEmpty() )
    {
      // No files shall be loaded
      qWarning() << "ASH: No Airspace files defined for loading!";
      return loadCounter;
    }

  if( files.first() != "All" )
    {
      for( int i = preselect.size() - 1; i >= 0; i-- )
        {
          QFileInfo fi(preselect.at(i));
          QString file;

          if( fi.suffix().left(2) == "tx" )
            {
              file = fi.completeBaseName() + ".txt";
            }
          else if( fi.suffix().left(2) == "ai" )
            {
              file = fi.completeBaseName() + ".aip";
            }

          if( files.contains( file ) == false )
            {
              preselect.removeAt(i);
            }
        }
    }

  OpenAirParser oap;
  OpenAip oaip;
  QString errorInfo;

  while( ! preselect.isEmpty() )
    {
      QString srcName;
      QString binName;

      if( preselect.first().endsWith(QString(".txt")) )
        {
          // there can't be the same name txc after this txt
          // parse found txt file
          srcName = preselect.first();

          if( oap.parse(srcName, list, true) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

      if( preselect.first().endsWith(QString(".aip")) )
        {
          // there can't be the same name aic after this aip
          // parse found aip file
          srcName = preselect.first();

          if( oaip.readAirspaces(srcName, list, errorInfo, true) )
            {
              loadCounter++;
            }

          preselect.removeAt(0);
          continue;
        }

      if( readSource == true )
        {
          // Source file read is required.
          // Remove the binary file from the list.
          preselect.removeAt(0);
          continue;
        }

      // At first we found a binary file with the extension aic or txc.
      binName = preselect.first();

      // Get file suffix, can be txc or aic
      QString binSuffix = QFileInfo(binName).suffix();
      QString srcSuffix;

      QDateTime h_creationDateTime;
      ProjectionBase* h_projection = 0;

      if( binSuffix == "txc" )
        {
          srcSuffix = "txt";
        }
      else if( binSuffix == "aic" )
        {
          srcSuffix = "aip";
        }

      srcName = binName.left(binName.size()-3) + srcSuffix;

      // Now we have to check if there's to find a source file with
      // the related extension after the binary file
      preselect.removeAt(0);
      QFileInfo binFi(binName);

      if( srcName == preselect.first() )
        {
          preselect.removeAt(0);
          // We found the related source file and will do some checks to
          // decide which type of file will be read in.

          // Lets check, if we can read the header of the compiled file
          if( ! AirspaceHelper::readHeaderData( binName,
                                                h_creationDateTime,
                                                &h_projection ) )
            {
              // Compiled file format is not the expected one, remove
              // wrong file and start a reparsing of source file.
              QFile::remove(binName);

              if (srcSuffix == "txt")
                {
                  if (oap.parse(srcName, list, true))
                    {
                      loadCounter++;
                    }
                }
              else if (srcSuffix == "aip")
                {
                  if (oaip.readAirspaces(srcName, list, errorInfo, true))
                    {
                      loadCounter++;
                    }
                }

              continue;
            }
        }

      // Do a date-time check. If the source file is younger in its
      // modification time as the compiled file, a new compilation
      // must be forced.
      QFileInfo fi(srcName);
      QDateTime lastModTxt = fi.lastModified();

      if( h_creationDateTime < lastModTxt )
        {
          // Modification date-time of source is younger as from
          // compiled file. Therefore we do start a reparsing of the
          // source file.
          QFile::remove(binName);

          if (srcSuffix == "txt")
            {
              if (oap.parse(srcName, list, true))
                {
                  loadCounter++;
                }
            }
          else if (srcSuffix == "aip")
            {
              if (oaip.readAirspaces(srcName, list, errorInfo, true))
                {
                  loadCounter++;
                }
            }

          continue;
        }

      // Check date-time against the configuration files
      QString confName1 = fi.path() + "/airspace_mappings.conf";
      QString confName2 = fi.path() + "/" + fi.baseName() + "_mappings.conf";
      QFileInfo fiConf1(confName1);
      QFileInfo fiConf2(confName2);

      if( (fiConf1.exists() && fi.isReadable() &&
           h_creationDateTime < fiConf1.lastModified()) ||
          (fiConf2.exists() && fi.isReadable() &&
           h_creationDateTime < fiConf2.lastModified()) )
        {
          // Configuration file was modified, make a new compilation.
          // It is not deeper checked, what was modified due to the effort and
          // in the assumption that a configuration file will not be changed
          // every minute.
          QFile::remove(binName);

          if (srcSuffix == "txt")
            {
              if (oap.parse(srcName, list, true))
                {
                  loadCounter++;
                }
            }
          else if (srcSuffix == "aip")
            {
              if (oaip.readAirspaces(srcName, list, errorInfo, true))
                {
                  loadCounter++;
                }
            }

          continue;
        }

      // Check, if the projection has been changed in the meantime
      ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

      if( ! MapContents::compareProjections(h_projection, currentProjection) )
        {
          // Projection has changed in the meantime
          if( h_projection )
            {
              delete h_projection;
              h_projection = 0;
            }

          QFile::remove(binName);

          if (srcSuffix == "txt")
            {
              if (oap.parse(srcName, list, true))
                {
                  loadCounter++;
                }
            }
          else if (srcSuffix == "aip")
            {
              if (oaip.readAirspaces(srcName, list, errorInfo, true))
                {
                  loadCounter++;
                }
            }

          continue;
        }

      // We will read the compiled file, because a source file is not to
      // find after it or all checks were successfully passed
      if (AirspaceHelper::readCompiledFile( binName, list ) )
        {
          loadCounter++;
        }
    } // End of While

  qDebug("ASH: %d Airspace file(s) loaded in %dms", loadCounter, t.elapsed());

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
  out << qint8( FILE_TYPE_AIRSPACE_C );
  out << quint16( FILE_VERSION_AIRSPACE_C );
  out << QDateTime::currentDateTime();
  SaveProjection( out, _globalMapMatrix->getProjection() );

  // write number of airfield records to be stored
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
          uAlt /= 100;
        }

      if( as->getLowerT() == Airspace::FL )
        {
          lAlt /= 100;
        }

      ShortSave( out, as->getName().toUtf8() );
      out.writeRawData( country, 2 );
      out << quint32( as->getId() );
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
 * Read the content of a compiled file and put it into the passed
 * list.
 *
 * @param path Full name with path of OpenAir binary file
 * @param list All airspace objects have to be stored in this list
 * @returns true (success) or false (error occurred)
 */
bool AirspaceHelper::readCompiledFile( QString &path, QList<Airspace*>& list )
{
  QTime t;
  t.start();

  QFile inFile(path);

  if ( !inFile.open(QIODevice::ReadOnly) )
    {
      qWarning("ASH: Cannot open airspace file %s!", path.toLatin1().data());
      return false;
    }

  qDebug() << "ASH: Reading" << path;

  QDataStream in(&inFile);
  in.setVersion( QDataStream::Qt_4_7 );

  quint32 magic;
  qint8 fileType;
  quint16 fileVersion;
  QDateTime creationDateTime;

#ifdef BOUNDING_BOX
  QRect boundingBox;
#endif

  ProjectionBase *projectionFromFile;
  qint32 buflen;

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
  in >> buflen;

  list.reserve( list.size() + buflen );

  uint counter = 0;

  QString name;
  qint32 id;
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

      ShortLoad(in, utf8_temp);
      name = QString::fromUtf8(utf8_temp);
      in.readRawData( country, 2 );
      in >> id;
      in >> type;
      in >> lowerType;
      in >> lower;
      in >> upperType;
      in >> upper;
      ShortLoad( in, pa );

      if( id >= 0 && addAirspaceIdentifier(id) == false )
        {
          // Airspace is already known. Ignore object.
          qDebug() << "ASH::readCompiledFile: Known Airspace" << name << "ignored!";
          continue;
        }

      Airspace *a = new Airspace( name,
                                  (BaseMapElement::objectType) type,
                                  pa,
                                  upper, (BaseMapElement::elevationType) upperType,
                                  lower, (BaseMapElement::elevationType) lowerType,
                                  id,
                                  QString(country) );
      list.append(a);
      counter++;
    }

  inFile.close();

  QFileInfo fi( path );

  qDebug( "ASH: %d airspace objects read from file %s in %dms",
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
  qint8 h_fileType = 0;
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
      qWarning( "ASH: wrong file type %x read! Aborting ...", h_fileType );
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
  m_airspaceTypeMap.insert("AirA", BaseMapElement::AirA);
  m_airspaceTypeMap.insert("AirB", BaseMapElement::AirB);
  m_airspaceTypeMap.insert("AirC", BaseMapElement::AirC);
  m_airspaceTypeMap.insert("AirD", BaseMapElement::AirD);
  m_airspaceTypeMap.insert("AirE", BaseMapElement::AirE);
  //m_airspaceTypeMap.insert("AirG", BaseMapElement::AirG);
  m_airspaceTypeMap.insert("WaveWindow", BaseMapElement::WaveWindow);
  m_airspaceTypeMap.insert("AirF", BaseMapElement::AirF);
  m_airspaceTypeMap.insert("ControlC", BaseMapElement::ControlC);
  m_airspaceTypeMap.insert("ControlD", BaseMapElement::ControlD);
  m_airspaceTypeMap.insert("Danger", BaseMapElement::Danger);
  m_airspaceTypeMap.insert("Restricted", BaseMapElement::Restricted);
  m_airspaceTypeMap.insert("Prohibited", BaseMapElement::Prohibited);
  m_airspaceTypeMap.insert("LowFlight", BaseMapElement::LowFlight);
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
      typeMap.insert("UKN", BaseMapElement::AirUkn);
      typeMap.insert("GP", BaseMapElement::Restricted);
      typeMap.insert("R", BaseMapElement::Restricted);
      typeMap.insert("P", BaseMapElement::Prohibited);
      typeMap.insert("TRA", BaseMapElement::Restricted);
      typeMap.insert("Q", BaseMapElement::Danger);
      typeMap.insert("CTR", BaseMapElement::ControlD);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("W", BaseMapElement::WaveWindow);
      typeMap.insert("GSEC", BaseMapElement::GliderSector);
    }
  else if( fi.suffix().toLower() == "aip" )
    {
      // OpenAip default airspace mapping
      typeMap.insert("A", BaseMapElement::AirA);
      typeMap.insert("B", BaseMapElement::AirB);
      typeMap.insert("C", BaseMapElement::AirC);
      typeMap.insert("D", BaseMapElement::AirD);
      typeMap.insert("E", BaseMapElement::AirE);
      typeMap.insert("F", BaseMapElement::AirF);
      typeMap.insert("FIR", BaseMapElement::AirFir);
      typeMap.insert("CTR", BaseMapElement::ControlD);
      typeMap.insert("DANGER", BaseMapElement::Danger);
      typeMap.insert("RESTRICTED", BaseMapElement::Restricted);
      typeMap.insert("PROHIBITED", BaseMapElement::Prohibited);
      typeMap.insert("TMA", BaseMapElement::ControlD);
      typeMap.insert("TMZ", BaseMapElement::Tmz);
      typeMap.insert("GLIDING", BaseMapElement::GliderSector);
      typeMap.insert("WAVE", BaseMapElement::WaveWindow);
      typeMap.insert("OTH", BaseMapElement::AirUkn);
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
          qDebug() << "Parsing mapping file" << fList.at(i);

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
