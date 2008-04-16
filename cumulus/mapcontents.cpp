/**********************************************************************
 **
 **   mapcontents.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include <stdlib.h>
#include <unistd.h>

#include <QDataStream>
#include <QTextStream>
#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QString>
#include <QRegion>
#include <QPolygon>
#include <QByteArray>
#include <QCoreApplication>

#include "mapcontents.h"
#include "mapmatrix.h"
#include "mapview.h"
#include "mapcalc.h"
#include "airport.h"
#include "airspace.h"
#include "basemapelement.h"
#include "glidersite.h"
#include "isohypse.h"
#include "lineelement.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "projectionbase.h"
#include "wgspoint.h"
#include "filetools.h"
#include "hwinfo.h"
#include "openairparser.h"
#include "welt2000.h"
#include "cucalc.h"
#include "flighttask.h"
#include "gpsnmea.h"
#include "waypointcatalog.h"
#include "generalconfig.h"

extern MapView *_globalMapView;

#define MAX_FILE_COUNT 16200
#define ISO_LINE_NUM 50

//general KFLOG file token: @KFL
#define KFLOG_FILE_MAGIC    0x404b464c

//uncompiled mapfiles
#define FILE_TYPE_GROUND      0x47
#define FILE_TYPE_TERRAIN     0x54
#define FILE_TYPE_MAP         0x4d
#define FILE_TYPE_AERO        0x41

//compiled mapfiles
#define FILE_TYPE_GROUND_C    0x67
#define FILE_TYPE_TERRAIN_C   0x74
#define FILE_TYPE_MAP_C       0x6d
#define FILE_TYPE_AIRSPACE_C  0x61  //aero files are split up into airspace and airfield files
#define FILE_TYPE_AIRFIELD_C  0x62

// versions
#define FILE_FORMAT_ID        101

#define FILE_VERSION_GROUND   100

#define FILE_VERSION_TERRAIN  100

#define FILE_VERSION_MAP      101

// compiled version

#define FILE_VERSION_GROUND_C   102

#define FILE_VERSION_TERRAIN_C  102

#define FILE_VERSION_MAP_C      102


#define READ_POINT_LIST\
  if (compiling) {\
    in >> locLength;\
    pN.resize(locLength);\
    for(unsigned int i = 0; i < locLength; i++) { \
      in >> lat_temp;          in >> lon_temp; \
      pN.setPoint(i, _globalMapMatrix->wgsToMap(lat_temp, lon_temp)); \
    }\
    ShortSave(out, pN);\
  } else\
    ShortLoad(in, pN);\

// minimum amount of required free memory to start loading a mapfile
#define MINIMUM_FREE_MEMORY 1024*4

// List of altitude-levels (50 in total):
const int MapContents::isoLines[] =
  {
    0, 10, 25, 50, 75, 100, 150, 200, 250,
    300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
    2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
    4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
    7500, 7750, 8000, 8250, 8500, 8750
  };

MapContents::MapContents(QObject* parent, WaitScreen* waitscreen)
  : QObject(parent),
    airportList(this),
    gliderSiteList(this),
    outList(this),
    isFirst(true)
{
  ws=waitscreen;

  ws->slot_SetText1(tr("Loading map contents..."));

  sectionArray.resize(MAX_FILE_COUNT);
  sectionArray.fill(false);

  // Wir nehmen zunaechst 4 Schachtelungstiefen an ...
  for(int loop = 0; loop < ( ISO_LINE_NUM * 4 ); loop++) {
    QList<Isohypse*> *list = new QList<Isohypse*>;
    isoList.append(list);
  }

  _nextIsoLevel=10000;
  _lastIsoLevel=-1;
  _isoLevelReset=true;
  _lastIsoEntry=0;

  // read in waypoint list
  WaypointCatalog wpCat;
  wpCat.read( 0, &wpList );

  currentTask=0;

  connect(this,SIGNAL(progress(int)),
          ws,SLOT(slot_Progress(int)));
  connect(this,SIGNAL(loadingFile(const QString&)),
          ws,SLOT(slot_SetText2(const QString&)));
  connect(this,SIGNAL(majorAction(const QString&)),
          ws,SLOT(slot_SetText1(const QString&)));
  qDebug("MapContents initialized...");
}


MapContents::~MapContents()
{
  delete currentTask;

  qDeleteAll (airportList);
  airportList.clear();

  qDeleteAll (obstacleList);
  obstacleList.clear();

  qDeleteAll (airspaceList);
  airspaceList.clear();

  qDeleteAll (cityList);
  cityList.clear();

  qDeleteAll (gliderSiteList);
  gliderSiteList.clear();

  qDeleteAll (hydroList);
  hydroList.clear();

  qDeleteAll (lakeList);
  lakeList.clear();

  for (int i=isoList.count()-1; i>=0;i--) {
    qDeleteAll(*isoList.at(i));
    isoList.at(i)->clear();
  }

  qDeleteAll (isoList);
  isoList.clear();

  qDeleteAll (landmarkList);
  landmarkList.clear();

  qDeleteAll (navList);
  navList.clear();

  qDeleteAll (obstacleList);
  obstacleList.clear();

  qDeleteAll (outList);
  outList.clear();

  qDeleteAll (railList);
  railList.clear();

  qDeleteAll (regIsoLines);
  regIsoLines.clear();

  qDeleteAll (reportList);
  reportList.clear();

  qDeleteAll (highwayList);
  highwayList.clear();

  qDeleteAll (roadList);
  roadList.clear();

//  qDeleteAll (stationList);
//  stationList.clear();

  qDeleteAll (topoList);
  topoList.clear();

  qDeleteAll (villageList);
  villageList.clear();

  qDeleteAll (wpList);
  wpList.clear();
}

  // save the current waypoint list
void MapContents::saveWaypointList()
{
  WaypointCatalog wpCat;
  wpCat.write( 0, &wpList );
}

// JD: Here is the new code for managing plain and precomputed map files */
bool MapContents::__readTerrainFile(const int fileSecID,
                                    const int fileTypeID)
{
  extern const MapMatrix * _globalMapMatrix;
  extern MapConfig * _globalMapConfig;
  bool kflExists, kfcExists;
  bool compiling = false;

  if( fileTypeID != FILE_TYPE_TERRAIN && fileTypeID != FILE_TYPE_GROUND ) {
    qWarning( "Cumulus: __readTerrainFile do not support requested file type %d!",
              fileTypeID );
    return false;
  }

  // first, check if we need to load this file at all...
  if ((fileTypeID==FILE_TYPE_TERRAIN) && (!_globalMapConfig->getLoadIsolines())) {
    return true;
  }

  if (memoryFull) {   //if we allready know the memory if full and can't be emptied at this point, just return.
    _globalMapView->message(tr("Out of memory! Map not loaded."));
    return false;
  }

  //check free memory
  int memFree = HwInfo::instance()->getFreeMemory();

  if( memFree < MINIMUM_FREE_MEMORY ) {
    if ( !unloadDone) {
      unloadMaps();  //try freeing some memory
      memFree = HwInfo::instance()->getFreeMemory();  //re-asses free memory
      if( memFree < MINIMUM_FREE_MEMORY ) {
        memoryFull=true; //set flag to indicate that we need not try loading any more mapfiles now.
        qWarning("Cumulus couldn't load file, low on memory! Memory needed: %d kB, free: %d kB", MINIMUM_FREE_MEMORY, memFree );
        _globalMapView->message(tr("Out of memory! Map not loaded."));
        return false;
      }
    } else {
      memoryFull=true; //set flag to indicate that we need not try loading any more mapfiles now.
      qWarning("Cumulus couldn't load file, low on memory! Memory needed: %d kB, free: %d kB", MINIMUM_FREE_MEMORY, memFree );
      _globalMapView->message(tr("Out of memory! Map not loaded."));
      return false;
    }
  }

  QString kflPathName, kfcPathName, pathName;
  QString kflName, kfcName;

  kflName.sprintf("landscape/%c_%.5d.kfl", fileTypeID, fileSecID);
  kflExists = locateFile(kflName, kflPathName);

  kfcName.sprintf("landscape/%c_%.5d.kfc", fileTypeID, fileSecID);
  kfcExists = locateFile(kfcName, kfcPathName);

  if ( ! (kflExists || kfcExists) ) {
    qWarning( "Cumulus: no map file (%s/%s) found! Please install %s file", kflName.toLatin1().data(), kfcName.toLatin1().data(), kflName.toLatin1().data() );
    return false; // file could not be located in any of the possible map directories.
  }

  if( kflExists ) {
    if( kfcExists ) {
      // kfl file newer than kfc ? Then compile it
      if (getDateFromMapFile( kflPathName ) > getDateFromMapFile( kfcPathName )) {
        compiling = true;
        qDebug("Map file %s has a newer date! Recompiling it from source.",
               kflPathName.toLatin1().data() );
      }
    } else {
      // no kfc file, we compile anyway
      compiling = true;
    }
  }

  // what file do we read after all ?
  if ( compiling ) {
    pathName = kflPathName;
    kfcPathName = kflPathName;
    kfcPathName.replace( kfcPathName.length()-1, 1, QString("c") );
  } else {
    pathName = kfcPathName;
    kflPathName = kfcPathName;
    kflPathName.replace( kflPathName.length()-1, 1, QString("l") );
  }

  QFile mapfile(pathName);

  if( !mapfile.open(QIODevice::ReadOnly) ) {
    qWarning("Cumulus: Can't open map file %s for reading", pathName.toLatin1().data() );

    if( ! compiling && kflExists ) {
      qDebug("Try to use file %s", kflPathName.toLatin1().data());
      // try to remove unopenable file, not sure if this works.
      unlink( pathName.toLatin1().data() );
      return __readTerrainFile( fileSecID, fileTypeID );
    }

    return false;
  }

  emit loadingFile(pathName);

  QDataStream in(&mapfile);
  in.setVersion(QDataStream::Qt_2_0);

  // qDebug("reading file %s", pathName.toLatin1().data());

  qint8 loadTypeID;
  quint16 loadSecID, formatID;
  quint32 magic;
  QDateTime createDateTime;
  ProjectionBase *projectionFromFile = 0;

  in >> magic;

  if( magic != KFLOG_FILE_MAGIC ) {
    if( ! compiling && kflExists ) {
      qWarning("Cumulus: wrong magic key %x read!\n Retry to compile %s.",
               magic, kflPathName.toLatin1().data());
      mapfile.close();
      unlink( pathName.toLatin1().data() );
      return __readTerrainFile( fileSecID, fileTypeID );
    }

    qWarning("Cumulus: wrong magic key %x read! Aborting ...", magic);
    return false;
  }

  in >> loadTypeID;

  if(loadTypeID != fileTypeID) // wrong type
    {
      mapfile.close();

      if( ! compiling && kflExists )
        {
          qWarning("Cumulus: wrong load type identifier %x read! "
                   "Retry to compile %s",
                   loadTypeID, kflPathName.toLatin1().data() );
          unlink( pathName.toLatin1().data() );
          return __readTerrainFile( fileSecID, fileTypeID );
        }

      qWarning("Cumulus: wrong load type identifier %x read! Aborting ...",
               loadTypeID );
      return false;
    }

  in >> formatID;

  /*qDebug ("File=%s, magic=%x, fileTypeID=%x, loadTypeID=%x, formatID=%d",
    pathName.toLatin1().data(), magic, fileTypeID, loadTypeID, formatID );*/


  // Determine, which file format id is expected
  int expFormatID, expComFormatID;

  if( fileTypeID == FILE_TYPE_TERRAIN ) {
    expFormatID = FILE_VERSION_TERRAIN;
    expComFormatID = FILE_VERSION_TERRAIN_C;
  } else {
    expFormatID = FILE_VERSION_GROUND;
    expComFormatID = FILE_VERSION_GROUND_C;
  }

  QFileInfo fi( pathName );

  qDebug("Reading File=%s, Magic=%xh, TypeId=%xh, formatId=%d, Date=%s",
         fi.fileName().toLatin1().data(), magic, loadTypeID, formatID,
         createDateTime.toString().toLatin1().data() );

  if( compiling ) {
    // Check map file
    if( formatID < expFormatID ) {
      // too old ...
      qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
               "Aborting ...", formatID, expFormatID );
      return false;
    } else if(formatID > expFormatID ) {
      // too new ...
      qWarning("Cumulus: File format too new! (version %d, expecting: %d) "
               "Aborting ...", formatID,expFormatID );
      return false;
    }
  } else {
    // Check compiled file
    if( formatID < expComFormatID ) {
      // too old ...
      if( kflExists ) {
        qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
                 "Retry to compile %s",
                 formatID, expComFormatID, kflPathName.toLatin1().data() );
        mapfile.close();
        unlink( pathName.toLatin1().data() );
        return __readTerrainFile( fileSecID, fileTypeID );
      }

      qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
               "Aborting ...", formatID, expFormatID );
      return false;
    } else if(formatID > expComFormatID ) {
      // too new ...
      if( kflExists ) {
        qWarning( "Cumulus: File format too new! (version %d, expecting: %d) "
                  "Retry to compile %s",
                  formatID, expComFormatID, kflPathName.toLatin1().data() );
        mapfile.close();
        unlink( pathName.toLatin1().data() );
        return __readTerrainFile( fileSecID, fileTypeID );
      }

      qWarning("Cumulus: File format too new! (version %d, expecting: %d) "
               "Aborting ...", formatID, expFormatID );
      return false;
    }
  }

  in >> loadSecID;

  if ( loadSecID != fileSecID ) {
    if( ! compiling && kflExists ) {
      qWarning( "Cumulus: %s: wrong section, bogus file name!"
                "\n Retry to compile %s",
                pathName.toLatin1().data(), kflPathName.toLatin1().data() );
      mapfile.close();
      unlink( pathName.toLatin1().data() );
      return __readTerrainFile( fileSecID, fileTypeID );
    }

    qWarning("Cumulus: %s: wrong section, bogus file name! Arborting ...",
             pathName.toLatin1().data() );
    return false;
  }

  in >> createDateTime;

  if( ! compiling ) {
    // check projection parameters from file against current used values
    projectionFromFile = LoadProjection(in);
    ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

    if( ! compareProjections( projectionFromFile, currentProjection ) ) {
      delete projectionFromFile;
      mapfile.close();

      if( kflExists ) {
        qWarning( "Cumulus: %s, can't use file, compiled for another projection!"
                  "\n Retry to compile %s",
                  pathName.toLatin1().data(), kflPathName.toLatin1().data() );

        unlink( pathName.toLatin1().data() );
        return __readTerrainFile( fileSecID, fileTypeID );
      }

      qWarning( "Cumulus: %s, can't use file, compiled for another projection!"
                " Please install %s file and restart.",
                pathName.toLatin1().data(), kflPathName.toLatin1().data() );
      return false;
    } else {
      // Must be deleted after use to avoid memory leak
      delete projectionFromFile;
    }
  }

  // Got to initialize "out" stream properly, even if write file is not needed
  QFile ausgabe(kfcPathName);
  QDataStream out(&ausgabe);

  if( compiling ) {
    out.setDevice(&ausgabe);
    out.setVersion(QDataStream::Qt_2_0);

    if(!ausgabe.open(QIODevice::WriteOnly)) {
      mapfile.close();
      qWarning("Cumulus: Can't open compiled map file %s for writing!"
               " Arborting...",
               kfcPathName.toLatin1().data() );
      return false;
    }

    qDebug("writing file %s", kfcPathName.toLatin1().data());

    out << magic;
    out << loadTypeID;
    out << quint16(expComFormatID);
    out << loadSecID;

    //set time one second later than the time of the original file;
    out << createDateTime.addSecs(1);

    // save current projection data
    SaveProjection(out, _globalMapMatrix->getProjection() );
  }

  uint loop = 0;

  while( !in.atEnd() ) {
    int sort_temp;

    quint8 type;
    qint16 elevation;
    qint8 valley, sort;
    qint32 locLength, latList_temp, lonList_temp, lastLat, lastLon;
    QPolygon pN;

    //we can safely reset lastLat and lastLon to 0, since that is a spot in the Atlantic ocean.
    lastLat=0;
    lastLon=0;

    in >> type;
    in >> elevation;
    in >> valley;
    in >> sort;

    if ( compiling ) {
      out << type;
      out << elevation;
      out << valley;
      out << sort;
      in >> locLength;

      pN.resize( locLength );

      for(int i = 0; i < locLength; i++) {
        in >> latList_temp;
        in >> lonList_temp;
        // Check for double points
        if (latList_temp == lastLat && lonList_temp == lastLon) {
          locLength--;
          pN.resize( locLength );
          i--;
          qDebug("  Skipping double entry");
        } else {
          // This is what causes the long delays !!! Lots of floating point calcs
          pN.setPoint(i, _globalMapMatrix->wgsToMap(latList_temp, lonList_temp));
          lastLat=latList_temp;
          lastLon=lonList_temp;
        }
      }

      // And that is the whole trick: saving the computed result. Takes the same space!
      ShortSave( out, pN );
    } else {
      // Reading the computed result from kfc file
      ShortLoad( in, pN );
    }

    sort_temp = -1;

    // We must ignore it, when sort is more than 3 or less than 0!
    if(sort >= 0 && sort <= 3) {
      for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
        if(isoLines[pos] == elevation)
          sort_temp = ISO_LINE_NUM * (int)sort + pos + 0;

      // If sort_temp is -1 here, we have an unused elevation and
      // must ignore it!
      if(sort_temp != -1) {
        Isohypse* newItem = new Isohypse(pN, elevation, valley, fileSecID);
        isoList.at(sort_temp)->append(newItem);
        // qDebug("Isohypse added: Size=%d, elevation=%d, valley=%d, fileTypeID=%X",
        //       pN.size(), elevation, valley ? 1 : 0, fileTypeID );
      }
    }

    // AP: Performance brake! emit progress calls waitscreen and
    // this steps into main loop
    if ( compiling && (++loop % 100) == 0 ) {
      emit progress(2);
    }
  }

  // qDebug("loop=%d", loop);
  mapfile.close();

  if( compiling ) {
    ausgabe.close();

    // kfl file is deleted after 'compilation' to save space, if is enabled
    // in configuration menu
    if ( _globalMapConfig->getDeleteMapfileAfterCompile() ) {
      mapfile.remove();
    }
  }

  ws->slot_SetText2(tr("Loading map ready"));
  return true;
}

