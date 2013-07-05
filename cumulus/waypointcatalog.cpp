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
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <unistd.h>

#include <QtGui>
#include <QtXml>

#include "distance.h"
#include "generalconfig.h"
#include "openaip.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "radiopoint.h"
#include "waitscreen.h"
#include "waypointcatalog.h"

extern MapMatrix* _globalMapMatrix;

#define KFLOG_FILE_MAGIC 0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50

#define WP_FILE_FORMAT_ID 100
#define WP_FILE_FORMAT_ID_1 101
#define WP_FILE_FORMAT_ID_2 102 // runway direction handling modified
#define WP_FILE_FORMAT_ID_3 103 // waypoint list size added
#define WP_FILE_FORMAT_ID_4 104 // runway list added

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

  // new element from format version 4
  QList<Runway> rwyList;

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

#ifdef ANDROID
      // The waitscreen is not centered over the parent and not limited in
      // its size under Android. Therefore this must be done by our self.
      ws->setGeometry ( MainWindow::mainWindow()->width() / 2 - 250,
                        MainWindow::mainWindow()->height() / 2 - 75,
                        500, 150 );
#endif

      ws->slot_SetText1( QObject::tr("Reading file") );
      ws->slot_SetText2( QFileInfo(catalog).fileName() );
      QCoreApplication::flush();
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
          //QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
          //                                 QEventLoop::ExcludeSocketNotifiers );
        }

      rwyList.clear();

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

      if( fileFormat < WP_FILE_FORMAT_ID_4 )
        {
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
        }

      in >> wpComment;
      in >> wpImportance;

      if( fileFormat >= WP_FILE_FORMAT_ID_3 )
        {
          in >> wpCountry;
        }

      if( fileFormat >= WP_FILE_FORMAT_ID_4 )
        {
          // The runway list has to be read
          quint8 listSize;
          quint16 length;
          quint16 width;
          quint16 heading;
          quint8 surface;
          quint8 isOpen;
          quint8 isBidirectional;

          in >> listSize;

          for( int i = 0; i < (int) listSize; i++ )
            {
              in >> length;
              in >> width;
              in >> heading;
              in >> surface;
              in >> isOpen;
              in >> isBidirectional;

              Runway rwy( length, heading, surface, isOpen, isBidirectional, width );
              rwyList.append(rwy);
            }
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
          wp.wgsPoint.setLat(wpLatitude);
          wp.wgsPoint.setLon(wpLongitude);
          wp.projPoint = _globalMapMatrix->wgsToMap(wp.wgsPoint);
          wp.comment = wpComment;
          wp.priority = ( enum Waypoint::Priority ) wpImportance;
          wp.country = wpCountry;
          wp.wpListMember = true;

          if( fileFormat < WP_FILE_FORMAT_ID_3 )
            {
              wp.elevation = wpElevation;
              wp.frequency = wpFrequency;
            }
          else
            {
              wp.elevation = wpElevation3;
              wp.frequency = wpFrequency3;
            }

          if( fileFormat >= WP_FILE_FORMAT_ID_4 )
            {
              // We have a runway list
              if( rwyList.size() )
                {
                  wp.rwyList = rwyList;
                }
            }
          else
            {
              if ( wpRunway > 0 )
                {
                  // Runway heading must be > 0 to be a right runway.
                  Runway rwy( wpLength3, wpRunway, wpSurface, true, true );
                  wp.rwyList.append( rwy );
                }
            }

          wpList->append(wp);
        }
    }

  file.close();

  if( _showProgress )
    {
      ws->setVisible( false );
      QCoreApplication::flush();
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
      out << quint16( WP_FILE_FORMAT_ID_4 );
      out << qint32( wpList.size() );

      for (int i = 0; i < wpList.size(); i++)
        {
          Waypoint &wp = wpList[i];
          wpName = wp.name.left(8).toUpper();
          wpDescription = wp.description;
          wpICAO = wp.icao;
          wpType = wp.type;
          wpLatitude = wp.wgsPoint.lat();
          wpLongitude = wp.wgsPoint.lon();
          wpElevation = wp.elevation;
          wpFrequency = wp.frequency;
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
          out << wpComment;
          out << wpImportance;
          out << wp.country;

          // The runway list is saved
          out << quint8( wp.rwyList.size() );

          for( int i = 0; i < wp.rwyList.size(); i++ )
            {
              Runway rwy = wp.rwyList.at(i);

              out << quint16( rwy.length );
              out << quint16( rwy.width );
              out << quint16( rwy.heading );
              out << quint8( rwy.surface );
              out << quint8( rwy.isOpen );
              out << quint8( rwy.isBidirectional );
            }
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
int WaypointCatalog::readXml( QString catalog, QList<Waypoint>* wpList, QString& errorMsg )
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

  QString errorText;
  int errorLine;
  int errorColumn;
  QDomDocument doc;

  bool ok = doc.setContent( &file, false, &errorText, &errorLine, &errorColumn );

  if( ! ok )
    {
      errorMsg = QString("<html>XML Error at line %1 column %2:<br><br>%3</html>")
                 .arg(errorLine).arg(errorColumn).arg(errorText);

      qWarning() << "WaypointCatalog::readXml(): XML parse error in File="
                 << catalog
                 << "Error=" << errorText
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
      w.wgsPoint.setLat(nm.namedItem("Latitude").toAttr().value().toInt());
      w.wgsPoint.setLon(nm.namedItem("Longitude").toAttr().value().toInt());
      w.projPoint = _globalMapMatrix->wgsToMap(w.wgsPoint);
      w.elevation = nm.namedItem("Elevation").toAttr().value().toFloat();
      w.frequency = nm.namedItem("Frequency").toAttr().value().toFloat();
      float length = nm.namedItem("Length").toAttr().value().toFloat();
      int surface = (enum Runway::SurfaceType)nm.namedItem("Surface").toAttr().value().toInt();

      int rdir = nm.namedItem("Runway").toAttr().value().toInt();

      if( rdir > 0 )
        {
          int rwh1 = rdir;
          int rwh2 = rwh1 <= 18 ? rwh1+18 : rwh1-18;

          // put both directions into one variable, each in a byte
          int heading = (rwh1) * 256 + (rwh2);

          // Store runways in the runway list.
          Runway rwy( length, heading, surface, true, true );
          w.rwyList.append( rwy );
        }

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
      if( ! takePoint( w.wgsPoint ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( wpList )
        {
          w.wpListMember = true;
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
      child.setAttribute( "Latitude", w.wgsPoint.lat() );
      child.setAttribute( "Longitude", w.wgsPoint.lon() );
      child.setAttribute( "Elevation", w.elevation );
      child.setAttribute( "Frequency", w.frequency );
      child.setAttribute( "Landable", w.rwyList.size() > 0 ? true : false );

      // Only the main runway is stored as 10...36
      if( w.rwyList.size() )
        {
          Runway rwy = w.rwyList.first();

          child.setAttribute( "Runway", rwy.heading/256 );
          child.setAttribute( "Length", rwy.length );
          child.setAttribute( "Surface", rwy.surface );
        }
      else
        {
          child.setAttribute( "Runway", 0 );
          child.setAttribute( "Length", 0 );
          child.setAttribute( "Surface", 0 );
        }

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

int WaypointCatalog::readOpenAip( QString catalog,
                                  QList<Waypoint>* wpList,
                                  QString& errorInfo )
{
  qDebug() << "WaypointCatalog::readOpenAip" << catalog;

  QFile file(catalog);

  if( !file.exists() )
    {
      errorInfo = QObject::tr("File does not exist!");
      return 0;
    }

  if( file.size() == 0 )
    {
      errorInfo = QObject::tr("File is empty!");
      return 0;
    }

  OpenAip openAip;

  // We have to look, which data is contained in the passed file to be read.
  QString dataFormat;
  QString dataElement;

  if( ! openAip.getRootElement( catalog, dataFormat, dataElement) )
    {
      errorInfo = QObject::tr("Invalid OpenAip data format!");
      return 0;
    }

  if( dataElement == "NAVAIDS" )
    {
      return readOpenAipNavAids( catalog, wpList, errorInfo );
    }
  else if( dataElement == "HOTSPOTS" )
    {
      return readOpenAipHotspots( catalog, wpList, errorInfo );
    }
  else if( dataElement == "WAYPOINTS" )
    {
      return readOpenAipAirfields( catalog, wpList, errorInfo );
    }
  else
    {
      errorInfo = QObject::tr("OpenAip %1 is unsupported!").arg(dataElement);
      return 0;
    }
}

int WaypointCatalog::readOpenAipNavAids( QString catalog,
                                         QList<Waypoint>* wpList,
                                         QString& errorInfo )
{
  OpenAip openAip;
  QList<RadioPoint> navAidList;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  if( ! openAip.readNavAids( catalog, navAidList, errorInfo ) )
    {
      QApplication::restoreOverrideCursor();
      return 0;
    }

  if( navAidList.size() == 0 )
    {
      // navAid list is empty
      QApplication::restoreOverrideCursor();
      return 0;
    }

  QSet<QString> namesInUse;
  int wpCount = 0;

  for( int i = 0; i < navAidList.size(); i++ )
    {
      const RadioPoint& rp = navAidList.at(i);

      // Check filter, if type should be taken
      if( ! takeType( rp.getTypeID() ) )
        {
          continue;
        }

      // Check radius filter
      WGSPoint wgsp = rp.getWGSPosition();

      if( ! takePoint( wgsp) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( wpList == 0 )
        {
          // Only the list size is desired and returned.
          wpCount++;
          continue;
        }

      Waypoint wp;
      wp.name = rp.getWPName();

      // We do check, if the waypoint name is already in use because
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

      // Copy all navAid data to waypoint object
      wp.type = rp.getTypeID();
      wp.wgsPoint = rp.getWGSPosition();
      wp.projPoint = rp.getPosition();
      wp.description = rp.getName();
      wp.icao = rp.getICAO();
      wp.comment = rp.getComment();
      wp.elevation = rp.getElevation();
      wp.frequency = rp.getFrequency();
      wp.priority = Waypoint::Low;
      wp.country = rp.getCountry();

      if( wp.comment.size() > 0 )
        {
          wp.comment += "\n";
        }

      wp.comment += "Channel " + rp.getChannel();

      // Add waypoint to list
      wp.wpListMember = true;
      wpList->append( wp );

      // Store used waypoint name in set.
      namesInUse.insert( wp.name );
      wpCount++;
    }

  QApplication::restoreOverrideCursor();
  return wpCount;
}

int WaypointCatalog::readOpenAipHotspots( QString catalog,
                                          QList<Waypoint>* wpList,
                                          QString& errorInfo )
{
  OpenAip openAip;
  QList<SinglePoint> hotspotList;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  if( ! openAip.readHotspots( catalog, hotspotList, errorInfo ) )
    {
      QApplication::restoreOverrideCursor();
      return 0;
    }

  if( hotspotList.size() == 0 )
    {
      // navAid list is empty
      QApplication::restoreOverrideCursor();
      return 0;
    }

  QSet<QString> namesInUse;
  int wpCount = 0;

  for( int i = 0; i < hotspotList.size(); i++ )
    {
      const SinglePoint& sp = hotspotList.at(i);

      // Check filter, if type should be taken
      if( ! takeType( sp.getTypeID() ) )
        {
          continue;
        }

      // Check radius filter
      WGSPoint wgsp = sp.getWGSPosition();

      if( ! takePoint( wgsp) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( wpList == 0 )
        {
          // Only the list size is desired and returned.
          wpCount++;
          continue;
        }

      Waypoint wp;
      wp.name = sp.getWPName();

      // We do check, if the waypoint name is already in use because
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

      // Copy all hotspot data into the waypoint object
      wp.type = sp.getTypeID();
      wp.wgsPoint = sp.getWGSPosition();
      wp.projPoint = sp.getPosition();
      wp.description = sp.getName();
      wp.comment = sp.getComment();
      wp.elevation = sp.getElevation();
      wp.priority = Waypoint::Low;
      wp.country = sp.getCountry();

      // Add waypoint to list
      wp.wpListMember = true;
      wpList->append( wp );

      // Store used waypoint name in set.
      namesInUse.insert( wp.name );
      wpCount++;
    }

  QApplication::restoreOverrideCursor();
  return wpCount;
}

int WaypointCatalog::readOpenAipAirfields( QString catalog,
                                           QList<Waypoint>* wpList,
                                           QString& errorInfo )
{
  OpenAip openAip;
  QList<Airfield> airfieldList;

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  if( ! openAip.readAirfields( catalog, airfieldList, errorInfo ) )
    {
      QApplication::restoreOverrideCursor();
      return 0;
    }

  if( airfieldList.size() == 0 )
    {
      // navAid list is empty
      QApplication::restoreOverrideCursor();
      return 0;
    }

  QSet<QString> namesInUse;
  int wpCount = 0;

  for( int i = 0; i < airfieldList.size(); i++ )
    {
      Airfield& af = airfieldList[i];

      // Check filter, if type should be taken
      if( ! takeType( af.getTypeID() ) )
        {
          continue;
        }

      // Check radius filter
      WGSPoint wgsp = af.getWGSPosition();

      if( ! takePoint( wgsp) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      if( wpList == 0 )
        {
          // Only the list size is desired and returned.
          wpCount++;
          continue;
        }

      Waypoint wp;
      wp.name = af.getWPName();

      // We do check, if the waypoint name is already in use because
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

      // Copy all airfield data into the waypoint object
      wp.type = af.getTypeID();
      wp.wgsPoint = af.getWGSPosition();
      wp.projPoint = af.getPosition();
      wp.description = af.getName();
      wp.comment = af.getComment();
      wp.elevation = af.getElevation();
      wp.priority = Waypoint::Low;
      wp.country = af.getCountry();
      wp.icao = af.getICAO();
      wp.frequency = af.getFrequency();

      if( af.getRunwayList().size() > 0 )
        {
          // Take over the whole runway list.
          wp.rwyList = af.getRunwayList();
        }

      // Add waypoint to list
      wp.wpListMember = true;
      wpList->append( wp );

      // Store used waypoint name in set.
      namesInUse.insert( wp.name );
      wpCount++;
    }

  QApplication::restoreOverrideCursor();
  return wpCount;
}

/**
 * Reads a Cambridge Aero Instruments or a Winpilot turnpoint file.
 *
 * \param catalog Catalog file name with directory path.
 *
 * \param wpList Waypoint list where the read waypoints are stored. If the
 *               wpList is NULL, waypoints are counted only.
 *
 * \return Number of read waypoints. In error case -1.
 */
int WaypointCatalog::readDat( QString catalog, QList<Waypoint>* wpList )
{
  // Found a file format description here:
  // http://www.gregorie.org/gliding/pna/cai_format.html
  qDebug() << "WaypointCatalog::readDat" << catalog;

  QFile file(catalog);

  if(!file.exists())
    {
      return -1;
    }

  if(file.size() == 0)
    {
      return 0;
    }

  if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
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

#ifdef ANDROID
      // The waitscreen is not centered over the parent and not limited in
      // its size under Android. Therefore this must be done by our self.
      ws->setGeometry ( MainWindow::mainWindow()->width() / 2 - 250,
                        MainWindow::mainWindow()->height() / 2 - 75,
                        500, 150 );
#endif

      ws->slot_SetText1( QObject::tr("Reading file") );
      ws->slot_SetText2( QFileInfo(catalog).fileName() );
      QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                       QEventLoop::ExcludeSocketNotifiers );
    }

  // Estimate the number of entries in the file. The basic assumption is, that
  // a single line is approximately 60 characters long. Only 20 animations
  // should be done because the animation is a performance blocker.
  int wsCycles = (file.size() / 60) / 20;

  if( wsCycles == 0 )
    {
      // avoid division by zero!
      wsCycles++;
    }

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  while (!in.atEnd())
    {
      QString line = in.readLine().trimmed();

      lineNo++;

      if( _showProgress && (lineNo % wsCycles) == 0 )
        {
          ws->slot_Progress( 2 );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                           QEventLoop::ExcludeSocketNotifiers );
        }

      if( line.size() == 0 || line.startsWith("*") )
        {
          // Filter out empty and comment lines
          continue;
        }

      bool ok;
      QStringList list = line.split( "," );

      /*
      Example turnpoints, two possible coordinate formats seems to be in use.
      0 ,1         ,2          ,3   ,4,5           ,6
      31,57:04.213N,002:47.239W,450F,T,AB1 AboynBrg,RdBroverRDee

      0,1        ,2         ,3  ,4  ,5           ,6
      1,52:08:39N,012:40:06E,66M,HAS,SP1 LUESSE  ,EDOJ
      */

      // Lines defining a turnpoint contain 7 fields, separated by commas.
      // The final field is terminated by the newline. Field 7 is optional.
      if( list.size() < 6 )
        {
          qWarning( "DAT Read (%d): Line contains too less elements! Ignoring it.",
                    lineNo );
          continue;
        }

      Waypoint wp;

      wp.priority = Waypoint::Low;
      wp.country = GeneralConfig::instance()->getHomeCountryCode();

      if( list[1].size() < 9 )
        {
          qWarning("DAT Read (%d): Format error latitude", lineNo);
          continue;
        }

      // latitude as 57:04.213N|S or 52:08:39N|S
      double degree = list[1].left(2).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error latitude degree", lineNo);
          continue;
        }

      double minutes = 0.0;
      double seconds = 0.0;

      if( list[1][5] == QChar('.') )
        {
          minutes = list[1].mid(3, 6).toDouble(&ok);
        }
      else if( list[1][5] == QChar(':') )
        {
          minutes = list[1].mid(3, 2).toDouble(&ok);

          if( ok )
            {
              seconds = list[1].mid(6, 2).toDouble(&ok);
            }
        }

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error latitude minutes/seconds", lineNo);
          continue;
        }

      double latTmp = (degree * 600000.) + (10000. * (minutes + seconds / 60. ));

      if( list[1].right(1).toUpper() == "S" )
        {
          latTmp = -latTmp;
        }

      // longitude as 002:47.239E|W or 012:40:06E|W
      if( list[2].size() < 10 )
        {
          qWarning("DAT Read (%d): Format error longitude", lineNo);
          continue;
        }

      degree = list[2].left(3).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error longitude degree", lineNo);
          continue;
        }

      minutes = 0.0;
      seconds = 0.0;

      if( list[2][6] == QChar('.') )
        {
          minutes = list[2].mid(4, 6).toDouble(&ok);
        }
      else if( list[2][6] == QChar(':') )
        {
          minutes = list[2].mid(4, 2).toDouble(&ok);

          if( ok )
            {
              seconds = list[2].mid(7, 2).toDouble(&ok);
            }
        }

      if( ! ok )
        {
          qWarning("DAT Read (%d): Format error longitude minutes/seconds", lineNo);
          continue;
        }

      double lonTmp = (degree * 600000.) + (10000. * (minutes + seconds / 60. ));


      if( list[2].right(1).toUpper() == "W" )
        {
          lonTmp = -lonTmp;
        }

      wp.wgsPoint.setLat((int) rint(latTmp));
      wp.wgsPoint.setLon((int) rint(lonTmp));

      // Check radius filter
      if( ! takePoint( wp.wgsPoint ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      // Sets the projected coordinates
      wp.projPoint = _globalMapMatrix->wgsToMap( wp.wgsPoint );

      // Height AMSL 9{1,5}[FM] 9=height, F=feet, M=metres.
      // two units are possible:
      // o meter: m
      // o feet:  ft
      if( list[3].size() ) // elevation in meter or feet
        {
          QString unit = list[3].right(1).toUpper();

          if( unit != "F" && unit != "M" )
            {
              qWarning("DAT Read (%d): Error reading elevation unit '%s'.",
                       lineNo, list[3].toLatin1().data());
              continue;
            }

          float tmpElev = list[3].left(list[3].size() - 1).toFloat(&ok);

          if( ! ok )
            {
              qWarning("DAT Read (%d): Error reading elevation value '%s'.",
                        lineNo,
                        list[3].left(list[3].size() - 1).toLatin1().data());
              continue;
            }

          if( unit == "M" )
            {
              wp.elevation = tmpElev;
            }
          else if( unit == "F" )
            {
              // Convert feet to meters
              wp.elevation = tmpElev * 0.3048;
            }
          else
            {
              qWarning("DAT Read (%d): Unknown elevation value '%s'.", lineNo,
                       unit.toLatin1().data());
              continue;
            }
        }

      /*
      Turnpoint attributes

      Cambridge documentation defines the following:
      Code    Meaning
      A       Airfield (not necessarily landable). All turnpoints marked 'A' in the UK are landable.
      L       Landable Point. Not necessarily an airfield.
      S       Start Point
      F       Finish Point
      H       Home Point
      M       Markpoint
      R       Restricted Point
      T       Turnpoint
      W       Waypoint
      */

      if( list[4].isEmpty() )
        {
          qWarning("DAT Read (%d): Missing turnpoint attributes", lineNo );
          continue;
        }

      // That is the default
      wp.type = BaseMapElement::Landmark;

      list[4] = list[4].toUpper();

      if( list[4].contains("T") )
        {
          wp.type = BaseMapElement::Turnpoint;
        }

      if( list[4].contains("A") )
        {
          wp.type = BaseMapElement::Airfield;
        }

      if( list[4].contains("L") )
        {
          // wp.isLandable = true;
        }

      // Check filter, if type should be taken
      if( ! takeType( (enum BaseMapElement::objectType) wp.type ) )
        {
          continue;
        }

      if( list[5].isEmpty() )
        {
          qWarning("DAT Read (%d): Missing turnpoint name", lineNo );
          continue;
        }

      // Short name of a waypoint has only 8 characters and upper cases in Cumulus.
      // That is handled in another way by Cambridge.
      wp.name = list[5].left(8).toUpper().trimmed();
      wp.description = list[5].trimmed();

      if( list.size() >= 7 )
        {
          QString comment = list[6].trimmed();

          if( ! comment.isEmpty() )
            {
              // A description is optional by Cambridge.
              wp.comment += comment;
            }
        }

      if( ! wp.comment.isEmpty() )
        {
          wp.comment.append("; ");
        }

      wp.comment.append( "DAT: ").append(list[4]);

      // We do check, if the waypoint name is already in use because DAT
      // short names are not always unique.
      if( wpList && namesInUse.contains( wp.name ) )
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
          wp.wpListMember = true;
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

#ifdef ANDROID
      // The waitscreen is not centered over the parent and not limited in
      // its size under Android. Therefore this must be done by our self.
      ws->setGeometry ( MainWindow::mainWindow()->width() / 2 - 250,
                        MainWindow::mainWindow()->height() / 2 - 75,
                        500, 150 );
#endif

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
      QString line = in.readLine().trimmed();

      lineNo++;

      if( _showProgress && (lineNo % wsCycles) == 0 )
        {
          ws->slot_Progress( 2 );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents|
                                           QEventLoop::ExcludeSocketNotifiers );
        }

      if( line.size() == 0 || line.startsWith("#") )
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

      Runway rwy;

      wp.priority = Waypoint::Low;

      if( list[0].length() ) // long name of waypoint
        {
          wp.description = list[0].replace( QRegExp("\""), "" );
        }
      else
        {
          wp.description = "";
        }

      if( list[1].isEmpty() )
        {
          // If no code is set, we assign the long name as code to have a workaround.
          list[1] = list[0];
        }

      // short name of a waypoint limited to 8 characters
      wp.name = list[1].replace( QRegExp("\""), "" ).left(8).toUpper();
      wp.country = list[2].left(2).toUpper();
      wp.icao = "";
      rwy.surface = Runway::Unknown;

      // waypoint type
      uint wpType = list[6].toUInt(&ok);

      if( ! ok )
        {
          qWarning("CUP Read (%d): Invalid waypoint type '%s'. Ignoring it.",
                   lineNo, list[6].toLatin1().data() );
          continue;
        }

      switch( wpType )
        {
        case 1:
          wp.type = BaseMapElement::Landmark;
          break;
        case 2:
          wp.type = BaseMapElement::Airfield;
          rwy.surface = Runway::Grass;
          wp.priority = Waypoint::Normal;
          break;
        case 3:
          wp.type = BaseMapElement::Outlanding;
          wp.priority = Waypoint::Normal;
          break;
        case 4:
          wp.type = BaseMapElement::Gliderfield;
          wp.priority = Waypoint::Normal;
          break;
        case 5:
          wp.type = BaseMapElement::Airfield;
          rwy.surface = Runway::Concrete;
          wp.priority = Waypoint::Normal;
          break;
        case 9:
          wp.type = BaseMapElement::Ndb;
          break;
        case 10:
          wp.type = BaseMapElement::Vor;
          break;
        case 11:
          // Mapped to thermal hotspot defined by http://glidinghotspots.eu/
          wp.type = BaseMapElement::Thermal;
          break;
        default:
          wp.type = BaseMapElement::Landmark;
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

      wp.wgsPoint.setLat((int) rint(latTmp));
      wp.wgsPoint.setLon((int) rint(lonTmp));

      // Check radius filter
      if( ! takePoint( wp.wgsPoint ) )
        {
          // Distance is greater than the defined radius around the center point.
          continue;
        }

      // Sets the projected coordinates
      wp.projPoint = _globalMapMatrix->wgsToMap( wp.wgsPoint );

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

              rwy.length = length;
            }
        }

      // two units are possible:
      // o meter: m
      // o feet:  ft
      if( list[5].length() ) // elevation in meter or feet
        {
          QString unit;
          int uStart = list[5].indexOf( QRegExp("[mf]") );

          if( uStart == -1 )
            {
              qWarning("CUP Read (%d): Error reading elevation unit '%s'.", lineNo,
                       list[5].toLatin1().data());
              continue;
            }

          unit = list[5].mid( uStart ).toLower();

          float tmpElev = (list[5].left(list[5].length() - unit.length())).toFloat(&ok);

          if( ! ok )
            {
              qWarning("CUP Read (%d): Error reading elevation value '%s'.", lineNo,
                       list[5].left(list[5].length() - unit.length()).toLatin1().data());
              continue;
            }

          if( unit == "m" )
            {
              wp.elevation = tmpElev;
            }
          else if( unit == "ft" )
            {
              wp.elevation = tmpElev * 0.3048;
            }
          else
            {
              qWarning("CUP Read (%d): Unknown elevation value '%s'.", lineNo,
                       unit.toLatin1().data());
              continue;
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
              int rwh1 = rdir;
              int rwh2 = rwh1 <= 180 ? rwh1+180 : rwh1-180;

              // put both directions into one variable, each in a byte
              rwy.heading = (rwh1/10) * 256 + (rwh2/10);
            }
        }

      if( list[8].trimmed().length() ) // runway length in meters
        {
          // three units are possible:
          // o meter: m
          // o nautical mile: nm
          // o statute mile: ml
          // o feet: ft, @AP: Note that is not conform to the SeeYou specification
          //                  but I saw it in an south African file.
          QString unit;
          int uStart = list[8].indexOf( QRegExp("[fmn]") );

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
                  else if( unit == "ft" ) // feet
                    {
                      length *= 0.3048;
                    }

                  rwy.length = length;
                  rwy.isOpen = true;
                  rwy.isBidirectional = true;

                  // Store runway in the runway list.
                  wp.rwyList.append( rwy );
                }
            }
        }

      if( list.count() == 11 && list[10].trimmed().length() ) // description, optional
        {
          wp.comment += list[10].replace( QRegExp("\""), "" );
        }

      // We do check, if the waypoint name is already in use because cup
      // short names are not always unique.
      if( wpList && namesInUse.contains( wp.name ) )
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
          wp.wpListMember = true;
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

  return list;
}

bool WaypointCatalog::takeType( enum BaseMapElement::objectType type )
{
  // Check filter, if waypoint type should be taken
  if( _radius != -1 && _type != All )
    {
      switch( _type )
        {
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
