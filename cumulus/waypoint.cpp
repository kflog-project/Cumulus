/***********************************************************************
**
**   waypoint.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004-2010 by Axel Pauli (axel@kflog.org)
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
  icao           = "";
  surface        = 0;
  runway         = 0;
  length         = 0;
  elevation      = 0;
  frequency      = 0.;
  isLandable     = false;
  importance     = Waypoint::Low;
  taskPointIndex = -1;
  taskPointType  = TaskPointTypes::NotSet;

  origP.setPos(0,0);
  projP.setX(0);
  projP.setY(0);
}

// Copy constructor
Waypoint::Waypoint(const Waypoint& inst)
{
  //   qDebug("Waypoint::Waypoint(const Waypoint& inst) name=%s, idx=%d",
  //          inst.name.toLatin1().data(), inst.taskPointIndex );
  name           = inst.name;
  type           = inst.type;
  origP          = inst.origP;
  projP          = inst.projP;
  description    = inst.description;
  icao           = inst.icao;
  comment        = inst.comment;
  surface        = inst.surface;
  runway         = inst.runway;
  length         = inst.length;
  elevation      = inst.elevation;
  frequency      = inst.frequency;
  isLandable     = inst.isLandable;
  importance     = inst.importance;
  taskPointIndex = inst.taskPointIndex;
  taskPointType  = inst.taskPointType;
}

Waypoint::~Waypoint()
{
  // qDebug("Waypoint::~Waypoint(): name=%s, %X", name.toLatin1().data(), (uint) this);
}

bool Waypoint::equals( const Waypoint *second ) const
{
  if( second == 0 )
    {
      return false;
    }

  if( this->name == second->name && this->description == second->description
      && this->origP == second->origP )
    {
      return true;
    }

  return false;
}

bool Waypoint::operator==( const Waypoint& second ) const
{
  if( name == second.name && description == second.description && origP
      == second.origP )
    {
      return true;
    }

  return false;
}

