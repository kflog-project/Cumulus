/***********************************************************************
**
**   OpenAip.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2021 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>
#include <QtXml>

#include "AirspaceHelper.h"
#include "generalconfig.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "OpenAip.h"

extern MapMatrix* _globalMapMatrix;

OpenAip::OpenAip() :
  m_filterRadius(0.0),
  m_filterRunwayLength(0.0)
{
  m_supportedDataFormats << "1.1";
}

OpenAip::~OpenAip()
{
}

void OpenAip::loadUserFilterValues()
{
  m_filterRadius = 0.0;

  m_homePosition = _globalMapMatrix->getHomeCoord();

  // Get filter radius around the home position in kilometers.
  m_filterRadius = GeneralConfig::instance()->getAirfieldHomeRadius() / 1000.;

  // Get runway length filter in meters.
  m_filterRunwayLength = GeneralConfig::instance()->getAirfieldRunwayLengthFilter();
}

void OpenAip::fillRunwaySurfaceMapper()
{
  m_runwaySurfaceMapper.insert( "UNKN", Runway::Unknown );
  m_runwaySurfaceMapper.insert( "GRAS", Runway::Grass );
  m_runwaySurfaceMapper.insert( "ASPH", Runway::Asphalt );
  m_runwaySurfaceMapper.insert( "CONC", Runway::Concrete );
  m_runwaySurfaceMapper.insert( "SAND", Runway::Sand );
  m_runwaySurfaceMapper.insert( "WATE", Runway::Water );
  m_runwaySurfaceMapper.insert( "GRVL", Runway::Gravel );
  m_runwaySurfaceMapper.insert( "ICE", Runway::Ice );
  m_runwaySurfaceMapper.insert( "SNOW", Runway::Snow );
  m_runwaySurfaceMapper.insert( "SOIL", Runway::Soil );
}

bool OpenAip::getRootElement( QString fileName,
                              QString& dataFormat,
                              QString& dataItem )
{
  QFile file( fileName );

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) || file.size() == 0 )
    {
      qWarning() << "OpenAip::getRootElement: cannot open file:" << fileName;
      return false;
    }

  QXmlStreamReader xml( &file );

  int elementCounter = 0;

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll go to next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          qDebug() << "OAIP: File=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          QString errorInfo = "OpenAip::getRootElement: ";

          if( elementCounter == 1 )
            {
              if( elementName != "OPENAIP" )
                {
                  errorInfo += "No OPENAIP XML file";
                  qWarning() << errorInfo;
                  file.close();
                  return false;
                }

              QString version;

              bool ok = readVersionAndFormat( xml, version, dataFormat );

              if( ! ok )
                {
                  errorInfo += "Missing VERSION or DATAFORMAT attribute";
                  qWarning() << errorInfo;
                  file.close();
                  return false;
                }

              if( ! m_supportedDataFormats.contains( dataFormat ) )
                {
                  errorInfo = "OPENAIP data format " + dataFormat +
                              " is unsupported!";
                  qWarning() << errorInfo;
                  file.close();
                  return false;
                }
            }
          else if( elementCounter == 2 )
            {
              dataItem = xml.name().toString();
              file.close();
              return true;
            }
        }
    }

  // If the while loop is left, something has going wrong.
  if( xml.hasError() )
    {
      QString errorInfo = "XML-Error: " + xml.errorString() +
                          " at line=" + QString::number(xml.lineNumber()) +
                          " column=" + QString::number(xml.columnNumber()) +
                          " offset=" + QString::number(xml.characterOffset());

      qWarning() << "OpenAip::getRootElement:" << errorInfo;
    }

  file.close();
  return false;
}

bool OpenAip::readVersionAndFormat( QXmlStreamReader& xml,
                                    QString& version,
                                    QString& format )
{
  int error = 0;

  // Get data format attribute
  QXmlStreamAttributes attributes = xml.attributes();

  if( attributes.hasAttribute("VERSION") )
    {
      version = attributes.value("VERSION").toString();
    }
  else
    {
      version.clear();
      error++;
    }

  if( attributes.hasAttribute("DATAFORMAT") )
    {
      format = attributes.value("DATAFORMAT").toString();
    }
  else
    {
      format.clear();
      error++;
    }

  return (error == 0) ? true : false;
}

bool OpenAip::readNavAids( QString fileName,
                           QList<RadioPoint>& navAidList,
                           QString& errorInfo,
                           bool useFiltering )
{
  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readNavAids: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readNavAids: cannot open file:" << fileName;
      return false;
    }

  QXmlStreamReader xml( &file );

  int elementCounter   = 0;
  bool oaipFormatOk = false;

  // Reset version and data format variable
  m_oaipVersion.clear();
  m_oaipDataFormat.clear();

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll go to next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          // qDebug() << "File=" << fileName
          //         << "DocVersion=" << xml.documentVersion().toString()
          //         << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( elementCounter == 1 && elementName == "OPENAIP" )
            {
              oaipFormatOk =
                  readVersionAndFormat( xml, m_oaipVersion, m_oaipDataFormat );

              qDebug() << "OAIP: File=" << fileName
                       << "Version=" << m_oaipVersion
                       << "DataFormat=" << m_oaipDataFormat;
            }

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "NAVAIDS") ||
              oaipFormatOk == false )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readNavAids" << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter > 2 && elementName == "NAVAID" )
            {
              RadioPoint rp;

              // read navAid record
              if( ! readNavAidRecord( xml, rp ) )
                {
                  break;
                }

              if( useFiltering == true )
                {
                  if( m_filterRadius > 0.0 )
                    {
                      double d = MapCalc::dist( &m_homePosition, rp.getWGSPositionPtr() );

                      if( d > m_filterRadius )
                        {
                          // The radius filter said no. To far away from home.
                          continue;
                        }
                    }
                }

              navAidList.append( rp );
            }

          continue;
        }

      if( token == QXmlStreamReader::EndElement )
        {
          // qDebug() << "EndElement=" << xml.name().toString();
          continue;
        }
    }

  if( xml.hasError() )
    {
      errorInfo = "XML-Error: " + xml.errorString() +
                  " at line=" + QString::number(xml.lineNumber()) +
                  " column=" + QString::number(xml.columnNumber()) +
                  " offset=" + QString::number(xml.characterOffset());

      qWarning() << "OpenAip::readNavAids: XML-Error" << errorInfo;
      file.close();
      return false;
    }

  file.close();
  return true;
}

bool OpenAip::readNavAidRecord( QXmlStreamReader& xml, RadioPoint& rp )
{
  // Read navaid type
  QXmlStreamAttributes attributes = xml.attributes();

  if( attributes.hasAttribute("TYPE") )
    {
      // OpenAip NavAidTypes: "DVOR" "DVOR-DME" "DVORTAC" "VOR" "VOR-DME" "VORTAC"
      // Cumulus: Vor = VOR, VorDme = VORDME, VorTac = VORTAC, Ndb = NDB, CompPoint = COMPPOINT
      QString type = attributes.value("TYPE").toString();

      if( type == "DVOR" || type == "VOR" )
        {
          rp.setTypeID( BaseMapElement::Vor );
        }
      else if( type == "DVOR-DME" || type == "VOR-DME" || type == "DME" )
        {
          rp.setTypeID( BaseMapElement::VorDme );
        }
      else if( type == "TACAN" )
        {
          rp.setTypeID( BaseMapElement::Tacan );
        }
      else if( type == "DVORTAC" || type == "VORTAC" )
        {
          rp.setTypeID( BaseMapElement::VorTac );
        }
      else if( type == "NDB" )
        {
          rp.setTypeID( BaseMapElement::Ndb );
        }
      else
        {
          rp.setTypeID( BaseMapElement::NotSelected );
          qWarning() << "OpenAip::readNavAidRecord: unknown Navaid type" << type;
        }
    }

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "NAVAID" )
            {
              // All record data have been read.
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "COUNTRY" )
            {
              rp.setCountry( xml.readElementText().left(2).toUpper() );
            }
          else if ( elementName == "NAME" )
            {
              QString name = xml.readElementText().trimmed();
              upperLowerName( name );
              rp.setName( name );
            }
          else if ( elementName == "ID" )
            {
              QString id = xml.readElementText();
              rp.setICAO( id );
              rp.setWPName( id.left(8) );
            }
          else if ( elementName == "GEOLOCATION" )
            {
              readGeoLocation( xml, rp );
            }
          else if ( elementName == "RADIO" )
            {
              readRadio( xml, rp );
            }
          else if ( elementName == "PARAMS" )
            {
              readParams( xml, rp );
            }
        }
    }

  return true;
}

bool OpenAip::readGeoLocation( QXmlStreamReader& xml, SinglePoint& sp )
{
  double lat  = INT_MIN;
  double lon  = INT_MIN;
  double elev = INT_MIN;
  QString unit;

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "GEOLOCATION" )
            {
              // All record data have been read.
              if( lat != INT_MIN && lon != INT_MIN )
                {
                  // Convert latitude and longitude into KFLog's internal
                  // integer format.
                  int ilat = static_cast<int> (rint( 600000.0 * lat ));
                  int ilon = static_cast<int> (rint( 600000.0 * lon ));
                  WGSPoint wgsPoint( ilat, ilon );
                  sp.setWGSPosition( wgsPoint );

                  // Map WGS point to map projection
                  sp.setPosition( _globalMapMatrix->wgsToMap(wgsPoint) );
                }

              if( elev != INT_MIN )
                {
                  if( unit == "M" )
                    {
                      // elevation has sometimes decimals. Therefore we round
                      // it to integer.
                      sp.setElevation( rint(elev) );
                    }
                  else if( unit == "FT" )
                    {
                      Distance dist;
                      dist.setFeet( elev);
                      sp.setElevation( dist.getMeters() );
                    }
                }

              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "LAT" )
            {
              lat = xml.readElementText().toDouble();
            }
          else if ( elementName == "LON" )
            {
              lon = xml.readElementText().toDouble();
            }
          else if ( elementName == "ELEV" )
            {
              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("UNIT") )
                {
                  unit = attributes.value("UNIT").toString().toUpper();
                }

              elev = xml.readElementText().toDouble();
            }
        }
    }

  return true;
}

bool OpenAip::readRadio( QXmlStreamReader& xml, RadioPoint& rp )
{
  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "RADIO" )
            {
              // All record data have been read.
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "FREQUENCY" )
            {
              bool ok = false;
              float fre = xml.readElementText().toFloat( &ok );

              if( rp.getTypeID() == BaseMapElement::Ndb )
                {
                  // TODO Workaround to handle openAIP NDB frequencies.
                  // The frequency value is given in KHz and not in MHz.
                  // That is a bug in openAIP version 1.x
                  fre /= 1000.0;
                }

              if( ok )
                {
                  rp.addFrequency( Frequency(fre) );
                }
              else
                {
                  qWarning() << "OpenAip::readRadio():"
                             << "Line=" << xml.lineNumber()
                             << "Column=" << xml.columnNumber()
                             << "Radio frequency" << fre << "is not a floating type!";
                }
             }
          else if ( elementName == "CHANNEL" )
            {
              rp.setChannel( xml.readElementText() );
            }
        }
    }

  return true;
}

bool OpenAip::readParams( QXmlStreamReader& xml, RadioPoint& rp )
{
  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "PARAMS" )
            {
              // All record data have been read.
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "DECLINATION" )
            {
              bool ok = false;
              float value = xml.readElementText().toFloat( &ok );

              if( ok) rp.setDeclination( value );
             }
          else if ( elementName == "ALIGNEDTOTRUENORTH" )
            {
              if( xml.readElementText() == "TRUE" )
                {
                  rp.setAligned2TrueNorth( true );
                }
              else if( xml.readElementText() == "FALSE" )
                {
        	  rp.setAligned2TrueNorth( false );
                }
            }
          else if ( elementName == "RANGE" )
            {
              QXmlStreamAttributes attributes = xml.attributes();

              if( ! attributes.hasAttribute("UNIT") )
                {
                  xml.skipCurrentElement();
                  return false;
                }

              QString unit = attributes.value("UNIT").toString();
              QString unitValue = xml.readElementText();

              bool ok;
              double range = unitValue.toDouble(&ok);

              if( ok == false )
                {
                  qWarning() << "OpenAip::readParams: wrong range value:"
                             << unitValue;
                  xml.skipCurrentElement();
                  return false;
                }

              if( unit == "NM" )
                {
                  Distance sr;
                  sr.setNautMiles( range );
                  rp.setRange( sr.getMeters() );
                  return true;
                }

              qWarning() << "OpenAip::readParams: Unknown range unit:" << unit;
              xml.skipCurrentElement();
              return false;
            }
        }
    }

  return true;
}

//------------------------------------------------------------------------------

bool OpenAip::readHotspots( QString fileName,
                            QList<SinglePoint>& hotspotList,
                            QString& errorInfo,
                            bool useFiltering )
{
  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readHotspots: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readHotspots: cannot open file:" << fileName;
      return false;
    }

  QXmlStreamReader xml( &file );

  int elementCounter   = 0;
  bool oaipFormatOk = false;

  // Start index in hotspotList
  int startIdx = hotspotList.size();

  // Number counter for short name building.
  uint hsno = 0;

  // Reset version and data format variable
  m_oaipVersion.clear();
  m_oaipDataFormat.clear();

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll go to next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          // qDebug() << "File=" << fileName
          //         << "DocVersion=" << xml.documentVersion().toString()
          //         << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( elementCounter == 1 && elementName == "OPENAIP" )
            {
              oaipFormatOk =
                  readVersionAndFormat( xml, m_oaipVersion, m_oaipDataFormat );

              qDebug() << "OAIP: File=" << fileName
                       << "Version=" << m_oaipVersion
                       << "DataFormat=" << m_oaipDataFormat;
            }

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "HOTSPOTS") ||
              oaipFormatOk == false )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readHotspots" << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter > 2 && elementName == "HOTSPOT" )
            {
              SinglePoint sp;

              // Set type as thermal
              sp.setTypeID( BaseMapElement::Thermal );

              // read hotspot record
              if( ! readHotspotRecord( xml, sp ) )
                {
                  break;
                }

              // Increment name counter for the hotspot.
              hsno++;

              if( useFiltering == true )
                {
                  if( m_filterRadius > 0.0 )
                    {
                      double d = MapCalc::dist( &m_homePosition, sp.getWGSPositionPtr() );

                      if( d > m_filterRadius )
                        {
                          // The radius filter said no. To far away from home.
                          continue;
                        }
                    }
                }

              // Set record number as WP name
              sp.setWPName( QString("%1").arg(hsno) );
              hotspotList.append( sp );
            }

          continue;
        }

      if( token == QXmlStreamReader::EndElement )
        {
          // qDebug() << "EndElement=" << xml.name().toString();
          continue;
        }
    }

  if( xml.hasError() )
    {
      errorInfo = "XML-Error: " + xml.errorString() +
                  " at line=" + QString::number(xml.lineNumber()) +
                  " column=" + QString::number(xml.columnNumber()) +
                  " offset=" + QString::number(xml.characterOffset());

      qWarning() << "OpenAip::readHotspots: XML-Error" << errorInfo;
      file.close();
      return false;
    }

  if( startIdx < hotspotList.size() )
    {
      // Create a short name for the hotspot. The name is build from:
      // H = Hotspot
      // Two letter country code
      // consecutive number
      int fill = QString("%1").arg(hotspotList.size() - startIdx ).size();

      for( int i = startIdx; i < hotspotList.size(); i++ )
        {
          SinglePoint& sp = hotspotList[i];
          int idx = sp.getWPName().toInt();
          sp.setWPName( "H" + sp.getCountry() + QString("%1").arg(idx, fill, 10, QChar('0')));
        }
    }

  file.close();
  return true;
}

bool OpenAip::readHotspotRecord( QXmlStreamReader& xml, SinglePoint& sp )
{
  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "HOTSPOT" )
            {
              // All record data have been read.
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "COUNTRY" )
            {
              sp.setCountry( xml.readElementText().left(2).toUpper() );
            }
          else if ( elementName == "NAME" )
            {
              QString name = xml.readElementText().trimmed();

              upperLowerName( name );

              // Long name
              sp.setName( name );

              // Short name only 8 characters long
              sp.setWPName( name.left(8) );
            }
          else if ( elementName == "COMMENT" )
            {
              sp.setComment( xml.readElementText() );
            }
          else if ( elementName == "GEOLOCATION" )
            {
              readGeoLocation( xml, sp );
            }
        }
    }

  return true;
}

bool OpenAip::readAirfields( QString fileName,
                             QList<Airfield>& airfieldList,
                             QString& errorInfo,
                             bool useFiltering )
{
  if( useFiltering )
    {
      // Load the user's defined filter data.
      loadUserFilterValues();
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readAirfields: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readAirfields: cannot open file:" << fileName;
      return false;
    }

  m_shortNameSet.clear();

  fillRunwaySurfaceMapper();

  QXmlStreamReader xml( &file );

  int elementCounter   = 0;
  bool oaipFormatOk = false;

  // Reset version and data format variable
  m_oaipVersion.clear();
  m_oaipDataFormat.clear();

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll go to next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          // qDebug() << "File=" << fileName
          //         << "DocVersion=" << xml.documentVersion().toString()
          //         << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( elementCounter == 1 && elementName == "OPENAIP" )
            {
              oaipFormatOk =
                  readVersionAndFormat( xml, m_oaipVersion, m_oaipDataFormat );

              qDebug() << "OAIP: File=" << fileName
                       << "Version=" << m_oaipVersion
                       << "DataFormat=" << m_oaipDataFormat;
            }

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "WAYPOINTS") ||
              oaipFormatOk == false )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readAirfields" << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter > 2 && elementName == "AIRPORT" )
            {
              Airfield af;

              // read airfield record
              if( ! readAirfieldRecord( xml, af ) )
                {
                  break;
                }

              if( useFiltering == true )
                {
                  if( af.getTypeID() == BaseMapElement::CivHeliport ||
                      af.getTypeID() == BaseMapElement::MilHeliport )
                    {
                      // Filter out heli ports.
                      continue;
                    }

                  if( m_filterRadius > 0.0 )
                    {
                      double d = MapCalc::dist( &m_homePosition, af.getWGSPositionPtr() );

                      if( d > m_filterRadius )
                        {
                          // The radius filter said no. To far away from home.
                          continue;
                        }
                    }

                  if( m_filterRunwayLength > 0.0 )
                    {
                      QList<Runway>& rl = af.getRunwayList();

                      if( rl.isEmpty() )
                        {
                          // No runways are defined, ignore these data
                          continue;
                        }

                      bool rwyLenOk = false;
                      float rwy2short = 0.0;

                      for( int i = 0; i < rl.size(); i++ )
                        {
                          if( rl.at(i).m_length < m_filterRunwayLength )
                            {
                              rwy2short = rl.at(i).m_length;
                              continue;
                            }

                          // One runway fulfills the length condition, break loop.
                          rwyLenOk = true;
                          break;
                        }

                      if( rwyLenOk == false )
                        {
                          qWarning() << "OpenAip::readAirfields:"
                                     << af.getName() << af.getCountry()
                                     << "runway length" << rwy2short << "to short!";
                          continue;
                        }
                    }
                }


              // Short name is only 8 characters long and must be unique
              af.setWPName( shortName(af.getName()) );

              airfieldList.append( af );
            }

          continue;
        }

      if( token == QXmlStreamReader::EndElement )
        {
          // qDebug() << "EndElement=" << xml.name().toString();
          continue;
        }
    }

  if( xml.hasError() )
    {
      errorInfo = "XML-Error: " + xml.errorString() +
                  " at line=" + QString::number(xml.lineNumber()) +
                  " column=" + QString::number(xml.columnNumber()) +
                  " offset=" + QString::number(xml.characterOffset());

      qWarning() << "OpenAip::readNavAid: XML-Error" << errorInfo;
      file.close();
      return false;
    }

  file.close();
  return true;
}

bool OpenAip::readAirfieldRecord( QXmlStreamReader& xml, Airfield& af )
{
  // Read airport type
  QXmlStreamAttributes attributes = xml.attributes();

  if( attributes.hasAttribute("TYPE") )
    {
      /* OpenAip types:

      "AD_CLOSED"
      "AD_MIL"
      "AF_CIVIL"
      "AF_MIL_CIVIL"
      "AF_WATER"
      "APT"
      "GLIDING"
      "HELI_CIVIL"
      "HELI_MIL"
      "INTL_APT"
      "LIGHT_AIRCRAFT"

      Cumulus types:
      IntAirport, Airport, MilAirport, CivMilAirport,
      Airfield, ClosedAirfield, CivHeliport,
      MilHeliport, AmbHeliport, Gliderfield, UltraLight
      */

      QString type = attributes.value("TYPE").toString();

      if( type == "AD_CLOSED" )
        {
          af.setTypeID( BaseMapElement::ClosedAirfield );
        }
      else if( type == "AD_MIL" )
        {
          af.setTypeID( BaseMapElement::MilAirport );
        }
      else if( type == "AF_CIVIL" || type == "AF_WATER" )
        {
          af.setTypeID( BaseMapElement::Airfield );
        }
      else if( type == "AF_MIL_CIVIL" )
        {
          af.setTypeID( BaseMapElement::CivMilAirport );
        }
      else if( type == "APT" )
        {
          af.setTypeID( BaseMapElement::Airport );
        }
      else if( type == "GLIDING" )
        {
          af.setTypeID( BaseMapElement::Gliderfield );
        }
      else if( type == "HELI_CIVIL" )
        {
          af.setTypeID( BaseMapElement::CivHeliport );
        }
      else if( type == "HELI_MIL" )
        {
          af.setTypeID( BaseMapElement::MilHeliport );
        }
      else if( type == "INTL_APT" )
        {
          af.setTypeID( BaseMapElement::IntAirport );
        }
      else if( type == "LIGHT_AIRCRAFT" )
        {
          af.setTypeID( BaseMapElement::UltraLight );
        }
      else
        {
          af.setTypeID( BaseMapElement::NotSelected );
          qWarning() << "OpenAip::readNavAidRecord: unknown airfield type" << type;
        }
    }

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "AIRPORT" )
            {
              // All record data have been read.
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "COUNTRY" )
            {
              af.setCountry( xml.readElementText().left(2).toUpper() );
            }
          else if ( elementName == "NAME" )
            {
              // Airfield name lowered.
              QString name = xml.readElementText().toLower().trimmed();

              // Convert airfield name to upper-lower cases
              upperLowerName( name );

              // Long name
              af.setName( name );

              // Short name is only 8 characters long
              af.setWPName( name.left(8) );
            }
          else if ( elementName == "ICAO" )
            {
              af.setICAO( xml.readElementText() );
            }
          else if ( elementName == "GEOLOCATION" )
            {
              readGeoLocation( xml, af );
            }
          else if ( elementName == "RADIO" )
            {
              readAirfieldRadio( xml, af );
            }
          else if ( elementName == "RWY" )
            {
              readAirfieldRunway( xml, af );
            }
        }
    }

  return true;
}

