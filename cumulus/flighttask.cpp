/***********************************************************************
**
**   flighttask.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   This class provides calculation, drawing and other support
**   for flight tasks and contains the data of a task.
**
***********************************************************************/

#include <cmath>

#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QRect>

#include "flighttask.h"
#include "generalconfig.h"
#include "map.h"
#include "mapcalc.h"
#include "cucalc.h"
#include "speed.h"
#include "reachablelist.h"

#undef CUMULUS_DEBUG

extern CuCalc      *calculator;
extern MapMatrix   *_globalMapMatrix;

FlightTask::FlightTask( QList<wayPoint*> *wpListIn,
                        bool faiRules,
                        QString taskName,
                        int speed ) :
  BaseMapElement("FlightTask", BaseMapElement::Task ),
  wpList(wpListIn),
  faiRules(faiRules),
  cruisingSpeed(speed),
  task_end(0),
  task_begin(0),
  olcPoints(0),
  taskPoints(0),
  flightType(FlightTask::NotSet),
  distance_total(0),
  distance_wert(0),
  distance_task(0),
  duration_total(0),
  __planningType(RouteBased),
  _taskName(taskName)
{
  // Check, if a valid object has been passed
  if( wpList == 0 )
    {
      // no, so let us create a new one
      wpList = new QList<wayPoint*>;
    }

  if( _taskName.isNull() )
    {
      _taskName = QObject::tr("unknown");
    }

  // only do this if wpList is not empty!
  if( wpList->count() != 0 )
    {
      updateTask();
    }
}

/**
 * Copy constructor
 **/
FlightTask::FlightTask( const FlightTask& inst )
  : BaseMapElement( inst )
{
  // qDebug("FlightTask::FlightTask( const FlighTask& inst ) is called");

  faiRules = inst.faiRules;
  cruisingSpeed = inst.cruisingSpeed;

  // create a deep copy of the waypoint list
  wpList = copyWpList( inst.wpList );

  task_end = inst.task_end;
  task_begin = inst.task_begin;

  olcPoints = inst.olcPoints;
  taskPoints = inst.taskPoints;
  flightType = inst.flightType;
  distance_total = inst.distance_total;
  distance_wert = inst.distance_wert;
  distance_task = inst.distance_task;
  duration_total = inst.duration_total;
  __planningType = inst.__planningType;
  _taskName = inst._taskName;
}

FlightTask::~FlightTask()
{
  // qDebug("FlightTask::~FlightTask(): name=%s, %X", _taskName.toLatin1().data(), this );
  qDeleteAll(*wpList);
  delete wpList;
}


/**
 * Determines the type of the task.
 **/
