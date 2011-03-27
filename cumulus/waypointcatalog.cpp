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
#include <QtXml>

#include "waypointcatalog.h"
#include "distance.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "waitscreen.h"

extern MapMatrix*  _globalMapMatrix;
extern MainWindow* _globalMainWindow;

#define KFLOG_FILE_MAGIC 0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50

#define WP_FILE_FORMAT_ID 100
#define WP_FILE_FORMAT_ID_1 101
#define WP_FILE_FORMAT_ID_2 102 // runway direction handling modified
#define WP_FILE_FORMAT_ID_3 103 // waypoint list size added

WaypointCatalog::WaypointCatalog() :
  _type(All),
  _radius(-1),
  _showProgress(false)
{
}

WaypointCatalog::~WaypointCatalog()
{
}

/** read a catalog from file */
int WaypointCatalog::readBinary( QString catalog, QList<Waypoint>* wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getBinaryWaypointFileName();
    }
  else
    {
      fName = catalog;
    }

  if( _globalMapMatrix == 0 )
    {
      qWarning("WaypointCatalog::readBinary: Global pointer '_globalMapMatrix' is Null!");
      return -1;
    }

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
  QString wpCountry="";

  quint32 fileMagic;
  qint8 fileType;
  quint16 fileFormat;
  qint32 wpListSize = 0;

  // new variables from format version 3
  float wpFrequency3;
  float wpElevation3;
  float wpLength3;

  int wpCount = 0;

  QFile file(fName);

  if( ! file.exists() )
    {
      qWarning() << "WaypointCatalog::readBinary: Catalog"
                 << catalog
                 << "not found!";
      return -1;
    }

  if( file.open( QIODevice::ReadOnly ) == false )
    {
      qWarning() << "WaypointCatalog::readBinary(): Cannot open catalog"
                 << catalog;
      return -1;
    }

  WaitScreen *ws = static_cast<WaitScreen *>(0);

  if( _showProgress )
    {
      ws = new WaitScreen( MainWindow::mainWindow() );
      ws->slot_SetText1( QObject::tr("Reading file") );
      ws->slot_SetText2( QFileInfo(catalog).fileName() );
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                       QEventLoop::ExcludeSocketNotifiers );
    }

  QDataStream in( &file );

  //check if the file has the correct format
  in >> fileMagic;

  if (fileMagic != KFLOG_FILE_MAGIC)
    {
      qWarning("Waypoint file not recognized as KFLog file type.");
      return -1;
    }

  in >> fileType;

  if (fileType != FILE_TYPE_WAYPOINTS)
    {
      qWarning("Waypoint file is a KFLog file, but not for waypoints!");
      return -1;
    }

  in >> fileFormat;

  if( fileFormat < WP_FILE_FORMAT_ID_2 )
    {
      qWarning() << "Wrong waypoint file format! Read format Id"
                 << fileFormat
                 << ". Expecting" << WP_FILE_FORMAT_ID_3 << ".";

      return -1;
    }

  // from here on, we assume that the file has the correct format.
  if( fileFormat == WP_FILE_FORMAT_ID_2 )
    {
      in.setVersion( QDataStream::Qt_2_0 );
    }
  else
    {
      in.setVersion( QDataStream::Qt_4_7 );
      in >> wpListSize;
    }

  // Only 20 animations should be done because the animation is a performance
  // blocker.
  qint64 wsCycles = wpListSize / 20;

  if( wsCycles == 0 )
    {
      // avoid division by zero!
      wsCycles++;
    }

  // from here on, we assume that the file has the correct format.
  while( ! in.atEnd() )
    {
      if( _showProgress == true && (wpCount % wsCycles) == 0 )
        {
          ws->slot_Progress( 2 );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                           QEventLoop::ExcludeSocketNotifiers );
        }

      // read values from file
      in >> wpName;
      in >> wpDescription;
      in >> wpICAO;
      in >> wpType;
      in >> wpLatitude;
      in >> wpLongitude;

      if( fileFormat < WP_FILE_FORMAT_ID_3 )
        {
          in >> wpElevation;
          in >> wpFrequency;
        }
      else
        {
          in >> wpElevation3;
          in >> wpFrequency3;
        }

      in >> wpLandable;
      in >> wpRunway;

      if( fileFormat < WP_FILE_FORMAT_ID_3 )
        {
          in >> wpLength;
        }
      else
        {
          in >> wpLength3;
        }

      in >> wpSurface;
      in >> wpComment;
      in >> wpImportance;

      if( fileFormat >= WP_FILE_FORMAT_ID_3 )
        {
          in >> wpCountry;
        }

      // Check filter, if type should be taken
      if( ! takeType( (enum BaseMapElement::objectType) wpType ) )
        {
          continue;
        }

      WGSPoint wgsp(wpLatitude, wpLongitude);

      // Check radius filter
      if( ! takePoint( wgsp ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      wpCount++;

      if( wpList )
        {
          // create waypoint object and set the correct properties
          Waypoint wp;

          wp.name = wpName.left(8).toUpper();
          wp.description = wpDescription;
          wp.icao = wpICAO;
          wp.type = wpType;
          wp.origP.setLat(wpLatitude);
          wp.origP.setLon(wpLongitude);
          wp.projP = _globalMapMatrix->wgsToMap(wp.origP);
          wp.isLandable = wpLandable;
          wp.runway = wpRunway;
          wp.surface = wpSurface;
          wp.comment = wpComment;
          wp.priority = ( enum Waypoint::Priority ) wpImportance;
          wp.country = wpCountry;

          if( fileFormat < WP_FILE_FORMAT_ID_3 )
            {
              wp.elevation = wpElevation;
              wp.frequency = wpFrequency;
              wp.length = wpLength;
            }
          else
            {
              wp.elevation = wpElevation3;
              wp.frequency = wpFrequency3;
              wp.length = wpLength3;
            }

          wpList->append(wp);
        }
    }

  file.close();

  if( _showProgress )
    {
      ws->setVisible( false );
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                       QEventLoop::ExcludeSocketNotifiers );
      delete ws;
    }

  return wpCount;
}