bool MapContents::__readBinaryFile(const int fileSecID,
                                   const char fileTypeID)
{
  extern const MapMatrix * _globalMapMatrix;
  extern MapConfig * _globalMapConfig;
  bool kflExists, kfcExists;
  bool compiling = false;

  //first, check if we need to load this file at all...
  if ((fileTypeID==FILE_TYPE_TERRAIN) && (!_globalMapConfig->getLoadIsolines()))
    return true;

  if (memoryFull) { //if we allready know the memory if full and can't be emptied at this point, just return.
    _globalMapView->message(tr("Out of memory! Map not loaded."));
    return false;
  }

  //check free memory
  int memFree = HwInfo::instance()->getFreeMemory();

  if( memFree < MINIMUM_FREE_MEMORY ) {
    if ( !unloadDone) {
      unloadMaps();  //try freeing some memory
      memFree = HwInfo::instance()->getFreeMemory();  //re-asses free memory
      if( memFree < MINIMUM_FREE_MEMORY ) {
        memoryFull=true; //set flag to indicate that we need not try loading any more mapfiles now.
        qWarning("Cumulus couldn't load file, low on memory! Memory needed: %d kB, free: %d kB", MINIMUM_FREE_MEMORY, memFree );
        _globalMapView->message(tr("Out of memory! Map not loaded."));
        return false;
      }
    } else {
      memoryFull=true; //set flag to indicate that we need not try loading any more mapfiles now.
      qWarning("Cumulus couldn't load file, low on memory! Memory needed: %d kB, free: %d kB", MINIMUM_FREE_MEMORY, memFree );
      _globalMapView->message(tr("Out of memory! Map not loaded."));
      return false;
    }
  }

  QString kflPathName, kfcPathName, pathName;
  QString kflName, kfcName;

  kflName.sprintf("landscape/%c_%.5d.kfl", fileTypeID, fileSecID);
  kflExists = locateFile(kflName, kflPathName);

  kfcName.sprintf("landscape/%c_%.5d.kfc", fileTypeID, fileSecID);
  kfcExists = locateFile(kfcName, kfcPathName);

  if ( ! (kflExists || kfcExists) ) {
    qWarning( "Cumulus: no map files (%s/%s) found! Please install %s.",
              kflName.toLatin1().data(), kfcName.toLatin1().data(), kflName.toLatin1().data() );
    return false; // file could not be located in any of the possible map directories.
  }

  if ( kflExists ) {
    if ( kfcExists )
      // kfl file newer than kfc ? Then compile it
      {
        if ( getDateFromMapFile( kflPathName ) > getDateFromMapFile( kfcPathName ) )
          {
            compiling = true;
            qDebug("Map file %s has a newer date! Recompiling it from source.",
                   kflPathName.toLatin1().data() );
          }
      }
    else {
      // no kfc file, we compile anyway
      compiling = true;
    }
  }

  // what file do we read after all ?
  if ( compiling ) {
    pathName = kflPathName;
    kfcPathName = kflPathName;
    kfcPathName.replace( kfcPathName.length()-1, 1, QString("c") );
  } else {
    pathName = kfcPathName;
    kflPathName = kfcPathName;
    kflPathName.replace( kflPathName.length()-1, 1, QString("l") );
  }

  QFile mapfile(pathName);

  if(!mapfile.open(QIODevice::ReadOnly)) {
    if( ! compiling && kflExists ) {
      qDebug("Cumulus: Can't open map file %s for reading!"
             " Try to use file %s",
             pathName.toLatin1().data(), kflPathName.toLatin1().data());
      // try to remove unopenable file, not sure if this works.
      unlink( pathName.toLatin1().data() );
      return __readBinaryFile( fileSecID, fileTypeID );
    }

    qWarning("Cumulus: Can't open map file %s for reading! Arborting ...",
             pathName.toLatin1().data() );
    return false;
  }

  emit loadingFile(pathName);

  QDataStream in(&mapfile);
  in.setVersion(QDataStream::Qt_2_0);

  // qDebug("reading file %s", pathName.toLatin1().data());

  qint8 loadTypeID;
  quint16 loadSecID, formatID;
  quint32 magic;
  QDateTime createDateTime;
  ProjectionBase *projectionFromFile = 0;

  in >> magic;

  if( magic != KFLOG_FILE_MAGIC ) {
    if( ! compiling && kflExists ) {
      qWarning("Cumulus: wrong magic key %x read!\n Retry to compile %s.",
               magic, kflPathName.toLatin1().data());
      mapfile.close();
      unlink( pathName.toLatin1().data() );
      return __readBinaryFile( fileSecID, fileTypeID );
    }

    qWarning("Cumulus: wrong magic key %x read! Aborting ...", magic);
    mapfile.close();
    return false;
  }

  in >> loadTypeID;

  /** Originally, the binary files were mend to come in different flavours.
   * Now, they are all of type 'm'. Use that fact to do check for the
   * compiled or the uncompiled version. */
  if (compiling) {
    // uncompiled maps have a different format identifier than compiled
    // maps
    if(loadTypeID != FILE_TYPE_MAP) {
      qWarning("Cumulus: wrong load type identifier %x read! Aborting ...",
               loadTypeID );
      mapfile.close();
      return false;
    }
  } else {
    if( loadTypeID != FILE_TYPE_MAP_C) // wrong type
      {
        mapfile.close();

        if( kflExists )
          {
            qWarning("Cumulus: wrong load type identifier %x read! "
                     "Retry to compile %s",
                     loadTypeID, kflPathName.toLatin1().data() );
            unlink( pathName.toLatin1().data() );
            return __readBinaryFile( fileSecID, fileTypeID );
          }

        qWarning("Cumulus: wrong load type identifier %x read! Aborting ...",
                 loadTypeID );
        return false;
      }
  }

  // check the version of the subtype. This can be different for the
  // compiled and the uncompiled version

  in >> formatID;

  QFileInfo fi( pathName );

  qDebug("Reading File=%s, Magic=%xh, TypeId=%xh, formatId=%d, Date=%s",
         fi.fileName().toLatin1().data(), magic, loadTypeID, formatID,
         createDateTime.toString().toLatin1().data() );

  if (compiling) {
    if( formatID < FILE_VERSION_MAP) {
      // to old ...
      qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
               "Aborting ...", formatID, FILE_VERSION_MAP );
      mapfile.close();
      return false;
    } else if(formatID > FILE_VERSION_MAP) {
      // to new ...
      qWarning("Cumulus: File format too new! (version %d, expecting: %d) "
               "Aborting ...", formatID, FILE_VERSION_MAP );
      mapfile.close();
      return false;
    }
  } else {
    if( formatID < FILE_VERSION_MAP_C) {
      // to old ...
      mapfile.close();

      if( kflExists ) {
        qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
                 "Retry to compile %s",
                 formatID, FILE_VERSION_MAP_C, kflPathName.toLatin1().data() );
        unlink( pathName.toLatin1().data() );
        return __readBinaryFile( fileSecID, fileTypeID );
      }

      qWarning("Cumulus: File format too old! (version %d, expecting: %d) "
               "Aborting ...", formatID, FILE_VERSION_MAP_C );
      return false;
    } else if(formatID > FILE_VERSION_MAP_C) {
      // to new ...
      mapfile.close();

      if( kflExists ) {
        qWarning( "Cumulus: File format too new! (version %d, expecting: %d) "
                  "Retry to compile %s",
                  formatID, FILE_VERSION_MAP_C, kflPathName.toLatin1().data() );
        unlink( pathName.toLatin1().data() );
        return __readBinaryFile( fileSecID, fileTypeID );
      }

      qWarning("Cumulus: File format too new! (version %d, expecting: %d) "
               "Aborting ...", formatID, FILE_VERSION_MAP_C );

      return false;
    }
  }

  // check if this section really covers the area we want to deal with

  in >> loadSecID;

  if ( loadSecID != fileSecID ) {
    mapfile.close();

    if( ! compiling && kflExists ) {
      qWarning( "Cumulus: %s: wrong section, bogus file name!"
                "\n Retry to compile %s",
                pathName.toLatin1().data(), kflPathName.toLatin1().data() );
      unlink( pathName.toLatin1().data() );
      return __readBinaryFile( fileSecID, fileTypeID );
    }

    qWarning("Cumulus: %s: wrong section, bogus file name! Aborting ...",
             pathName.toLatin1().data() );
    return false;
  }

  in >> createDateTime;

  if( ! compiling ) {
    // check projection parameters from file against current used values
    projectionFromFile = LoadProjection(in);
    ProjectionBase *currentProjection = _globalMapMatrix->getProjection();

    if( ! compareProjections( projectionFromFile, currentProjection ) ) {
      delete projectionFromFile;
      mapfile.close();

      if( kflExists ) {
        qWarning( "Cumulus: %s, can't use file, compiled for another projection!"
                  "\n Retry to compile %s",
                  pathName.toLatin1().data(), kflPathName.toLatin1().data() );

        unlink( pathName.toLatin1().data() );
        return __readBinaryFile( fileSecID, fileTypeID );
      }

      qWarning( "Cumulus: %s, can't use file, compiled for another projection!"
                " Please install %s file and restart.",
                pathName.toLatin1().data(), kflPathName.toLatin1().data() );
      return false;
    } else {
      // Must be deleted after use to avoid memory leak
      delete projectionFromFile;
    }
  }

  QFile ausgabe(kfcPathName);
  QDataStream out(&ausgabe);

  if ( compiling ) {
    out.setDevice(&ausgabe);
    out.setVersion(QDataStream::Qt_2_0);
    if(!ausgabe.open(QIODevice::WriteOnly)) {
      qWarning("Cumulus: Can't open compiled map file %s for writing!"
               " Arborting ...",
               kfcPathName.toLatin1().data() );
      mapfile.close();
      return false;
    }

    qDebug("writing file %s", kfcPathName.toLatin1().data());

    out << magic;
    loadTypeID=FILE_TYPE_MAP_C;
    formatID=  FILE_VERSION_MAP_C;
    out << loadTypeID;
    out << formatID;
    out << loadSecID;
    out << createDateTime.addSecs(1);   //set time one second later than the time of the original file;
    SaveProjection(out, _globalMapMatrix->getProjection());
  }

  quint8 lm_typ;
  qint8 sort, elev;
  qint32 lat_temp, lon_temp;
  quint32 locLength = 0;
  QString name = "";


  unsigned int gesamt_elemente = 0;
  //  unsigned int river = 0;
  //  unsigned int rivert = 0;

  uint loop = 0;

  while( ! in.atEnd() ) {
    BaseMapElement::objectType typeIn = BaseMapElement::NotSelected;
    in >> (quint8&)typeIn;
    if ( compiling )
      out << (quint8&)typeIn;

    locLength = 0;
    name = "";

    QPolygon pN;
    QPoint single;

    gesamt_elemente++;

    switch (typeIn) {
    case BaseMapElement::Highway:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadHighways())
          break;
      highwayList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Road:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadRoads())
          break;
      roadList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Trail:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadRoads())
          break;
      roadList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Railway:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadRailroads())
          break;
      railList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Railway_D:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadRailroads())
          break;
      railList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Aerial_Cable:
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadRailroads())
          break;
      railList.append(new LineElement("", typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::Canal:
    case BaseMapElement::River:
    case BaseMapElement::River_T:
      typeIn=BaseMapElement::River; //don't use the River_T type internally
      if(formatID >= FILE_FORMAT_ID) {
        if ( compiling ) {
          in >> name;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadWaterways())
          break;
      hydroList.append(new LineElement(name, typeIn, pN, false, fileSecID));
      break;
    case BaseMapElement::City:
      in >> sort;
      if ( compiling )
        out << sort;
      if(formatID >= FILE_FORMAT_ID) {
        if ( compiling ) {
          in >> name;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadCities())
          break;
      cityList.append(new LineElement(name, typeIn, pN, sort, fileSecID));
      // qDebug("added city '%s'", name.toLatin1().data());
      break;
    case BaseMapElement::Lake:
    case BaseMapElement::Lake_T:
      typeIn=BaseMapElement::Lake; //don't use the Lake_T type internally
      in >> sort;
      if ( compiling )
        out << sort;
      if(formatID >= FILE_FORMAT_ID) {
        if ( compiling ) {
          in >> name;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      READ_POINT_LIST
        lakeList.append(new LineElement(name, typeIn, pN, sort, fileSecID));
      // qDebug("appended lake, name '%s', pointcount %d", name.toLatin1().data(), pN.count());
      break;
    case BaseMapElement::Forest:
    case BaseMapElement::Glacier:
    case BaseMapElement::PackIce:
      in >> sort;
      if ( compiling )
        out << sort;
      if(formatID >= FILE_FORMAT_ID) {
        if ( compiling ) {
          in >> name;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      READ_POINT_LIST
        if (!_globalMapConfig->getLoadForests())
          break;
      topoList.append(new LineElement(name, typeIn, pN, sort, fileSecID));
      break;
    case BaseMapElement::Village:
      if(formatID >= FILE_FORMAT_ID) {
        if ( compiling ) {
          in >> name;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      in >> lat_temp;
      in >> lon_temp;
      if (!_globalMapConfig->getLoadCities())
        break;
      if ( compiling ) {
        single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);
        out << single;
      } else
        in >> single;
      villageList.append(new SinglePoint(name, "", typeIn,
                                         WGSPoint(lat_temp, lon_temp), single, 0, fileSecID));
      // qDebug("added village '%s'", name.toLatin1().data());
      break;
    case BaseMapElement::Spot:
      if(formatID >= FILE_FORMAT_ID) {
        in >> elev;
        if ( compiling )
          out << elev;
      }
      in >> lat_temp;
      in >> lon_temp;
      if (!_globalMapConfig->getLoadCities())
        break;
      if ( compiling ) {
        single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);
        out << single;
      } else
        in >> single;
      obstacleList.append(new SinglePoint("Spot", "", typeIn,
                                          WGSPoint(lat_temp, lon_temp), single, 0, fileSecID));
      break;
    case BaseMapElement::Landmark:
      if(formatID >= FILE_FORMAT_ID) {
        in >> lm_typ;
        if ( compiling ) {
          in >> name;
          out << lm_typ;
          ShortSave(out, name);
        } else {
          ShortLoad(in, name);
        }
      }
      in >> lat_temp;
      in >> lon_temp;
      if (!_globalMapConfig->getLoadCities())
        break;
      if ( compiling ) {
        single = _globalMapMatrix->wgsToMap(lat_temp, lon_temp);
        out << single;
      } else
        in >> single;
      landmarkList.append(new SinglePoint(name, "", typeIn,
                                          WGSPoint(lat_temp, lon_temp), single, 0, fileSecID));
      // qDebug("added landmark '%s'", name.toLatin1().data());
      break;
    default:
      qWarning ("MapContents::__readBinaryFile; type not handled in switch: %d", typeIn);
    }

    // @AP: Performance brake! emit progress calls waitscreen and
    // this steps into main loop
    if ( compiling && (++loop % 100) == 0 ) {
      emit progress(2);
    }
  }

  // qDebug("loop=%d", loop);
  mapfile.close();

  if ( compiling ) {
    ausgabe.close();
    // kfl file is deleted after 'compilation' to save space. Please handle this "al gusto" ...
    if ( _globalMapConfig->getDeleteMapfileAfterCompile() )
      mapfile.remove();
  }

  ws->slot_SetText2(tr("Loading map ready"));
  return true;
}

void MapContents::proofeSection()
{
  // qDebug("MapContents::proofeSection()");

  // @AP: defined a static mutex variable, to prevent the recursive
  // calling of this method

  static bool mutex = false;

  if( mutex ) {
    // qDebug("MapContents::proofeSection(): is recursive called, returning");
    return; // return immediately, if reentry in method is not possible
  }

  mutex = true;

  extern MapMatrix * _globalMapMatrix;
  extern MapConfig * _globalMapConfig;
  QRect mapBorder;
  // emit majorAction(tr("Loading maps..."));
  ///////////////////////////////////////////////////////////////////////////
  //mapDir1 = QDir::homeDirPath()+"/Applications/cumulus/maps"; //subdir of homedir
  //mapDir2 = "/mnt/cf/maps";    //compact flash card
  //mapDir3 = "/mnt/card/maps";  //secure digital card
  //////////////////////////////////////////////////////////////////////////

    mapBorder = _globalMapMatrix->getViewBorder();

    int westCorner = ( ( mapBorder.left() / 600000 / 2 ) * 2 + 180 ) / 2;
    int eastCorner = ( ( mapBorder.right() / 600000 / 2 ) * 2 + 180 ) / 2;
    int northCorner = ( ( mapBorder.top() / 600000 / 2 ) * 2 - 88 ) / -2;
    int southCorner = ( ( mapBorder.bottom() / 600000 / 2 ) * 2 - 88 ) / -2;

    if(mapBorder.left() < 0)
      westCorner -= 1;
    if(mapBorder.right() < 0)
      eastCorner -= 1;
    if(mapBorder.top() < 0)
      northCorner += 1;
    if(mapBorder.bottom() < 0)
      southCorner += 1;

    if(isFirst) {
      ws->slot_SetText1(tr("Loading maps..."));
      ws->slot_SetText2(tr("Reading OpenAir Files"));

      OpenAirParser oap;
      oap.load( airspaceList );

      //finally, sort the airspaces
      airspaceList.sort();

      ws->slot_SetText2(tr("Reading Welt 2000 File"));
      // @AP: Look for and if available load a welt2000 airfield file
      Welt2000 welt2000;
      welt2000.load( airportList, gliderSiteList );
    }

    unloadDone = false;
    memoryFull = false;
    char step, hasstep; //used as small integers
    TilePartMap::Iterator it;

    for(int row = northCorner; row <= southCorner; row++) {
      for(int col = westCorner; col <= eastCorner; col++) {
        int secID=row + ( col + ( row * 179 ) );

        if( isFirst ) {
          // Animate a little bit during first load. later on in flight,
          // we need the time for gps processing.
          emit progress(2);
        }

        if(0<=secID & secID <=16200) {
          if( !sectionArray.testBit( secID ) ) {
            // Tile is missing
            if( isFirst ) {
              ws->slot_SetText1(tr("Loading maps..."));
            } else {
              _globalMapView->message(tr("Loading maps..."));
              // @AP: remove of all unused maps to get place
              // in heap. That can be disabled here because
              // the loading routines will also check the
              // available memory and call the unloadMaps()
              // method is necessary. But the disadvantage
              // is in that case that the freeing needs a
              // lot of time (several seconds).

              if (_globalMapConfig->getUnloadUnneededMap()) {
                unloadMaps(0);

              }
            }

            // qDebug("Going to load section %d", secID);

            step=0;
            //check to see if parts of this tile has already been loaded before
            it=tilePartMap.find(secID);
            if (it==tilePartMap.end()) { //not found
              hasstep=0;
            } else {
              hasstep=it.value();
            }

            //try loading the currently unloaded files
            if (!(hasstep & 1)) {
              if (__readTerrainFile(secID, FILE_TYPE_GROUND))
                step|=1;

            }

            if (!(hasstep & 2)) {
              if (__readTerrainFile(secID, FILE_TYPE_TERRAIN))
                step|=2;
            }

            if (!(hasstep & 4)) {
              if (__readBinaryFile(secID, FILE_TYPE_MAP))
                step|=4;
            }

            //set the correct flags for this maptile
            if (step==7) {
              sectionArray.setBit( secID, true );
              tilePartMap.remove(secID); //make sure we don't leave
            } else {
              if (step > 0) {
                tilePartMap.insert(secID, step);
              }
            }
          }
        }
      }
    }

    if( isFirst ) {
      ws->slot_SetText1(tr("Loading maps done"));
    } else {
      _globalMapView->message( tr("Loading maps done") );
    }

    isFirst = false;
    mutex = false; // unlock mutex
}


// Distance unit is expected as meters.
void MapContents::unloadMaps(unsigned int distance)
{
  // qDebug("MapContents::unloadMaps() is called");

  if (unloadDone)
    return;  //we only unload mapdata once (per mapredrawing round)

  extern MapMatrix * _globalMapMatrix;
  QRect mapBorder;

  mapBorder = _globalMapMatrix->getViewBorder();

  // scale uses unit meter/pixel
  double scale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  int width  = (int) rint(scale * distance);
  int height = width;

  //   qDebug("left=%d, right=%d, top=%d, bottom=%d, wight=%d, height=%d, scale=%f",
  //   mapBorder.left(), mapBorder.right(), mapBorder.top(), mapBorder.bottom(),
  //   width, height, scale );

  int westCorner = ( ( ( mapBorder.left() - width ) / 600000 / 2 ) * 2 + 180 ) / 2;
  int eastCorner = ( ( ( mapBorder.right() + width ) / 600000 / 2 ) * 2 + 180 ) / 2;
  int northCorner = ( ( ( mapBorder.top() - height ) / 600000 / 2 ) * 2 - 88 ) / -2;
  int southCorner = ( ( ( mapBorder.bottom() + height ) / 600000 / 2 ) * 2 - 88 ) / -2;

  if(mapBorder.left() < 0)
    westCorner -= 1;
  if(mapBorder.right() < 0)
    eastCorner -= 1;
  if(mapBorder.top() < 0)
    northCorner += 1;
  if(mapBorder.bottom() < 0)
    southCorner += 1;

  QBitArray maskArray(MAX_FILE_COUNT);
  maskArray.fill(false);

  for(int row = northCorner; row <= southCorner; row++) {
    for(int col = westCorner; col <= eastCorner; col++) {
      int secID=row + ( col + ( row * 179 ) );
      if(0<=secID && secID <=MAX_FILE_COUNT) {
        // qDebug( "unloadMaps-active-secId=%d",  secID );
        maskArray.setBit( secID, true );
      }
    }
  }

  // @AP: check, if something is to free, otherwise we can return to spare
  // processing time

  bool something2free = false;

  for( int i=0; i < maskArray.size(); i++ ) {
    if( sectionArray.testBit(i) != maskArray.testBit(i) ) {
      tilePartMap.remove(i); // reset map tile parts
      something2free = true;
      continue;
    }
  }

  if( ! something2free ) {
    return;
  }

  sectionArray &= maskArray;

#ifdef DEBUG_UNLOAD_SUM
  // save free memory
  int memFreeBegin = HwInfo::instance()->getFreeMemory();
#endif

  QTime t;
  uint sum = 0;
  t.start();

  unloadMapObjects(&cityList);
  sum += t.elapsed();

#ifdef DEBUG_UNLOAD

  qDebug("Unload cityList(%d), elapsed=%d", cityList.count(), t.restart());
#endif

  unloadMapObjects(&hydroList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload hydroList(%d), elapsed=%d", hydroList.count(), t.restart());
#endif

  unloadMapObjects(&lakeList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload lakeList(%d), elapsed=%d", lakeList.count(), t.restart());
#endif

  unloadMapObjects(&isoList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload isoList(%d), elapsed=%d", isoList.count(), t.restart());
#endif

  unloadMapObjects(&landmarkList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload landmarkList(%d), elapsed=%d", landmarkList.count(), t.restart());
#endif

  unloadMapObjects(&navList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload navList(%d), elapsed=%d", navList.count(), t.restart());
#endif

  unloadMapObjects(&obstacleList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload obstacleList(%d), elapsed=%d", obstacleList.count(), t.restart());
#endif
  //  unloadMapObjects(&outList);

  unloadMapObjects(&railList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload railList(%d), elapsed=%d", railList.count(), t.restart());
#endif

  unloadMapObjects(&reportList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload reportList(%d), elapsed=%d", reportList.count(), t.restart());
#endif

  unloadMapObjects(&highwayList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload highwayList(%d), elapsed=%d", highwayList.count(), t.restart());
#endif

  unloadMapObjects(&roadList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload roadList(%d), elapsed=%d", roadList.count(), t.restart());
#endif

  unloadMapObjects(&topoList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload topoList(%d), elapsed=%d", topoList.count(), t.restart());
#endif

  unloadMapObjects(&villageList);
  sum += t.elapsed();
#ifdef DEBUG_UNLOAD

  qDebug("Unload villageList(%d), elapsed=%d", villageList.count(), t.restart());
#endif

  unloadDone=true;

#ifdef DEBUG_UNLOAD_SUM
  // save free memory
  int memFreeEnd = HwInfo::instance()->getFreeMemory();

  // qDebug("Unloaded unneeded map elements. Elapsed Time=%dms, MemStart=%dKB, MemEnd=%dKB, SEDelta=%dKB", sum, memFreeBegin, memFreeEnd, memFreeBegin-memFreeEnd );
#endif

}


void MapContents::unloadMapObjects(QList<LineElement*> * list)
{
  for (int i=list->count()-1; i>=0;i--) {
    if (!sectionArray[list->at(i)->getMapSegment()]) {
      list->removeAt(i);
    }
  }
}


void MapContents::unloadMapObjects(QList<SinglePoint*> * list)
{
  for (int i=list->count()-1; i>=0;i--) {
    if (!sectionArray[list->at(i)->getMapSegment()]) {
      list->removeAt(i);
    }
  }
}


void MapContents::unloadMapObjects(QList<RadioPoint*> * list)
{
  for (int i=list->count()-1; i>=0;i--) {
    if (!sectionArray[list->at(i)->getMapSegment()]) {
      list->removeAt(i);
    }
  }
}


void MapContents::unloadMapObjects(QList< QList<Isohypse*>*>* list)
{
  for (int i=list->count()-1; i>=0;i--) {
    for (int j=list->at(i)->count()-1; j>=0;j--) {
      if (!sectionArray[list->at(i)->at(j)->getMapSegment()]) {
        list->at(i)->removeAt(j);
      }
    }
  }
}


unsigned int MapContents::getListLength(int listIndex) const
{
  switch(listIndex) {
  case AirportList:
    return airportList.count();
  case GliderSiteList:
    return gliderSiteList.count();
  case OutList:
    return outList.count();
  case NavList:
    return navList.count();
  case AirspaceList:
    return airspaceList.count();
  case ObstacleList:
    return obstacleList.count();
  case ReportList:
    return reportList.count();
  case CityList:
    return cityList.count();
  case VillageList:
    return villageList.count();
  case LandmarkList:
    return landmarkList.count();
  case HighwayList:
    return highwayList.count();
  case RoadList:
    return roadList.count();
  case RailList:
    return railList.count();
    //    case StationList:
    //      return stationList.count();
  case HydroList:
    return hydroList.count();
  case LakeList:
    return lakeList.count();
  case TopoList:
    return topoList.count();
  default:
    return 0;
  }
}


Airspace* MapContents::getAirspace(unsigned int index)
{
  return (Airspace*)airspaceList.at(index);
}


Airport* MapContents::getAirport(unsigned int index)
{
  return (Airport*)airportList.at(index);
}


GliderSite* MapContents::getGlidersite(unsigned int index)
{
  return (GliderSite*)gliderSiteList.at(index);
}


BaseMapElement* MapContents::getElement(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return airportList.at(index);
  case GliderSiteList:
    return gliderSiteList.at(index);
  case OutList:
    return outList.at(index);
  case NavList:
    return navList.at(index);
  case AirspaceList:
    return airspaceList.at(index);
  case ObstacleList:
    return obstacleList.at(index);
  case ReportList:
    return reportList.at(index);
  case CityList:
    return cityList.at(index);
  case VillageList:
    return villageList.at(index);
  case LandmarkList:
    return landmarkList.at(index);
  case HighwayList:
    return highwayList.at(index);
  case RoadList:
    return roadList.at(index);
  case RailList:
    return railList.at(index);
    //    case StationList:
    //      return stationList.at(index);
  case HydroList:
    return hydroList.at(index);
  case LakeList:
    return lakeList.at(index);
  case TopoList:
    return topoList.at(index);
  default:
    // Should never happen!
    qCritical("Cumulus: trying to access unknown map element list");
    return 0;
  }
}


SinglePoint* MapContents::getSinglePoint(int listIndex, unsigned int index)
{
  switch(listIndex) {
  case AirportList:
    return (SinglePoint*)airportList.at(index);
  case GliderSiteList:
    return (SinglePoint*)gliderSiteList.at(index);
  case OutList:
    return (SinglePoint*)outList.at(index);
  case NavList:
    return (SinglePoint*)navList.at(index);
  case ObstacleList:
    return obstacleList.at(index);
  case ReportList:
    return reportList.at(index);
  case VillageList:
    return villageList.at(index);
  case LandmarkList:
    return landmarkList.at(index);
    //      case StationList:
    //        return stationList.at(index);
  default:
    return 0;
  }
}


void MapContents::slotReloadMapData()
{
  // @AP: defined a static mutex variable, to prevent the recursive
  // calling of this method

  static bool mutex = false;

  if( mutex ) {
    // qDebug("MapContents::slotReloadMapData(): mutex is locked, returning");
    return; // return immediately, if reentry in method is not possible
  }

  mutex = true;

  // We must block all gps signals during the reload time to avoid
  // system crash due to outdated data.
  gps->blockSignals( true );

  // qDebug("MapContents::slotReloadMapData() is called");

  qDeleteAll(addSitesList); addSitesList.clear();
  qDeleteAll(airportList); airportList.clear();
  qDeleteAll(airspaceList); airspaceList.clear();
  qDeleteAll(cityList); cityList.clear();
  qDeleteAll(gliderSiteList); gliderSiteList.clear();
  qDeleteAll(hydroList); hydroList.clear();
  qDeleteAll(lakeList); lakeList.clear();
  qDeleteAll(landmarkList); landmarkList.clear();
  qDeleteAll(navList); navList.clear();
  qDeleteAll(obstacleList); obstacleList.clear();
  qDeleteAll(outList); outList.clear();
  qDeleteAll(railList); railList.clear();
  qDeleteAll(reportList); reportList.clear();
  qDeleteAll(highwayList); highwayList.clear();
  qDeleteAll(roadList); roadList.clear();
  //  qDeleteAll(stationList); stationList.clear();
  qDeleteAll(topoList); topoList.clear();
  qDeleteAll(villageList); villageList.clear();

  // isoList is a special pointer list
  for( int i=isoList.count() - 1; i >= 0; i--)
    {
      qDeleteAll(*isoList.at(i));
      isoList.at(i)->clear();
    }

  qDeleteAll (isoList); isoList.clear();
  
  // Wir nehmen zunaechst 4 Schachtelungstiefen an ...
  for(int loop = 0; loop < ( ISO_LINE_NUM * 4 ); loop++) {
    QList<Isohypse*> *list = new QList<Isohypse*>;
    isoList.append(list);
  }

  sectionArray.fill(false);
  tilePartMap.clear();

  isFirst = true;

  // @AP: Reload all data, that must be done after a projection value change
  proofeSection();

  // Check for a selected waypoint, this one must be also new
  // projected.
  extern CuCalc      *calculator;
  extern MapContents *_globalMapContents;
  extern MapMatrix   *_globalMapMatrix;

  wayPoint *wp = (wayPoint *) calculator->getselectedWp();

  if( wp )
    {
      wp->projP = _globalMapMatrix->wgsToMap(wp->origP);
    }

  // Update the waypoint list
  for( int loop = 0; loop < wpList.count(); loop++ )
    {
      // recalculate projection data
      wpList.at(loop)->projP = _globalMapMatrix->wgsToMap(wpList.at(loop)->origP);
    }

  // Check for a flight task, the waypoint list must be updated too
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( task != 0 )
    {
      task->updateProjection();
    }

  emit mapDataReloaded();

  // enable gps data receiving
  gps->ignoreConnectionLost();
  gps->blockSignals( false );

  mutex = false; // unlock mutex
}

/** Reload welt 2000 data file. Can be called after a configuration
    change. */
void MapContents::slotReloadWelt2000Data()
{
  // @AP: defined a static mutex variable, to prevent the recursive
  // calling of this method

  static bool mutex = false;

  if( mutex ) {
    qDebug("MapContents::slotReloadWelt2000Data(): mutex is locked, returning");
    return; // return immediately, if reentry in method is not possible
  }

  mutex = true;

  // We must block all gps signals during the reload time to avoid
  // system crash due to outdated data.
  gps->blockSignals( true );

  // clear event queue
  qDebug("========= MapContents::slotReloadWelt2000Data() calls processEvents =========");
  QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

  qDeleteAll(airportList); airportList.clear();
  qDeleteAll(gliderSiteList);  gliderSiteList.clear();

  _globalMapView->message( tr("Reloading Welt2000 started") );

  Welt2000 welt2000;
  welt2000.load( airportList, gliderSiteList );

  _globalMapView->message( tr("Reloading Welt2000 finished") );

  emit mapDataReloaded();

  // enable gps data receiving
  gps->ignoreConnectionLost();
  gps->blockSignals( false );

  mutex = false; // unlock mutex
}


void MapContents::printContents(QPainter* targetPainter, bool isText)
{
  proofeSection();

  for (int i = 0; i < topoList.size(); i++)
    topoList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < hydroList.size(); i++)
    hydroList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < lakeList.size(); i++)
    lakeList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < railList.size(); i++)
    railList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < highwayList.size(); i++)
    highwayList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < roadList.size(); i++)
    roadList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < cityList.size(); i++)
    cityList.at(i)->printMapElement(targetPainter, isText);

  //  for (int i = 0; i < villageList.size(); i++)
  //      villageList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < navList.size(); i++)
    navList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < airspaceList.size(); i++)
    airspaceList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < obstacleList.size(); i++)
    obstacleList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < reportList.size(); i++)
    reportList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < landmarkList.size(); i++)
    landmarkList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < airportList.size(); i++)
    airportList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < gliderSiteList.size(); i++)
    gliderSiteList.at(i)->printMapElement(targetPainter, isText);

  for (int i = 0; i < outList.size(); i++)
    outList.at(i)->printMapElement(targetPainter, isText);
}


void MapContents::drawList(QPainter* targetPainter,
                           unsigned int listID)
{
  //const char *list="";
  //uint len = 0;

  //QTime t;
  //t.start();

  switch(listID) {
  case AirportList:
    //list="AirportList";
    //len=airportList.count();
    showProgress2WaitScreen( tr("Drawing airports") );
    for (int i = 0; i < airportList.size(); i++)
      airportList.at(i)->drawMapElement(targetPainter);
    break;

  case GliderSiteList:
    //list="GliderList";
    //len=gliderSiteList.count();
    showProgress2WaitScreen( tr("Drawing gilder sites") );
    for (int i = 0; i < gliderSiteList.size(); i++)
      gliderSiteList.at(i)->drawMapElement(targetPainter);
    break;

  case OutList:
    //list="OutList";
    //len=outList.count();
    showProgress2WaitScreen( tr("Drawing outlanding sites") );
    for (int i = 0; i < outList.size(); i++)
      outList.at(i)->drawMapElement(targetPainter);
    break;

  case NavList:
    //list="NavList";
    //len=navList.count();
    showProgress2WaitScreen( tr("Drawing navigation elements") );
    for (int i = 0; i < navList.size(); i++)
      navList.at(i)->drawMapElement(targetPainter);
    break;

  case AirspaceList:
    //list="AirspaceList";
    //len=airspaceList.count();
    showProgress2WaitScreen( tr("Drawing airspaces") );
    for (int i = 0; i < airspaceList.size(); i++)
      airspaceList.at(i)->drawMapElement(targetPainter);
    break;

  case ObstacleList:
    //list="ObstacleList";
    //len=obstacleList.count();
    showProgress2WaitScreen( tr("Drawing obstacles") );
    for (int i = 0; i < obstacleList.size(); i++)
      obstacleList.at(i)->drawMapElement(targetPainter);
    break;

  case ReportList:
    //list="ReportList";
    //len=reportList.count();
    showProgress2WaitScreen( tr("Drawing reporting points") );
    for (int i = 0; i < reportList.size(); i++)
      reportList.at(i)->drawMapElement(targetPainter);
    break;

  case CityList:
    //list="CityList";
    //len=cityList.count();
    showProgress2WaitScreen( tr("Drawing cities") );
    for (int i = 0; i < cityList.size(); i++)
      cityList.at(i)->drawMapElement(targetPainter);
    break;

  case VillageList:
    //list="VillageList";
    showProgress2WaitScreen( tr("Drawing villages") );
    for (int i = 0; i < villageList.size(); i++)
      villageList.at(i)->drawMapElement(targetPainter);
    break;

  case LandmarkList:
    //list="LandmarkList";
    //len=landmarkList.count();
    showProgress2WaitScreen( tr("Drawing landmarks") );
    for (int i = 0; i < landmarkList.size(); i++)
      landmarkList.at(i)->drawMapElement(targetPainter);
    break;

  case HighwayList:
    //list="HighwayList";
    //len=highwayList.count();
    showProgress2WaitScreen( tr("Drawing highways") );
    for (int i = 0; i < highwayList.size(); i++)
      highwayList.at(i)->drawMapElement(targetPainter);
    break;

  case RoadList:
    //list="RoadList";
    //len=roadList.count();
    showProgress2WaitScreen( tr("Drawing roads") );
    for (int i = 0; i < roadList.size(); i++)
      roadList.at(i)->drawMapElement(targetPainter);
    break;

  case RailList:
    //list="RailList";
    //len=railList.count();
    showProgress2WaitScreen( tr("Drawing railroads") );
    for (int i = 0; i < railList.size(); i++)
      railList.at(i)->drawMapElement(targetPainter);
    break;

  case HydroList:
    //list="HydroList";
    //len=hydroList.count();
    showProgress2WaitScreen( tr("Drawing hydro") );
    for (int i = 0; i < hydroList.size(); i++)
      hydroList.at(i)->drawMapElement(targetPainter);
    break;

  case LakeList:
    //list="LakeList";
    //len=lakeList.count();
    showProgress2WaitScreen( tr("Drawing lakes") );
    for (int i = 0; i < lakeList.size(); i++)
      lakeList.at(i)->drawMapElement(targetPainter);
    break;

  case TopoList:
    //list="TopoList";
    //len=topoList.count();
    showProgress2WaitScreen( tr("Drawing topography") );
    for (int i = 0; i < topoList.size(); i++)
      topoList.at(i)->drawMapElement(targetPainter);
    break;

  default:
    qWarning("MapContents::drawList(): unknown listID %d", listID);
    return;
  }

  // qDebug( "List=%s, Length=%d, drawTime=%dms", list, len, t.elapsed() );
}


void MapContents::drawIsoList(QPainter* targetP)
{
  // qDebug("MapContents::drawIsoList():");

  QTime t;
  t.start();

  extern MapConfig * _globalMapConfig;
  extern MapMatrix * _globalMapMatrix;
  int height = 0;
  _lastIsoEntry=0;
  _isoLevelReset=true;
  qDeleteAll(regIsoLines); regIsoLines.clear();

  bool isolines = false;

  targetP->setPen(QPen(Qt::black, 1, Qt::NoPen));

  if (_globalMapConfig->getShowIsolineBorders())
    {
      int scale = (int)rint(_globalMapMatrix->getScale(MapMatrix::CurrentScale)); 

      if( scale < 160 )
        { // Draw Isolines at higher scales
          targetP->setPen(QPen(Qt::black, 1, Qt::DotLine));
          isolines = true;
        }
    }

  targetP->save();

  for (int i = 0; i < isoList.size(); i++)
  {
    showProgress2WaitScreen( tr("Drawing isolines") );
    QList<Isohypse*>* iso = isoList.at(i);

    if(iso->size() == 0)
      {
        continue;
      }

    Isohypse* first = iso->first();
    
    for(unsigned int pos = 0; pos < ISO_LINE_NUM; pos++)
      {
        if(isoLines[pos] == first->getElevation())
          {
            if(first->isValley())
              {
                height = pos + 1;
              }
            else
              {
                height = pos + 2;
              }
            
            break;
          }
      }

    if( _globalMapConfig->getdrawIsoLines() )
      {
        // choose iso color
        targetP->setBrush(QBrush(_globalMapConfig->getIsoColor(height), Qt::SolidPattern));
      }
    else
      {
        // Choose a brighter color, when isoline drawing is switched
        // off by the user to get a better readability.
        targetP->setBrush(QBrush(_globalMapConfig->getIsoColor(6), Qt::SolidPattern));
      }

      for (int j = 0; j < iso->size(); j++)
      {
        Isohypse* iso2 = iso->at(j);
        QRegion * reg = iso2->drawRegion(targetP, _globalMapView->rect(),
                                         true, isolines);
        if (reg)
          {
            // store drawn region in extra list for elevation finding
            IsoListEntry* entry = new IsoListEntry(reg, iso2->getElevation());
            regIsoLines.append(entry);
            //qDebug("  added Iso: %04x, %d", (int)reg, iso2->getElevation() );
          }
      }
  }

  targetP->restore();
  regIsoLines.sort();
  _isoLevelReset=false;

  qDebug( "IsoList, drawTime=%dms", t.elapsed() );

  /* QString isos;

  for( int i = 0; i < regIsoLines.count(); i++ )
    {
      isos += QString("%1, ").arg(regIsoLines.at(i)->height);
    }
  
    qDebug( isos.toLatin1().data() ); */
}

/**
 * shows a progress message at the wait screen, if it is visible
 */
void MapContents::showProgress2WaitScreen( QString message )
{
  if( ws && ws->isVisible() )
    {
      ws->slot_SetText1( message );
      ws->slot_Progress(1);
    }
}

/** This function checks all possible map directories for the map
    file. If found, it returns true and returns the complete path in
    pathName. */
bool MapContents::locateFile(const QString& fileName, QString& pathName)
{
  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      QFile test;

      test.setFileName( mapDirs.at(i) + "/" + fileName );

      if( test.exists() )
        {
          pathName=test.fileName();
          return true;
        }      
    }

  // lower case tests
  for( int i = 0; i < mapDirs.size(); ++i )
    {
      QFile test;

      test.setFileName( mapDirs.at(i) + "/" + fileName.toLower() );

      if( test.exists() )
        {
          pathName=test.fileName();
          return true;
        }      
    }

  // so, let's try upper case
  for( int i = 0; i < mapDirs.size(); ++i )
    {
      QFile test;

      test.setFileName( mapDirs.at(i) + "/" + fileName.toUpper() );

      if( test.exists() )
        {
          pathName=test.fileName();
          return true;
        }      
    }

  return false;
}


void MapContents::addDir (QStringList& list, const QString& _path, const QString& filter)
{
  //  qDebug ("addDir (%s, %s)", _path.toLatin1().data(), filter.toLatin1().data());
  QDir path (_path, filter);

  //JD was a bit annoyed by many notifications about nonexisting dirs
  if ( ! path.exists() )
    return;

  QStringList entries (path.entryList());

  for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) {
    bool found = false;
    // look for other entries with same filename
    for (QStringList::Iterator it2 =  list.begin(); it2 != list.end(); ++it2) {
      QFileInfo path2 (*it2);
      if (path2.fileName() == *it)
        found = true;
    }
    if (!found)
      list += path.absoluteFilePath (*it);
  }
  //  qDebug ("entries: %s", list.join(";").toLatin1().data());
}


/** Read property of FlightTask * currentTask. */
FlightTask* MapContents::getCurrentTask()
{
  return currentTask;
}


/** Write property of FlightTask * currentTask. */
void MapContents::setCurrentTask( FlightTask * _newVal)
{
  // an old task instance must be deleted
  if( currentTask != 0 ) {
    delete currentTask;
  }

  currentTask = _newVal;
}


/** Returns true if the coordinates of the waypoint in the argument matches one of the waypoints in the list. */
bool MapContents::getIsInWaypointList(const wayPoint * wp)
{
  int i,n;
  n =  wpList.count();
  wayPoint * wpi;

  for (i=0; i < n; i++) {
    wpi=(wayPoint*)wpList.at(i);
    if (wp->origP==wpi->origP)
      return true;
  }

  return false;
}


QDateTime MapContents::getDateFromMapFile( const QString& path )
{
  QDateTime createDateTime;
  QFile mapFile( path );
  if(!mapFile.open(QIODevice::ReadOnly)) {
    qWarning("Cumulus: can't open map file %s for reading date", path.toLatin1().data() );
    createDateTime.setDate( QDate(1900,1,1) );
    return createDateTime;
  }

  QDataStream in(&mapFile);
  in.setVersion(QDataStream::Qt_2_0);

  mapFile.seek( 9 );
  in >> createDateTime;
  mapFile.close();
  //qDebug("Map file %s created %s", path.toLatin1().data(), createDateTime.toString().toLatin1().data() );
  return createDateTime;
}


/** Add a point to a rectangle, so the rectangle will be the bounding box
 * of all points added to it. If the point allready lies within the borders
 * of the QRect, the QRect is unchanged. If the point is outside the
 * defined QRect, the QRox will be modified so the point lies inside the
 * new QRect. If the QRect is empty, the QRect will be set to a rect of
 * size (1,1) at the location of the point. */
void MapContents::AddPointToRect(QRect& rect, const QPoint& point)
{
  if (rect.isValid()) {
    rect.setCoords(
                   MIN(rect.left(), point.x()),
                   MIN(rect.top(), point.y()),
                   MAX(rect.right(), point.x()),
                   MAX(rect.bottom(), point.y()));
  } else {
    rect.setCoords(point.x(),point.y(),point.x(),point.y());
  }
}


/**
 * Compares two projection objects for equality.
 * @Returns true if equal; otherwise false
 */
bool MapContents::compareProjections(ProjectionBase* p1, ProjectionBase* p2)
{
  if( p1->projectionType() != p2->projectionType() ) {
    return false;
  }

  if( p1->projectionType() == ProjectionBase::Lambert ) {
    ProjectionLambert* l1 = (ProjectionLambert *) p1;
    ProjectionLambert* l2 = (ProjectionLambert *) p2;

    if( l1->getStandardParallel1() != l2->getStandardParallel1() ||
        l1->getStandardParallel2() != l2->getStandardParallel2() ||
        l1->getOrigin() != l2->getOrigin() ) {
      return false;
    }

    return true;
  }

  if( p1->projectionType() == ProjectionBase::Cylindric ) {
    ProjectionCylindric* c1 = (ProjectionCylindric*) p1;
    ProjectionCylindric* c2 = (ProjectionCylindric*) p2;

    if( c1->getStandardParallel() != c2->getStandardParallel() ) {
      return false;
    }

    return true;
  }

  // What's that? Det kennen wir noch nicht :( Rejection!

  return false;
}

int MapContents::findElevation(const QPoint& coordP, Distance * errorDist)
{
  extern MapMatrix * _globalMapMatrix;

  IsoListEntry* entry = 0;
  int height = 0;
  double error = 0.0;
    
  QPoint coordP1 = _globalMapMatrix->wgsToMap(coordP.x(), coordP.y());
  QPoint coord = _globalMapMatrix->map(coordP1);

  IsoList* list = getIsohypseRegions();

  // qDebug("list->count() %d", list->count() );
  //qDebug("==searching for elevation==");
  //qDebug("_lastIsoLevel %d, _nextIsoLevel %d",_lastIsoLevel, _nextIsoLevel);
  if (_isoLevelReset)
    {
      qDebug("findElevation: Busy rebuilding the isomap. Returning last known result...");
      return _lastIsoLevel;
    }

  int cnt = list->count();

  for( int i=0; i<cnt;i++ )
    {
      entry = list->at(i);
      // qDebug("i: %d entry->height %d contains %d",i,entry->height, entry->region->contains(coord) );
      // qDebug("Point x:%d y:%d", coord.x(), coord.y() );
      // qDebug("boundingRect l:%d r:%d t:%d b:%d", entry->region->boundingRect().left(),
      //                                 entry->region->boundingRect().right(),
      //                                 entry->region->boundingRect().top(),
      //                                 entry->region->boundingRect().bottom() );

      if (entry->height>height && /*there is no reason to search a lower level if we allready have a hit on a higher one*/
        entry->height <= _nextIsoLevel) /* since the odds of skipping a level between two fixes are not too high, we can ignore higher levels, making searching more efficient.*/
      {
        if (entry->height==_lastIsoLevel && _lastIsoEntry)
          {
            //qDebug("Trying previous entry...");
            if (_lastIsoEntry->region->contains(coord))
              {
                height=MAX(height,entry->height);
                //qDebug("Found on height %d",entry->height);
                break;
              }
          }

        if (entry == _lastIsoEntry)
          {
            continue; //we already tried this one, and it wasn't it.
          }

        //qDebug("Probing on height %d...", entry->height);

        if (entry->region->contains(coord))
          {
            height=MAX(height,entry->height);
            //qDebug("Found on height %d",entry->height);
            _lastIsoEntry=entry;
            break;
          }
      }
    }

  _lastIsoLevel=height;

  // The real altitude is between the current and the next
  // isolevel, therefore reduce error by taking the middle
  if( height <100 ) {
    _nextIsoLevel=height+25;
    height += 12;
    error=12.5;
  } else if( (height >=100) && (height < 500) ) {
    _nextIsoLevel=height+50;
    height += 25;
    error=25.0;
  } else if( (height >=500) && (height < 1000) ) {
    _nextIsoLevel=height+100;
    height += 50;
    error=50.0;
  } else {
    _nextIsoLevel=height+250;
    height += 125;
    error=125.0;
  }

  // if errorDist is set, set the correct error margin
  if (errorDist)
    {
      errorDist->setMeters(error);
    }

  return height;
}




