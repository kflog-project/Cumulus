/***********************************************************************
**
**   XCSoar.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2021 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>
#include <QXmlStreamReader>

#include "singlepoint.h"
#include "taskpoint.h"
#include "XCSoar.h"

/**
 * Reads a single XCSoar task file created by WeGlide and returns a FlightTask
 * object.
 *
 * \param fileName of task with extention .tsk
 *
 * \param errorInfo info in case of error.
 *
 * \return FlightTask object in case of success other wise 0.
 */

FlightTask* XCSoar::reakTaskFile( QString fileName, QString& errorInfo )
{
  qDebug() << "XCSoar::readTaskFile():" << fileName;

  if( fileName.isEmpty() )
    {
      return nullptr;
    }

  if( fileName.toLower().endsWith( ".tsk") == false )
    {
      return nullptr;
    }

  QFile file( fileName );

  if( ! file.open(QIODevice::ReadOnly | QIODevice::Text) || file.size() == 0 )
    {
      qWarning() << "XCSoar::reakTaskFile: cannot open file:" << fileName;
      return nullptr;
    }

  QString taskName = QFileInfo( fileName ).baseName();
  QList<TaskPoint> tpList;
  int elementCounter = 0;
  QString taskType;
  QString pointType;
  QString wpName;
  QString wpLatitude;
  QString wpLongitude;
  QString ozRadius;
  QString ozType;

  QXmlStreamReader xml( &file );

  while( !xml.atEnd() && ! xml.hasError() )
    {
      QXmlStreamAttributes attributes;

      /* Read the next element from the stream.*/
      QXmlStreamReader::TokenType token = xml.readNext();

      /* If token is just StartDocument, we'll goto next.*/
      if( token == QXmlStreamReader::StartDocument )
        {
          qDebug() << "Taskfile=" << fileName
                   << "DocVersion=" << xml.documentVersion().toString()
                   << "DocEncoding=" << xml.documentEncoding().toString();
          continue;
        }

      /* If token is StartElement, we'll see if we can read it.*/
      if( token == QXmlStreamReader::StartElement )
        {
          elementCounter++;

          QString elementName = xml.name().toString();
          errorInfo = "XCSoar::getRootElement: ";

          // read attributes of token
          attributes = xml.attributes();

          if( elementCounter == 1 )
            {
              if( elementName != "Task" )
                {
                  errorInfo += "No XCSoar Task XML file";
                  qWarning() << errorInfo;
                  file.close();
                  return nullptr;
                }

              if( attributes.hasAttribute( "type" ) )
                {
                  taskType = attributes.value( "type" ).toString();
                }

              continue;
            }

          if( elementName == "Point" )
            {
              if( attributes.hasAttribute( "type" ) )
                {
                  pointType = attributes.value( "type" ).toString();
                }

              continue;
            }

          if( elementName == "Waypoint" )
            {
              if( attributes.hasAttribute( "name" ) )
                {
                  wpName = attributes.value( "name" ).toString();
                }

              continue;
            }

          if( elementName == "Location" )
            {
               if( attributes.hasAttribute( "latitude" ) )
                {
                  wpLatitude = attributes.value( "latitude" ).toString();
                }

              if( attributes.hasAttribute( "longitude" ) )
                {
                  wpLongitude = attributes.value( "longitude" ).toString();
                }

              continue;
            }

          if( elementName == "ObservationZone" )
            {
              if( attributes.hasAttribute( "radius" ) )
                {
                  ozRadius = attributes.value( "radius" ).toString();
                }

              if( attributes.hasAttribute( "type" ) )
                {
                  ozType = attributes.value( "type" ).toString();
                }

              continue;
            }
        }

      if( token == QXmlStreamReader::EndElement )
        {
          // qDebug() << "EndElement=" << xml.name().toString();

          if( xml.name().toString() == "Point" )
            {
              // create taskpoint and add it to the list
              double dlat = wpLatitude.toDouble();
              double dlon = wpLongitude.toDouble();

              // Convert latitude and longitude into KFLog's internal
              // integer format.
              int ilat = static_cast<int> (rint( 600000.0 * dlat ));
              int ilon = static_cast<int> (rint( 600000.0 * dlon ));
              WGSPoint wgsPoint( ilat, ilon );

              SinglePoint sp( wpName,
                              wpName.left(8),
                              BaseMapElement::Turnpoint,
                              wgsPoint,
                              QPoint(0,0) );

              TaskPoint tp( sp, mapPointType( pointType ) );

              if( ozType == "Cylinder" )
                {
                  tp.setActiveTaskPointFigureScheme( GeneralConfig::Circle );
                  Distance radius( ozRadius.toDouble() );
                  tp.setTaskCircleRadius( radius );
                }
              else if( ozType == "FAISector" )
                 {
                   tp.setActiveTaskPointFigureScheme( GeneralConfig::Sector );
                   Distance radius( ozRadius.toDouble() );
                   tp.setTaskSectorInnerRadius( Distance( 0 ) );
                   tp.setTaskSectorOuterRadius( radius );
                   tp.setTaskSectorAngle( 90 );
                 }

              tpList.append( tp );

              // Reset variables
              pointType.clear();
              wpName.clear();
              wpLatitude.clear();
              wpLongitude.clear();
              ozRadius.clear();
              ozType.clear();
            }

          continue;
        }

    } // End of while

  if( xml.hasError() )
    {
      errorInfo = "XML-Error: " + xml.errorString() +
                  " at line=" + QString::number(xml.lineNumber()) +
                  " column=" + QString::number(xml.columnNumber()) +
                  " offset=" + QString::number(xml.characterOffset());

      qWarning() << "reakTaskFile()" << "XML-Error" << errorInfo;
      file.close();
      return nullptr;
    }

  file.close();

  // Create the flight task object
  FlightTask* ft = new FlightTask( tpList, true, taskName, Speed(0.0) );

  return ft;
}

/**
 * Map a point type to the related cumulus type.
 *
 * @param type
 * @return
 */
TaskPointTypes::TaskPointType XCSoar::mapPointType( QString& type )
{
  if( type == "Start" || type == "OptionalStart" )
    {
      return TaskPointTypes::Start;
    }

  if( type == "Turn" )
    {
      return TaskPointTypes::Turn;
    }

  if( type == "Finish" )
    {
      return TaskPointTypes::Finish;
    }

  return TaskPointTypes::Unknown;
}