/** write a catalog to file */
bool WaypointCatalog::writeBinary( QString catalog, QList<Waypoint>& wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getBinaryWaypointFileName();

      // Make a backup of an existing file
      if( QFile::exists( fName ) )
        {
          QString oldFn = GeneralConfig::instance()->getUserDataDirectory() +
                          "/Cumulus_backup.kwp";

          QFile::remove( oldFn );
          QFile::rename( fName, oldFn );
        }
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
  float wpElevation;
  float wpFrequency;
  qint8 wpLandable;
  qint16 wpRunway;
  float wpLength;
  qint8 wpSurface;
  QString wpComment="";
  quint8 wpImportance;

  QFile file( fName );

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  if( file.open( QIODevice::WriteOnly ) )
    {
      QDataStream out( &file );
      out.setVersion( QDataStream::Qt_4_7 );

      // write file header
      out << quint32( KFLOG_FILE_MAGIC );
      out << qint8( FILE_TYPE_WAYPOINTS );
      out << quint16( WP_FILE_FORMAT_ID_3 );
      out << qint32( wpList.size() );

      for (int i = 0; i < wpList.size(); i++)
        {
          Waypoint &wp = wpList[i];
          wpName = wp.name.left(8).toUpper();
          wpDescription = wp.description;
          wpICAO = wp.icao;
          wpType = wp.type;
          wpLatitude = wp.origP.lat();
          wpLongitude = wp.origP.lon();
          wpElevation = wp.elevation;
          wpFrequency = wp.frequency;
          wpLandable = wp.isLandable;
          wpRunway = wp.runway;
          wpLength = wp.length;
          wpSurface = wp.surface;
          wpComment = wp.comment;
          wpImportance = wp.priority;

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
          out << wp.country;
        }

      file.close();
    }
  else
    {
      qWarning("WaypointCatalog::writeBinary(): Open File Error");
      ok = false;
    }

  qDebug() << "WaypointCatalog::writeBinary():"
           << wpList.count() << "entries written to"
           << fName;

  QApplication::restoreOverrideCursor();
  return ok;
}