bool OpenAip::readAirfieldRadio( QXmlStreamReader& xml, Airfield& af )
{
  // Read airfield radio category
  QXmlStreamAttributes attributes = xml.attributes();

  if( ! attributes.hasAttribute("CATEGORY") )
    {
      xml.skipCurrentElement();
      return false;
    }

  QString value = attributes.value("CATEGORY").toString();

  /*
  <RADIO CATEGORY="COMMUNICATION"> APPROACH, FIS, INFO, GROUND, TOWER, OTHER
  <RADIO CATEGORY="INFORMATION"> ATIS
  <RADIO CATEGORY="NAVIGATION"> ILS
  <RADIO CATEGORY="OTHER">
  */
  if( value != "COMMUNICATION" && value != "INFORMATION" )
    {
      xml.skipCurrentElement();
      return true;
    }

  // Only communication and information is interesting for us.
  float frequency = 0.0;
  bool ok = false;
  QString type;
  QString description;

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "RADIO" )
            {
              // All record data have been read inclusive the end element.
              if( ok && frequency >= 118.0 )
                {
                  // We add only speech frequencies.
                  af.addFrequency( Frequency(frequency, type) );
                }

              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "FREQUENCY" )
            {
              QString freqStr = xml.readElementText();

              frequency = freqStr.toFloat(&ok);

              if( ok == false )
                {
                  qWarning () << "OpenAip::readAirfieldRadio():" << "Line="
                      << xml.lineNumber () << "Column=" << xml.columnNumber ()
                      << "Radio frequency" << freqStr
                      << "is not a floating type!";
                }
            }
          else if ( elementName == "TYPE" )
            {
              type = xml.readElementText();
            }
          else if ( elementName == "DESCRIPTION" )
            {
              description = xml.readElementText();
            }
        }
    }

  return false;
}

