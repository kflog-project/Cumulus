/***********************************************************************
 **
 **   waypointcatalog.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001      by Harald Maier
 **                   2002      by André Somers,
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "waypointcatalog.h"
#include "distance.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "generalconfig.h"

extern MapMatrix*  _globalMapMatrix;
extern MainWindow* _globalMainWindow;

#define KFLOG_FILE_MAGIC 0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50

#define WP_FILE_FORMAT_ID 100
#define WP_FILE_FORMAT_ID_1 101
#define WP_FILE_FORMAT_ID_2 102 // runway direction handling modified

WaypointCatalog::WaypointCatalog() : radius(-1)
{
}

WaypointCatalog::~WaypointCatalog()
{
}

/** read a catalog from file */
bool WaypointCatalog::readBinary( QString catalog, QList<Waypoint>& wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getWaypointFile();
    }
  else
    {
      fName = catalog;
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

          if ( fileFormat != WP_FILE_FORMAT_ID &&
               fileFormat != WP_FILE_FORMAT_ID_1 &&
               fileFormat != WP_FILE_FORMAT_ID_2 )
            {
              qWarning("Waypoint file does not have the correct format. It returned %d, where %d was expected.", fileFormat, WP_FILE_FORMAT_ID);
              return false;
            }

          // from here on, we assume that the file has the correct format.
          while( !in.atEnd() )
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

              if( fileFormat>=WP_FILE_FORMAT_ID_1 )
                {
                  in >> wpImportance;
                }
              else
                {
                  wpImportance = Waypoint::Normal;
                }

              if( fileFormat < WP_FILE_FORMAT_ID_2 )
                {
                  // Runway has only one direction entry 0...360.
                  // We split it into two parts.
                  int rwh1 = wpRunway <= 180 ? wpRunway+180 : wpRunway-180;
                  int rwh2 = rwh1 <= 180 ? rwh1+180 : rwh1-180;

                  // put both directions into one variable, each in a byte
                  wpRunway = (rwh1/10) * 256 + (rwh2/10);
                }

              // create waypoint object and set the correct properties
              Waypoint wp;

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
              wp.runway = wpRunway;
              wp.length = wpLength;
              wp.surface = wpSurface;
              wp.comment = wpComment;
              wp.importance = ( enum Waypoint::Importance ) wpImportance;
              // qDebug("Waypoint read: %s (%s - %s)",wp.name.toLatin1().data(),wp.description.latin1(),wp.icao.latin1());

              wpList.append(wp);
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
bool WaypointCatalog::writeBinary( QString catalog, QList<Waypoint>& wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getWaypointFile();
    }
  else
    {
      fName = catalog;
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

      // Use the new format with importance field and two runway directions.
      out << quint16(WP_FILE_FORMAT_ID_2);

      for (int i = 0; i < wpList.count(); i++)
        {
          Waypoint &wp = wpList[i];
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

/** read a waypoint catalog from a SeeYou cup file, only waypoint part */
int WaypointCatalog::readCup( QString catalog, QList<Waypoint>* wpList )
{
  QFile file(catalog);

  if( !file.exists() )
    {
      QMessageBox::warning( _globalMainWindow,
                             QObject::tr("Error occurred!"),
                             "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(catalog) + "</html>",
                             QMessageBox::Ok );
      return -1;
    }

  if( file.size() == 0 )
    {
      QMessageBox::warning( _globalMainWindow,
                            QObject::tr("Error occurred!"),
                            "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is empty!").arg(catalog) + "</html>",
                            QMessageBox::Ok );
      return -1;
    }

  if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      return -1;
    }

  QSet<QString> names;

  int lineNo = 0;

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  int wpCount = 0;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  while( !in.atEnd() )
    {
      QString line = in.readLine();

      lineNo++;

      if( line.size() == 0 )
        {
          continue;
        }

      bool ok;

      QList<QString> list = splitCupLine( line, ok );

      if( list[0] == "-----Related Tasks-----" )
        {
          // Task part starts, we will ignore it and break up reading
          break;
        }

      // 10 elements are mandatory, element 11 description is optional
      if( list.count() < 10 ||
          list[0].toLower() == "name" ||
          list[1].toLower() == "code" ||
          list[2].toLower() == "country" )
        {
          // too less elements or a description line, ignore this
          continue;
        }

      // A cup line consists of the following elements:
      //
      // Name,Code,Country,Latitude,Longitude,Elevation,Style,Direction,Length,Frequency,Description
      //
      // See here for more info: http://download.naviter.com/docs/cup_format.pdf
      Waypoint wp;

      wp.isLandable = false;
      wp.importance = Waypoint::Low;

      if( list[0].length() ) // long name of waypoint
        {
          wp.description = list[0].replace( QRegExp("\""), "" );
        }
      else
        {
          wp.description = "";
        }

      wp.name = list[1].replace( QRegExp("\""), "" ); // short name of waypoint
      wp.comment = list[2] + ": ";
      wp.icao = "";
      wp.surface = Runway::Unknown;

      // waypoint type
      uint wpType = list[6].toUInt(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Invalid waypoint type '%s'. Ignoring it.",
                   lineNo, list[6].toAscii().data() );
          continue;
        }

      switch( wpType )
        {
        case 0:
          wp.type = BaseMapElement::NotSelected;
          break;
        case 1:
          wp.type = BaseMapElement::Landmark;
          break;
        case 2:
          wp.type = BaseMapElement::Airfield;
          wp.surface = Runway::Grass;
          wp.isLandable = true;
          wp.importance = Waypoint::Normal;
          break;
        case 3:
          wp.type = BaseMapElement::Outlanding;
          wp.importance = Waypoint::Normal;
          break;
        case 4:
          wp.type = BaseMapElement::Gliderfield;
          wp.isLandable = true;
          wp.importance = Waypoint::Normal;
          break;
        case 5:
          wp.type = BaseMapElement::Airfield;
          wp.surface = Runway::Concrete;
          wp.isLandable = true;
          wp.importance = Waypoint::Normal;
          break;
        case 6:
          wp.type = BaseMapElement::Landmark;
          break;
        case 7:
          wp.type = BaseMapElement::Landmark;
          break;
        default:
          wp.type = BaseMapElement::NotSelected;
          break;
        }

      // Check filter, if type should be taken
      if( radius != -1 && type != All )
        {
          switch( type )
            {
              // Airfields, Gliderfields, Outlandings, UlFlields, OtherPoints };
              case Airfields:

                if( wp.type != BaseMapElement::Airfield )
                  {
                    continue;
                  }

                break;

              case Gliderfields:

                if( wp.type != BaseMapElement::Gliderfield )
                  {
                    continue;
                  }

                break;

              case Outlandings:

                if( wp.type != BaseMapElement::Outlanding )
                  {
                    continue;
                  }

                break;

              case OtherPoints:

                if( wp.type == BaseMapElement::Airfield ||
                    wp.type == BaseMapElement::Gliderfield ||
                    wp.type == BaseMapElement::Outlanding )
                  {
                    continue;
                  }

                break;

              default:
                continue;
            }
        }

      // latitude as ddmm.mmm(N|S)
      double degree = list[3].left(2).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (1)", lineNo);
          continue;
        }

      double minutes = list[3].mid(2,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (N/S) (2)", lineNo);
          continue;
        }

      double latTmp = (degree * 600000.) + (minutes * 10000.0);

      if( list[3].right(1).toUpper() == "S" )
        {
          latTmp = -latTmp;
        }

      // longitude dddmm.mmm(E|W)
      degree = list[4].left(3).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (E/W) (1)", lineNo);
          continue;
        }

      minutes = list[4].mid(3,6).toDouble(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Error reading coordinate (E/W) (2)", lineNo);
          continue;
        }

      double lonTmp = (degree * 600000.) + (minutes * 10000.0);


      if( list[4].right(1).toUpper() == "W" )
        {
          lonTmp = -lonTmp;
        }

      wp.origP.setLat((int) rint(latTmp));
      wp.origP.setLon((int) rint(lonTmp));

      // Check radius filter
      if( radius != -1 )
        {
          double radiusInKm = Distance::convertToMeters( radius ) / 1000.;

          double d = dist( &centerPoint, &wp.origP );

          if( d > radiusInKm )
            {
              // Distance is greater than the defined radius around the center point.
              continue;
            }
        }

      if( list[5].length() > 1 ) // elevation in meter or feet
        {
          double tmpElev = (int) rint((list[5].left(list[5].length()-1)).toDouble(&ok));

          if( ! ok )
            {
              qWarning("CUP Read (%d): Error reading elevation '%s'.", lineNo,
                       list[5].left(list[5].length()-1).toLatin1().data());
              continue;
            }

          if( list[5].right( 1 ).toLower() == "f" )
            {
              wp.elevation = (int) rint( tmpElev * 0.3048 );
            }
          else
            {
              wp.elevation = (int) rint( tmpElev );
            }
        }

      if( list[9].trimmed().length() ) // airport frequency
        {
          double frequency = list[9].replace( QRegExp("\""), "" ).toDouble(&ok);

          if( ok )
            {
              wp.frequency = frequency;
            }
        }

      if( list[7].trimmed().length() ) // runway direction 010...360
        {
          uint rdir = list[7].toInt(&ok);

          if( ok )
            {
              // Runway has only one direction entry 0...360.
              // We split it into two parts.
              int rwh1 = rdir <= 180 ? rdir+180 : rdir-180;
              int rwh2 = rwh1 <= 180 ? rwh1+180 : rwh1-180;

              // put both directions into one variable, each in a byte
              wp.runway = (rwh1/10) * 256 + (rwh2/10);
            }
        }

      if( list[8].trimmed().length() ) // runway length in meters
        {
          // three units are possible:
          // o meter: m
          // o nautical mile: nm
          // o statute mile: ml
          QString unit;
          int uStart = list[8].indexOf( QRegExp("[lmn]") );

          if( uStart != -1 )
            {
              unit = list[8].mid( uStart ).toLower();
              double length = list[8].left( list[8].length()-unit.length() ).toDouble(&ok);

              if( ok )
                {
                  if( unit == "nm" ) // nautical miles
                    {
                      length *= 1852;
                    }
                  else if( unit == "ml" ) // statute miles
                    {
                      length *= 1609.34;
                    }

                  wp.length = (int) rint( length );
                }
            }
        }

      if( list.count() == 11 && list[10].trimmed().length() ) // description, optional
        {
          wp.comment += list[10].replace( QRegExp("\""), "" );
        }

      // We do check, if the waypoint name is already in use because cup
      // short names are not always unique.
      if( names.contains( wp.name ) )
        {
          for( int i = 0; i < 100; i++ )
            {
              // Hope that not more as 100 same names will be exist.
              QString number = QString::number(i);
               wp.name = wp.name.left(wp.name.size() - number.size()) + number;

              if( names.contains( wp.name ) == false )
                {
                  break;
                }
            }
        }

      if( wpList )
        {
          // Add waypoint to list
          wpList->append( wp );
        }

      wpCount++;

      // Store used waypoint name in set.
      names.insert( wp.name );
    }

  file.close();
  QApplication::restoreOverrideCursor();
  return wpCount;
}