/** read in KFLog xml data catalog from file name */
int WaypointCatalog::readXml( QString catalog, QList<Waypoint>* wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getXmlWaypointFileName();
    }
  else
    {
      fName = catalog;
    }

  if( _globalMapMatrix == 0 )
    {
      qWarning( "WaypointCatalog::readXml: Global pointer '_globalMapMatrix' is Null!" );
      return -1;
    }

  QFile file( fName );

  if( ! file.exists() )
    {
      qWarning() << "WaypointCatalog::readXml: Catalog"
                 << catalog
                 << "not found!";
      return -1;
    }

  if( file.size() == 0 )
    {
      return 0;
    }

  if( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      qWarning() << "WaypointCatalog::readXml(): Cannot open catalog"
                 << catalog;
      return -1;
    }

  QString errorMsg;
  int errorLine;
  int errorColumn;
  QDomDocument doc;

  bool ok = doc.setContent( &file, false, &errorMsg, &errorLine, &errorColumn );

  if( ! ok )
    {
      qWarning() << "WaypointCatalog::readXml(): XML parse error in File=" << catalog
                 << "Error=" << errorMsg
                 << "Line=" << errorLine
                 << "Column=" << errorColumn;
      return -1;
    }

  if( doc.doctype().name() != "KFLogWaypoint" )
    {
      qWarning() << "WaypointCatalog::readXml(): Wrong XML format of file"
                 << catalog;
      return -1;
    }

  QDomNodeList nl = doc.elementsByTagName("Waypoint");

  int wpCount = 0;

  for( int i = 0; i < nl.count(); i++ )
    {
      QDomNamedNodeMap nm =  nl.item(i).attributes();
      Waypoint w;

      w.name = nm.namedItem("Name").toAttr().value().left(8).toUpper();
      w.description = nm.namedItem("Description").toAttr().value();
      w.icao = nm.namedItem("ICAO").toAttr().value().toUpper();
      w.type = nm.namedItem("Type").toAttr().value().toInt();
      w.origP.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
      w.origP.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
      w.projP = _globalMapMatrix->wgsToMap(w.origP);
      w.elevation = nm.namedItem("Elevation").toAttr().value().toFloat();
      w.frequency = nm.namedItem("Frequency").toAttr().value().toFloat();
      w.isLandable = nm.namedItem("Landable").toAttr().value().toInt();

      int rdir = nm.namedItem("Runway").toAttr().value().toInt();

      if( rdir > 0 )
        {
          int rwh1 = rdir <= 18 ? rdir+18 : rdir-18;
          int rwh2 = rwh1 <= 18 ? rwh1+18 : rwh1-18;

          // put both directions into one variable, each in a byte
          w.runway = (rwh1) * 256 + (rwh2);
        }

      w.length = nm.namedItem("Length").toAttr().value().toFloat();
      w.surface = (enum Runway::SurfaceType)nm.namedItem("Surface").toAttr().value().toInt();
      w.comment = nm.namedItem("Comment").toAttr().value();
      w.priority = (enum Waypoint::Priority) nm.namedItem("Importance").toAttr().value().toInt();

      if( nm.contains("Country") )
        {
          w.country = nm.namedItem("Country").toAttr().value();
        }

      // Check filter, if type should be taken
      if( ! takeType( (enum BaseMapElement::objectType) w.type ) )
        {
          continue;
        }

      // Check radius filter
      if( ! takePoint( w.origP ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( wpList )
        {
          wpList->append( w );
        }

      wpCount++;
    }

  file.close();

  return wpCount;
}

/** write out KFLog xml data catalog to file name */
bool WaypointCatalog::writeXml( QString catalog, QList<Waypoint>& wpList )
{
  QString fName;

  if( catalog.isEmpty() )
    {
      // use default file name
      fName = GeneralConfig::instance()->getUserDataDirectory() + "/" +
              GeneralConfig::instance()->getXmlWaypointFileName();

      // Make a backup of an existing file
      if( QFile::exists( fName ) )
        {
          QString oldFn = GeneralConfig::instance()->getUserDataDirectory() +
                          "/Cumulus_backup.kflogwp";

          QFile::remove( oldFn );
          QFile::rename( fName, oldFn );
        }
    }
  else
    {
      fName = catalog;
    }

  bool ok = true;

  QDomDocument doc( "KFLogWaypoint" );
  QDomElement root = doc.createElement( "KFLogWaypoint" );
  QDomElement child;

  root.setAttribute( "Application", "Cumulus" );
  root.setAttribute( "Creator", getlogin() );
  root.setAttribute( "Time", QTime::currentTime().toString( "HH:mm:mm" ) );
  root.setAttribute( "Date", QDate::currentDate().toString( Qt::ISODate ) );
  root.setAttribute( "Version", "1.0" );
  root.setAttribute( "Entries", wpList.size() );

  doc.appendChild(root);

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  for( int i = 0; i < wpList.size(); i++ )
    {
      Waypoint& w = wpList[i];

      child = doc.createElement( "Waypoint" );

      child.setAttribute( "Name", w.name.left(8).toUpper() );
      child.setAttribute( "Description", w.description );
      child.setAttribute( "ICAO", w.icao );
      child.setAttribute( "Type", w.type );
      child.setAttribute( "Latitude", w.origP.lat() );
      child.setAttribute( "Longitude", w.origP.lon() );
      child.setAttribute( "Elevation", w.elevation );
      child.setAttribute( "Frequency", w.frequency );
      child.setAttribute( "Landable", w.isLandable );

      // Only the main runway heading is stored as 10...36
      child.setAttribute( "Runway", w.runway/256 );
      child.setAttribute( "Length", w.length );
      child.setAttribute( "Surface", w.surface );
      child.setAttribute( "Comment", w.comment );
      child.setAttribute( "Importance", w.priority );
      child.setAttribute( "Country", w.country );

      root.appendChild( child );
    }

  QFile file(fName);

  if( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      const int IndentSize = 4;

      QTextStream out( &file );
      doc.save( out, IndentSize );
      file.close();

      qDebug() << "WaypointCatalog::writeXml():"
               << wpList.count() << "entries written to"
               << fName;
    }
  else
    {
      qWarning("WaypointCatalog::writeXml(): Open File Error");
      ok = false;
    }

  QApplication::restoreOverrideCursor();
  return ok;
}

/** read a waypoint catalog from a SeeYou cup file, only waypoint part */
int WaypointCatalog::readCup( QString catalog, QList<Waypoint>* wpList )
{
  QFile file(catalog);

  if( ! file.exists() )
    {
      return -1;
    }

  if( file.size() == 0 )
    {
      return 0;
    }

  if( ! file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      return -1;
    }

  QSet<QString> namesInUse;
  int lineNo = 0;
  int wpCount = 0;

  WaitScreen *ws = static_cast<WaitScreen *>(0);

  if( _showProgress )
    {
      ws = new WaitScreen( MainWindow::mainWindow() );
      ws->slot_SetText1( QObject::tr("Reading file") );
      ws->slot_SetText2( QFileInfo(catalog).fileName() );
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                       QEventLoop::ExcludeSocketNotifiers );
    }

  // Estimate the number of entries in the file. The basic assumption is, that
  // a single line is approximately 70 characters long. Only 20 animations
  // should be done because the animation is a performance blocker.
  int wsCycles = (file.size() / 70) / 20;

  if( wsCycles == 0 )
    {
      // avoid division by zero!
      wsCycles++;
    }

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  while( !in.atEnd() )
    {
      QString line = in.readLine();

      lineNo++;

      if( _showProgress && (lineNo % wsCycles) == 0 )
        {
          ws->slot_Progress( 2 );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                           QEventLoop::ExcludeSocketNotifiers );
        }

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
      wp.priority = Waypoint::Low;

      if( list[0].length() ) // long name of waypoint
        {
          wp.description = list[0].replace( QRegExp("\""), "" );
        }
      else
        {
          wp.description = "";
        }

      // short name of a waypoint limited to 8 characters
      wp.name = list[1].replace( QRegExp("\""), "" ).left(8).toUpper();
      wp.country = list[2].left(2).toUpper();
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
          wp.priority = Waypoint::Normal;
          break;
        case 3:
          wp.type = BaseMapElement::Outlanding;
          wp.priority = Waypoint::Normal;
          break;
        case 4:
          wp.type = BaseMapElement::Gliderfield;
          wp.isLandable = true;
          wp.priority = Waypoint::Normal;
          break;
        case 5:
          wp.type = BaseMapElement::Airfield;
          wp.surface = Runway::Concrete;
          wp.isLandable = true;
          wp.priority = Waypoint::Normal;
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
      if( ! takeType( (enum BaseMapElement::objectType) wp.type ) )
        {
          continue;
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

      // Sets the projected coordinates
      wp.projP = _globalMapMatrix->wgsToMap( wp.origP );

      // Check radius filter
      if( ! takePoint( wp.origP ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( list[5].length() > 1 ) // elevation in meter or feet
        {
          float tmpElev = (list[5].left(list[5].length()-1)).toFloat(&ok);

          if( ! ok )
            {
              qWarning("CUP Read (%d): Error reading elevation '%s'.", lineNo,
                       list[5].left(list[5].length()-1).toLatin1().data());
              continue;
            }

          if( list[5].right( 1 ).toLower() == "f" )
            {
              wp.elevation = tmpElev * 0.3048;
            }
          else
            {
              wp.elevation = tmpElev;
            }
        }

      if( list[9].trimmed().length() ) // airport frequency
        {
          float frequency = list[9].replace( QRegExp("\""), "" ).toFloat(&ok);

          if( ok )
            {
              wp.frequency = frequency;
            }
          else
            {
              wp.frequency = 0.0;
            }
        }

      if( list[7].trimmed().length() ) // runway direction 010...360
        {
          uint rdir = list[7].toInt(&ok);

          if( ok )
            {
              // Runway has only one direction entry 010...360.
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
              float length = list[8].left( list[8].length()-unit.length() ).toFloat(&ok);

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

                  wp.length = length;
                }
            }
        }

      if( list.count() == 11 && list[10].trimmed().length() ) // description, optional
        {
          wp.comment += list[10].replace( QRegExp("\""), "" );
        }

      // We do check, if the waypoint name is already in use because cup
      // short names are not always unique.
      if( namesInUse.contains( wp.name ) )
        {
          for( int i = 0; i < 100; i++ )
            {
              // Hope that not more as 100 same names will be exist.
              QString number = QString::number(i);
               wp.name = wp.name.left(wp.name.size() - number.size()) + number;

              if( namesInUse.contains( wp.name ) == false )
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
      namesInUse.insert( wp.name );
    }

  file.close();

  if( _showProgress )
    {
      ws->setVisible( false );
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                       QEventLoop::ExcludeSocketNotifiers );
      delete ws;
    }

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

bool WaypointCatalog::takeType( enum BaseMapElement::objectType type )
{
  // Check filter, if waypoint type should be taken
  if( _radius != -1 && _type != All )
    {
      switch( _type )
        {
          // Airfields, Gliderfields, Outlandings, UlFlields, OtherPoints };
          case Airfields:

            if( type != BaseMapElement::Airfield )
              {
                return false;
              }

            break;

          case Gliderfields:

            if( type != BaseMapElement::Gliderfield )
              {
                return false;
              }

            break;

          case Outlandings:

            if( type != BaseMapElement::Outlanding )
              {
                return false;
              }

            break;

          case OtherPoints:

            if( type == BaseMapElement::Airfield ||
                type == BaseMapElement::Gliderfield ||
                type == BaseMapElement::Outlanding )
              {
                return false;
              }

            break;

          default:

            return false;
        }
    }

  return true;
}

bool WaypointCatalog::takePoint( WGSPoint& point )
{
  // Check radius filter
  if( _radius != -1 )
    {
      double radiusInKm = Distance::convertToMeters( _radius ) / 1000.;

      double d = dist( &centerPoint, &point );

      if( d > radiusInKm )
        {
          // Distance is greater than the defined radius around the center point.
          return false;
        }
    }

  return true;
}
