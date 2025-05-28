/***********************************************************************
**
**   flighttask.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2007-2025 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>
#include <QtGui>

#include "mapcalc.h"
#include "calculator.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "layout.h"
#include "map.h"
#include "mapcalc.h"
#include "speed.h"

#undef CUMULUS_DEBUG

extern Calculator *calculator;
extern MapMatrix  *_globalMapMatrix;

/**
  * Creates an empty task.
  */
FlightTask:: FlightTask() :
  BaseMapElement("FlightTask", BaseMapElement::Task ),
  faiRules(false),
  windDirection(0),
  windSpeed(0),
  wtCalculation(false),
  flightType(FlightTask::NotSet),
  distance(0.0),
  duration_total(0),
  _planningType(RouteBased),
  _taskName(QObject::tr("unknown"))
{
  cruisingSpeed.setMps( 0.0 );
}

FlightTask::FlightTask( QList<TaskPoint>& tpListIn,
                        bool faiRules,
                        QString taskName,
                        Speed tas ) :
  BaseMapElement("FlightTask", BaseMapElement::Task ),
  tpList(tpListIn),
  faiRules(faiRules),
  cruisingSpeed(tas),
  windDirection(0),
  windSpeed(0),
  wtCalculation(false),
  flightType(FlightTask::NotSet),
  distance(0.0),
  duration_total(0),
  _planningType(RouteBased),
  _taskName(taskName)
{
  if( _taskName.isNull() )
    {
      _taskName = QObject::tr("unknown");
    }

  // only do this if tpList is not empty!
  if( tpList.count() > 0 )
    {
      updateTask();
    }
}

/**
 * Copy constructor
 **/
FlightTask::FlightTask (const FlightTask& inst) :
    BaseMapElement (inst)
{
  // qDebug("FlightTask::FlightTask( const FlighTask& inst ) is called");

  faiRules = inst.faiRules;
  cruisingSpeed = inst.cruisingSpeed;
  windDirection = inst.windDirection;
  windSpeed = inst.windSpeed;
  wtCalculation = inst.wtCalculation;
  tpList = inst.tpList;
  flightType = inst.flightType;
  distance = inst.distance;
  duration_total = inst.duration_total;
  _planningType = inst._planningType;
  _taskName = inst._taskName;
  _declarationDateTime = inst._declarationDateTime;
  _taskStartTime = inst._taskStartTime;
  _taskEndTime = inst._taskEndTime;
}

FlightTask::~FlightTask()
{
  // qDebug("FlightTask::~FlightTask(): name=%s, %X", _taskName.toLatin1().data(), this );
}

/**
 * Determines the type of the task.
 **/
void FlightTask::determineTaskType()
{
  distance = 0.0;

  if( tpList.size() > 0 )
    {
      for( int loop = 0; loop < tpList.size(); loop++)
        {
          // qDebug() << "distance" << loop << ":" << tpList.at(loop).distance;
          distance += tpList.at(loop).distance;
        }
      // qDebug() << "Total Distance:" << distance;
    }

  if( tpList.size() < 2 )
    {
      flightType = FlightTask::NotSet;
      return;
    }

  // Short distances should be calculated by using pythagoras formula. All
  // other formulas can lead to nan (not a number), if distance is zero.
  double dp =
      MapCalc::distP( tpList[0].getWGSPositionPtr()->lat(),
		      tpList[0].getWGSPositionPtr()->lon(),
                      tpList[tpList.count()-1].getWGSPositionPtr()->lat(),
		      tpList[tpList.count()-1].getWGSPositionPtr()->lon() );

  // qDebug() << "Entfernung Beginn-Ende:" << dp;

  if( dp < 1.0 )
    {
      // Distance between start and finish point is lower as one km. We
      // check the FAI rules
      switch( tpList.count() )
        {
	case 2:
	  // two points only inside of distance of 1km -> unknown
	  flightType = FlightTask::Unknown;
	  break;

        case 3:
          // Zielrückkehr
          flightType = FlightTask::ZielR;
          break;

        case 4:
          // FAI Dreieck
          if(isFAI( distance, tpList.at(1).distance,
                    tpList.at(2).distance, tpList.at(3).distance))
            flightType = FlightTask::FAI;
          else
            // Dreieck
            flightType = FlightTask::Dreieck;
          break;

        case 5:
          // Check the DMSt Viereck rules
          if( isDMStViereck( tpList[0].getWGSPositionPtr(),
                             tpList[1].getWGSPositionPtr(),
                             tpList[2].getWGSPositionPtr(),
                             tpList[3].getWGSPositionPtr(),
                             tpList[4].getWGSPositionPtr()) )
            {
              flightType = FlightTask::DMStViereck;
            }
          else
            {
              // Vieleck
              flightType = FlightTask::Vieleck;
            }

          break;

        default:
          // unknown
          flightType = FlightTask::Unknown;
          break;
        }
    }
  else
    {
      if( tpList.count() >= 2 )
        // Zielstrecke
        flightType = FlightTask::ZielS;
    }
}