bool OpenAip::readAirfieldRunway( QXmlStreamReader& xml, Airfield& af )
{
  Runway runway;

  // Get RWY OPERATIONS attribute
  QXmlStreamAttributes attributes = xml.attributes();

  if( attributes.hasAttribute("OPERATIONS") )
    {
      QString operations = attributes.value("OPERATIONS").toString();

      if( operations == "ACTIVE" )
        {
          runway.m_isOpen = true;
        }
    }

  int rwyNumber = 0;

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "RWY" )
            {
              // All record data have been read inclusive the end element.
              af.addRunway( runway );
              // runway.printData();
              return true;
            }
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          if( elementName == "NAME" )
            {
              // That element contains the usable runway headings
              QString name = xml.readElementText();

              runway.m_name = name;
            }
          else if ( elementName == "DIRECTION" )
            {
              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("TC") )
                {
                  bool ok;
                  ushort dir = attributes.value("TC").toString().toUShort( &ok );

                  rwyNumber++;

                  // Only two rwy are accepted.
                  if( ok && rwyNumber <= 2 )
                    {
                      // round up direction
                      dir = (dir + 5) / 10;

                      if( rwyNumber == 1 )
                        {
                          runway.m_heading = (dir << 8) + dir;
                        }
                      else if( rwyNumber == 2 )
                        {
                          runway.m_heading = (runway.m_heading & 0xff00) + (dir & 0xff);
                          runway.m_isBidirectional = true;
                        }
                    }
                }
            }
          else if( elementName == "SFC" )
            {
              /*
              <SFC>ASPH</SFC>
              <SFC>CONC</SFC>
              <SFC>GRAS</SFC>
              <SFC>GRVL</SFC>
              <SFC>SAND</SFC>
              <SFC>UNKN</SFC>
              */
              QString sfc = xml.readElementText();

              if( m_runwaySurfaceMapper.contains(sfc) == true )
                {
                  runway.m_surface = m_runwaySurfaceMapper.value(sfc);
                }
              else
                {
                  runway.m_surface = Runway::Unknown;

                  if( sfc.size() > 0 )
                    {
                      qWarning() << "OpenAip::readAirfieldRunway: unknown runway surface type"
                                 << sfc;
                    }
                }
            }
          else if ( elementName == "LENGTH" )
            {
              float length = 0.0;
              QString unit;

              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("UNIT") )
                {
                  unit = attributes.value("UNIT").toString().toUpper();
                }

              if( getUnitValueAsFloat( xml.readElementText(), unit, length ) )
                {
                  runway.m_length = length;
                }
            }
          else if ( elementName == "WIDTH" )
            {
              float width = 0;
              QString unit;

              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("UNIT") )
                {
                  unit = attributes.value("UNIT").toString().toUpper();
                }

              if( getUnitValueAsFloat( xml.readElementText(), unit, width ) )
                {
                  runway.m_width = width;
                }
            }
        }
    }

  return false;
}

