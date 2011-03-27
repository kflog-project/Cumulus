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
 **                         2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef __Waypoint__
#define __Waypoint__

#include <QPoint>
#include <QString>

#include "wgspoint.h"

/**
 * \class TaskPointTypes
 *
 * \author Heiner Lamprecht, Florian Ehinger, André Somers, Axel Pauli
 *
 * \brief Kinds of a task point.
 *
 * Definitions of possible task point types. Must be done here to avoid
 * recursive include loop.
 *
 * \date 1999-2011
 *
 * \version $Id$
 */
class TaskPointTypes
{
  public:
  /**
   * The possible task point types.
   */
  enum TaskPointType { NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                       End = 8, FreeP = 16, Landing = 32 };
};

/**
 * \class Waypoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, André Somers, Axel Pauli
 *
 * \brief This class contains all data items of a waypoint.
 *
 * \date 1999-2011
 *
 * \version $Id$
 */

class Waypoint
{
 public:
  /**
   * contains an priority indication for a waypoint
   */
  enum Priority { Low=0, Normal=1, High=2, Top=3 };

  Waypoint();
  Waypoint(const Waypoint& inst);
  ~Waypoint();

  /** Compare current instance with another */
  bool equals( const Waypoint *second ) const;
  bool operator==( const Waypoint& second ) const;

  /** The short name of the waypoint limited to 8 characters and upper cases. */
  QString name;
  /** The type of the waypoint */
  short type;
  /** The original lat/lon position (WGS84) of the waypoint in KFLOG's internal format. */
  WGSPoint origP;
  /** The projected map position of the waypoint. */
  QPoint projP;
  /** long name or description of waypoint */
  QString description;
  /** ICAO name */
  QString icao;
  /** comment concerning point*/
  QString comment;
  /** internal surface id */
  short surface;
  /** Heading of runway. Range 0-36 inclusive. 0 stands for an undefined runway. */
  short runway;
  /** length of runway, in meters */
  float length;
  /** elevation of runway, in meters */
  float elevation;
  /** frequency of contact for waypoint, in MHz */
  float frequency;
  /** flag for landable*/
  bool isLandable;
  /** contains an priority indication for the waypoint
   * 0=low
   * 1=normal
   * 2=high  */
  enum Priority priority;
  /** The index of the waypoint in the flight task list. A valid index is a
   *  positive number and is set, when the waypoint is added to a flight task
   *  list. The index is used in the automatic task point switch handling in the
   *  Calculator class.
   */
  /** Index of waypoint in flight task list */
  short taskPointIndex;
  /** The type of the task point, if waypoint is used as a task point*/
  enum TaskPointTypes::TaskPointType taskPointType;
  /** Country as two letter code, where the waypoint is to find. */
  QString country;
};

#endif