void FlightTask::__determineTaskType()
{
  distance_task = 0;
  distance_total = 0;
  double distance_task_d = 0;

  if( wpList->count() > 0 )
    {
      for(int loop = 1; loop <= wpList->count() - 1; loop++)
        {
          // qDebug("distance: %f", wpList->at(loop)->distance);
          distance_total += wpList->at(loop)->distance;
        }
      // qDebug("Total Distance: %f", distance_total);
    }

  if(wpList->count() < 4)
    {
      flightType = FlightTask::NotSet;
      return;
    }


  distance_task = distance_total - wpList->at(1)->distance
                  - wpList->at(wpList->count() - 1)->distance;

  if(dist(wpList->at(1),wpList->at(wpList->count() - 2)) < 1.0)
    {
      // Distance between Takeoff and End point is lower as one km. We
      // check the FAI rules
      switch(wpList->count() - 4)
        {
        case 0:
          // Fehler
          flightType = FlightTask::NotSet;
          break;

        case 1:
          // Zielrückkehr
          flightType = FlightTask::ZielR;
          break;

        case 2:
          // FAI Dreieck
          if(isFAI(distance_task,wpList->at(2)->distance,
                   wpList->at(3)->distance, wpList->at(4)->distance))
            flightType = FlightTask::FAI;
          else
            // Dreieck
            flightType = FlightTask::Dreieck;
          break;

        case 3:
          // Start auf Schenkel oder Vieleck
          // Vieleck Ja/Nein kann endgültig erst bei der Analyse des Fluges
          // bestimmt werden!
          //
          // Erste Abfrage je nachdem ob Vieleck oder Dreieck mehr Punkte geben
          // würde
          distance_task_d = distance_task - wpList->at(2)->distance
                            - wpList->at(5)->distance + dist(wpList->at(2), wpList->at(4));

          if(isFAI(distance_task_d, dist(wpList->at(2), wpList->at(4)),
                   wpList->at(3)->distance, wpList->at(4)->distance))
            {
              if(distance_task > distance_task_d * (1.0 + 1.0/3.0))
                flightType = FlightTask::Vieleck;
              else
                {
                  flightType = FlightTask::FAI_S;
                  distance_task = distance_task_d;
                }
            }
          else
            {
              if(distance_task > distance_task_d * (1.0 + 1.0/6.0))
                flightType = FlightTask::Vieleck;
              else
                {
                  flightType = FlightTask::Dreieck_S;
                  distance_task = distance_task_d;
                }
            }
          break;

        case 5:
          // 2x Dreieck nur als FAI gültig
          flightType = Unknown;
          if( (distance_task / 2 <= 100) && (wpList->at(1) == wpList->at(4)) &&
              (wpList->at(2) == wpList->at(5)) &&
              (wpList->at(3) == wpList->at(6)) &&
              isFAI(distance_task / 2, wpList->at(2)->distance,
                    wpList->at(3)->distance, wpList->at(4)->distance))
            flightType = FlightTask::FAI_2;
          break;

        case 6:
          // 2x Dreieck auf Schenkel FAI
          flightType = FlightTask::Unknown;
          distance_task_d = distance_task - wpList->at(2)->distance
                            - wpList->at(5)->distance
                            + dist(wpList->at(2), wpList->at(4)) * 2;

          if( (distance_task_d / 2 <= 100) &&
              (wpList->at(2) == wpList->at(5)) &&
              (wpList->at(3) == wpList->at(6)) &&
              (wpList->at(4) == wpList->at(7)) &&
              isFAI(distance_task, dist(wpList->at(2), wpList->at(4)),
                    wpList->at(3)->distance, wpList->at(4)->distance))
            {
              flightType = FlightTask::FAI_S2;
              distance_task = distance_task_d;
            }
          break;

        case 8:
          // 3x FAI Dreieck
          flightType = Unknown;
          if( (distance_task / 3 <= 100) &&
              (wpList->at(1) == wpList->at(4)) &&
              (wpList->at(2) == wpList->at(5)) &&
              (wpList->at(3) == wpList->at(6)) &&
              (wpList->at(1) == wpList->at(7)) &&
              (wpList->at(2) == wpList->at(8)) &&
              (wpList->at(3) == wpList->at(9)) &&
              isFAI(distance_task / 3, wpList->at(2)->distance,
                    wpList->at(3)->distance, wpList->at(4)->distance))
            flightType = FlightTask::FAI_3;
          break;

        case 9:
          // 3x FAI Dreieck Start auf Schenkel
          distance_task_d = distance_task - wpList->at(2)->distance
                            - wpList->at(5)->distance
                            + dist(wpList->at(2), wpList->at(4)) * 3;

          flightType = Unknown;
          if( (distance_task_d / 3 <= 100) &&
              (wpList->at(2) == wpList->at(5)) &&
              (wpList->at(3) == wpList->at(6)) &&
              (wpList->at(4) == wpList->at(7)) &&
              (wpList->at(2) == wpList->at(8)) &&
              (wpList->at(3) == wpList->at(9)) &&
              (wpList->at(4) == wpList->at(10)) &&
              isFAI(distance_task, dist(wpList->at(2), wpList->at(4)),
                    wpList->at(3)->distance, wpList->at(4)->distance))
            {
              flightType = FlightTask::FAI_S3;
              distance_task = distance_task_d;
            }

        default:
          flightType = FlightTask::Unknown;
        }
    }
  else
    {
      if( wpList->count() <= 5 )
        // Zielstrecke
        flightType = FlightTask::ZielS;
      else
        flightType = FlightTask::Unknown;
    }
}


/**
 * Calculates the task point sector angles in radian. The sector angle
 * between two task points is the bisecting line of the angle.
 */
double FlightTask::__calculateSectorAngles( int loop )
{
  // get configured sector angle
  GeneralConfig *conf = GeneralConfig::instance();
  const double sectorAngle = conf->getTaskSectorAngle() * M_PI/180.;

  double bisectorAngle = 0.0;
  double minAngle = 0.0;
  double maxAngle = 0.0;

#ifdef CUMULUS_DEBUG
  QString part = "WP-NN";
#endif

  // In some cases during planning, this method is called with wrong
  // loop-values. Therefore we must check the id before calculating
  // the direction
  switch( wpList->at(loop)->taskPointType )
    {
    case wayPoint::Begin:

#ifdef CUMULUS_DEBUG
      part = "WP-Begin (" + wpList->at(loop)->name + "-" + wpList->at(loop+1)->name + ")";
#endif

      // directions to the next point
      if(wpList->count() >= loop + 1)
        {
          bisectorAngle = getBearing(wpList->at(loop)->origP, wpList->at(loop+1)->origP);
        }
      break;

    case wayPoint::RouteP:

#ifdef CUMULUS_DEBUG
      part = "WP-RouteP ( " + wpList->at(loop-1)->name + "-" +
             wpList->at(loop)->name + "-" + wpList->at(loop+1)->name + ")";
#endif

      if( loop >= 1 && wpList->count() >= loop + 1 )
        {
          // vector pointing to the outside of the two points
          bisectorAngle = outsideVector(wpList->at(loop)->origP,
                                        wpList->at(loop-1)->origP,
                                        wpList->at(loop+1)->origP);
        }
      break;

    case wayPoint::End:

#ifdef CUMULUS_DEBUG
      part = "WP-End (" + wpList->at(loop)->name +"-" + wpList->at(loop-1)->name + ")";
#endif

      if(loop >= 1 && loop < wpList->count())
        {
          // direction to the previous point:
          bisectorAngle=getBearing( wpList->at(loop)->origP, wpList->at(loop-1)->origP );
        }
      break;

    default:
      break;
    }

  // save bisector angle
  bisectorAngle = normalize( bisectorAngle );
  wpList->at(loop)->angle = bisectorAngle;

  // invert bisector angle
  double invertAngle = bisectorAngle;
  invertAngle >= M_PI ? invertAngle -= M_PI : invertAngle += M_PI;

  // calculate min and max bisector angles
  minAngle = normalize( invertAngle - (sectorAngle/2.) );
  maxAngle = normalize( invertAngle + (sectorAngle/2.) );

  // save min and max bisector angles
  wpList->at(loop)->minAngle = minAngle;
  wpList->at(loop)->maxAngle = maxAngle;


#ifdef CUMULUS_DEBUG
  qDebug( "Loop=%d, Part=%s, Name=%s, Scale=%f, BisectorAngle=%3.1f, minAngle=%3.1f, maxAngle=%3.1f",
          loop, part.latin1(), wpList->at(loop)->name.latin1(), glMapMatrix->getScale(),
          bisectorAngle*180/M_PI, minAngle*180/M_PI, maxAngle*180/M_PI );
#endif

  return bisectorAngle;
}