bool OpenAip::getUnitValueAsInteger( const QString number,
                                     const QString unit,
                                     int& result )
{
  bool ok = false;

  double dValue = number.toDouble( &ok );

  if( !ok )
    {
      return false;
    }

  if( unit.toUpper() == "M" )
    {
      // Number can have decimals. Therefore we round it to integer.
      result = static_cast<int>(rint(dValue));
      return true;
    }
  else if( unit.toUpper() == "FT" )
    {
      // Number has the unit feet. We must convert it to meter.
      Distance dist;
      dist.setFeet( dValue );
      result = static_cast<int>(rint( dist.getMeters() ));
      return true;
    }

  return false;
}

bool OpenAip::getUnitValueAsFloat( const QString number,
                                   const QString unit,
                                   float& result )
{
  bool ok = false;

  float fValue = number.toFloat( &ok );

  if( !ok )
    {
      return false;
    }

  if( unit.toUpper() == "M" )
    {
      // Number can have decimals. Therefore we round it to integer.
      result = fValue;
      return true;
    }
  else if( unit.toUpper() == "FT" )
    {
      // Number has the unit feet. We must convert it to meter.
      Distance dist;
      dist.setFeet( fValue );
      result = static_cast<float>(dist.getMeters());
      return true;
    }

  return false;
}

