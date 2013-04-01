/***********************************************************************
 **
 **   taskpoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2010-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QObject>
#include <QtCore>

#include "generalconfig.h"
#include "mapcalc.h"
#include "taskpoint.h"

// Near check distance state as meters
#define NEAR_DISTANCE 1000.0

TaskPoint::TaskPoint()
{
  bearing = -1;
  angle = 0.;
  minAngle = 0.;
  maxAngle = 0.;
  distance = 0.;
  distTime = 0;
  wca = 0.;
  trueHeading = -1;
  groundSpeed = 0.0;
  wtResult = false;

  GeneralConfig *conf = GeneralConfig::instance();

  m_taskActiveTaskPointFigureScheme = conf->getActiveTaskObsScheme();
  m_taskLine.setLineLength( conf->getTaskStartLineLength().getMeters() );
  m_taskCircleRadius = conf->getTaskObsCircleRadius();
  m_taskSectorInnerRadius = conf->getTaskObsSectorInnerRadius();
  m_taskSectorOuterRadius = conf->getTaskObsSectorOuterRadius();
  m_taskSectorAngle = conf->getTaskObsSectorAngle();
  m_autoZoom = conf->getTaskPointAutoZoom();
  m_userEdited = false;
}

/** Construct object from waypoint reference */
TaskPoint::TaskPoint( const Waypoint& wp ) : Waypoint( wp )
{
  bearing = -1;
  angle = 0.;
  minAngle = 0.;
  maxAngle = 0.;
  distance = 0.;
  distTime = 0;
  wca = 0;
  trueHeading = -1;
  groundSpeed = 0.0;
  wtResult = false;

  m_taskLine.setLineCenter( wp.origP );

  // The waypoint class contains the taskpoint type. We must initialize the
  // taskpoint according to its given type.
  setConfigurationDefaults();
}

/** Copy constructor */
TaskPoint::TaskPoint( const TaskPoint& inst ) : Waypoint( inst )
{
  bearing     = inst.bearing;
  angle       = inst.angle;
  minAngle    = inst.minAngle;
  maxAngle    = inst.maxAngle;
  distance    = inst.distance;
  distTime    = inst.distTime;
  wca         = inst.wca;
  trueHeading = inst.trueHeading;
  groundSpeed = inst.groundSpeed;
  wtResult    = inst.wtResult;

  m_taskActiveTaskPointFigureScheme = inst.m_taskActiveTaskPointFigureScheme;
  m_taskLine = inst.m_taskLine;
  m_taskCircleRadius = inst.m_taskCircleRadius;
  m_taskSectorInnerRadius = inst.m_taskSectorInnerRadius;
  m_taskSectorOuterRadius = inst.m_taskSectorOuterRadius;
  m_taskSectorAngle = inst.m_taskSectorAngle;
  m_autoZoom = inst.m_autoZoom;
  m_userEdited = inst.m_userEdited;
}

TaskPoint::~TaskPoint()
{
  // qDebug( "TaskPoint::~TaskPoint(): name=%s, %X", name.toLatin1().data(), (uint) this );
}

enum TaskPoint::PassageState TaskPoint::checkPassage( const Distance& dist2TP,
                                                      const QPoint& position )
{
  // qDebug() << "TaskPoint::checkPassage: TP-IDX=" << taskPointIndex;

  if( taskPointIndex == -1 )
    {
      // no active task point
      return Outside;
    }

  // get user defined scheme items
  const enum GeneralConfig::ActiveTaskFigureScheme scheme = getActiveTaskPointFigureScheme();

  // qDebug() << "TaskPoint::checkPassage: Scheme=" << scheme;

  if( scheme == GeneralConfig::Circle )
    {
      // Circle scheme is active
      double circleRadius = getTaskCircleRadius().getMeters();

      if( dist2TP.getMeters() < circleRadius )
        {
          // We are inside the circle
          return Passed;
        }

      if( dist2TP.getMeters() < circleRadius + NEAR_DISTANCE )
        {
          return Near;
        }

      return Outside;
    }

  if( scheme == GeneralConfig::Line )
    {
      if( dist2TP.getMeters() < getTaskLineLength().getMeters() )
        {
          // We are inside of our observation zone to decide, if a line crossing
          // has started.
          bool state = getTaskLine().checkCrossing( position );

          if( state == true )
            {
              return Passed;
            }
          else
            {
              return Near;
            }
        }
      else if( dist2TP.getMeters() <= NEAR_DISTANCE )
        {
          return Near;
        }

      return Outside;
    }

  if( scheme != GeneralConfig::Sector )
    {
      return Outside;
    }

  // Sector scheme is active, get sector radiuses
  const double innerRadius = getTaskSectorInnerRadius().getMeters();
  const double outerRadius = getTaskSectorOuterRadius().getMeters();

  if( dist2TP.getMeters() > outerRadius + NEAR_DISTANCE )
    {
      // we are outside of outer radius, sector angle has not to be
      // considered
      return Outside;
    }

  if( dist2TP.getMeters() > outerRadius )
    {
      // we are outside of outer radius, sector angle has not to be
      // considered
      return Near;
    }

  // the inner radius is considered as minimum. If we are inside the
  // inner radius, we are outside of the sector.
  if( ( innerRadius > 0.0 || innerRadius == outerRadius ) &&
      dist2TP.getMeters() < innerRadius )
    {
      // we are inside of inner radius, sector angle has not to be
      // considered
      return Near;
    }

  // Do special check for landing point
  if( taskPointType == TaskPointTypes::Landing )
    {
      // Here we do apply only the outer radius check.
      if( dist2TP.getMeters() < outerRadius )
        {
          // We are inside outer radius
          return Passed;
        }

      if( dist2TP.getMeters() < outerRadius + NEAR_DISTANCE )
        {
          // We are inside outer radius
          return Near;
        }
      else
        {
          return Outside;
        }
    }

  // Here we are inside of outer radius, therefore we have to check the sector angle.
  // Calculate bearing from TP to current position
  const double bearing = getBearingWgs( origP, position );

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
      return Passed;
    }

  if( bearing > minAngle && bearing < maxAngle )
    {
      // we are inside of sector between 0...360
      return Passed;
    }

  return Outside;
}

/** Returns the type of a task point in a string format. */
QString TaskPoint::getTaskPointTypeString() const
{
  switch( taskPointType )
    {
    case TaskPointTypes::TakeOff:
      return QObject::tr( "TO" );
    case TaskPointTypes::Begin:
      return QObject::tr( "B" );
    case TaskPointTypes::RouteP:
      return QObject::tr( "R" );
    case TaskPointTypes::End:
      return QObject::tr( "E" );
    case TaskPointTypes::FreeP:
      return QObject::tr( "F" );
    case TaskPointTypes::Landing:
      return QObject::tr( "LG" );
    case TaskPointTypes::NotSet:
    default:
      return QObject::tr( "NS" );
    }

  return QObject::tr( "NS" );
}

/** Returns the scheme of a task point in a string format. */
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

  switch( taskPointType )
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

/**
 * Create a circle icon.
 */
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

/**
 * Create a line icon.
 */
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