/*
 * Sets the status of the waypoints, the durations in seconds, the
 * distances in km and the bearings.
 */

void FlightTask::__setTaskPointTypes()
{
  unsigned int cnt = wpList->count();

  if (cnt == 0)
    {
      return;
    }

  wpList->at(0)->taskPointType = wayPoint::FreeP;
  wpList->at(0)->distTime = 0;

  //  First waypoint has always bearing -1.
  wpList->at(0)->bearing = -1.;

  // First waypoint has always distance 0.0
  wpList->at(0)->distance = 0.0;

  // Reset total duration
  duration_total = 0;

  // get current speed unit and initialize speed instance
  enum Speed::speedUnit unit = Speed::getHorizontalUnit();
  Speed speed;

  switch( unit )
    {
    case Speed::knots:
      speed.setKnot(cruisingSpeed);
      break;
    case Speed::milesPerHour:
      speed.setMph(cruisingSpeed);
      break;
    case Speed::metersPerSecond:
      speed.setMps(cruisingSpeed);
      break;
    case Speed::kilometersPerHour:
      speed.setKph(cruisingSpeed);
      break;
    default:
      speed.setMps(cruisingSpeed);
      break;
    }

  // Distances, durations and bearings calculation. Note the speed can
  // be zero!
  for( uint n = 1; n  < cnt; n++ )
    {
      wpList->at(n)->distance = dist(wpList->at(n-1), wpList->at(n));

      if( wpList->at(n-1)->origP == wpList->at(n)->origP )
        {
          wpList->at(n)->bearing = -1.;
        }
      else
        {
          wpList->at(n)->bearing  = getBearing( wpList->at(n-1)->origP,
                                                wpList->at(n)->origP );
        }

      wpList->at(n)->taskPointType = wayPoint::FreeP;

      double cs = speed.getMps();

      if( cs > 0 )
        {
          // t=s/v distance is calculated in km, duration time in seconds
          wpList->at(n)->distTime =
            int( rint(wpList->at(n)->distance * 1000 / cs) );

          // summarize total duration as seconds
          duration_total += wpList->at(n)->distTime;
        }
      else
        {
          // reset duration to zero
          wpList->at(n)->distTime = 0;
        }

#ifdef CUMULUS_DEBUG
      qDebug("WP=%s, dist=%f, duration=%d",
             wpList->at(n)->name.latin1(),
             wpList->at(n)->distance,
             wpList->at(n)->distTime);
#endif

    }

  // to less waypoints
  if (cnt < 4)
    {
      return;
    }

  wpList->at(0)->taskPointType = wayPoint::TakeOff;
  wpList->at(1)->taskPointType = wayPoint::Begin;
  wpList->at(cnt - 2)->taskPointType = wayPoint::End;
  wpList->at(cnt - 1)->taskPointType = wayPoint::Landing;

  for(uint n = 2; n + 2 < cnt; n++)
    {
      wpList->at(n)->taskPointType = wayPoint::RouteP;
    }
}


QString FlightTask::getTaskTypeString() const
  {
    switch(flightType)
      {
      case FlightTask::NotSet:
        return QObject::tr("not set");
      case FlightTask::ZielS:
        return QObject::tr("Free Distance");
      case FlightTask::ZielR:
        return QObject::tr("Free Out and Return Distance");
      case FlightTask::FAI:
        return QObject::tr("FAI Triangle");
      case FlightTask::Dreieck:
        return QObject::tr("Triangle");
      case FlightTask::FAI_S:
        return QObject::tr("FAI Triangle Start on leg");
      case FlightTask::Dreieck_S:
        return QObject::tr("Triangle Start on leg");
      case FlightTask::Abgebrochen:
        return QObject::tr("Broken off");
      }

    return QObject::tr("Unknown");
  }

