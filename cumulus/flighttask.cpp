/***********************************************************************
**
**   flighttask.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2007-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include <QtGui>

#include "calculator.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "map.h"
#include "mapcalc.h"
#include "speed.h"

#undef CUMULUS_DEBUG

extern Calculator *calculator;
extern MapMatrix  *_globalMapMatrix;

FlightTask::FlightTask( QList<TaskPoint *> *tpListIn,
                        bool faiRules,
                        QString taskName,
                        int tas ) :
  BaseMapElement("FlightTask", BaseMapElement::Task ),
  tpList(tpListIn),
  faiRules(faiRules),
  cruisingSpeed(tas),
  windDirection(0),
  windSpeed(0),
  wtCalculation(false),
  flightType(FlightTask::NotSet),
  distance_total(0),
  distance_task(0),
  duration_total(0),
  _planningType(RouteBased),
  _taskName(taskName)
{
  // Check, if a valid object has been passed
  if( tpList == static_cast<QList<TaskPoint *> *> (0) )
    {
      // no, so let us create a new one
      tpList = new QList<TaskPoint *>;
    }

  if( _taskName.isNull() )
    {
      _taskName = QObject::tr("unknown");
    }

  // only do this if tpList is not empty!
  if( tpList->count() != 0 )
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
  windDirection = inst.windDirection;
  windSpeed = inst.windSpeed;
  wtCalculation = inst.wtCalculation;

  // create a deep copy of the task point list
  tpList = copyTpList( inst.tpList );

  flightType = inst.flightType;
  distance_total = inst.distance_total;
  distance_task = inst.distance_task;
  duration_total = inst.duration_total;
  _planningType = inst._planningType;
  _taskName = inst._taskName;
}

FlightTask::~FlightTask()
{
  // qDebug("FlightTask::~FlightTask(): name=%s, %X", _taskName.toLatin1().data(), this );
  qDeleteAll( *tpList );
  delete tpList;
}

/**
 * Determines the type of the task.
 **/