void OpenAip::upperLowerName( QString& name )
{
  name = name.toLower();

  QSet<QChar> set;
  set.insert( ' ' );
  set.insert( '/' );
  set.insert( '-' );
  set.insert( '(' );

  QChar lastChar(' ');

  // Convert name to upper-lower cases
  for( int i=0; i < name.size(); i++ )
    {
      if( set.contains(lastChar) )
        {
          name.replace( i, 1, name.mid(i,1).toUpper() );
        }

      lastChar = name[i];
    }
}

/**
 * Create an unique short name by removing undesired characters.
 *
 * \param name The name to be shorten.
 *
 * \return new short name 8 characters long
 */
QString OpenAip::shortName( const QString& name )
{
  QString shortName;

  for( int i = 0; i < name.size(); i++ )
    {
      if( name.at(i).isLetterOrNumber() == false )
        {
          continue;
        }

      shortName.append( name[i] );

      if( shortName.size() == 8 )
        {
          // Limit short name to 8 characters.
          break;
        }
    }

  // Check, if short name is already in use. In this case create another one.
  if( ! m_shortNameSet.contains( shortName) )
    {
      m_shortNameSet.insert( shortName );
    }
  else
    {
      // Try to generate an unique short name. The assumption is that we never have
      // more than 9 equal names.
      for( int i=1; i < 10; i++ )
        {
          shortName.replace( shortName.length()-1, 1, QString::number(i) );

          if( ! m_shortNameSet.contains( shortName) )
            {
              m_shortNameSet.insert( shortName );
              break;
            }
        }
    }

  return shortName;
}