/** Check for small or large FAI triangle */
bool FlightTask::isFAI(double d_wp, double d1, double d2, double d3)
{
  if( ( d_wp < 500.0 ) &&
      ( d1 >= 0.28 * d_wp && d2 >= 0.28 * d_wp && d3 >= 0.28 * d_wp ) )
    // small FAI
    return true;
  else if( ( d1 > 0.25 * d_wp && d2 > 0.25 * d_wp && d3 > 0.25 * d_wp ) &&
           ( d1 <= 0.45 * d_wp && d2 <= 0.45 * d_wp && d3 <= 0.45 * d_wp ) )
    // large FAI
    return true;

  return false;
}

/**
 * Draws course lines and turn point sectors/circles according to the
 * user configuration.
 */

void FlightTask::drawMapElement( QPainter* painter )
{
  // qDebug ("FlightTask::drawMapElement");

  // get user defined scheme items
  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getActiveCSTaskScheme() == GeneralConfig::Cylinder )
    {
      // Cylinder scheme is active, handle this in extra method
      return circleSchemeDrawing( painter );
    }

  // Draw task point sectors according to FAI Rules
  const bool fillShape = conf->getTaskFillShape();
  const bool drawShape = conf->getTaskDrawShape();

  const int sectorAngle = conf->getTaskSectorAngle();

  const double OUTR = conf->getTaskSectorOuterRadius().getMeters();
  const double INR = conf->getTaskSectorInnerRadius().getMeters();

  const int ora = (int) rint(OUTR / glMapMatrix->getScale());
  const int ira = (int) rint(INR / glMapMatrix->getScale());

  // fetch map measures
  const int w = Map::getInstance()->size().width();
  const int h = Map::getInstance()->size().height();

  // qDebug("QDesktop: w=%d, h=%d, ora=%d", w, h, ora );

  QRect viewport( -ora, -ora, w+2*ora, h+2*ora);
  painter->setClipRegion( viewport );
  painter->setClipping( true );

  for( int loop=0; loop < wpList->count(); loop++ )
    {
      if( flightType == Unknown )
	{
	  if( loop )
	    {
	      painter->setPen(QPen(QColor(Qt::cyan), 3));
	      // Draws the course line
	      painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
				 glMapMatrix->map(wpList->at(loop)->projP) );
	    }
	}

      // map projected point to map
      QPoint mPoint(glMapMatrix->map(wpList->at(loop)->projP));

      // convert biangle (90...180) from radian to degrees
      int biangle = (int) rint( ((wpList->at(loop)->angle) / M_PI ) * 180.0 );

      switch( wpList->at(loop)->taskPointType )
	{
	case wayPoint::RouteP:

	  if( viewport.contains(mPoint) )
	    {
	      QColor c;

	      if( fillShape )
		{
		  c = QColor(Qt::green);
		}

	      drawSector( painter,
			  mPoint,
			  ira,
			  ora,
			  biangle,
			  sectorAngle,
			  c,
			  drawShape );
	    }

	  if( loop )
	    {
	      painter->setPen(QPen(QColor(Qt::cyan), 3));
	      painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
				 glMapMatrix->map(wpList->at(loop)->projP) );
	    }

	  break;

	case wayPoint::Begin:

	  if( viewport.contains(mPoint) )
	    {
	      QColor c;

	      if( fillShape )
		{
		  c = QColor(Qt::green);
		}

	      drawSector( painter,
			  mPoint,
			  ira,
			  ora,
			  biangle,
			  sectorAngle,
			  c,
			  drawShape );
	    }

	  // Draw line from take off to begin, if both not identical
	  if( loop &&
	      wpList->at(loop - 1)->origP != wpList->at(loop)->origP )
	    {
	      painter->setPen(QPen(Qt::cyan, 3));
	      painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
				 glMapMatrix->map(wpList->at(loop)->projP) );
	    }

	  break;

	case wayPoint::End:

	  if( viewport.contains(mPoint) )
	    {
	      QColor c;

	      if( fillShape )
		{
		  c = QColor(Qt::cyan);
		}

	      drawSector( painter,
			  mPoint,
			  ira,
			  ora,
			  biangle,
			  sectorAngle,
			  c,
			  drawShape );
	    }

	  painter->setPen(QPen(QColor(Qt::cyan), 3));
	  painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
			     glMapMatrix->map(wpList->at(loop)->projP) );
	  break;

	default:

	  // Can be take off or landing point
	  // Draw line from End to Landing point, if both not identical
	  if( loop &&
	      wpList->at(loop - 1)->origP != wpList->at(loop)->origP )
	    {
	      painter->setPen(QPen(Qt::cyan, 3));
	      painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
				 glMapMatrix->map(wpList->at(loop)->projP ) );
	    }

	  break;
	}
    }
}

