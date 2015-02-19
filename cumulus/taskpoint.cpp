/***********************************************************************
 **
 **   taskpoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2010-2015 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtCore>

#include "generalconfig.h"
#include "mapcalc.h"
#include "taskpoint.h"

// Near check distance state as meters for circle and sector figure
#define NEAR_DISTANCE 1000.0

// Near check distance state as meters for a line figure
#define NEAR_DISTANCE_LINE 2000.0

TaskPoint::TaskPoint( enum TaskPointTypes::TaskPointType type ) :
  SinglePoint(),
  angle(0.0),
  minAngle(0.0),
  maxAngle(0.0),
  bearing(-1),
  distance(0.0),
  distTime(0),
  wca(0.0),
  trueHeading(-1.0),
  groundSpeed(0.0),
  wtResult(false),
  m_taskPointType(type),
  m_taskActiveTaskPointFigureScheme(GeneralConfig::Undefined),
  m_lastPassageState(Outside),
  m_lastDistance(-1),
  m_taskSectorAngle(0),
  m_autoZoom(false),
  m_userEdited(false),
  m_flightTaskListIndex(-1)
{
  setTypeID( BaseMapElement::Turnpoint );
  setConfigurationDefaults();
}

TaskPoint::TaskPoint( const Waypoint& wp, enum TaskPointTypes::TaskPointType type ) :
  SinglePoint( wp.description,
               wp.name,
               BaseMapElement::Turnpoint,
               wp.wgsPoint,
               wp.projPoint,
               wp.elevation,
               wp.country,
               wp.comment ),
  angle(0.0),
  minAngle(0.0),
  maxAngle(0.0),
  bearing(-1),
  distance(0.0),
  distTime(0),
  wca(0.0),
  trueHeading(-1.0),
  groundSpeed(0.0),
  wtResult(false),
  m_taskPointType(type),
  m_taskActiveTaskPointFigureScheme(GeneralConfig::Undefined),
  m_lastPassageState(Outside),
  m_lastDistance(-1),
  m_taskSectorAngle(0),
  m_autoZoom(false),
  m_userEdited(false),
  m_flightTaskListIndex(-1)
{
  m_taskLine.setLineCenter( wp.wgsPoint );
  setConfigurationDefaults();
}

TaskPoint::~TaskPoint()
{
  // qDebug( "TaskPoint::~TaskPoint(): name=%s, %X", name.toLatin1().data(), (uint) this );
}

Waypoint* TaskPoint::getWaypointObject()
{
  // Update data of waypoint object
  m_wpObject.name = getWPName();
  m_wpObject.description = getName();
  m_wpObject.comment = getComment();
  m_wpObject.type = getTypeID();
  m_wpObject.wgsPoint = getWGSPosition();
  m_wpObject.projPoint = getPosition();
  m_wpObject.elevation = getElevation();
  m_wpObject.taskPointIndex = getFlightTaskListIndex();
  m_wpObject.country = getCountry();
  m_wpObject.priority = Waypoint::High;

  return &m_wpObject;
}

enum TaskPoint::PassageState TaskPoint::checkPassage( const Distance& dist2Tp,
                                                      const QPoint& position )
{
  // qDebug() << "TaskPoint::checkPassage: TP-IDX=" << m_flightTaskListIndex;

  if( m_taskPointType == -1 )
    {
      // no active task point
      return Outside;
    }

  // get user defined scheme item
  const enum GeneralConfig::ActiveTaskFigureScheme scheme = getActiveTaskPointFigureScheme();

  if( scheme == GeneralConfig::Line )
    {
      return determineLinePassageState( dist2Tp, position );
    }

  if( scheme == GeneralConfig::Circle )
    {
      // Circle scheme is active
      return determineCirclePassageState( dist2Tp.getMeters(),
                                          m_taskCircleRadius.getMeters() );
    }

  if( scheme == GeneralConfig::Sector )
    {
      return determineSectorPassageState( dist2Tp.getMeters(), position );
    }

  // That should normally not happen
  m_lastDistance = -1.0;
  m_lastPassageState = Outside;
  return Outside;
}

enum TaskPoint::PassageState
  TaskPoint::determineLinePassageState( const Distance& dist2Tp,
                                        const QPoint& position )
{
  if( m_lastPassageState == Touched )
    {
      // A passed state was set before but as touched reported. Now we
      // report the passed state.
      m_lastPassageState = Outside;
      return Passed;
    }

  if( dist2Tp.getMeters() < getTaskLineLength().getMeters() )
    {
      // We are inside of our observation zone to decide, if a line crossing
      // has started.
      bool state = getTaskLine().checkCrossing( position );

      if( state == true )
        {
          m_lastPassageState = Touched;

          // WE return as first a touched that the logger interval is
          // minimized. As next a passed is returned in every case.
          return Touched;
        }
      else
        {
          m_lastPassageState = Near;
          return Near;
        }
    }
  else if( dist2Tp.getMeters() <= NEAR_DISTANCE_LINE )
    {
      m_lastPassageState = Near;
      return Near;
    }

  m_lastPassageState = Outside;
  return Outside;
}

enum TaskPoint::PassageState
  TaskPoint::determineCirclePassageState( const double dist2Tp,
                                          const double insideRadius )
{
  enum GeneralConfig::ActiveTaskSwitchScheme tsSchema =
    GeneralConfig::instance()->getActiveTaskSwitchScheme();

  if( m_lastPassageState == TaskPoint::Touched )
    {
      if( tsSchema == GeneralConfig::Touched )
        {
          // We had one touch and report now the passage as valid.
          m_lastDistance = -1.0;
          m_lastPassageState = Outside;
          return Passed;
        }

      if( tsSchema == GeneralConfig::Nearst )
        {
          // We had a touch and must now observe the further approach to the center
          // of the taskpoint. If the minimum approach is reached, passed is reported
          // otherwise touched.
          if( dist2Tp < m_lastDistance )
            {
              // We are in further approach
              m_lastDistance = dist2Tp;
              m_lastPassageState = TaskPoint::Touched;
              return TaskPoint::Touched;
            }
          else
            {
              // We have passed the minimum approach
              m_lastDistance = -1.0;
              m_lastPassageState = Outside;
              return Passed;
            }
        }

      qWarning() << "TaskPoint::determinePassageState(): unknown Task figure schema"
                 << tsSchema;
    }

  if( dist2Tp < insideRadius && m_lastPassageState != TaskPoint::Touched )
    {
      // We have entered the circle the first time.
      // There are two different situations now.
      //
      // First: If touched schema is set, we report as first touched and as next
      //        passed.
      //
      // Second: If nearest scheme is set, we report as first touched and
      //         the passed state not until the minimum approach is passed.
      m_lastDistance = dist2Tp;
      m_lastPassageState = TaskPoint::Touched;
      return TaskPoint::Touched;
    }

  if( dist2Tp < insideRadius + NEAR_DISTANCE )
    {
      m_lastDistance = dist2Tp;
      m_lastPassageState = TaskPoint::Near;
      return TaskPoint::Near;
    }

  m_lastPassageState = TaskPoint::Outside;
  m_lastDistance = -1.0;
  return TaskPoint::Outside;
}

enum TaskPoint::PassageState
  TaskPoint::determineSectorPassageState( const double dist2Tp,
                                          const QPoint& position )
{
  enum GeneralConfig::ActiveTaskSwitchScheme tsSchema =
    GeneralConfig::instance()->getActiveTaskSwitchScheme();

  // get sector radii
  const double innerRadius = getTaskSectorInnerRadius().getMeters();
  const double outerRadius = getTaskSectorOuterRadius().getMeters();

  if( m_lastPassageState == Touched )
    {
      if( tsSchema == GeneralConfig::Touched )
        {
          // Report passed because we had one touch.
          m_lastDistance = -1.0;
          m_lastPassageState = Outside;
          return Passed;
        }

      if( tsSchema == GeneralConfig::Nearst )
        {
          if( dist2Tp > m_lastDistance )
            {
              // Report passed because we are moving away from the touched taskpoint.
              m_lastDistance = -1.0;
              m_lastPassageState = Outside;
              return Passed;
            }

          if( innerRadius > 0 && dist2Tp < innerRadius )
            {
              // We have left the sector part.
              m_lastDistance = -1.0;
              m_lastPassageState = Outside;
              return Passed;
            }
        }
    }

  if( m_lastPassageState != Touched )
    {
      if( dist2Tp > outerRadius + NEAR_DISTANCE )
        {
          // we are outside of outer radius, sector angle has not to be
          // considered
          m_lastDistance = -1.0;
          m_lastPassageState = Outside;
          return Outside;
        }

      if( dist2Tp > outerRadius )
        {
          // we are outside of outer radius, sector angle has not to be
          // considered
          m_lastDistance = dist2Tp;
          m_lastPassageState = Near;
          return Near;
        }

      // the inner radius is considered as minimum. If we are inside the
      // inner radius, we are outside of the sector.
      if( ( innerRadius > 0.0 || innerRadius == outerRadius ) &&
          dist2Tp < innerRadius )
        {
          // we are inside of inner radius, sector angle has not to be
          // considered
          m_lastDistance = dist2Tp;
          m_lastPassageState = Near;
          return Near;
        }
    }

  // Here we are inside of outer radius, therefore we have to check the sector angle.
  // Calculate bearing from TP to current position
  const double bearing = MapCalc::getBearingWgs( getWGSPosition(), position );

#ifdef CUMULUS_DEBUG
  qDebug( "TP::checkSector(): minAngle=%f, maxAngel=%f, bearing=%f",
          minAngle*180./M_PI,
          maxAngle*180./M_PI,
          bearing*180./M_PI );
#endif

  if( minAngle > maxAngle &&
      ( bearing > minAngle || bearing < maxAngle ) )
    {
      // We are inside of sector and sector includes north direction
      m_lastDistance = dist2Tp;
      m_lastPassageState = Touched;
      return Touched;
    }

  if( bearing > minAngle && bearing < maxAngle )
    {
      // We are inside of sector between 0...360
      m_lastDistance = dist2Tp;
      m_lastPassageState = Touched;
      return Touched;
    }

  if( m_lastPassageState == Touched )
    {
      // The sector is left in nearest schema. We report the passing now.
      m_lastDistance = -1.0;
      m_lastPassageState = Outside;
      return Passed;
    }

  m_lastDistance = -1.0;
  m_lastPassageState = Outside;
  return Outside;
}

QString TaskPoint::getTaskPointTypeString( bool detailed ) const
{
  switch( m_taskPointType )
    {
    case TaskPointTypes::TakeOff:
      if( detailed ) return QObject::tr( "Takeoff" );
      else return QObject::tr( "TO" );
    case TaskPointTypes::Begin:
      if( detailed ) return QObject::tr( "Begin" );
      else return QObject::tr( "B" );
    case TaskPointTypes::RouteP:
      if( detailed ) return QObject::tr( "Route" );
      else return QObject::tr( "R" );
    case TaskPointTypes::End:
      if( detailed ) return QObject::tr( "End" );
      else return QObject::tr( "E" );
    case TaskPointTypes::FreeP:
      if( detailed ) return QObject::tr( "Free Point" );
      else return QObject::tr( "F" );
    case TaskPointTypes::Landing:
      if( detailed ) return QObject::tr( "Landing" );
      else return QObject::tr( "LG" );
    case TaskPointTypes::NotSet:
    default:
      if( detailed ) return QObject::tr( "Not set" );
      else return QObject::tr( "NS" );
    }

  if( detailed )
    {
      return QObject::tr("Not set");
    }
  else
    {
      return QObject::tr("NS");
    }
}

QString TaskPoint::getTaskPointFigureString() const
{
  switch( m_taskActiveTaskPointFigureScheme )
  {
    case GeneralConfig::Circle:
      return QObject::tr ("Circle radius: %1").arg(m_taskCircleRadius.getMeters());
    case GeneralConfig::Sector:
      return QObject::tr ("Sector radius: %1").arg(m_taskSectorOuterRadius.getMeters());
    case GeneralConfig::Line:
      return QObject::tr ("Line: %1").arg(m_taskLine.getLineLength());
    default:
      return QObject::tr ("unknown");
  }
}

QString TaskPoint::getTaskPointTypeFigureString() const
{
  QString result = getTaskPointTypeString();

  result += "-";

  switch( m_taskActiveTaskPointFigureScheme )
    {
      case GeneralConfig::Circle:
        result += QObject::tr("C");
        break;
      case GeneralConfig::Sector:
        result += QObject::tr("S");
        break;
      case GeneralConfig::Line:
        result += QObject::tr("L");
        break;
      default:
        result += QObject::tr("U");
        break;
    }

  return result;
}

void TaskPoint::setConfigurationDefaults()
{
  GeneralConfig* conf = GeneralConfig::instance();

  switch( m_taskPointType )
    {
      case TaskPointTypes::Begin:
        setTaskCircleRadius( conf->getTaskStartRingRadius() );
        setTaskSectorInnerRadius( conf->getTaskStartSectorIRadius() );
        setTaskSectorOuterRadius( conf->getTaskStartSectorORadius() );
        getTaskLine().setLineLength( conf->getTaskStartLineLength().getMeters() );
        setTaskSectorAngle( conf->getTaskStartSectorAngel() );
        setActiveTaskPointFigureScheme( conf->getActiveTaskStartScheme() );
        break;

      case TaskPointTypes::End:
        setTaskCircleRadius( conf->getTaskFinishRingRadius() );
        setTaskSectorInnerRadius( conf->getTaskFinishSectorIRadius() );
        setTaskSectorOuterRadius( conf->getTaskFinishSectorORadius() );
        getTaskLine().setLineLength( conf->getTaskFinishLineLength().getMeters() );
        setTaskSectorAngle( conf->getTaskFinishSectorAngel() );
        setActiveTaskPointFigureScheme( conf->getActiveTaskFinishScheme() );
        break;

      // These points are not editable and always from type cylinder, radius 500m.
      case TaskPointTypes::TakeOff:
      case TaskPointTypes::Landing:
        setTaskCircleRadius( Distance(500) );
        setTaskSectorInnerRadius( Distance(0) );
        setTaskSectorOuterRadius( Distance(0) );
        getTaskLine().setLineLength( 0.0 );
        setTaskSectorAngle( 0 );
        setActiveTaskPointFigureScheme( GeneralConfig::Circle );
        break;

      default:
        setTaskCircleRadius( conf->getTaskObsCircleRadius() );
        setTaskSectorInnerRadius( conf->getTaskObsSectorInnerRadius() );
        setTaskSectorOuterRadius( conf->getTaskObsSectorOuterRadius() );
        getTaskLine().setLineLength( 0.0 );
        setTaskSectorAngle( conf->getTaskObsSectorAngle() );
        setActiveTaskPointFigureScheme( conf->getActiveTaskObsScheme() );
        break;
    }

  m_lastPassageState = Outside;
  m_lastDistance = -1;

  setAutoZoom( conf->getTaskPointAutoZoom() );

  // Reset user edited flag
  setUserEditFlag( false );
}

QPixmap& TaskPoint::getIcon( const int iconSize )
{
  static QPixmap emptyPm;

  switch( m_taskActiveTaskPointFigureScheme )
  {
    case GeneralConfig::Circle:
      return createCircleIcon( iconSize );

    case GeneralConfig::Sector:
      return createSectorIcon( iconSize );

    case GeneralConfig::Line:
      return createLineIcon( iconSize );

    default:
      // In default case an empty pixmap is returned.
      emptyPm = QPixmap( iconSize, iconSize );
      emptyPm.fill( Qt::transparent );
      return emptyPm;
  }

  return emptyPm;
}

QPixmap& TaskPoint::createCircleIcon( const int iconSize )
{
  static QPixmap circleIcon;

  if( ! circleIcon.isNull() && circleIcon.height() == iconSize )
    {
      // It is already created
      return circleIcon;
    }

  circleIcon = QPixmap( iconSize, iconSize );
  circleIcon.fill( Qt::transparent );

  QPainter painter(&circleIcon);
  QPen pen(Qt::red);
  painter.setPen( pen );
  painter.setBrush( QBrush( Qt::red, Qt::SolidPattern ) );
  painter.drawEllipse( 2, 2, iconSize-4, iconSize-4 );
  return circleIcon;
}

/**
 * Create a sector icon.
 */