bool OpenAip::readAirspaces( QString fileName,
                             QList<Airspace*>& airspaceList,
                             QString& errorInfo,
                             bool doCompile )
{
  const char* method = "OpenAip::readAirspaces:";

  QFileInfo fi(fileName);
  int listStartIdx = airspaceList.size();

  if( fi.suffix().toLower() != "aip" )
    {
      errorInfo = fileName + " " + QObject::tr("has not suffix .aip!");
      qWarning() << method << fileName << "has not suffix .aip!";
      return false;
    }

  QFile file( fileName );

  if( file.exists() && file.size() == 0 )
    {
      errorInfo = QObject::tr("File %1 is empty").arg(fileName);
      qWarning() << "OpenAip::readAirspaces: File" << fileName << "is empty!";
      return false;
    }

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << method << "cannot open file:" << fileName;
      return false;
    }

  // Initialize airspace type mapper
  m_airspaceTypeMapper = AirspaceHelper::initializeAirspaceTypeMapping(fileName);

  if( m_airspaceTypeMapper.isEmpty() )
    {
      errorInfo = QObject::tr("Cannot load airspace type mapper for") + " " + fileName;
      qWarning() << method << "Cannot load airspace type mapper for" << fileName;
      return false;
    }

  QXmlStreamReader xml( &file );

  int elementCounter   = 0;
  bool oaipFormatOk = false;

  // Reset version and data format variable
  m_oaipVersion.clear();
  m_oaipDataFormat.clear();

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll go to next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          // qDebug() << "File=" << fileName
          //         << "DocVersion=" << xml.documentVersion().toString()
          //         << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it. */
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( elementCounter == 1 && elementName == "OPENAIP" )
            {
              oaipFormatOk = readVersionAndFormat( xml, m_oaipVersion, m_oaipDataFormat );

              qDebug() << "OAIP: File=" << fileName
                       << "Version=" << m_oaipVersion
                       << "DataFormat=" << m_oaipDataFormat;
            }

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "AIRSPACES") ||
              oaipFormatOk == false )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << method << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter > 2 && elementName == "ASP" )
            {
              Airspace as;

              // read airspace record
              if( readAirspaceRecord( xml, as ) )
                {
                  // Check lower and upper altitude
                  if( as.getLowerAltitude() == as.getUpperAltitude() )
                    {
                      qWarning() << "QIAP::Airspace Error"
                                 << "ID="
                                 << as.getId()
                                 << "Name="
                                 << as.getName()
                                 << "lower and upper altitude ("
                                 << as.getLowerAltitude().getFeet()
                                 << ") are the same!";
                      continue;
                    }

                  if( AirspaceHelper::addAirspaceIdentifier(as.getId()) )
                    {
                      Airspace* elem = as.createAirspaceObject();
                      airspaceList.append( elem );
                    }
                  else
                    {
                      // Airspace is already known. Ignore object.
                      qDebug() << "QIAP:: Known Airspace"
                               << as.getName()
                               << "ignored!";
                    }
                }
            }

          continue;
        }

      if( token == QXmlStreamReader::EndElement )
        {
          // qDebug() << "EndElement=" << xml.name().toString();
          continue;
        }
    }

  if( xml.hasError() )
    {
      errorInfo = "XML-Error: " + xml.errorString() +
                  " at line=" + QString::number(xml.lineNumber()) +
                  " column=" + QString::number(xml.columnNumber()) +
                  " offset=" + QString::number(xml.characterOffset());

      qWarning() << method << "XML-Error" << errorInfo;
      file.close();
      return false;
    }

  file.close();

  if( doCompile )
    {
      // Build the compiled file name with the extension .aic from
      // the source file name
       QString cfn = fi.path() + "/" + fi.completeBaseName() + ".aic";

       AirspaceHelper::createCompiledFile( cfn, airspaceList, listStartIdx );
    }

  return true;
}

