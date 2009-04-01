/***********************************************************************
 **
 **   waypoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
 **                         2002 adjusted by André Somers for Cumulus
 **                         2008 ported to X11 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef __wayPoint__
#define __wayPoint__

#include <QPoint>
#include <QString>

#include "wgspoint.h"

/**
 * This class contains the data of a waypoint.
 */

class wayPoint
{

 public:
  /**
   * The task point types.
   */
  enum TaskPointType {NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                      End = 8, FreeP = 16, Landing = 32};

  /**
   * contains an importance indication for a waypoint
   */
  enum Importance { Low=0, Normal=1, High=2 };

  wayPoint();
  wayPoint(const wayPoint& inst);
  ~wayPoint();
  /** */
  QString getTaskPointTypeString() const;
  /** Compare current instance with another */
  bool equals( const wayPoint *second ) const;
  bool operator==( const wayPoint& second ) const;

  /** The name of the waypoint. */
  QString name;
  /** The original lat/lon position (WGS84) of the waypoint. */
  WGSPoint origP;
  /** The projected map position of the waypoint. */
  QPoint projP;
  /** The time, sector 1 has been reached. */
  unsigned int sector1;
  /** The time, sector 2 has been reached. */
  unsigned int sector2;
  /** The time, the fai-sector has been reached. */
  unsigned int sectorFAI;
  /** The angle of the sector in radian */
  double angle;
  /** The minimum angle of the sector in radian */
  double minAngle;
  /** The maximum angle of the sector in radian */
  double maxAngle;
  /** The type of the waypoint */
  short type;
  /** The task point type of the waypoint */
  enum  TaskPointType taskPointType;
  /** The waypoint index in the task list */
  short taskPointIndex;
  /** The bearing from the previous waypoint in radian */
  double bearing;
  /** The distance to the previous waypoint in km */
  double distance;
  /** The time distance to the previous waypoint in seconds */
  int distTime;

  /** Improvements for planning */
  /** long name or description (internal only) */
  QString description;
  /** ICAO name */
  QString icao;
  /** */
  QString comment;
  /** internal surface id */
  short surface;
  /** direction of runway. Range 0-36 inclusive */
  short runway;
  /** length of runway, in meters */
  short length;
  /** elevation of runway, in meters */
  int elevation;
  /** frequency of contact for waypoint, in MHz */
  double frequency;
  /** flag for landable*/
  bool isLandable;
  /** contains an importance indication for the waypoint
   * 0=low
   * 1=normal
   * 2=high  */
  enum Importance importance;
};

#endif
