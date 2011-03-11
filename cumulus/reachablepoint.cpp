/***********************************************************************
 **
 **   reachablepoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by Eckhard Völlm
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "reachablepoint.h"
#include "reachablelist.h"

// Construction from airfield database
ReachablePoint::ReachablePoint(QString name,
                               QString icao,
                               QString description,
                               QString country,
                               bool orignAfl,
                               short type,
                               float frequency,
                               WGSPoint pos,
                               QPoint   ppos,
                               float elevation,
                               QString comment,
                               Distance distance,
                               short bearing,
                               Altitude arrivAlt,
                               short rwDir,
                               float rwLen,
                               short rwSurf,
                               bool rwOpen )
{
  _wp.name = name;
  _wp.icao = icao;
  _wp.description = description;
  _wp.country = country;
  _wp.frequency = frequency;
  _wp.elevation = elevation;
  _wp.comment = comment;
  _wp.importance = Waypoint::High; // high to make sure it is visible
  _wp.isLandable = rwOpen;
  _wp.surface = rwSurf;
  _wp.runway = rwDir;
  _wp.length = rwLen;
  _wp.origP = pos;
  _wp.projP = ppos;
  _wp.type = type;

  _orignAfl   = orignAfl;
  _distance   = distance;
  _arrivalAlt = arrivAlt;
  _bearing    = bearing;
};

// Construction from another WP
ReachablePoint::ReachablePoint(Waypoint& wp,
                               bool orignAfl,
                               Distance& distance,
                               short bearing,
                               Altitude& arrivAlt )
{
  _wp = wp;
  _orignAfl   = orignAfl;
  _distance   = distance;
  _arrivalAlt = arrivAlt;
  _bearing    = bearing;
};

ReachablePoint::~ReachablePoint()
{
}

ReachablePoint::reachable ReachablePoint::getReachable()
{
  if ( _arrivalAlt.isValid() && _arrivalAlt.getMeters() > 0 )
    {
      return ReachablePoint::yes;
    }
  else if ( _arrivalAlt.isValid() && _arrivalAlt.getMeters() > -ReachableList::getSafetyAltititude() )
    {
      return ReachablePoint::belowSafety;
    }
  else
    {
      return ReachablePoint::no;
    }
}

bool ReachablePoint::operator < (const ReachablePoint& other) const
{
  if ( ReachableList::getModeAltitude() )
    {
      return (_arrivalAlt.getMeters() < other._arrivalAlt.getMeters());
    }
  else
    {
      return (_distance.getKilometers() > other._distance.getKilometers());
    }
}