bool OpenAip::readAirspaceRecord( QXmlStreamReader& xml, Airspace& as )
{
  const char* method ="OpenAip::readAirspaceRecord:";

  ushort error = 0;

  // Read airspace category
  QXmlStreamAttributes attributes = xml.attributes();

  if( attributes.hasAttribute("CATEGORY") )
    {
      /* OpenAip airspace categories:
      "A"
      "B"
      "C"
      "CTR"
      "D"
      "DANGER"
      "E"
      "F"
      "FIR"
      "G"
      "GLIDING"
      "OTH"
      "RESTRICTED"
      "TMA"
      "RMZ"  // maybe defined in the future
      "TMZ"
      "WAVE"
      */

      QString category = attributes.value("CATEGORY").toString();

      if( m_airspaceTypeMapper.contains(category) == false )
        {
          qWarning() << method
                     << "Line:" << xml.lineNumber()
                     << "Ignoring Airspace, unknown category:" << category;

          xml.skipCurrentElement();
          return false;
        }

      // Apply user mapping rules
      as.setTypeID( m_airspaceTypeMapper.value(category, BaseMapElement::AirUkn) );
    }

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream. */
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "ASP" )
            {
              // All record data have been read.
              return (error == 0 ? true : false);
            }

          continue;
        }

      BaseMapElement::elevationType altReference;
      Altitude altitude;

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          QString elementName = xml.name().toString();

          // qDebug() << "Element=" << elementName;

          if( elementName == "ID" )
            {
              // We store the id in the airspace object, to filter out
              // duplicates.
              bool ok;
              int id = xml.readElementText().toInt(&ok);

              if( ok )
                {
                  as.setId(id);
                }
            }
          else if( elementName == "COUNTRY" )
            {
              as.setCountry( xml.readElementText().left(2).toUpper() );
            }
          else if ( elementName == "NAME" )
            {
              // Airspace name
              as.setName( xml.readElementText() );
            }
          else if ( elementName == "ALTLIMIT_TOP" )
            {
              bool ok = readAirspaceLimitReference( xml, altReference );

              if( ! ok )
                {
                  error++;
                }
              else
                {
                  as.setUpperT( altReference );

                  QString unit;
                  ok = readAirspaceAltitude( xml, unit, altitude );

                  if( ! ok )
                    {
                      error++;
                    }
                  else
                    {
                      as.setUpperL( altitude );

                      // Cumulus sets this combination to FL internally.
                      if( altReference == BaseMapElement::STD && unit == "FL" )
                        {
                          as.setUpperT( BaseMapElement::FL );
                        }
                    }
                }
            }
          else if ( elementName == "ALTLIMIT_BOTTOM" )
            {
              bool ok = readAirspaceLimitReference( xml, altReference );

              if( ! ok )
                {
                  error++;
                }
              else
               {
                 as.setLowerT( altReference );

                 QString unit;
                 ok = readAirspaceAltitude( xml, unit, altitude );

                 if( ! ok )
                   {
                     error++;
                   }
                 else
                   {
                     as.setLowerL( altitude );

                     // Cumulus sets this combination to FL internally.
                     if( altReference == BaseMapElement::STD && unit == "FL" )
                       {
                         as.setLowerT( BaseMapElement::FL );
                       }
                   }
               }
            }
          else if ( elementName == "GEOMETRY" )
            {
              bool ok = readAirspaceGeometrie( xml, as );

              if( ! ok )
                {
                  error++;
                }
            }
        }
    }

  return (error == 0 ? true : false);
}

