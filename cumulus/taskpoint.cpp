/***********************************************************************
 **
 **   taskpoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2010 by Axel Pauli (axel@kflog.org)
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * This class is an extension of the waypoint class. It handles all data
 * items concerning a flight task.
 */

#include <QObject>

#include "taskpoint.h"

TaskPoint::TaskPoint()
{
  sector1 = 0;
  sector2 = 0;
  sectorFAI = 0;
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
}

/** Construct object from waypoint reference */
TaskPoint::TaskPoint( const wayPoint& wp ) : wayPoint( wp )
{
  sector1 = 0;
  sector2 = 0;
  sectorFAI = 0;
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
}

/** Copy constructor */
TaskPoint::TaskPoint( const TaskPoint& inst ) : wayPoint( inst )
{
  sector1 = inst.sector1;
  sector2 = inst.sector2;
  sectorFAI = inst.sectorFAI;
  bearing = inst.bearing;
  angle = inst.angle;
  minAngle = inst.minAngle;
  maxAngle = inst.maxAngle;
  distance = inst.distance;
  distTime = inst.distTime;
  wca = inst.wca;
  trueHeading = inst.wca;
  groundSpeed = inst.groundSpeed;
  wtResult = inst.wtResult;
}

TaskPoint::~TaskPoint()
{
  // qDebug( "TaskPoint::~TaskPoint(): name=%s, %X", name.toLatin1().data(), (uint) this );
}

/** Returns the type of a task point in a string format. */
QString TaskPoint::getTaskPointTypeString() const
{
  switch( taskPointType )
    {
    case TaskPointTypes::TakeOff:
      return QObject::tr( "Takeoff" );
    case TaskPointTypes::Begin:
      return QObject::tr( "Begin" );
    case TaskPointTypes::RouteP:
      return QObject::tr( "Route" );
    case TaskPointTypes::End:
      return QObject::tr( "End" );
    case TaskPointTypes::FreeP:
      return QObject::tr( "Free" );
    case TaskPointTypes::Landing:
      return QObject::tr( "Landing" );
    case TaskPointTypes::NotSet:
      return QObject::tr( "not set" );
    }

  return QObject::tr( "not set" );
}