QPixmap& TaskPoint::createSectorIcon( const int iconSize )
{
  static QPixmap sectorIcon;

  if( ! sectorIcon.isNull() && sectorIcon.height() == iconSize )
    {
      // It is already created
      return sectorIcon;
    }

  QPixmap pm( iconSize*2, iconSize*2 );
  pm.fill( Qt::transparent );

  QPainter painter;
  painter.begin( &pm );
  QPen pen(Qt::red);
  painter.setPen( pen );
  painter.setBrush( QBrush( Qt::red, Qt::SolidPattern ) );
  painter.drawPie( 3, 3, (iconSize*2)-2*3, (iconSize*2)-2*3, -118*16, 56*16 );
  painter.end();

  // Does not work under Maemo4
  //  sectorIcon = pm.copy( (pm.width() / 4),  pm.height() / 2,
  //                        pm.width() / 2,  pm.height() / 2 );

  sectorIcon = QPixmap( iconSize, iconSize );
  sectorIcon.fill( Qt::transparent );

  painter.begin( &sectorIcon );
  painter.drawPixmap( 0, 0,
                      pm,
                      pm.width() / 4,  pm.height() / 2,
                      pm.width() / 2,  pm.height() / 2 );
  painter.end();

  return sectorIcon;
}

QPixmap& TaskPoint::createLineIcon( const int iconSize )
{
  static QPixmap lineIcon;

  if( ! lineIcon.isNull() && lineIcon.height() == iconSize )
    {
      // It is already created
      return lineIcon;
    }

  lineIcon = QPixmap( iconSize, iconSize );
  lineIcon.fill( Qt::transparent );

  QPainter painter(&lineIcon);
  QPen pen(Qt::red);
  pen.setWidth(8);
  painter.setPen( pen );
  painter.drawLine( 0, iconSize/2, iconSize, iconSize/2 );
  return lineIcon;
}