bool OpenAip::readAirspaceLimitReference( QXmlStreamReader& xml,
                                          BaseMapElement::elevationType& reference )
{
  // Read airspace limit reference
  QXmlStreamAttributes attributes = xml.attributes();

  reference = BaseMapElement::NotSet;

  if( ! attributes.hasAttribute("REFERENCE") )
    {
      xml.skipCurrentElement();
      return false;
    }

  QString value = attributes.value("REFERENCE").toString();

  if( value == "GND" )
    {
      reference = BaseMapElement::GND;
    }
  else if( value == "MSL" )
    {
      reference = BaseMapElement::MSL;
    }
  else if( value == "STD" )
    {
      reference = BaseMapElement::STD;
    }
  else
    {
      return false;
    }

  return true;
}

bool OpenAip::readAirspaceAltitude( QXmlStreamReader& xml,
                                    QString& unit,
                                    Altitude& altitude )
{
  xml.readNextStartElement();

  if( xml.atEnd() || xml.hasError() || ! xml.isStartElement() || xml.name() != "ALT" )
    {
      xml.skipCurrentElement();
      altitude = Altitude();
      return false;
    }

  QXmlStreamAttributes attributes = xml.attributes();

  if( ! attributes.hasAttribute("UNIT") )
    {
      xml.skipCurrentElement();
      altitude = Altitude();
      return false;
    }

  unit = attributes.value("UNIT").toString();
  QString unitValue = xml.readElementText();

  bool ok;
  double alt = unitValue.toDouble(&ok);

  if( ok == false )
    {
      qWarning() << "OpenAip::readAirspaceAltitude: wrong altitude value:"
                 << unitValue;
      xml.skipCurrentElement();
      altitude = Altitude();
      return false;
    }

  if( unit == "F" || unit == "FL" )
    {
      altitude.setFeet( alt );
      return true;
    }

  qWarning() << "OpenAip::readAirspaceAltitude: Unknown altitude unit:" << unit;
  xml.skipCurrentElement();
  altitude = Altitude();
  return false;
}

bool OpenAip::readAirspaceGeometrie( QXmlStreamReader& xml, Airspace& as )
{
  const char* method = "OpenAip::readAirspaceGeometrie:";

  xml.readNextStartElement();

  if( xml.atEnd() || xml.hasError() || ! xml.isStartElement() || xml.name() != "POLYGON" )
    {
      xml.skipCurrentElement();
      return false;
    }

  QStringList polygonList = xml.readElementText().split(QRegExp(",?\\s+"), QString::SkipEmptyParts );

  if( polygonList.isEmpty() )
    {
      qWarning() << method << "Polygon list is empty at line" << xml.lineNumber();
      xml.skipCurrentElement();
      return false;
    }

  // The list consists of pairs Longitude, Latitude
  if( polygonList.size() % 2 )
    {
      qWarning() << method << "Polygon list is odd at line" << xml.lineNumber();
      xml.skipCurrentElement();
      return false;
    }

  QPolygon asPolygon( polygonList.size() / 2 );
  extern MapMatrix* _globalMapMatrix;

  for( int i = 0; i < polygonList.size(); i += 2 )
    {
      // The list consists of pairs Longitude, Latitude
      // Convert coordinate to double and check range
      float lon = 0.0, lat = 0.0;
      bool okLon = false, okLat = false;
      int error = 0;

      lon = polygonList.at(i).toFloat(&okLon);
      lat = polygonList.at(i+1).toFloat(&okLat);

      if( okLon == false || lon < -180.0 || lon > 180.0 )
        {
          qWarning() << method << "Wrong longitude value"
                     << polygonList.at(i)
                     << "read at line" << xml.lineNumber();
          error++;
        }

      if( okLat == false || lat < -90.0 || lat > 90.0 )
        {
          qWarning() << method << "Wrong latitude value"
                     << polygonList.at(i+1)
                     << "read at line" << xml.lineNumber();
          error++;
        }

      if( error > 0 )
        {
          return false;
        }

      // Convert coordinates into KFLog format
      int latInt = static_cast<int> (rint(600000.0 * lat));
      int lonInt = static_cast<int> (rint(600000.0 * lon));

      // Project coordinates to map datum and store them in a polygon
      asPolygon.setPoint( i/2, _globalMapMatrix->wgsToMap( latInt, lonInt ) );
    }

  if( asPolygon.count() < 2 )
    {
      qWarning() << method << "Line" << xml.lineNumber()
                 << "Airspace" << as.getName()
                 << "contains to less coordinates! Ignoring it.";
      return false;
    }

  // Airspaces are stored as polygons and should not contain the start point
  // twice as done in OpenAip description.
  if ( asPolygon.count() > 2 && asPolygon.first() == asPolygon.last() )
    {
      // remove the last point because it is identical to the first point
      asPolygon.remove(asPolygon.count()-1);
    }

  as.setProjectedPolygon( asPolygon );
  return true;
}
