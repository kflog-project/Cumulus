/***********************************************************************
 **
 **   taskpoint.h
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
 * \class TaskPoint
 *
 * \author Axel Pauli
 *
 * \brief Contains all data attributes of a task point.
 *
 * This class is an extension of the waypoint class. It handles all data
 * items concerning a flight task.
 *
 * \date 2010
 */

#ifndef TASK_POINT_H_
#define TASK_POINT_H_

#include "waypoint.h"

class TaskPoint : public Waypoint
{
 public:

  TaskPoint();
  TaskPoint( const Waypoint& wp );
  TaskPoint( const TaskPoint& inst );
  virtual ~TaskPoint();

  /**
   * \return The type of a task point in a string format.
   */
  QString getTaskPointTypeString() const;

  /** The angle of the sector in radian */
  double angle;
  /** The minimum angle of the sector in radian */
  double minAngle;
  /** The maximum angle of the sector in radian */
  double maxAngle;
  /** The bearing from the previous task point in radian */
  double bearing;
  /** The distance to the previous task point in km */
  double distance;
  /** The time distance to the previous task point in seconds */
  int distTime;
  /** wind correction angle from the previous task point in degree. */
  double wca;
  /** The true heading ( wind was considered) from the previous task point in degree. */
  double trueHeading;
  /**
   * The ground speed ( wind was considered) from the previous task point
   * to this point.
   */
  double groundSpeed;
  /** Result of wind triangle calculation. */
  bool wtResult;
};

#endif /* TASKPOINT_H_ */
