/***********************************************************************
**
**   waypoint.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004-2013 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypoint.h"

Waypoint::Waypoint()
{
  name           = "";
  type           = 0;
  description    = "";
  comment        = "";
  country        = "";
  icao           = "";
  elevation      = 0;
  frequency      = 0.;
  priority       = Waypoint::Low;
  taskPointIndex = -1;
  wpListMember   = false;

  wgsPoint.setPos(0,0);
  projPoint.setX(0);
  projPoint.setY(0);
}

// Copy constructor
Waypoint::Waypoint(const Waypoint& inst)
{
  //   qDebug("Waypoint::Waypoint(const Waypoint& inst) name=%s, idx=%d",
  //          inst.name.toLatin1().data(), inst.taskPointIndex );
  name           = inst.name;
  type           = inst.type;
  wgsPoint       = inst.wgsPoint;
  projPoint      = inst.projPoint;
  description    = inst.description;
  icao           = inst.icao;
  comment        = inst.comment;
  country        = inst.country;
  elevation      = inst.elevation;
  frequency      = inst.frequency;
  priority       = inst.priority;
  taskPointIndex = inst.taskPointIndex;
  wpListMember   = inst.wpListMember;
  rwyList        = inst.rwyList;
}

Waypoint::~Waypoint()
{
  // qDebug("Waypoint::~Waypoint(): name=%s, %X", name.toLatin1().data(), (uint) this);
}

bool Waypoint::equals( const Waypoint *second ) const
{
  if( second == static_cast<Waypoint *>(0) )
    {
      return false;
    }

  if( this->name == second->name &&
      this->type == second->type &&
      this->description == second->description &&
      this->wgsPoint == second->wgsPoint &&
      this->taskPointIndex == second->taskPointIndex )
    {
      return true;
    }

  return false;
}

bool Waypoint::operator==( const Waypoint& second ) const
{
  if( name == second.name &&
      type == second.type &&
      description == second.description &&
      wgsPoint == second.wgsPoint &&
      taskPointIndex == second.taskPointIndex )
    {
      return true;
    }

  return false;
}
