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
 **                   2008-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <unistd.h>

#include <QtGui>
#include <QtXml>

#include "distance.h"
#include "filetools.h"
#include "Frequency.h"
#include "generalconfig.h"
#include "OpenAip.h"
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
#define WP_FILE_FORMAT_ID_5 105 // runway length stored as float to avoid rounding issues between ft - m
#define WP_FILE_FORMAT_ID_6 106 // frequency list introduced
#define WP_FILE_FORMAT_ID_7 107 // runway name added

// Note! If you introduce a new WP_FILE_FORMAT_ID, consider readOldVersion
// handling!

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

  bool readOldVersion = false;

  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  qint8 wpType;
  qint32 wpLatitude;
  qint32 wpLongitude;
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

  // new element from format 6
  QList<Frequency> frequencyList;

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
      file.close();
      return -1;
    }

  in >> fileType;

  if (fileType != FILE_TYPE_WAYPOINTS)
    {
      qWarning("Waypoint file is a KFLog file, but not for waypoints!");
      file.close();
      return -1;
    }

  in >> fileFormat;

  if( fileFormat < WP_FILE_FORMAT_ID_6 )
    {
      // we have read an old version.
      readOldVersion = true;
    }

  if( fileFormat < WP_FILE_FORMAT_ID_2 )
    {
      qWarning() << "Wrong waypoint file format! Read format Id"
                 << fileFormat
                 << ". Expecting" << WP_FILE_FORMAT_ID_3 << ".";

      file.close();
      return -1;
    }

  // from here on, we assume that the file has the correct format.
  if( fileFormat < WP_FILE_FORMAT_ID_6 )
    {
      in.setVersion( QDataStream::Qt_4_7 );
    }
  else
    {
      in.setVersion( QDataStream::Qt_4_8 );
    }

  in >> wpListSize;

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

      frequencyList.clear();
      rwyList.clear();

      // read values from file
      in >> wpName;
      in >> wpDescription;
      in >> wpICAO;
      in >> wpType;
      in >> wpLatitude;
      in >> wpLongitude;
      in >> wpElevation3;

      if( fileFormat < WP_FILE_FORMAT_ID_6 )
        {
          in >> wpFrequency3;
        }
      else
        {
          quint8 listSize;
          float frequency;
          QString type;

          in >> listSize;

          for( int i = 0; i < (int) listSize; i++ )
            {
              in >> frequency;
              in >> type;
              frequencyList.append( Frequency( frequency, type) );
            }
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
      in >> wpCountry;

      if( fileFormat >= WP_FILE_FORMAT_ID_4 )
        {
          // The runway list has to be read
          QByteArray utf8_temp;
          QString name;
          quint8 listSize;
          quint16 ilength;
          float   flength;
          quint16 iwidth;
          float   fwidth;
          quint16 heading;
          quint8 surface;
          quint8 isOpen;
          quint8 isBidirectional;

          in >> listSize;

          for( int i = 0; i < (int) listSize; i++ )
            {
              if( fileFormat >= WP_FILE_FORMAT_ID_7 )
                {
                  // read runway name
                  ShortLoad(in, utf8_temp);
                  name = QString::fromUtf8(utf8_temp);
                }

              if( fileFormat >= WP_FILE_FORMAT_ID_5 )
                {
                  in >> flength;
                  in >> fwidth;
                }
              else
                {
                  in >> ilength;
                  flength = static_cast<float>(ilength);
                  in >> iwidth;
                  fwidth = static_cast<float>(iwidth);
                }

              in >> heading;
              in >> surface;
              in >> isOpen;
              in >> isBidirectional;

              Runway rwy( name, flength, heading, surface, isOpen, isBidirectional, fwidth );
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
          wp.elevation = wpElevation3;

          if( fileFormat < WP_FILE_FORMAT_ID_6 )
            {
              wp.addFrequency( Frequency(wpFrequency3) );
            }
          else
            {
              // The read frequency list has to be assigned
              wp.frequencyList = frequencyList;
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
                  Runway rwy( "", wpLength3, wpRunway, wpSurface, true, true );
                  wp.rwyList.append( rwy );
                }
            }

          wpList->append(wp);
        }
    }

  file.close();

  if( readOldVersion == true )
    {
      // create a new waypoint file in the changed format
      writeBinary( catalog, *wpList );
    }

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
  QString wpComment="";
  quint8 wpImportance;

  QFile file( fName );

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  if( file.open( QIODevice::WriteOnly ) )
    {
      QDataStream out( &file );
      out.setVersion( QDataStream::Qt_4_8 );

      // write file header
      out << quint32( KFLOG_FILE_MAGIC );
      out << qint8( FILE_TYPE_WAYPOINTS );
      out << quint16( WP_FILE_FORMAT_ID_7 );
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
          wpComment = wp.comment;
          wpImportance = wp.priority;

          out << wpName;
          out << wpDescription;
          out << wpICAO;
          out << wpType;
          out << wpLatitude;
          out << wpLongitude;
          out << wpElevation;

          // The frequency list is saved
          out << quint8( wp.frequencyList.size() );

          for( int i = 0; i < wp.frequencyList.size(); i++ )
            {
              Frequency fre = wp.frequencyList.at(i);

              out << fre.getFrequency();
              out << fre.getType();
            }

          out << wpComment;
          out << wpImportance;
          out << wp.country;

          // The runway list is saved
          out << quint8( wp.rwyList.size() );

          for( int i = 0; i < wp.rwyList.size(); i++ )
            {
              Runway rwy = wp.rwyList.at(i);

              // element name
              ShortSave(out, rwy.getName().toUtf8());
              out << rwy.m_length;
              out << rwy.m_width;
              out << quint16( rwy.m_heading );
              out << quint8( rwy.m_surface );
              out << quint8( rwy.m_isOpen );
              out << quint8( rwy.m_isBidirectional );
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
      w.addFrequency(nm.namedItem("Frequency").toAttr().value().toFloat());

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
          Runway rwy( "", length, heading, surface, true, true );
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

      if( w.frequencyList.isEmpty() )
        {
          child.setAttribute( "Frequency", 0 );
        }
      else
        {
          child.setAttribute( "Frequency", w.frequencyList.at(0).getFrequency() );
        }

      child.setAttribute( "Landable", w.rwyList.size() > 0 ? true : false );

      // Only the main runway is stored as 10...36
      if( w.rwyList.size() )
        {
          Runway rwy = w.rwyList.first();

          child.setAttribute( "Runway", rwy.m_heading/256 );
          child.setAttribute( "Length", rwy.m_length );
          child.setAttribute( "Surface", rwy.m_surface );
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

  if( ! openAip.readNavAids( catalog, navAidList, errorInfo, false ) )
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
      RadioPoint& rp = navAidList[i];

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
      wp.frequencyList = rp.getFrequencyList();
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
      wp.frequencyList = af.getFrequencyList();

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


int WaypointCatalog::readBgaDos( QString catalog,
                                 QList<Waypoint>* wpList,
                                 QString& errorInfo )
{
  // Found a file format description here:
  // http://www.gregorie.org/gliding/pna/cai_format.html
  qDebug() << "WaypointCatalog::readBgaDos" << catalog;

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

  if (! file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      errorInfo = QObject::tr("Cannot open File!");
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

  QTextStream in(&file);
  in.setCodec( "ISO 8859-15" );

  // Only 25 animations should be done because the animation is a performance
  // blocker.
  int wsMove = file.size() / 25;
  int nextWsMove = wsMove;
  int curPos = 0;

  /**
    The DOS format is as follows.

     0. Full Name
     1. TriGraph
     2. Category for findability (A-D) and airspace # and ##
     3. Exact point
     4. Description & Remarks
     5. Dist from main feature, NMl
     6. Direction from main feature
     7. Main feature (one of several large towns and cities used as locators)
     8. Grid ref km East and N of OS Datum
     9. Lat/Long to WGS84 Geodetic Datum
    10. Altitude (in Feet, accuracy not Guaranteed)
    11. TriGraph

     0. Aboyne Bridge
     1. AB1
     2. A
     3. Road Br over R Dee
     4. S side of village bet A93 and B976, 2NMl E of airfield, under CTA base 3000
     5. 22
     6. W
     7. Aberdeen
     8. 352.36 797.96
     9. 57 04.213N 002 47.239W
    10. 450
    11. AB1
  */

  while( true )
    {
      QStringList record;
      int readLines;
      int start = lineNo + 1;

      for( readLines = 0; readLines < 13; readLines++ )
        {
          QString line = in.readLine();
          curPos += line.size() + 2;
          record << line.trimmed();
          lineNo++;

          if( in.atEnd() )
            {
              break;
            }
        }

      if( record.size() < 12 )
        {
          // EOF is reached
          break;
        }

      if( _showProgress && curPos >= nextWsMove )
        {
          nextWsMove += wsMove;
          ws->slot_Progress( 2 );
          QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents |
                                           QEventLoop::ExcludeSocketNotifiers );
        }

      if( record[1] != record[11] )
        {
          // Filter out corrupted lines
          qWarning("DOS Read (%d): Short code unequal final code", start );
          continue;
        }

      if( record[0].isEmpty() ||
          record[1].isEmpty() ||
          record[2].isEmpty() ||
          record[9].isEmpty() ||
          record[10].isEmpty() ||
          record[11].isEmpty() )
        {
          qWarning( "DOS Read (%d): Record contains too less elements! Ignoring it.",
                    start );
          continue;
        }

      Waypoint wp;

      wp.type = BaseMapElement::Landmark;
      wp.priority = Waypoint::Low;
      wp.country = GeneralConfig::instance()->getHomeCountryCode();

      // Name is composed from TriGraph "-" and Category for findability (A-D)
      // and airspace # and ##
      wp.name = record[1] + "-" + record[2];

      // Short name of a waypoint has only 8 characters and upper cases in Cumulus.
      wp.name = wp.name.left(8).toUpper().trimmed();
      wp.description = record[0];

      // latitude and longitude as 57 04.213N 002 47.239W
      QStringList latlon = record[9].split(" ");

      if( latlon.size() != 4 )
        {
          qWarning("DOS Read (%d): Format error WGS84 geodetic datum", start);
          continue;
        }

      bool ok;

      // Extract latitude
      double degree = latlon[0].toDouble(&ok);

      if( ! ok )
        {
          qWarning("DOS Read (%d): Format error degree WGS84 latitude", start);
          continue;
        }

      double minutes = latlon[1].left(latlon[1].size() - 1 ).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DOS Read (%d): Format error minutes WGS84 latitude", start);
          continue;
        }

      double latTmp = (degree * 600000.) + (10000. * minutes);

      if( latlon[1].right(1).toUpper() == "S" )
        {
          latTmp = -latTmp;
        }

      // Extract longitude
      degree = latlon[2].toDouble(&ok);

      if( ! ok )
        {
          qWarning("DOS Read (%d): Format error degree WGS84 longitude", start);
          continue;
        }

      minutes = latlon[3].left(latlon[3].size() - 1 ).toDouble(&ok);

      if( ! ok )
        {
          qWarning("DOS Read (%d): Format error minutes WGS84 longitude", start);
          continue;
        }

      double lonTmp = (degree * 600000.) + (10000. * minutes);

      if( latlon[3].right(1).toUpper() == "W" )
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

      // Altitude (in Feet, accuracy not Guaranteed)
      float tmpElev = record[10].toFloat(&ok);

      if( ! ok )
        {
          qWarning("DOS Read (%d): Error reading elevation value '%s'.",
                    start, record[10].toLatin1().data());
          continue;
        }

      // Convert feet to meters
      wp.elevation = tmpElev * 0.3048;

      // Check filter, if type should be taken
      if( ! takeType( (enum BaseMapElement::objectType) wp.type ) )
        {
          continue;
        }

      wp.comment = record[3] + "\n\n" + record[4];

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
      rwy.m_surface = Runway::Unknown;

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
          rwy.m_surface = Runway::Grass;
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
          rwy.m_surface = Runway::Concrete;
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

              rwy.m_length = length;
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
              wp.addFrequency( Frequency(frequency, "CUP") );
            }
          else
            {
              wp.addFrequency( Frequency(0.0) );
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
              rwy.m_heading = (rwh1/10) * 256 + (rwh2/10);
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

                  rwy.m_length = length;
                  rwy.m_isOpen = true;
                  rwy.m_isBidirectional = true;

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

      double d = MapCalc::dist( &centerPoint, &point );

      if( d > radiusInKm )
        {
          // Distance is greater than the defined radius around the center point.
          return false;
        }
    }

  return true;
}