/**
 * Does all map drawing actions for task points using cylinder scheme.
 * FAI rules does not play a role for drawing.
 *
 */
void FlightTask::circleSchemeDrawing( QPainter* painter )
{
  // get user defined scheme items
  GeneralConfig *conf = GeneralConfig::instance();

  const bool fillShape = conf->getTaskFillShape();
  const bool drawShape = conf->getTaskDrawShape();

  // fetch current scale, scale uses unit meter/pixel
  const double cs = glMapMatrix->getScale(MapMatrix::CurrentScale);

  // fetch configured radius
  const double radius = conf->getTaskCylinderRadius().getMeters();
  // scale radius to map
  const int r = (int) rint(radius / glMapMatrix->getScale());

  // fetch desktop measures
  const int w = QApplication::desktop()->width();
  const int h = QApplication::desktop()->height();

  QRect viewport( -10-r, -10-r, w+2*r, h+2*r );

  for( int loop=0; loop < wpList->count(); loop++ )
    {
      if( loop )
	{
	  // Draws the course line
	  painter->setPen(QPen(QColor( Qt::cyan ), 3));

	  painter->drawLine( glMapMatrix->map(wpList->at(loop - 1)->projP),
                             glMapMatrix->map(wpList->at(loop)->projP) );
	}

      if( cs > 350.0 || ( drawShape == false && fillShape == false ) )
        {
          // Don't make sense to draw task points at this scale. They are not really
          // visible resp. drawing is disabled by user
          continue;
        }

      QPoint mPoint (glMapMatrix->map(wpList->at(loop)->projP));

      // qDebug("MappedPoint: x=%d, y=%d", mPoint.x(), mPoint.y() );

      if( ! viewport.contains(mPoint) )
	{
	  // ignore not visible points
	  continue;
	}

      QColor color;

      if( fillShape )
        {
          // determine fill color
          switch(wpList->at(loop)->taskPointType)
            {
            case wayPoint::TakeOff:

              if( wpList->count() > loop &&
                  wpList->at(loop)->origP == wpList->at(loop+1)->origP )
                {
                  // TakeOff and Begin point are identical
                  continue;
                }

              color = QColor(Qt::gray);
              break;

            case wayPoint::Begin:

              if( wpList->count() > 1 &&
                  wpList->at(loop)->origP == wpList->at(wpList->count()-2)->origP )
                {
                  color = QColor(Qt::yellow); // Begin is also End, yellow is taken as color
                }
              else
                {
                  color = QColor(Qt::red);
                }
              break;

            case wayPoint::RouteP:
              color = QColor(Qt::green);
              break;

            case wayPoint::End:

              if( wpList->count() > 1
                  && wpList->at(loop)->origP == wpList->at(1)->origP )
                {
                  continue; // End is also Begin, yellow is taken as color
                }

              color = QColor(Qt::cyan);
              break;

            case wayPoint::Landing:
              if( loop > 1 &&
                  wpList->at(loop)->origP == wpList->at(loop-1)->origP )
                {
                  // End and Landing point are identical
                  continue;
                }

              color = QColor(Qt::gray);
              break;

            default:
              // should normally never happens
              color = QColor(Qt::magenta);
              break;
            }
        }

      // Draw circle around given position
      drawCircle( painter, mPoint, r, color, drawShape );

    } // End of For
}

/**
 * Draws a circle around the given position.
 *
 * coordinate as projected position of the point
 * scaled radius as meters
 * fillColor, do not fill, if set to invalid
 * drawShape, if set to true, draw outer circle with black color
 */

void FlightTask::drawCircle( QPainter* painter, QPoint& centerCoordinate,
			     const int radius,
			     QColor& fillColor, const bool drawShape )
{
  if( radius == 0 || (fillColor.isValid() == false && drawShape == false) )
    {
      return;
    }

  if( fillColor.isValid() )
    {
      const qreal ALPHA = GeneralConfig::instance()->getTaskShapeAlpha();

      // A valid color has passed, we have to fill the shape
      painter->setBrush( fillColor );
      painter->setOpacity( ALPHA/100.0 );
      painter->setPen( Qt::NoPen );
      painter->drawEllipse( centerCoordinate.x()-radius, centerCoordinate.y()-radius,
                            radius*2, radius*2 );
      painter->setOpacity( 1.0 );
    }

  if( drawShape )
    {
      // We draw the shape after the filling because filling would
      // overwrite our shape
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QPen(QColor(Qt::black), 2));
      painter->drawEllipse( centerCoordinate.x()-radius, centerCoordinate.y()-radius,
                            radius*2, radius*2 );
    }
}

/**
 * Draws a sector around the given position.
 *
 * Painter as Cumulus special painter device
 * coordinate as projected position of the point
 * scaled inner radius as meters
 * scaled outer radius as meters
 * bisector angle in debrees
 * spanning angle in degrees
 * fillColor, do not fill, if set to invalid
 * drawShape, if set to true, draw outer circle with black color
 */

