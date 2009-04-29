/***********************************************************************
 **
 **   reachablepoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by Eckhard Völlm,
 **                   2008-2009 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef REACHABLE_POINT_H
#define REACHABLE_POINT_H

#include <QString>
#include <QPoint>

#include "distance.h"
#include "altitude.h"
#include "wgspoint.h"
#include "waypoint.h"

/** Class for one entry in the ReachableList class. It covers all belonging
 * to a reachable point element.
 */

class ReachablePoint
{
 public:

   enum reachable{no, belowSafety, yes};

  ReachablePoint(QString name,
                 QString icao,
                 QString description,
                 bool orignAfl,
                 short type,
                 double frequency,
                 WGSPoint pos,
                 QPoint ppos,
                 unsigned int elevation,
                 QString comment,
                 Distance distance,
                 short bearing,
                 Altitude arrivAlt,
                 short rwDir,
                 short rwLen,
                 short rwSurf,
                 bool rwOpen );


  ReachablePoint(wayPoint& wp,
                 bool orignAfl,
                 Distance& distance,
                 short bearing,
                 Altitude& arrivAlt );

  ~ReachablePoint();

  Distance getDistance() const
  {
    return _distance;
  };

  void setDistance( const Distance& dist )
  {
    _distance = dist;
  };

  QString getName() const
  {
    return _wp.name;
  };

  QString getDescription() const
  {
    return _wp.description;
  };

  QString getComment() const
  {
    return _wp.comment;
  };

  void setOrignAfl(const bool orign)
  {
    _orignAfl = orign;
  };

  bool isOrignAfl() const
  {
    return _orignAfl;
  };

  int getElevation() const
  {
    return _wp.elevation;
  };

  short getType() const
  {
    return _wp.type;
  };

  Altitude getArrivalAlt() const
  {
    return _arrivalAlt;
  };

  short getBearing() const
  {
    return _bearing;
  };

  void setBearing(const short b)
  {
    _bearing = b;
  };

  WGSPoint& getWgsPos()
  {
    return _wp.origP;
  };

  const wayPoint *getWaypoint() const
  {
    return &_wp;
  };

  void setArrivalAlt( const Altitude& alt )
  {
    _arrivalAlt = alt;
  };

  reachable getReachable();

  /**
   * compares two entries to sort list either by distance or arrival altitude
   */
  bool operator < (const ReachablePoint& other) const;

 private:
  bool         _orignAfl; // Origin is taken from airfield list
  wayPoint     _wp;
  Distance     _distance;
  short        _bearing;
  Altitude     _arrivalAlt;
};

#endif /* REACHABLE_POINT_H */
