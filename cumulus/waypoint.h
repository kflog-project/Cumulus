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
 **                         2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef __Waypoint__
#define __Waypoint__

#include <QList>
#include <QPoint>
#include <QString>

#include "runway.h"
#include "wgspoint.h"

/**
 * \class Waypoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, André Somers, Axel Pauli
 *
 * \brief This class contains all data items of a waypoint.
 *
 * \date 1999-2013
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

  virtual ~Waypoint();

  /** Compare current instance with another */
  bool equals( const Waypoint *second ) const;
  bool operator==( const Waypoint& second ) const;

  /** The short name of the waypoint limited to 8 characters and upper cases. */
  QString name;
  /** The type of the waypoint */
  short type;
  /** The original lat/lon position (WGS84) of the waypoint in KFLOG's internal format. */
  WGSPoint wgsPoint;
  /** The projected map position of the waypoint. */
  QPoint projPoint;
  /** long name or description of waypoint */
  QString description;
  /** ICAO name */
  QString icao;
  /** comment concerning point*/
  QString comment;

  /**
   * A list of runways is managed by the airfield object. If a waypoint is
   * derived temporary from an airfield object, these data is also taken over.
   */
  QList<Runway> rwyList;

  /** elevation of waypoint in meters */
  float elevation;

  /** frequency of contact for waypoint in MHz */
  float frequency;

  /** contains an priority indication for the waypoint
   * 0=low
   * 1=normal
   * 2=high
   * 3=top
   */
  enum Priority priority;

  /**
   * The index of the taskpoint in the flight task list, if the waypoint is
   * filled with taskpoint data only. A valid index is a positive number and
   * is set, when the taskpoint is added to a flight task list. The index is
   * used in the automatic task point switch handling in the Calculator class.
   */
  short taskPointIndex;

  /** Country as two letter code, where the waypoint is to find. */
  QString country;

  /** Flag to indicate that the waypoint is a member of the waypoint list. */
  bool wpListMember;
};

#endif
