/***********************************************************************
 **
 **   waypointcatalog.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Harald Maier, 2002 André Somers,
 **                   2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QObject>
#include <QDataStream>
#include <QFile>
#include <QRegExp>
#include <QString>

#include "waypointcatalog.h"
#include "mapmatrix.h"
#include "generalconfig.h"

extern MapMatrix *_globalMapMatrix;

#define KFLOG_FILE_MAGIC 0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define WP_FILE_FORMAT_ID 100
#define WP_FILE_FORMAT_ID_2 101

WaypointCatalog::WaypointCatalog()
{
}

WaypointCatalog::~WaypointCatalog()
{}

/** read a catalog from file */
bool WaypointCatalog::read( QString *catalog, Q3PtrList<wayPoint> *wpList )
{
  QString fName;

  if( !catalog )
    {
      // use default file name
      fName = GeneralConfig::instance()->getWaypointFile();
    }
  else
    {
      fName = *catalog;
    }

  if( ! wpList )
    {
      qWarning("WaypointCatalog::read: Waypoint file is Null!");
      return false;
    }

  if( _globalMapMatrix == 0 )
    {
      qWarning("WaypointCatalog::read: Global pointer '_globalMapMatrix' is Null!");
      return false;      
    }

  bool ok = false;
  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  qint8 wpType;
  qint32 wpLatitude;
  qint32 wpLongitude;
  qint16 wpElevation;
  double wpFrequency=0.;
  qint8 wpLandable;
  qint16 wpRunway;
  qint16 wpLength;
  qint8 wpSurface;
  QString wpComment="";
  quint8 wpImportance;

  quint32 fileMagic;
  qint8 fileType;
  quint16 fileFormat;

  // qDebug("Read waypoint catalog from %s", fName.latin1() );

  // default file location: $HOME/cumulus/cumulus.kwp

  QFile f(fName);

  if (f.exists())
    {
      if (f.open(IO_ReadOnly))
        {

          QDataStream in(&f);
          in.setVersion(2);

          //check if the file has the correct format
          in >> fileMagic;
          if (fileMagic != KFLOG_FILE_MAGIC)
            {
              qWarning("Waypoint file not recognized as KFLog file type.");
              return false;
            }

          in >> fileType;
          if (fileType != FILE_TYPE_WAYPOINTS)
            {
              qWarning("Waypoint file is a KFLog file, but not for waypoints.");
              return false;
            }

          in >> fileFormat;
          if (fileFormat != WP_FILE_FORMAT_ID && fileFormat != WP_FILE_FORMAT_ID_2)
            {
              qWarning("Waypoint file does not have the correct format. It returned %d, where %d was expected.", fileFormat, WP_FILE_FORMAT_ID);
              return false;
            }
          //from here on, we assume that the file has the correct format.

          while(!in.atEnd())
            {
              // read values from file
              in >> wpName;
              in >> wpDescription;
              in >> wpICAO;
              in >> wpType;
              in >> wpLatitude;
              in >> wpLongitude;
              in >> wpElevation;
              in >> wpFrequency;
              in >> wpLandable;
              in >> wpRunway;
              in >> wpLength;
              in >> wpSurface;
              in >> wpComment;
              if (fileFormat>=WP_FILE_FORMAT_ID_2)
                {
                  in >> wpImportance;
                }
              else
                {
                  wpImportance=1; //normal importance
                };

              //create new waypoint object and set the correct properties
              wayPoint *w = new wayPoint;

              w->name = wpName;
              w->description = wpDescription;
              w->icao = wpICAO;
              w->type = wpType;
              w->origP.setLat(wpLatitude);
              w->origP.setLon(wpLongitude);
              w->projP = _globalMapMatrix->wgsToMap(w->origP);
              w->elevation = wpElevation;
              w->frequency = wpFrequency;
              w->isLandable = wpLandable;
              w->runway =wpRunway;
              w->length = wpLength;
              w->surface = wpSurface;
              w->comment = wpComment;
              w->importance = ( enum wayPoint::Importance ) wpImportance;
              // qDebug("Waypoint read: %s (%s - %s)",w->name.latin1(),w->description.latin1(),w->icao.latin1());

              wpList->append(w);

              GeneralConfig *conf = GeneralConfig::instance();

              if (conf->getHomeWp()->origP == w->origP)
                {
                  qDebug("Found homesite: %s",wpName.latin1());
                  wayPoint * wp=new wayPoint(*w); //copy
                  conf->setHomeWp(wp);
                }
            }

          ok = true;
        }
    }
  else
    {
      qWarning("WaypointCatalog::read(): Waypoint catalog not found.");
      return false;
    }

  qDebug("WaypointCatalog::read(): %d items read from %s",
         wpList->count(), fName.latin1() );

  return ok;
}


/** write a catalog to file */
bool WaypointCatalog::write( QString *catalog, Q3PtrList<wayPoint> *wpList )
{
  QString fName;

  if( !catalog )
    {
      fName = GeneralConfig::instance()->getWaypointFile(); // use default
    }
  else
    {
      fName = *catalog;
    }

  if( ! wpList )
    {
      qWarning("WaypointCatalog::write: Waypoint file is Null!");
      return false;
    }

  bool ok = true;
  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  qint8 wpType;
  qint32 wpLatitude;
  qint32 wpLongitude;
  qint16 wpElevation;
  double wpFrequency;
  qint8 wpLandable;
  qint16 wpRunway;
  qint16 wpLength;
  qint8 wpSurface;
  QString wpComment="";
  quint8 wpImportance;

  QFile f;

  f.setName(fName);

  if (f.open(IO_WriteOnly))
    {
      // qDebug("WaypointCatalog::write(): fileName=%s", fName.latin1() );
      QDataStream out(& f);
      // write fileheader
      out << quint32(KFLOG_FILE_MAGIC);
      out << qint8(FILE_TYPE_WAYPOINTS);
      out << quint16(WP_FILE_FORMAT_ID_2); //use the new format with importance field.

      Q3PtrListIterator<wayPoint> it(*wpList);
      for ( wayPoint *w = it.current(); w; w = ++it )
        {
          wpName=w->name;
          wpDescription=w->description;
          wpICAO=w->icao;
          wpType=w->type;
          wpLatitude=w->origP.lat();
          wpLongitude=w->origP.lon();
          wpElevation=w->elevation;
          wpFrequency=w->frequency;
          wpLandable=w->isLandable;
          wpRunway=w->runway;
          wpLength=w->length;
          wpSurface=w->surface;
          wpComment=w->comment;
          wpImportance=w->importance;

          out << wpName;
          out << wpDescription;
          out << wpICAO;
          out << wpType;
          out << wpLatitude;
          out << wpLongitude;
          out << wpElevation;
          out << wpFrequency;
          out << wpLandable;
          out << wpRunway;
          out << wpLength;
          out << wpSurface;
          out << wpComment;
          out << wpImportance;
        }

      f.close();
    }
  else
    {
      qWarning("WaypointCatalog::write(): File Open Error");
      return false;
    }

  qDebug("WaypointCatalog::write(): %d items written to %s",
         wpList->count(), fName.latin1());

  return ok;
}



