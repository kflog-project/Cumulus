/***********************************************************************
**
**   openaip.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>
#include <QtXml>

#include "distance.h"
#include "generalconfig.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "openaip.h"

extern MapMatrix* _globalMapMatrix;

OpenAip::OpenAip() :
  m_filterRadius(0.0)
{
}

OpenAip::~OpenAip()
{
}

bool OpenAip::getRootElement( QString fileName, QString& dataItem )
{
  QFile file( fileName );

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
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
          qDebug() << "File=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          if( elementCounter == 1 && elementName != "OPENAIP" )
            {
              QString errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readNavAids" << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter == 2 )
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
                          " at line=" + xml.lineNumber() +
                          " column=" + xml.columnNumber() +
                          " offset=" + xml.characterOffset();

      qWarning() << "OpenAip::readNavAids: XML-Error" << errorInfo;
     }

  file.close();
  return false;
}

bool OpenAip::readNavAids( QString fileName,
                           QList<RadioPoint>& navAidList,
                           QString& errorInfo )
{
  QFile file( fileName );

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readNavAids: cannot open file:" << fileName;
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
          qDebug() << "File=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "NAVAIDS") )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readNavAids" << errorInfo;
              file.close();
              return false;
            }

          if( elementCounter > 2 && elementName == "NAVAID" )
            {
              RadioPoint rp;

              // read navaid record
              if( ! readNavAidRecord( xml, rp ) )
                {
                  break;
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
                  " at line=" + xml.lineNumber() +
                  " column=" + xml.columnNumber() +
                  " offset=" + xml.characterOffset();

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
      else if( type == "DVOR-DME" || type == "VOR-DME" )
        {
          rp.setTypeID( BaseMapElement::VorDme );
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
              rp.setName( xml.readElementText() );
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

              if( ok) rp.setFrequency( fre );
             }
          else if ( elementName == "CHANNEL" )
            {
              rp.setChannel( xml.readElementText() );
            }
        }
    }

  return true;
}

//------------------------------------------------------------------------------

bool OpenAip::readHotspots( QString fileName,
                            QList<SinglePoint>& hotspotList,
                            QString& errorInfo )
{
  QFile file( fileName );

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readHotspots: cannot open file:" << fileName;
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
          qDebug() << "File=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "HOTSPOTS") )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readNavAid" << errorInfo;
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
                  " at line=" + xml.lineNumber() +
                  " column=" + xml.columnNumber() +
                  " offset=" + xml.characterOffset();

      qWarning() << "OpenAip::readNavAid: XML-Error" << errorInfo;
      file.close();
      return false;
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
              QString name = xml.readElementText();

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

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
      errorInfo = QObject::tr("Cannot open file") + " " + fileName;
      qWarning() << "OpenAip::readAirfields: cannot open file:" << fileName;
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
          qDebug() << "File=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();

          // qDebug() << "StartElement=" << elementName;

          if( (elementCounter == 1 && elementName != "OPENAIP") ||
              (elementCounter == 2 && elementName != "WAYPOINTS") )
            {
              errorInfo = QObject::tr("Wrong XML data format");
              qWarning() << "OpenAip::readNavAid" << errorInfo;
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

                  if( m_countryFilterSet.isEmpty() == false )
                    {
                      if( m_countryFilterSet.contains(af.getCountry()) == false )
                        {
                          // The country filter said no
                          continue;
                        }
                    }

                  if( m_filterRadius > 0.0 )
                    {
                      double d = dist( &m_homePosition, af.getWGSPositionPtr() );

                      if( d > m_filterRadius )
                        {
                          // The radius filter said no. To far away from home.
                          continue;
                        }
                    }
                }

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
                  " at line=" + xml.lineNumber() +
                  " column=" + xml.columnNumber() +
                  " offset=" + xml.characterOffset();

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
      else if( type == "AF_CIVIL" )
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
              QString name = xml.readElementText().toLower();

              // Convert airfield name to upper-lower cases
              upperLowerName( name );

              // Long name
              af.setName( name );

              // Short name is only 8 characters long
              QString sn = shortName(name);

              af.setWPName( shortName(sn) );
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
              if( ok )
                {
                  if ( type == "INFO" || type == "TOWER" || type == "OTHER" )
                    {
                      af.setFrequency( frequency );
                    }
                  else if( type == "ATIS" )
                    {
                      af.setAtis( frequency );
                    }
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
              frequency = xml.readElementText().toFloat(&ok);
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

  while( !xml.atEnd() && ! xml.hasError() )
    {
      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      if( token == QXmlStreamReader::EndElement )
        {
          if( xml.name() == "RWY" )
            {
              runway.isOpen = true;

              // All record data have been read inclusive the end element.
              af.addRunway( runway );
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

              if( name.size() == 2 )
                {
                  // We have only one runway heading 06
                  runway.heading = (name.toUShort() * 256) + name.toUShort();
                }
              else if( name.size() == 5 )
                {
                  // WE have two runway headings 06/24
                  ushort dir1 = name.left(2).toUShort() * 256;
                  ushort dir2 = name.mid(3, 2).toUShort();

                  runway.heading = dir1 + dir2;
                }
              else if( name.size() == 7 )
                {
                  // WE have two directions beside 06R/24L 06L/24R
                  ushort dir1 = name.left(2).toUShort() * 256;
                  ushort dir2 = name.mid(4, 2).toUShort();

                  runway.heading = dir1 + dir2;
                }
            }
          else if ( elementName == "BIDIRECTIONAL" )
            {
              if( xml.readElementText() == "TRUE" )
                {
                  runway.isBidirectional = true;
                }
              else if( xml.readElementText() == "FALSE" )
                {
                  runway.isBidirectional = false;
                }
            }
          else if( elementName == "SFC" )
            {
              /*
              <SFC>ASPH</SFC>
              <SFC>CONC</SFC>
              <SFC>GRAS</SFC>
              <SFC>GRVL</SFC>
              <SFC>UNKN</SFC>
              */
              QString sfc = xml.readElementText();

              if( sfc == "ASPH" )
                {
                  runway.surface = Runway::Asphalt;
                }
              else if( sfc == "CONC" )
                {
                  runway.surface = Runway::Concrete;
                }
              else if( sfc == "GRAS" )
                {
                  runway.surface = Runway::Grass;
                }
              else if( sfc == "GRVL" )
                {
                  runway.surface = Runway::Sand;
                }
              else if( sfc == "UNKN" )
                {
                  runway.surface = Runway::Unknown;
                }
              else
                {
                  runway.surface = Runway::Unknown;

                  if( sfc.size() > 0 )
                    {
                      qWarning() << "OpenAip::readAirfieldRunway: unknown runway surface type"
                                 << sfc;
                    }
                }
            }
          else if ( elementName == "LENGTH" )
            {
              int length = 0;
              QString unit;

              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("UNIT") )
                {
                  unit = attributes.value("UNIT").toString().toUpper();
                }

              if( getUnitValueAsInteger( xml.readElementText(), unit, length ) )
                {
                  runway.length = static_cast <ushort>(length);
                }
            }
          else if ( elementName == "WIDTH" )
            {
              int width = 0;
              QString unit;

              QXmlStreamAttributes attributes = xml.attributes();

              if( attributes.hasAttribute("UNIT") )
                {
                  unit = attributes.value("UNIT").toString().toUpper();
                }

              if( getUnitValueAsInteger( xml.readElementText(), unit, width ) )
                {
                  runway.width = static_cast <ushort>(width);
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
      // NUmber has the unit feet. We must convert it to meter.
      Distance dist;
      dist.setFeet( dValue );
      result = static_cast<int>(rint( dist.getMeters() ));
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
 * Create a short name by removing undesired characters.
 *
 * \param name The name to be shorten.
 *
 * \return new short name 8 characters long
 */
QString OpenAip::shortName( QString& name )
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

  return shortName;
}

void OpenAip::loadUserFilterValues()
{
  m_countryFilterSet.clear();
  m_filterRadius = 0.0;

  m_homePosition = _globalMapMatrix->getHomeCoord();

  QString cFilter = GeneralConfig::instance()->getWelt2000CountryFilter().toUpper();

  QStringList clist = cFilter.split( QRegExp("[, ]"), QString::SkipEmptyParts );

  for( int i = 0; i < clist.size(); i++ )
    {
      m_countryFilterSet.insert( clist.at(i) );
    }

  int iRadius = GeneralConfig::instance()->getWelt2000HomeRadius();

  // We must look, what unit the user has chosen. This unit must
  // be considered during load of data items.
  m_filterRadius = Distance::convertToMeters( iRadius ) / 1000.;
}