void FlightTask::determineTaskType()
{
  distance_task = 0;
  distance_total = 0;
  double distance_task_d = 0;

  if( tpList->count() > 0 )
    {
      for(int loop = 1; loop <= tpList->count() - 1; loop++)
        {
          // qDebug("distance: %f", tpList->at(loop)->distance);
          distance_total += tpList->at(loop)->distance;
        }
      // qDebug("Total Distance: %f", distance_total);
    }

  if(tpList->count() < 4)
    {
      flightType = FlightTask::NotSet;
      return;
    }


  distance_task = distance_total - tpList->at(1)->distance
                  - tpList->at(tpList->count() - 1)->distance;

  if(dist(tpList->at(1),tpList->at(tpList->count() - 2)) < 1.0)
    {
      // Distance between Takeoff and End point is lower as one km. We
      // check the FAI rules
      switch(tpList->count() - 4)
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
          if(isFAI(distance_task,tpList->at(2)->distance,
                   tpList->at(3)->distance, tpList->at(4)->distance))
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
          distance_task_d = distance_task - tpList->at(2)->distance
                            - tpList->at(5)->distance + dist(tpList->at(2), tpList->at(4));

          if(isFAI(distance_task_d, dist(tpList->at(2), tpList->at(4)),
                   tpList->at(3)->distance, tpList->at(4)->distance))
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
          if( (distance_task / 2 <= 100) && (tpList->at(1) == tpList->at(4)) &&
              (tpList->at(2) == tpList->at(5)) &&
              (tpList->at(3) == tpList->at(6)) &&
              isFAI(distance_task / 2, tpList->at(2)->distance,
                    tpList->at(3)->distance, tpList->at(4)->distance))
            flightType = FlightTask::FAI_2;
          break;

        case 6:
          // 2x Dreieck auf Schenkel FAI
          flightType = FlightTask::Unknown;
          distance_task_d = distance_task - tpList->at(2)->distance
                            - tpList->at(5)->distance
                            + dist(tpList->at(2), tpList->at(4)) * 2;

          if( (distance_task_d / 2 <= 100) &&
              (tpList->at(2) == tpList->at(5)) &&
              (tpList->at(3) == tpList->at(6)) &&
              (tpList->at(4) == tpList->at(7)) &&
              isFAI(distance_task, dist(tpList->at(2), tpList->at(4)),
                    tpList->at(3)->distance, tpList->at(4)->distance))
            {
              flightType = FlightTask::FAI_S2;
              distance_task = distance_task_d;
            }
          break;

        case 8:
          // 3x FAI Dreieck
          flightType = Unknown;
          if( (distance_task / 3 <= 100) &&
              (tpList->at(1) == tpList->at(4)) &&
              (tpList->at(2) == tpList->at(5)) &&
              (tpList->at(3) == tpList->at(6)) &&
              (tpList->at(1) == tpList->at(7)) &&
              (tpList->at(2) == tpList->at(8)) &&
              (tpList->at(3) == tpList->at(9)) &&
              isFAI(distance_task / 3, tpList->at(2)->distance,
                    tpList->at(3)->distance, tpList->at(4)->distance))
            flightType = FlightTask::FAI_3;
          break;

        case 9:
          // 3x FAI Dreieck Start auf Schenkel
          distance_task_d = distance_task - tpList->at(2)->distance
                            - tpList->at(5)->distance
                            + dist(tpList->at(2), tpList->at(4)) * 3;

          flightType = Unknown;
          if( (distance_task_d / 3 <= 100) &&
              (tpList->at(2) == tpList->at(5)) &&
              (tpList->at(3) == tpList->at(6)) &&
              (tpList->at(4) == tpList->at(7)) &&
              (tpList->at(2) == tpList->at(8)) &&
              (tpList->at(3) == tpList->at(9)) &&
              (tpList->at(4) == tpList->at(10)) &&
              isFAI(distance_task, dist(tpList->at(2), tpList->at(4)),
                    tpList->at(3)->distance, tpList->at(4)->distance))
            {
              flightType = FlightTask::FAI_S3;
              distance_task = distance_task_d;
            }
          break;

        default:
          flightType = FlightTask::Unknown;
          break;
        }
    }
  else
    {
      if( tpList->count() <= 5 )
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
double FlightTask::calculateSectorAngles( int loop )
{
  // get configured sector angle
  const double sectorAngle = tpList->at(loop)->getTaskSectorAngle() * M_PI/180.;

  double bisectorAngle = 0.0;
  double minAngle = 0.0;
  double maxAngle = 0.0;

#ifdef CUMULUS_DEBUG
  QString part = "WP-NN";
#endif

  // In some cases during planning, this method is called with wrong
  // loop-values. Therefore we must check the id before calculating
  // the direction
  int taskPointType = tpList->at(loop)->taskPointType;

  switch( taskPointType )
    {
    case TaskPointTypes::Begin:

#ifdef CUMULUS_DEBUG
      part = "WP-Begin (" + tpList->at(loop)->name + "-" + tpList->at(loop+1)->name + ")";
#endif

      // directions to the next point
      if(tpList->count() >= loop + 1)
        {
          bisectorAngle = getBearing(tpList->at(loop)->origP, tpList->at(loop+1)->origP);
        }
      break;

    case TaskPointTypes::RouteP:

#ifdef CUMULUS_DEBUG
      part = "WP-RouteP ( " + tpList->at(loop-1)->name + "-" +
             tpList->at(loop)->name + "-" + tpList->at(loop+1)->name + ")";
#endif

      if( loop >= 1 && tpList->count() >= loop + 1 )
        {
          // vector pointing to the outside of the two points
          bisectorAngle = outsideVector(tpList->at(loop)->origP,
                                        tpList->at(loop-1)->origP,
                                        tpList->at(loop+1)->origP);
        }
      break;

    case TaskPointTypes::End:

#ifdef CUMULUS_DEBUG
      part = "WP-End (" + tpList->at(loop)->name +"-" + tpList->at(loop-1)->name + ")";
#endif

      if(loop >= 1 && loop < tpList->count())
        {
          // direction to the previous point:
          bisectorAngle = getBearing( tpList->at(loop)->origP, tpList->at(loop-1)->origP );
        }
      break;

    default:
      break;
    }

  // set bisector angle of task point
  bisectorAngle = normalize( bisectorAngle );
  tpList->at(loop)->angle = bisectorAngle;

  // Update line settings, if required.
  if( tpList->at(loop)->getActiveTaskPointFigureScheme() == GeneralConfig::Line )
    {
      // set bisector angle for the task line as course direction
      if( taskPointType != TaskPointTypes::End )
        {
          tpList->at(loop)->getTaskLine().setDirection( static_cast<int>(rint(bisectorAngle * 180.0/M_PI)) );
        }
      else
        {
          // direction to the previous point must be inverted in case of end point.
          tpList->at(loop)->getTaskLine().setDirection( static_cast<int>(rint(bisectorAngle * 180.0/M_PI)) + 180 );
        }

      // set the center point of the task line
      tpList->at(loop)->getTaskLine().setLineCenter( tpList->at(loop)->origP );

      // calculate all line elements after a new setting
      tpList->at(loop)->getTaskLine().calculateElements();
    }

  // invert bisector angle
  double invertAngle = bisectorAngle;
  invertAngle >= M_PI ? invertAngle -= M_PI : invertAngle += M_PI;

  // calculate min and max bisector angles
  minAngle = normalize( invertAngle - (sectorAngle/2.) );
  maxAngle = normalize( invertAngle + (sectorAngle/2.) );

  // save min and max bisector angles
  tpList->at(loop)->minAngle = minAngle;
  tpList->at(loop)->maxAngle = maxAngle;


#ifdef CUMULUS_DEBUG
  qDebug( "Loop=%d, Part=%s, Name=%s, Scale=%f, BisectorAngle=%3.1f, minAngle=%3.1f, maxAngle=%3.1f",
          loop, part.toLatin1().data(), tpList->at(loop)->name.toLatin1().data(),
          glMapMatrix->getScale(),
          bisectorAngle*180/M_PI, minAngle*180/M_PI, maxAngle*180/M_PI );
#endif

  return bisectorAngle;
}

/*
 * Sets the status of the task points, the durations in seconds, the
 * distances in km, the bearings in radian, the true heading and the wca
 * in degree the ground speed in m/s.
 */
void FlightTask::setTaskPointData()
{
  int cnt = tpList->count();

  if (cnt == 0)
    {
      return;
    }

  tpList->at(0)->taskPointType = TaskPointTypes::FreeP;

  //  First task point is always set to these values
  tpList->at(0)->distTime = 0;
  tpList->at(0)->bearing = -1.;
  tpList->at(0)->distance = 0.0;
  tpList->at(0)->wca = 0;
  tpList->at(0)->trueHeading = -1;
  tpList->at(0)->groundSpeed = 0.0;
  tpList->at(0)->wtResult = false;

  // Reset total duration
  duration_total = 0;

  // Initialize TAS and wind speed instances by using user defined units.
  Speed tas(0);
  Speed wind(0);

  tas.setHorizontalValue( cruisingSpeed );
  wind.setWindValue( windSpeed );

  if( windSpeed <= 0 )
    {
      // No wind triangle calculation possible
      wtCalculation = false;
    }
  else
    {
      // consider wind in calculations
      wtCalculation = true;
    }

  // Distances, durations and bearings calculation. Note that TAS or the
  // distance between two points can be zero!
  for( int n = 1; n < cnt; n++ )
    {
      // Set default parameters for every item
      tpList->at(n)->bearing = -1.;
      tpList->at(n)->distance = 0.;
      tpList->at(n)->wca = 0;
      tpList->at(n)->trueHeading = -1.;
      tpList->at(n)->groundSpeed = 0.0;
      tpList->at(n)->wtResult = false;

      if( tpList->at(n-1)->origP != tpList->at(n)->origP )
        {
          // Points are not identical, do calculate navigation parameters.
          // calculate bearing
          tpList->at(n)->bearing = getBearing( tpList->at(n-1)->origP,
                                               tpList->at(n)->origP );

          // calculate distance
          tpList->at(n)->distance = dist(tpList->at(n-1), tpList->at(n));

          // calculate wind parameters, if wind speed is defined. Ground
          // speed unit is meter per second.
          if( wtCalculation )
              {
                tpList->at(n)->wtResult =
                    windTriangle( tpList->at(n)->bearing * 180/M_PI,
                                  tas.getMps(),
                                  windDirection,
                                  wind.getMps(),
                                  tpList->at(n)->groundSpeed,
                                  tpList->at(n)->wca,
                                  tpList->at(n)->trueHeading );

                if( tpList->at(n)->wtResult == false )
                  {
                    // No wind triangle calculation possible. In such a case
                    // we do reset the global wt calculation flag too.
                    wtCalculation = false;
                  }
              }
        }

      tpList->at(n)->taskPointType = TaskPointTypes::FreeP;

      double cs = tas.getMps();

      // Calculate all without wind because wind can be too strong at
      // one of the next legs.
      if( cs > 0. && tpList->at(n)->distance > 0.)
        {
          // t=s/v distance unit is m, duration time unit is seconds and
          // TAS unit is meter per second.
          tpList->at(n)->distTime =
            int( rint(tpList->at(n)->distance * 1000 / cs) );

          // summarize total duration as seconds
          duration_total += tpList->at(n)->distTime;
        }
      else
        {
          // reset duration to zero
          tpList->at(n)->distTime = 0;
        }

#ifdef CUMULUS_DEBUG
      qDebug("Without Wind: WP=%s, TAS=%f, dist=%f, duration=%d, tc=%f, th=%f",
             tpList->at(n)->name.toLatin1().data(),
             tas.getKph(),
             tpList->at(n)->distance,
             tpList->at(n)->distTime,
             tpList->at(n)->bearing,
             tpList->at(n)->trueHeading);
#endif

    }

  // Check, if wt calculation was successful for all legs. In this case the
  // duration time is calculated with wind influence by using ground speed.
  if( wtCalculation == true )
    {
      // Reset total duration
      duration_total = 0;

      for( int n = 1; n < cnt; n++ )
        {
          double gs = tpList->at(n)->groundSpeed;

          // Calculate all without wind because wind can be too strong at
          // one of the next legs.
          if( gs > 0. && tpList->at(n)->distance > 0.)
            {
              // t=s/v distance unit is m, duration time unit is seconds and
              // ground speed unit is meter per second.
              tpList->at(n)->distTime =
                int( rint(tpList->at(n)->distance * 1000 / gs) );

              // summarize total duration as seconds
              duration_total += tpList->at(n)->distTime;
            }
          else
            {
              // reset duration to zero
              tpList->at(n)->distTime = 0;
            }

#ifdef CUMULUS_DEBUG
          qDebug("With Wind: WP=%s, GS=%f, Wca=%f, dist=%f, duration=%d, th=%f",
                 tpList->at(n)->name.toLatin1().data(),
                 gs*3.6,
                 tpList->at(n)->wca,
                 tpList->at(n)->distance,
                 tpList->at(n)->distTime,
                 tpList->at(n)->trueHeading);
#endif

        }
    }

  // to less task points
  if (cnt < 4)
    {
      return;
    }

  tpList->at(0)->taskPointType       = TaskPointTypes::TakeOff;
  tpList->at(1)->taskPointType       = TaskPointTypes::Begin;
  tpList->at(cnt - 2)->taskPointType = TaskPointTypes::End;
  tpList->at(cnt - 1)->taskPointType = TaskPointTypes::Landing;

  for(int n = 2; n + 2 < cnt; n++)
    {
      tpList->at(n)->taskPointType = TaskPointTypes::RouteP;
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
      return QObject::tr("Free Out and Return");
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
void FlightTask::drawTask( QPainter* painter, QList<Waypoint*> &drawnTp )
{
  // qDebug() << __PRETTY_FUNCTION__;

  // get user defined scheme items
  GeneralConfig* conf = GeneralConfig::instance();

  // Load the currently selected task point.
  const TaskPoint* selectedTp = calculator->getselectedTp();

  // load task point label option
  const bool drawTpLabels = conf->getMapShowTaskPointLabels();

  // Draw task point sectors according to FAI Rules
  const bool fillShape = conf->getTaskFillShape();
  const bool drawShape = conf->getTaskDrawShape();

  // fetch map measures
  const int w = Map::getInstance()->size().width();
  const int h = Map::getInstance()->size().height();

  // Set pen color and width for the course line
  QColor courseLineColor = conf->getTaskLineColor();
  qreal courseLineWidth  = conf->getTaskLineWidth();

  // qDebug("QDesktop: w=%d, h=%d, ora=%d", w, h, ora );

  // Save the current painter state.
  painter->save();

  for( int loop=0; loop < tpList->count(); loop++ )
    {
      // qDebug() << "draw taskpoint:" << tpList->at(loop)->name
      //          << tpList->at(loop)->getTaskPointFigureString();

      // Load the sector data
      const int SectorAngle = tpList->at(loop)->getTaskSectorAngle();
      const double Sor      = tpList->at(loop)->getTaskSectorOuterRadius().getMeters();
      const double Sir      = tpList->at(loop)->getTaskSectorInnerRadius().getMeters();

      int sorScaled = 0; // scaled sector outer radius
      int sirScaled = 0; // scaled sector inner radius
      int crScaled  = 0;  // scaled circle radius
      int llScaled  = 0;  // scaled line length

      // Load the circle data
      const double circleRadius = tpList->at(loop)->getTaskCircleRadius().getMeters();

      // Load the line data
      const double lineLength = tpList->at(loop)->getTaskLineLength().getMeters();

      // Determine what task figure has to be drawn.
      enum GeneralConfig::ActiveTaskFigureScheme figure =
          tpList->at(loop)->getActiveTaskPointFigureScheme();

      QRect viewport;

      switch( figure )
      {
        case GeneralConfig::Line:
          // scale length to map
          llScaled = (int) rint(lineLength / glMapMatrix->getScale());
          viewport.setRect( -10-llScaled, -10-llScaled, w+llScaled, h+llScaled );
          break;
        case GeneralConfig::Circle:
          // scale radius to map
          crScaled = (int) rint(circleRadius / glMapMatrix->getScale());
          viewport.setRect( -10-crScaled, -10-crScaled, w+2*crScaled, h+2*crScaled );
          break;
        case GeneralConfig::Sector:
          // scale radius to map
          sorScaled = (int) rint(Sor / glMapMatrix->getScale());
          sirScaled = (int) rint(Sir / glMapMatrix->getScale());
          viewport.setRect(-sorScaled, -sorScaled, w+2*sorScaled, h+2*sorScaled);
          break;
        default:
          qWarning() << "FlightTask::drawTask:" << "Unknown task figure type" << figure;
          continue;
      }

      painter->setClipRegion( viewport );
      painter->setClipping( true );

      // Append all waypoints to the label list on user request
      if( drawTpLabels )
        {
          drawnTp.append( tpList->at(loop) );
        }

      // map projected point to map display
      QPoint mPoint(glMapMatrix->map(tpList->at(loop)->projP));

      bool mPointIsContained = viewport.contains(mPoint);

      if( mPointIsContained )
        {
          painter->setPen(QPen(Qt::black));
          painter->setBrush( QBrush( Qt::black, Qt::SolidPattern ) );
          painter->drawRect( mPoint.x()-5, mPoint.y()-5, 10, 10 );
        }

      if( flightType == Unknown )
        {
          if( loop )
            {
              painter->setPen(QPen(courseLineColor, courseLineWidth));
              // Draws the course line
              painter->drawLine( glMapMatrix->map(tpList->at(loop - 1)->projP),
                                 glMapMatrix->map(tpList->at(loop)->projP) );
            }
        }

      // convert biangle (90...180) from radian to degrees
      int biangle = (int) rint( ((tpList->at(loop)->angle) / M_PI ) * 180.0 );

      switch( tpList->at(loop)->taskPointType )
      {
      case TaskPointTypes::RouteP:

        if( mPointIsContained )
        {
            QColor color;

            if( fillShape )
              {
                color = QColor(Qt::green);
              }

            switch (figure)
            {
                case GeneralConfig::Line:
                    tpList->at(loop)->getTaskLine().drawLine(painter);
                    break;
                case GeneralConfig::Circle:
                    // Draw circle around given position
                    drawCircle( painter, mPoint, crScaled, color, drawShape );
                    break;
                case GeneralConfig::Sector:
                    drawSector( painter,
                                mPoint,
                                sirScaled,
                                sorScaled,
                                biangle,
                                SectorAngle,
                                color,
                                drawShape );
                    break;
                default:
                    qWarning() << "FlightTask::drawTask:" << "Unknown task figure type" << figure;
                    continue;
            }
        }

        if( loop )
          {
            painter->setPen(QPen(courseLineColor, courseLineWidth));
            painter->drawLine( glMapMatrix->map(tpList->at(loop - 1)->projP),
                               glMapMatrix->map(tpList->at(loop)->projP) );
          }

        break;

      case TaskPointTypes::Begin:

        if( mPointIsContained )
          {
            if( selectedTp != 0 && selectedTp->taskPointIndex != -1 &&
                // Task end point is selected
                selectedTp->taskPointIndex == tpList->at( tpList->size() - 2 )->taskPointIndex &&
                // Check, if task start and end point are identically
                tpList->at(1)->origP == tpList->at( tpList->size() - 2 )->origP )
              {
                // The selected TP is the end point and start and end point are
                // identically. In this case the start task figure
                // is not drawn to get more clearness about the task end figure.
                break;
              }

            QColor color;

            if( fillShape )
              {
                color = QColor(Qt::green);
              }

            switch ( figure )
            {
            case GeneralConfig::Line:
                tpList->at(loop)->getTaskLine().drawLine(painter);
                // draw boxes for debugging purposes
                // tpList->at(loop)->getTaskLine().drawRegionBoxes( painter );
                break;
            case GeneralConfig::Circle:
                drawCircle( painter, mPoint, crScaled, color, drawShape );
                break;
            case GeneralConfig::Sector:
                drawSector( painter,
                            mPoint,
                            sirScaled,
                            sorScaled,
                            biangle,
                            SectorAngle,
                            color,
                            drawShape );
                break;
            default:
                qWarning() << "FlightTask::drawTask:" << "Unknown task figure type" << figure;
                continue;
            }
          }

        // Draw line from take off to begin, if both not identical
        if( loop &&
            tpList->at(loop - 1)->origP != tpList->at(loop)->origP )
          {
            painter->setPen(QPen(courseLineColor, courseLineWidth));
            painter->drawLine( glMapMatrix->map(tpList->at(loop - 1)->projP),
                               glMapMatrix->map(tpList->at(loop)->projP) );
          }

        break;

      case TaskPointTypes::End:

        if( selectedTp != 0 && selectedTp->taskPointIndex != -1 &&
            // Check, if task start and end point are identically
            tpList->at(1)->origP == tpList->at( tpList->size() - 2 )->origP &&
            // Check if task start point or first route point is selected
            ( selectedTp->taskPointIndex == tpList->at(1)->taskPointIndex ||
              selectedTp->taskPointIndex == tpList->at(2)->taskPointIndex ) )
          {
            // The selected TP is the start point or the first route point
            // and start and end point are identically.
            // In this case the end task figure is not drawn to get more
            // clearness about the task start figure.
            mPointIsContained = false;
          }

        if( mPointIsContained )
          {
            QColor color;

            if( fillShape )
              {
                color = QColor(Qt::cyan);
              }

            switch ( figure )
            {
                case GeneralConfig::Line:
                    tpList->at(loop)->getTaskLine().drawLine(painter);
                    // draw boxes for debugging purposes
                    // tpList->at(loop)->getTaskLine().drawRegionBoxes( painter );
                    break;
                case GeneralConfig::Circle:
                    drawCircle( painter, mPoint, crScaled, color, drawShape );
                    break;
                case GeneralConfig::Sector:
                    drawSector( painter,
                                mPoint,
                                sirScaled,
                                sorScaled,
                                biangle,
                                SectorAngle,
                                color,
                                drawShape );
                    break;
                default:
                    qWarning() << "FlightTask::drawTask:" << "Unknown task figure type" << figure;
                    continue;
            }
          }

        painter->setPen(QPen(courseLineColor, courseLineWidth));
        painter->drawLine( glMapMatrix->map(tpList->at(loop - 1)->projP),
                           glMapMatrix->map(tpList->at(loop)->projP) );
        break;

      default:

       // Can be take off or landing point
       if( loop == 0 && mPointIsContained &&
           tpList->size() >= 2 &&
           // take off point != begin point
           tpList->at(0)->origP != tpList->at(1)->origP &&
           // no taskpoint is selected
           ( selectedTp == 0 ||
           // a taskpoint selection is active
           ( selectedTp != 0 && selectedTp->taskPointIndex != -1 &&
           // task end point is not selected
           selectedTp->taskPointIndex != tpList->at( tpList->size() - 2 )->taskPointIndex &&
           // task landing point is not selected
           selectedTp->taskPointIndex != tpList->at( tpList->size() - 1 )->taskPointIndex ) ))
          {
           // Draw a circle around the start point, if start and begin point
           // are not the same and task end point and task landing point
           // are not selected.
            QColor color;

            if( fillShape )
              {
                color = QColor(Qt::green);
              }

            drawCircle( painter, mPoint, crScaled, color, drawShape );
          }
        else if( loop == tpList->size()-1 && mPointIsContained &&
                 tpList->size() >= 2 &&
                 // landing point != end point
                 tpList->at(loop)->origP != tpList->at(loop-1)->origP &&
                 // no taskpoint is selected
                 ( selectedTp == 0 ||
                 // a taskpoint selection is active
                 ( selectedTp != 0 && selectedTp->taskPointIndex != -1 &&
                 // task take off point is not selected
                 selectedTp->taskPointIndex != tpList->at(0)->taskPointIndex &&
                 // task begin point is not selected
                 selectedTp->taskPointIndex != tpList->at(1)->taskPointIndex )))
          {
            // Draw a circle around the landing point, if end and landing point
            // are not the same and task take off point and task begin point
            // are not selected.
            QColor color;

            if( fillShape )
              {
                color = QColor(Qt::cyan);
              }

            drawCircle( painter, mPoint, crScaled, color, drawShape );
          }

        // Draw a line from the End to the Landing point, if both are not identical
        if( loop && tpList->size() >= 2 &&
            tpList->at(loop - 1)->origP != tpList->at(loop)->origP )
          {
            painter->setPen(QPen(courseLineColor, courseLineWidth));
            painter->drawLine( glMapMatrix->map(tpList->at(loop - 1)->projP),
                               glMapMatrix->map(tpList->at(loop)->projP ) );
          }

        break;
      }
    }

  // Restore the previous painter state.
  painter->restore();
}

/**
 * Draws a circle around the given position.
 *
 * coordinate as projected position of the point
 * scaled radius as meters
 * fillColor, do not fill, if set to invalid
 * drawShape, if set to true, draw outer circle with black color
 */

void FlightTask::drawCircle( QPainter* painter,
                             QPoint& centerCoordinate,
                             const int radius,
                             QColor& fillColor,
                             const bool drawShape )
{
  // fetch current scale, scale uses unit meter/pixel
  const double cs = glMapMatrix->getScale(MapMatrix::CurrentScale);

  if( cs > 350 || radius == 0 ||
      (fillColor.isValid() == false && drawShape == false) )
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
      qreal lineWidth  = GeneralConfig::instance()->getTaskFiguresLineWidth();
      QColor& color = GeneralConfig::instance()->getTaskFiguresColor();
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QPen(color, lineWidth));
      painter->drawEllipse( centerCoordinate.x()-radius, centerCoordinate.y()-radius,
                            radius*2, radius*2 );
    }
}

/**
 * Draws a sector around the given position.
 *
 * Painter as painter device
 * coordinate as projected position of the point
 * scaled inner radius as meters
 * scaled outer radius as meters
 * bisector angle in degrees
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
      qreal lineWidth  = GeneralConfig::instance()->getTaskFiguresLineWidth();
      QColor& color = GeneralConfig::instance()->getTaskFiguresColor();
      painter->setBrush(Qt::NoBrush);
      painter->setPen(QPen(color, lineWidth));
      painter->drawPath( pp );
    }
}

/**
 *
 * Calculates the sector array used for the drawing of the task point
 * sector.
 *
 * pp sector result painter path
 * ocx  scaled outer radius center coordinate x
 * ocy  scaled outer radius center coordinate y
 * icx  scaled inner radius center coordinate x
 * icy  scaled inner radius center coordinate y
 * ora  scaled outer radius
 * ira  scaled inner radius
 * sba  sector biangle in degrees
 * sa sector angle in degrees
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

      // The small arc around the center point and inside to the
      // big arc. Can have the same size as big arc.
      pp.arcTo( (qreal) icx, (qreal) icy, (qreal) (2 * ira), (qreal) (2 * ira), (qreal) (w1 - sa/2), (qreal) sa );
    }

  pp.closeSubpath();
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

ReachablePoint::reachable
FlightTask::calculateFinalGlidePath( const int taskPointIndex,
                                          Altitude &arrivalAlt,
                                          Speed &bestSpeed )
{
  int wpCount = tpList->count();

  arrivalAlt.setInvalid();
  bestSpeed.setInvalid();

  if( taskPointIndex >= wpCount )
    {
      // taskPointIndex points behind the end of the list
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

  // calculate bearing from current position to the next task point from
  // task in radian
  int bearing = int ( rint( getBearingWgs( calculator->getlastPosition(),
                                           tpList->at( taskPointIndex )->origP ) ));

  // calculate distance from current position to the next task point from
  // task in km
  QPoint p1 = calculator->getlastPosition();
  QPoint p2 = tpList->at( taskPointIndex )->origP;
  double distance = dist( &p1, &p2 );

  bool res = calculator->glidePath( bearing, Distance(distance * 1000.0),
                                    tpList->at( taskPointIndex )->elevation,
                                    arrAlt, bestSpeed );

  if( ! res )
    {
      return ReachablePoint::no; // glide path calculation failed, no glider selected
    }

#ifdef CUMULUS_DEBUG
  qDebug( "WP=%s, Bearing=%.1f°, Dist=%.1fkm, Ele=%.1fm, ArrAlt=%.1f",
          tpList->at( taskPointIndex )->name.toLatin1().data(),
          bearing*180/M_PI,
          distance,
          tpList->at( taskPointIndex )->elevation,
          arrAlt.getMeters() );
#endif

  // Summarize single altitudes, if the taskpoint is not the last one.
  usedAlt = curAlt-arrAlt;

  for( int i=taskPointIndex; i+1 < wpCount; i++ )
    {
      if( tpList->at(i)->origP == tpList->at(i+1)->origP )
        {
          continue; // points are equal, we ignore them
        }

      res = calculator->glidePath( (int) rint( tpList->at( i+1 )->bearing ),
                                   Distance( tpList->at( i+1 )->distance * 1000.0),
                                   tpList->at( i+1 )->elevation,
                                   arrAlt, speed );
#ifdef CUMULUS_DEBUG
      qDebug( "WP=%s, Bearing=%.1f°, Dist=%.1fkm, Ele=%.1m, ArrAlt=%.1f",
              tpList->at( i+1 )->name.toLatin1().data(),
              tpList->at( i+1 )->bearing*180/M_PI,
              tpList->at( i+1 )->distance,
              tpList->at( i+1 )->elevation,
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

QString FlightTask::getTotalDistanceString( bool unit ) const
  {
    if(flightType == FlightTask::NotSet)
      {
        return "--";
      }

    return Distance::getText(distance_total*1000, unit, 1);
  }


QString FlightTask::getTaskDistanceString( bool unit ) const
  {
    if(flightType == FlightTask::NotSet)
      {
        return "--";
      }

    return Distance::getText( distance_task*1000, unit, 1 );
  }

/**
 * @returns a string representing the total distance time of task as
 * hh:mm to use for user-display. The time is rounded up if seconds
 * are greater than 30.
 */
QString FlightTask::getTotalDistanceTimeString()
{
  return getDistanceTimeString( duration_total );
}

/**
 * @returns a string representing the distance time as hh:mm to use
 * for user-display. The time is rounded up if seconds are greater
 * than 30. Input has to be passed as seconds.
 */
QString FlightTask::getDistanceTimeString(const int timeInSec)
{
  if( timeInSec == 0 )
    {
      return "-";
    }

  int dt = timeInSec;

  // Round up, if seconds over 30. Must be done because the seconds
  // will be truncated from the time string without rounding
  if( dt % 60 > 30 ) dt +=30;

  QString duration;

  duration = QString("%1:%2")
                .arg(dt/3600)
                .arg((dt % 3600)/60, 2, 10, QChar('0') );

  return duration;
}

/**
 * @returns a string representing the cruising speed to use for
 * user-display.
 */
QString FlightTask::getSpeedString() const
{
  if( flightType == FlightTask::NotSet || cruisingSpeed == 0 )
    {
      return QObject::tr("none");
    }

  QString v;

  v = QString("%1%2").arg(cruisingSpeed).arg(Speed::getHorizontalUnitText());

  return v;
}

/** Returns wind direction and speed in string format "Degree/Speed". */
QString FlightTask::getWindString() const
{
  if( windSpeed == 0 )
    {
      return QObject::tr("none");
    }

  if( wtCalculation == false )
    {
      return QObject::tr("too strong!");
    }

  QString w;

  w = QString( "%1%2/%3%4")
         .arg( windDirection, 3, 10, QChar('0') )
         .arg( QString(Qt::Key_degree) )
         .arg( windSpeed )
         .arg( Speed::getWindUnitText() );

  return w;
}

/**
 * Takes over a task point list. An old list is deleted.
 */
void FlightTask::setTaskPointList(QList<TaskPoint*> *newtpList)
{
  if( newtpList == 0 )
    {
      return; // ignore null object
    }

   // Remove old content of list
  qDeleteAll(*tpList);
  delete tpList;

  // @AP: take over ownership about the new passed list
  tpList = newtpList;
  updateTask();
}

/**
 * updates the internal task data
 */
void FlightTask::updateTask()
{
  setTaskPointData();
  determineTaskType();

  for (int loop = 0; loop < tpList->count(); loop++)
    {
      // number point with index
      tpList->at(loop)->taskPointIndex = loop;

      // calculate turn point sector angles
      calculateSectorAngles(loop);
    }
}

/**
 * updates projection data of all task point elements in the list
 */
void FlightTask::updateProjection()
{
  for(int loop = 0; loop < tpList->count(); loop++)
    {
      // calculate projection data
      tpList->at(loop)->projP = _globalMapMatrix->wgsToMap(tpList->at(loop)->origP);
    }
}

/**
 * @AP: Add a new task point to the list. This takes over the ownership
 * of the passed object.
 */
void FlightTask::addTaskPoint( TaskPoint *newTp )
{
  if( newTp == static_cast<TaskPoint *>(0) )
    {
      return; // ignore null object
    }

  tpList->append( newTp );
  updateTask();
}

/**
 * Returns a deep copy of the task point list. The ownership of the list
 * is taken over by the caller.
 */
QList<TaskPoint*> *FlightTask::getCopiedTpList()
{
  return copyTpList( tpList );
}

  /**
   * Returns a deep copy of the input list. The ownership of the
   * list is taken over by the caller.
   */
QList<TaskPoint*> *FlightTask::copyTpList(QList<TaskPoint*> *tpListIn)
{
  QList<TaskPoint *> *outList = new QList<TaskPoint *>;

  if( tpListIn == 0 )
    {
      // returns an empty list on no input
      return outList;
    }

  for(int loop = 0; loop < tpListIn->count(); loop++)
    {
      TaskPoint *tp = new TaskPoint( *tpListIn->at(loop) );
      outList->append( tp );
    }

  return outList;
}

void FlightTask::setPlanningType(const int type)
{
  _planningType = type;
  setTaskPointData();
}
