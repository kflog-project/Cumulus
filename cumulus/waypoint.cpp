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

wayPoint::wayPoint()
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
  importance     = wayPoint::Low;
  taskPointIndex = -1;
  taskPointType  = TaskPointTypes::NotSet;

  origP.setPos(0,0);
  projP.setX(0);
  projP.setY(0);
}

// Copy constructor
wayPoint::wayPoint(const wayPoint& inst)
{
  //   qDebug("wayPoint::wayPoint(const wayPoint& inst) name=%s, idx=%d",
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

wayPoint::~wayPoint()
{
  // qDebug("wayPoint::~wayPoint(): name=%s, %X", name.toLatin1().data(), (uint) this);
}

bool wayPoint::equals( const wayPoint *second ) const
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

bool wayPoint::operator==( const wayPoint& second ) const
{
  if( name == second.name && description == second.description && origP
      == second.origP )
    {
      return true;
    }

  return false;
}