/**
 * Calculates the task point sector angles in radian. The sector angle
 * between two task points is the bisecting line of the angle.
 */
double FlightTask::calculateSectorAngles( int loop )
{
  // get configured sector angle
  const double sectorAngle = tpList.at(loop).getTaskSectorAngle() * M_PI/180.;

  double bisectorAngle = 0.0;
  double minAngle = 0.0;
  double maxAngle = 0.0;

#ifdef CUMULUS_DEBUG
  QString part = "WP-NN";
#endif

  // In some cases during planning, this method is called with wrong
  // loop-values. Therefore we must check the id before calculating
  // the direction
  int taskPointType = tpList.at(loop).getTaskPointType();

  switch( taskPointType )
    {
    case TaskPointTypes::Start:

#ifdef CUMULUS_DEBUG
      part = "WP-Begin (" + tpList.at(loop)->name + "-" + tpList.at(loop+1)->name + ")";
#endif

      // directions to the next point
      if(tpList.count() >= loop + 1)
        {
          bisectorAngle = MapCalc::getBearing(tpList.at(loop).getWGSPosition(),
                                              tpList.at(loop+1).getWGSPosition());
        }
      break;

    case TaskPointTypes::Turn:

#ifdef CUMULUS_DEBUG
      part = "WP-RouteP ( " + tpList.at(loop-1).name + "-" +
             tpList.at(loop).name + "-" + tpList.at(loop+1).name + ")";
#endif

      if( loop >= 1 && tpList.count() >= loop + 1 )
        {
          // vector pointing to the outside of the two points
          bisectorAngle = MapCalc::outsideVector(tpList.at(loop).getWGSPosition(),
                                                 tpList.at(loop-1).getWGSPosition(),
                                                 tpList.at(loop+1).getWGSPosition());
        }
      break;

    case TaskPointTypes::Finish:

#ifdef CUMULUS_DEBUG
      part = "WP-End (" + tpList.at(loop).name +"-" + tpList.at(loop-1).name + ")";
#endif

      if(loop >= 1 && loop < tpList.count())
        {
          // direction to the previous point:
          bisectorAngle = MapCalc::getBearing( tpList.at(loop).getWGSPosition(),
                                               tpList.at(loop-1).getWGSPosition() );
        }
      break;

    default:
      break;
    }

  // set bisector angle of task point
  bisectorAngle = MapCalc::normalize( bisectorAngle );
  tpList[loop].angle = bisectorAngle;

  // Update line settings, if required.
  if( tpList[loop].getActiveTaskPointFigureScheme() == GeneralConfig::Line )
    {
      // set bisector angle for the task line as course direction
      if( taskPointType != TaskPointTypes::Finish )
        {
          tpList[loop].getTaskLine().setDirection( static_cast<int>(rint(bisectorAngle * 180.0/M_PI)) );
        }
      else
        {
          // direction to the previous point must be inverted in case of end point.
          tpList[loop].getTaskLine().setDirection( static_cast<int>(rint(bisectorAngle * 180.0/M_PI)) + 180 );
        }

      // set the center point of the task line
      tpList[loop].getTaskLine().setLineCenter( tpList[loop].getWGSPosition() );

      // calculate all line elements after a new setting
      tpList[loop].getTaskLine().calculateElements();
    }

  // invert bisector angle
  double invertAngle = bisectorAngle;
  invertAngle >= M_PI ? invertAngle -= M_PI : invertAngle += M_PI;

  // calculate min and max bisector angles
  minAngle = MapCalc::normalize( invertAngle - (sectorAngle/2.) );
  maxAngle = MapCalc::normalize( invertAngle + (sectorAngle/2.) );

  // save min and max bisector angles
  tpList[loop].minAngle = minAngle;
  tpList[loop].maxAngle = maxAngle;


#ifdef CUMULUS_DEBUG
  qDebug( "Loop=%d, Part=%s, Name=%s, Scale=%f, BisectorAngle=%3.1f, minAngle=%3.1f, maxAngle=%3.1f",
          loop, part.toLatin1().data(), tpList[loop].name.toLatin1().data(),
          glMapMatrix.getScale(),
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
  int cnt = tpList.size();

  if (cnt == 0)
    {
      return;
    }

  tpList[0].setTaskPointType(TaskPointTypes::Unknown);

  //  First task point is always set to these values
  tpList[0].distTime = 0;
  tpList[0].bearing = -1.;
  tpList[0].distance = 0.0;
  tpList[0].wca = 0;
  tpList[0].trueHeading = -1;
  tpList[0].groundSpeed = 0.0;
  tpList[0].wtResult = false;

  // Reset total duration
  duration_total = 0;

  // Initialize TAS and wind speed instances by using user defined units.
  if( windSpeed.getMps() == 0.0 )
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
      tpList[n].bearing = -1.;
      tpList[n].distance = 0.;
      tpList[n].wca = 0;
      tpList[n].trueHeading = -1.;
      tpList[n].groundSpeed = 0.0;
      tpList[n].wtResult = false;

      if( tpList[n-1].getWGSPosition() != tpList[n].getWGSPosition() )
        {
          // Points are not identical, do calculate navigation parameters.
          QPair<double, double> pair;

          // Calculate distance and bearing according to DMST specification.
          pair = MapCalc::distVinc( double( tpList[n-1].getWGSPositionPtr()->x() ),
                                    double( tpList[n-1].getWGSPositionPtr()->y() ),
                                    double( tpList[n].getWGSPositionPtr()->x() ),
                                    double( tpList[n].getWGSPositionPtr()->y() ) );

          tpList[n].distance = pair.first;
          tpList[n].bearing = pair.second;

          // calculate wind parameters, if wind speed is defined. Ground
          // speed unit is meter per second.
          if( wtCalculation )
              {
                tpList[n].wtResult =
                    MapCalc::windTriangle( tpList[n].bearing * 180/M_PI,
				           cruisingSpeed.getMps(),
				 	   windDirection,
					   windSpeed.getMps(),
					   tpList[n].groundSpeed,
					   tpList[n].wca,
					   tpList[n].trueHeading );

                if( tpList[n].wtResult == false )
                  {
                    // No wind triangle calculation possible. In such a case
                    // we do reset the global wt calculation flag too.
                    wtCalculation = false;
                  }
              }
        }

      tpList[n].setTaskPointType(TaskPointTypes::Unknown);

      double cs = cruisingSpeed.getMps();

      // Calculate all without wind because wind can be too strong at
      // one of the next legs.
      if( cs > 0. && tpList[n].distance > 0.)
        {
          // t=s/v distance unit is m, duration time unit is seconds and
          // TAS unit is meter per second.
          tpList[n].distTime =
            int( rint(tpList[n].distance * 1000 / cs) );

          // summarize total duration as seconds
          duration_total += tpList[n].distTime;
        }
      else
        {
          // reset duration to zero
          tpList[n].distTime = 0;
        }

#ifdef CUMULUS_DEBUG
      qDebug("Without Wind: WP=%s, TAS=%f, dist=%f, duration=%d, tc=%f, th=%f",
             tpList[n].name.toLatin1().data(),
             cruisingSpeed.getKph(),
             tpList[n].distance,
             tpList[n].distTime,
             tpList[n].bearing,
             tpList[n].trueHeading);
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
          double gs = tpList[n].groundSpeed;

          // Calculate all without wind because wind can be too strong at
          // one of the next legs.
          if( gs > 0. && tpList[n].distance > 0.)
            {
              // t=s/v distance unit is m, duration time unit is seconds and
              // ground speed unit is meter per second.
              tpList[n].distTime =
                int( rint(tpList[n].distance * 1000 / gs) );

              // summarize total duration as seconds
              duration_total += tpList[n].distTime;
            }
          else
            {
              // reset duration to zero
              tpList[n].distTime = 0;
            }

#ifdef CUMULUS_DEBUG
          qDebug("With Wind: WP=%s, GS=%f, Wca=%f, dist=%f, duration=%d, th=%f",
                 tpList[n].name.toLatin1().data(),
                 gs*3.6,
                 tpList[n].wca,
                 tpList[n].distance,
                 tpList[n].distTime,
                 tpList[n].trueHeading);
#endif

        }
    }

  // to less task points
  if (cnt < 2)
    {
      return;
    }

  tpList[0].setTaskPointType(TaskPointTypes::Start);
  tpList[cnt - 1].setTaskPointType(TaskPointTypes::Finish);

  for(int n = 1; n + 1 < cnt; n++)
    {
      tpList[n].setTaskPointType(TaskPointTypes::Turn);
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
    case FlightTask::DMStViereck:
      return QObject::tr("DMSt 4");
    case FlightTask::Vieleck:
      return QObject::tr("Polygon");
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

bool FlightTask::isDMStViereck( QPoint* p1,
                                QPoint* p2,
                                QPoint* p3,
                                QPoint* p4,
                                QPoint* p5 )
{
  double d15 = MapCalc::distVinc( p1, p5 ).first;
  double d12 = MapCalc::distVinc( p1, p2 ).first;
  double d23 = MapCalc::distVinc( p2, p3 ).first;
  double d34 = MapCalc::distVinc( p3, p4 ).first;
  double d41 = MapCalc::distVinc( p4, p1 ).first;
  double d13 = MapCalc::distVinc( p1, p3 ).first;

  double distTotal1 = d12 + d23 + d13;
  double distTotal2 = d13 + d34 + d41;

  if( d15 > 1.0 )
    {
      // Distance between start and finish must be less equal 1Km
      return false;
    }

  if( p2 == p3 || p2 == p4 || p3 == p4 )
    {
      // P2 and P4 must be different in every case
      return false;
    }

  if( isFAI( distTotal1, d12, d23, d13) == true &&
      isFAI( distTotal2, d13, d34, d41) == true )
    {
      return true;
    }

  return false;
}

/**
 * Draws course lines and turn point sectors/circles according to the
 * user configuration.
 */
void FlightTask::drawTask( QPainter* painter, QList<TaskPoint*>& drawnTp )
{
  // qDebug() << __PRETTY_FUNCTION__;

  // get user defined scheme items
  GeneralConfig* conf = GeneralConfig::instance();

  // Load the currently selected task point.
  TaskPoint selectedTp;

  if( calculator->getTargetWp() )
    {
      int tpIdx = calculator->getTargetWp()->taskPointIndex;

      if( tpIdx >= 0 && tpIdx < tpList.count() )
        {
          // Get selected taskpoint from the list.
          selectedTp = tpList[tpIdx];
        }
     }

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
  painter->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );

  for( int loop=0; loop < tpList.count(); loop++ )
    {
      // Load the sector data
      const int SectorAngle = tpList[loop].getTaskSectorAngle();
      const double Sor      = tpList[loop].getTaskSectorOuterRadius().getMeters();
      const double Sir      = tpList[loop].getTaskSectorInnerRadius().getMeters();

      int sorScaled = 0; // scaled sector outer radius
      int sirScaled = 0; // scaled sector inner radius
      int crScaled  = 0;  // scaled circle radius
      int llScaled  = 0;  // scaled line length

      // Load the circle data
      const double circleRadius = tpList[loop].getTaskCircleRadius().getMeters();

      // Load the line data
      const double lineLength = tpList[loop].getTaskLineLength().getMeters();

      // Determine what task figure has to be drawn.
      enum GeneralConfig::ActiveTaskFigureScheme figure =
          tpList[loop].getActiveTaskPointFigureScheme();

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
        case GeneralConfig::Keyhole:
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
          drawnTp.append( &tpList[loop] );
        }

      // map projected point to map display
      QPoint mPoint( glMapMatrix->map(tpList[loop].getPosition()) );

      bool mPointIsContained = viewport.contains(mPoint);

      if( mPointIsContained )
        {
          int size = static_cast<int>(10.0 * Layout::getScaledDensity());

          painter->setPen(QPen(Qt::black));
          painter->setBrush( QBrush( Qt::black, Qt::SolidPattern ) );
          painter->drawRect( mPoint.x() - size/2, mPoint.y() - size/2, size,size );
        }

      if( flightType == Unknown )
        {
          if( loop )
            {
              painter->setPen(QPen(courseLineColor, courseLineWidth));
              // Draws the course line
              painter->drawLine( glMapMatrix->map(tpList.at(loop - 1).getPosition()),
                                 glMapMatrix->map(tpList.at(loop).getPosition()) );
            }
        }

      // convert biangle (90...180) from radian to degrees
      int biangle = (int) rint( ((tpList.at(loop).angle) / M_PI ) * 180.0 );

      switch( tpList.at(loop).getTaskPointType() )
      {
      case TaskPointTypes::Turn:

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
                    tpList[loop].getTaskLine().drawLine(painter);
                    break;
                case GeneralConfig::Circle:
                    // Draw circle around given position
                    drawCircle( painter, mPoint, crScaled, color, drawShape );
                    break;
                case GeneralConfig::Keyhole:
                    drawKeyhole( painter,
                                 mPoint,
                                 sirScaled,
                                 sorScaled,
                                 biangle,
                                 SectorAngle,
                                 color,
                                 drawShape );
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
            painter->drawLine( glMapMatrix->map(tpList.at(loop - 1).getPosition()),
                               glMapMatrix->map(tpList.at(loop).getPosition()) );
          }

        break;

      case TaskPointTypes::Start:

        if( mPointIsContained )
          {
            if( selectedTp.getFlightTaskListIndex() != -1 &&
                // Task start point is not selected
                selectedTp.getFlightTaskListIndex() != tpList[0].getFlightTaskListIndex() &&
                // Check, if task start and finish point are identically
                tpList[0].getWGSPosition() == tpList.at( tpList.size() - 1 ).getWGSPosition() )
              {
                // The selected TP is not the point and start and finish point are
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
                tpList[loop].getTaskLine().drawLine(painter);
                // draw boxes for debugging purposes
                // tpList.at(loop).getTaskLine().drawRegionBoxes( painter );
                break;
            case GeneralConfig::Circle:
                drawCircle( painter, mPoint, crScaled, color, drawShape );
                break;
            case GeneralConfig::Keyhole:
                drawKeyhole( painter,
                             mPoint,
                             sirScaled,
                             sorScaled,
                             biangle,
                             SectorAngle,
                             color,
                             drawShape );
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

        break;

      case TaskPointTypes::Finish:

        if( selectedTp.getFlightTaskListIndex() != -1 &&
            // Task start point is selected
            selectedTp.getFlightTaskListIndex() == tpList[0].getFlightTaskListIndex() &&
            // Check, if task start and finish point are identically
            tpList[0].getWGSPosition() == tpList.at( tpList.size() - 1 ).getWGSPosition() )
          {
            // The selected TP is the start point
            // and start and finish point are identically.
            // In this case the finish task figure is not drawn to get more
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
                    tpList[loop].getTaskLine().drawLine(painter);
                    // draw boxes for debugging purposes
                    // tpList.at(loop).getTaskLine().drawRegionBoxes( painter );
                    break;
                case GeneralConfig::Circle:
                    drawCircle( painter, mPoint, crScaled, color, drawShape );
                    break;
                case GeneralConfig::Keyhole:
                    drawKeyhole( painter,
                                 mPoint,
                                 sirScaled,
                                 sorScaled,
                                 biangle,
                                 SectorAngle,
                                 color,
                                 drawShape );
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
        painter->drawLine( glMapMatrix->map(tpList.at(loop - 1).getPosition()),
                           glMapMatrix->map(tpList.at(loop).getPosition()) );
        break;

      default:
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
 * Draws a keyhole around the given position.
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

void FlightTask::drawKeyhole( QPainter* painter,
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
      painter->drawEllipse( centerCoordinate.x()-innerRadius,
                            centerCoordinate.y()-innerRadius,
                            innerRadius*2,
                            innerRadius*2 );
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
      painter->drawEllipse( centerCoordinate.x()-innerRadius,
                            centerCoordinate.y()-innerRadius,
                            innerRadius*2,
                            innerRadius*2 );
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
  int wpCount = tpList.count();

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

  // calculate distance and bearing from current position to the next task point
  // from the task in km
  QPoint p1 = calculator->getlastPosition();
  QPoint p2 = tpList.at( taskPointIndex ).getWGSPosition();

  QPair<double, double> p = MapCalc::distVinc( &p1, &p2 );

  double distance = p.first;
  int bearing = static_cast<int>( rint( p.second ) );

  bool res = calculator->glidePath( bearing, Distance( distance * 1000.0 ),
                                    tpList.at( taskPointIndex ).getElevation(),
                                    arrAlt, bestSpeed );

  if( ! res )
    {
      return ReachablePoint::no; // glide path calculation failed, no glider selected
    }

#ifdef CUMULUS_DEBUG
  qDebug( "WP=%s, Bearing=%.1f°, Dist=%.1fkm, Ele=%.1fm, ArrAlt=%.1f",
          tpList.at( taskPointIndex ).name.toLatin1().data(),
          bearing*180/M_PI,
          distance,
          tpList.at( taskPointIndex ).elevation,
          arrAlt.getMeters() );
#endif

  // Summarize single altitudes, if the taskpoint is not the last one.
  usedAlt = curAlt-arrAlt;

  for( int i=taskPointIndex; i+1 < wpCount; i++ )
    {
      if( tpList.at(i).getWGSPosition() == tpList.at(i+1).getWGSPosition() )
        {
          continue; // points are equal, we ignore them
        }

      res = calculator->glidePath( (int) rint( tpList.at( i+1 ).bearing ),
                                   Distance( tpList.at( i+1 ).distance * 1000.0),
                                   tpList.at( i+1 ).getElevation(),
                                   arrAlt, speed );
#ifdef CUMULUS_DEBUG
      qDebug( "WP=%s, Bearing=%.1f°, Dist=%.1fkm, Ele=%.1m, ArrAlt=%.1f",
              tpList.at( i+1 ).name.toLatin1().data(),
              tpList.at( i+1 ).bearing*180/M_PI,
              tpList.at( i+1 ).distance,
              tpList.at( i+1 ).elevation,
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

QString FlightTask::getTaskDistanceString( bool unit ) const
{
  if( flightType == FlightTask::NotSet )
    {
      return "--";
    }

  return Distance::getText(distance * 1000, unit, 1);
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

  v = QString("%1%2").arg(cruisingSpeed.getHorizontalValue()).arg(Speed::getHorizontalUnitText());

  return v;
}

/** Returns wind direction and speed in string format "Degree/Speed". */
QString FlightTask::getWindString() const
{
  if( windSpeed.getMps() == 0 )
    {
      return "-/-";
    }

  if( wtCalculation == false )
    {
      return QObject::tr("too strong!");
    }

  QString w;

  w = QString( "%1%2/%3%4")
         .arg( windDirection, 3, 10, QChar('0') )
         .arg( QString(Qt::Key_degree) )
         .arg( windSpeed.getWindValue() )
         .arg( Speed::getWindUnitText() );

  return w;
}

/**
 * Takes over a task point list. An old list is deleted.
 */
void FlightTask::setTaskPointList( QList<TaskPoint>& newtpList )
{
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

  for (int loop = 0; loop < tpList.count(); loop++)
    {
      // number point with index
      tpList[loop].setFlightTaskListIndex( loop );

      // calculate turn point sector angles
      calculateSectorAngles(loop);
    }
}

/**
 * updates projection data of all task point elements in the list
 */
void FlightTask::updateProjection()
{
  for(int loop = 0; loop < tpList.count(); loop++)
    {
      // calculate projection data
      tpList[loop].setPosition( _globalMapMatrix->wgsToMap(tpList[loop].getWGSPosition()) );
    }
}

/**
 * Add a new task point to the list.
 */
void FlightTask::addTaskPoint( TaskPoint& newTp )
{
  tpList.append( newTp );
  updateTask();
}

void FlightTask::setPlanningType(const int type)
{
  _planningType = type;
  setTaskPointData();
}

/**
 * Returns the flown time of the task in seconds. The result is null, if
 * task has not been started.
 */
int FlightTask::getFlightTime()
{
  if( _taskStartTime.isNull() )
    {
      // task time is invalid
      return 0;
    }

  if( _taskEndTime.isNull() )
    {
      // Task is not yet finished, calculate against current time.
      return _taskStartTime.secsTo( QDateTime::currentDateTime() );
    }

    // Task is finished.
  return _taskStartTime.secsTo( _taskEndTime );
}

/**
 * Reset all time entries in the flight task. E.g. start, pass and finish
 * times.
 */
void FlightTask::resetTimes()
{
  _taskStartTime = QDateTime();
  _taskEndTime = QDateTime();

  for( int i=0; i < tpList.size(); i++ )
    {
      tpList[i].resetPassTime();
    }
  }

/**
 * Called to calculate an average speed since task start. The speed is invalid,
 * when the calculation is not possible.
 */
Speed FlightTask::calAverageSpeed()
{
  Speed speed;

  if( tpList.size() == 0 || _taskStartTime.isNull() == true )
    {
      // Speed is invalid
      return speed;
    }

  double flownDistance = 0.0;
  int i = 0;

  for( i = 0; i < tpList.size(); i++ )
    {
      TaskPoint& tp = tpList[i];

      if( tp.getPassTime().isNull() )
        {
          break;
        }

      // distance in km
      flownDistance += tp.distance;
    }

  if( i == 0 )
    {
      // Check, if not the first task element has not time set. In this case
      // we cannot calculate a speed. Normally that should not be happen ;-)
      return speed;
    }

  if( _taskEndTime.isNull() )
    {
      // The task end is not reached. We have to calculate the distance to the
      // last passed taskpoint, to calculate the average speed.
      // calculate distance in km.
      QPoint cPos = calculator->getlastPosition();
      double lastdistance = MapCalc::distVinc( tpList[i-1].getWGSPositionPtr(),
                                               &cPos ).first;

      // The task is unfinished. Calculate the result speed in m/s.
      flownDistance += lastdistance;
      double av = flownDistance * 1000 / getFlightTime();
      speed.setMps( av );
    }
  else
    {
      // The task is finished. Calculate the result speed in m/s.
      double av = flownDistance * 1000 / getFlightTime();
      speed.setMps( av );
    }

  /*
  qDebug() << "FlightTask::calAverageSpeed()"
           << "Distance" << flownDistance
           << "AV" << speed.getHorizontalText();
  */

  return speed;
}

/**
 * Reset all time entries from all task points and set the task start date
 * and time as local time.
 */
void FlightTask::setStartTime()
{
  resetTimes();
  _taskStartTime = QDateTime::currentDateTime();

  if( tpList.size() > 0 )
    {
      TaskPoint& tp = tpList[0];
      tp.setPassTime();
    }
}

/** Set the task end date and time as local time. */
void FlightTask::setEndTime()
{
  _taskEndTime = QDateTime::currentDateTime();

  if( tpList.size() > 0 )
    {
      TaskPoint& tp = tpList[ tpList.size() - 1 ];
      tp.setPassTime();
    }
}
