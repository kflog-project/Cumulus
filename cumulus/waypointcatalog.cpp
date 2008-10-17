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
bool WaypointCatalog::read( QString *catalog, QList<wayPoint>& wpList )
{
  QString fName;

  if( !catalog )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getWaypointFile();
    }
  else
    {
      fName = *catalog;
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

  // qDebug("Read waypoint catalog from %s", fName.toLatin1().data() );

  // default file location: $HOME/cumulus/cumulus.kwp

  QFile f(fName);

  if (f.exists())
    {
      if (f.open(QIODevice::ReadOnly))
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

              // create waypoint object and set the correct properties
              wayPoint wp;

              wp.name = wpName;
              wp.description = wpDescription;
              wp.icao = wpICAO;
              wp.type = wpType;
              wp.origP.setLat(wpLatitude);
              wp.origP.setLon(wpLongitude);
              wp.projP = _globalMapMatrix->wgsToMap(wp.origP);
              wp.elevation = wpElevation;
              wp.frequency = wpFrequency;
              wp.isLandable = wpLandable;
              wp.runway =wpRunway;
              wp.length = wpLength;
              wp.surface = wpSurface;
              wp.comment = wpComment;
              wp.importance = ( enum wayPoint::Importance ) wpImportance;
              // qDebug("Waypoint read: %s (%s - %s)",wp.name.toLatin1().data(),wp.description.latin1(),wp.icao.latin1());

              wpList.append(wp);

              GeneralConfig *conf = GeneralConfig::instance();

              if (conf->getHomeWp()->origP == wp.origP)
                {
                  qDebug("Found homesite: %s", wpName.toLatin1().data() );
                  conf->setHomeWp(&wp);
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
         wpList.count(), fName.toLatin1().data() );

  return ok;
}

/** write a catalog to file */
bool WaypointCatalog::write( QString *catalog, QList<wayPoint>& wpList )
{
  QString fName;

  if( !catalog )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getWaypointFile();
    }
  else
    {
      fName = *catalog;
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

  f.setFileName(fName);

  if (f.open(QIODevice::WriteOnly))
    {
      // qDebug("WaypointCatalog::write(): fileName=%s", fName.toLatin1().data() );
      QDataStream out(& f);

      // write file header
      out << quint32(KFLOG_FILE_MAGIC);
      out << qint8(FILE_TYPE_WAYPOINTS);
      out << quint16(WP_FILE_FORMAT_ID_2); //use the new format with importance field.

      for (int i = 0; i < wpList.count(); i++)
        {
          wayPoint &wp = wpList[i];
          wpName=wp.name;
          wpDescription=wp.description;
          wpICAO=wp.icao;
          wpType=wp.type;
          wpLatitude=wp.origP.lat();
          wpLongitude=wp.origP.lon();
          wpElevation=wp.elevation;
          wpFrequency=wp.frequency;
          wpLandable=wp.isLandable;
          wpRunway=wp.runway;
          wpLength=wp.length;
          wpSurface=wp.surface;
          wpComment=wp.comment;
          wpImportance=wp.importance;

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
         wpList.count(), fName.toLatin1().data() );

  return ok;
}
