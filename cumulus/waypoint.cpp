/***********************************************************************
**
**   waypoint.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 Axel Pauli axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypoint.h"

wayPoint::wayPoint()
{
  name = "";
  description = "";
  comment = "";
  icao = "";
  sector1 = 0;
  sector2 = 0;
  sectorFAI = 0;
  bearing = -1;
  angle = 0.;
  minAngle = 0.;
  maxAngle = 0.;
  type = 0;
  distance = 0.;
  distTime = 0;
  surface = 0;
  runway = 0;
  length = 0;
  elevation = 0;
  frequency = 0.;
  isLandable = false;
  importance = wayPoint::Low;
  origP.setPos(0,0);
  projP.setX(0);
  projP.setY(0);
  taskPointType = NotSet;
  taskPointIndex = -1;
}

// Copy constructor
wayPoint::wayPoint(const wayPoint& inst)
{
  //   qDebug("wayPoint::wayPoint(const wayPoint& inst) name=%s, idx=%d",
  //          inst.name.latin1(), inst.taskPointIndex );

  name = inst.name;
  origP = inst.origP;
  projP = inst.projP;
  sector1 = inst.sector1;
  sector2 = inst.sector2;
  sectorFAI = inst.sectorFAI;
  bearing = inst.bearing;
  angle = inst.angle;
  minAngle = inst.minAngle;
  maxAngle = inst.maxAngle;
  type = inst.type;
  taskPointType = inst.taskPointType;
  taskPointIndex = inst.taskPointIndex;
  distance = inst.distance;
  distTime = inst.distTime;
  description = inst.description;
  icao = inst.icao;
  comment = inst.comment;
  surface = inst.surface;
  runway = inst.runway;
  length = inst.length;
  elevation = inst.elevation;
  frequency = inst.frequency;
  isLandable = inst.isLandable;
  importance = inst.importance;
}

wayPoint::~wayPoint()
{
  // qDebug("wayPoint::~wayPoint(): name=%s, %X", name.latin1(), this);
}

QString wayPoint::getTaskPointTypeString() const
{
  switch(taskPointType) {
  case wayPoint::TakeOff:
    return QObject::tr("Takeoff");
  case wayPoint::Begin:
    return QObject::tr("Begin");
  case wayPoint::RouteP:
    return QObject::tr("Route");
  case wayPoint::End:
    return QObject::tr("End");
  case wayPoint::FreeP:
    return QObject::tr("Free");
  case wayPoint::Landing:
    return QObject::tr("Landing");
  case wayPoint::NotSet:
    return QObject::tr("not set");
  }

  return QObject::tr("not set");
}

bool wayPoint::equals( const wayPoint *second ) const
{
  if(second == 0) {
    return false;
  }

  if(this->name == second->name &&
     this->description == second->description &&
     this->origP == second->origP) {
    return true;
  }

  return false;
}


bool wayPoint::operator==( const wayPoint& second ) const
{
  if(name == second.name &&
     description == second.description &&
     origP == second.origP) {
    return true;
  }

  return false;
}