QList<QString> WaypointCatalog::splitCupLine( QString& line, bool &ok )
{
  // A cup line consists of elements separated by commas. String elements
  // are enclosed in quotation marks. Inside such a string element, a
  // comma is allowed and is not to interpret as separator!
  QList<QString> list;

  int start, pos, len, idx;
  start = pos = len = idx = 0;

  line = line.trimmed();

  len = line.size();

  while( true )
    {
      if( line[pos] == QChar('"') )
        {
          // Handle quoted string
          pos++;

          if( pos >= len )
            {
              ok = false;
              return list;
            }

          // Search the end quote
          idx = line.indexOf( QChar('"'), pos );

          if( idx == -1 )
            {
              // Syntax error, abort split
              ok = false;
              return list;
            }

          pos = idx; // set current position to index of quote sign
        }

      idx = line.indexOf( QChar(','), pos );

      if( idx == -1 )
        {
          // No comma found, maybe we are at the end
          if( start < len )
            {
              list.append( line.mid( start, len-start ) );
            }
          else
            {
              list.append( QString("") );
            }

          ok = true;
          return list;
        }

      if( start < idx )
        {
          list.append( line.mid( start, idx-start ) );
        }
      else
        {
          list.append( QString("") );
        }

      if( (idx + 1) >= len )
        {
          // No more data available
          ok = true;
          return list;
        }

      pos = start = idx + 1;
    }
}