void FlightTask::drawSector( QPainter* painter,
			     QPoint& centerCoordinate,
			     const int innerRadius,
			     const int outerRadius,
			     const int biangle,
			     const int spanningAngle,
			     QColor& fillColor,
			     const bool drawShape )
{
  // fetch current scale, scale uses unit meter/pixel
  const double cs = glMapMatrix->getScale(MapMatrix::CurrentScale);

  if(  cs > 350.0 || outerRadius == 0 ||
       (fillColor.isValid() == false && drawShape == false) )
    {
      // Don't make sense to draw task points at this scale. They are
      // not really visible resp. drawing is disabled by user
      return;
    }

  QPainterPath pp;

  // calculate sector array
  calculateSector( pp,
                   centerCoordinate.x()-outerRadius,
                   centerCoordinate.y()-outerRadius,
                   centerCoordinate.x()-innerRadius,
                   centerCoordinate.y()-innerRadius,
                   outerRadius,
                   innerRadius,
                   biangle,
                   spanningAngle );

  if( fillColor.isValid() )
    {
      const qreal ALPHA = GeneralConfig::instance()->getTaskShapeAlpha();

      // A valid color has passed, we have to fill the shape
      painter->setBrush( fillColor );
      painter->setOpacity( ALPHA/100.0 );
      painter->setPen( Qt::NoPen );
      painter->drawPath( pp );
      painter->setOpacity( 1.0 );
    }

  if( drawShape )
    {
      // We draw the shape after the filling because filling would
      // overwrite our shape
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QPen(QColor(Qt::black), 2));
      painter->drawPath( pp );
    }
}

/**
 *
 * Calculates the sector array used for the drawing of the task point
 * sector.
 *
 * pp	sector result painter path
 * ocx	scaled outer radius center coordinate x
 * ocy	scaled outer radius center coordinate y
 * icx	scaled inner radius center coordinate x
 * icy	scaled inner radius center coordinate y
 * ora	scaled outer radius
 * ira	scaled inner radius
 * sba	sector biangle in degrees
 * sa	sector angle in degrees
 *
 */
void FlightTask::calculateSector( QPainterPath& pp,
				  int ocx, int ocy,
				  int icx, int icy,
				  int ora, int ira,
				  int sba, int sa )
{
  // @AP: Correct angel, drawArc starts at 3 o'clock position.
  // Must be turned by 90 degrees to get the right position.
  int w1 = (sba+90) * -1;

  if( ira == 0 )
    {
      pp.moveTo( (qreal) ocx + (qreal) ora, (qreal) ocy + (qreal) ora );
      // The big arc around the center point.
      pp.arcTo( (qreal) ocx,(qreal) ocy, (qreal) (2 * ora), (qreal) (2 * ora), (qreal) (w1+sa/2),(qreal) -sa );
    }

  else if( ira == ora )
    {
      // Inner and outer radius are equal, we have to draw a circle
      pp.addEllipse( (qreal) ocx, (qreal) ocy, (qreal) (2 * ora), (qreal) (2 * ora) );
    }

  else if( ira > 0 && ira < ora )
    {
      // move pointer to start point
      pp.arcMoveTo( (qreal) ocx,(qreal) ocy, (qreal) (2 * ora), (qreal) (2 * ora), (qreal) (w1+sa/2) );
      // The big arc around the center point.
      pp.arcTo( (qreal) ocx,(qreal) ocy, (qreal) (2 * ora), (qreal) (2 * ora), (qreal) (w1+sa/2), (qreal) -sa );

      // The small arc around the center point and opposite to the
      // big arc. Can have the same size as big arc.
      pp.arcTo( (qreal) icx, (qreal) icy, (qreal) (2 * ira), (qreal) (2 * ira), (qreal) (w1 - sa+sa/2), (qreal) -(360-sa) );
    }

  pp.closeSubpath();
}

/** Check, if task sector has been arrived.
 *
 * dist2TP: distance to task point
 * position: current position as WGS84 datum
 * taskPointIndex: index of TP in waypoint list
 *
 * returns true, if inside of sector otherwise false
 */
bool FlightTask::checkSector( const Distance& dist2TP,
                              const QPoint& position,
                              const int taskPointIndex )
{
  if( taskPointIndex >= wpList->count() )
    {
      // taskPointIndex out of range
      return false;
    }

  // get user defined scheme items
  GeneralConfig *conf = GeneralConfig::instance();

  const enum GeneralConfig::ActiveCSTaskScheme scheme = conf->getActiveCSTaskScheme();

  if( scheme == GeneralConfig::Cylinder )
    {
      // Cylinder scheme is active
      double cylinderRadius = conf->getTaskCylinderRadius().getMeters();

      if( dist2TP.getMeters() < cylinderRadius )
        {
          // We are inside cylinder
          return true;
        }

      return false;
    }

  // Sector scheme is active, get sector radiuses
  const double innerRadius = conf->getTaskSectorInnerRadius().getMeters();
  const double outerRadius = conf->getTaskSectorOuterRadius().getMeters();

  if( dist2TP.getMeters() > outerRadius )
    {
      // we are outside of outer radius, sector angle has not to be
      // considered
      return false;
    }

  if( ( innerRadius > 0.0 || innerRadius == outerRadius ) &&
      dist2TP.getMeters() < innerRadius )
    {
      // we are inside of inner radius, sector angle has not to be
      // considered
      return true;
    }

  // Do special check for landing point
  if( wpList->at( taskPointIndex )->taskPointType == wayPoint::Landing )
    {
      // 1. Here we do apply only the outer radius check.
      // 2. DMST said target reached, if 1km radius has been arrived.
      if( dist2TP.getMeters() < outerRadius ||
          dist2TP.getMeters() < 1000.0 )
        {
          // We are inside outer radius
          return true;
        }
      else
        {
          return false;
        }
    }

  // Here we are inside of outer radius, therefore we have to
  // check the sector angle.
  const double minAngle = wpList->at( taskPointIndex )->minAngle;
  const double maxAngle = wpList->at( taskPointIndex )->maxAngle;

  // calculate bearing from tp to current position
  const double bearing = getBearingWgs( wpList->at( taskPointIndex )->origP, position );

#ifdef CUMULUS_DEBUG
  qDebug( "FlightTask::checkSector(): minAngle=%f, maxAngel=%f, bearing=%f",
          minAngle*180./M_PI,
          maxAngle*180./M_PI,
          bearing*180./M_PI );
#endif

  if( minAngle > maxAngle &&
      ( bearing > minAngle || bearing < maxAngle ) )
    {
      // we are inside of sector and sector includes north direction
      return true;
    }

  if( bearing > minAngle && bearing < maxAngle )
    {
      // we are inside of sector between 0...360
      return true;
    }

  return false;
}

/**
 * Calculates the glide path to final target of flight task from the
 * current position.
 *
 * taskPointIndex: index of next TP in waypoint list
 * arrivalAlt: returns arrival altitude
 * bestSpeed:  returns assumed speed
 * reachable:  returns info about reachability
 *
 */

int FlightTask::calculateFinalGlidePath( const int taskPointIndex,
                                         Altitude &arrivalAlt,
                                         Speed &bestSpeed )
{
  uint wpCount = wpList->count();

  arrivalAlt.setInvalid();
  bestSpeed.setInvalid();

  if( (uint) (taskPointIndex + 1) >= wpCount )
    {
      // taskPointIndex points to the end of list
      return ReachablePoint::no;
    }

  // fetch current altitude
  Altitude curAlt = calculator->getlastAltitude();

  // fetch minimal arrival altitude
  Altitude minAlt( GeneralConfig::instance()->getSafetyAltitude().getMeters() );

  // used altitude
  Altitude usedAlt(0);
  Altitude arrAlt(0);
  Speed speed(0);

  // calculate bearing from current position to next waypoint from
  // task in radian
  int bearing = int ( rint( getBearingWgs( calculator->getlastPosition(),
                                           wpList->at( taskPointIndex )->origP ) ));

  // calculate distance from current position to next waypoint from
  // task in km
  QPoint p1 = calculator->getlastPosition();
  QPoint p2 = wpList->at( taskPointIndex )->origP;
  double distance = dist( &p1, &p2 );

  bool res = calculator->glidePath( bearing, Distance(distance * 1000.0),
                                    wpList->at( taskPointIndex )->elevation,
                                    arrAlt, speed );

  if( ! res )
    {
      return ReachablePoint::no; // glide path calculation failed, no glider selected
    }

#ifdef CUMULUS_DEBUG
  qDebug( "WP=%s, Bearing=%.1f�, Dist=%.1fkm, Ele=%dm, ArrAlt=%.1f",
          wpList->at( taskPointIndex )->name.latin1(),
          bearing*180/M_PI,
          distance,
          wpList->at( taskPointIndex )->elevation,
          arrAlt.getMeters() );
#endif

  // summarize single altitudes
  usedAlt = curAlt-arrAlt;

  for( uint i=taskPointIndex; i+1 < wpCount; i++ )
    {
      if( wpList->at(i)->origP == wpList->at(i+1)->origP )
        {
          continue; // points are equal, we ignore them
        }

      res = calculator->glidePath( (int) rint( wpList->at( i+1 )->bearing ),
                                   Distance( wpList->at( i+1 )->distance * 1000.0),
                                   wpList->at( i+1 )->elevation,
                                   arrAlt, speed );
#ifdef CUMULUS_DEBUG
      qDebug( "WP=%s, Bearing=%.1f�, Dist=%.1fkm, Ele=%dm, ArrAlt=%.1f",
              wpList->at( i+1 )->name.latin1(),
              wpList->at( i+1 )->bearing*180/M_PI,
              wpList->at( i+1 )->distance,
              wpList->at( i+1 )->elevation,
              arrAlt.getMeters() );
#endif

      // We calculate the real altitude usage. The glidePath method
      // delivers only an arrival altitude for one calculation. But we
      // need the total consumed altitude. Because the minimal arrival
      // altitude is already contained in the first done calculation,
      // we don't need it to consider again.
      usedAlt += curAlt - arrAlt - minAlt;
    }

  arrivalAlt = curAlt-usedAlt;

  if( arrivalAlt >= minAlt )
    {
      return ReachablePoint::yes;
    }

  if( arrivalAlt.getMeters() > 0.0 )
    {
      return ReachablePoint::belowSafety;
    }

  return ReachablePoint::no;
}

QString FlightTask::getTotalDistanceString() const
  {
    if(flightType == FlightTask::NotSet)
      return "--";

    return Distance::getText(distance_total*1000,true,1);
  }


QString FlightTask::getTaskDistanceString() const
  {
    if(flightType == FlightTask::NotSet)
      return "--";

    return Distance::getText(distance_task*1000,true,1);
  }


QString FlightTask::getPointsString() const
  {
    if(flightType == FlightTask::NotSet)
      return "--";

    int points1 = (int) taskPoints;
    if((int) ( (taskPoints - points1) * 10 ) > 5)
      points1++;

    QString pointString;
    pointString.sprintf("%d", points1);

    return pointString;
  }

/**
 * @returns a string representing the total distance time of task as
 * hh:mm to use for user-display. The time is rounded up if seconds
 * are greather than 30.
 */
QString FlightTask::getTotalDistanceTimeString()
{
  return getDistanceTimeString( duration_total );
}

/**
 * @returns a string representing the distance time as hh:mm to use
 * for user-display. The time is rounded up if seconds are greather
 * than 30. Input has to be passed as seconds.
 */
QString FlightTask::getDistanceTimeString(const int timeInSec)
{
  if( timeInSec == 0 )
    {
      return "0:00";
    }

  int dt = timeInSec;

  // Roud up, if seconds over 30. Must be done because the seconds
  // will be truncated from the time string without rounding
  if( dt % 60 > 30 ) dt +=30;

  QString duration;
  duration.sprintf("%d:%02d", dt/3600, (dt % 3600)/60 );
  return duration;
}

/**
 * @returns a string representing the cruising spped to use for
 * user-display.
 */
QString FlightTask::getSpeedString() const
{
  if( flightType == FlightTask::NotSet || cruisingSpeed == 0 )
    {
      return "--";
    }

  QString v;
  v.sprintf( "%d %s", cruisingSpeed, Speed::getHorizontalUnitText().toLatin1().data() );
  return v;
}


/**
 * Overtakes a waypoint list. An old list is deleted.
 */
void FlightTask::setWaypointList(QList<wayPoint*> *newWpList)
{
  if( newWpList == 0 )
    {
      return; // ignore null object
    }

   // Remove old content of list
  qDeleteAll(*wpList);
  delete wpList;

  // @AP: overtake ownership about the new passed list
  wpList = newWpList;
  updateTask();
}


/**
 * updates the internal task data
 */
void FlightTask::updateTask()
{
  __setTaskPointTypes();
  __determineTaskType();

  for(int loop = 0; loop < wpList->count(); loop++)
    {
      // number point with index
      wpList->at(loop)->taskPointIndex = loop;

      // calculate turn point sector angles
      __calculateSectorAngles( loop );
    }
}

/**
 * updates projection data of waypoint list
 */
void FlightTask::updateProjection()
{
  for(int loop = 0; loop < wpList->count(); loop++)
    {
      // calculate projection data
      wpList->at(loop)->projP = _globalMapMatrix->wgsToMap(wpList->at(loop)->origP);
    }
}

/**
 * @AP: Add a new waypoint to the list. This overtakes the ownership
 * of the passed object.
 */
void FlightTask::addWaypoint( wayPoint *newWp )
{
  if( newWp == 0 )
    {
      return; // ignore null object
    }

  wpList->append( newWp );
  updateTask();
}

/**
 * Returns a deep copy of the waypoint list. The ownership of the list
 * is overtaken by the caller.
 */
QList<wayPoint*> *FlightTask::getCopiedWPList()
{
  return copyWpList( wpList );
}

  /**
   * Returns a deep copy of the input list. The ownership of the
   * list is overtaken by the caller.
   */
QList<wayPoint*> *FlightTask::copyWpList(QList<wayPoint*> *wpListIn)
{
  QList<wayPoint*> *outList = new QList<wayPoint*>;

  if( wpListIn == 0 )
    {
      // returns an empty list on no input
      return outList;
    }

  for(int loop = 0; loop < wpListIn->count(); loop++)
    {
      wayPoint *wp = new wayPoint( *wpListIn->at(loop) );
      outList->append( wp );
    }

  return outList;
}

void FlightTask::setPlanningType(const int type)
{
  __planningType = type;
  __setTaskPointTypes();
}
