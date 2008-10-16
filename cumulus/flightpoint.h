/***********************************************************************
 **
 **   flightpoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008 Axel Pauli  (axel@kflog.org)
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef __FlightPoint__
#define __FlightPoint__

#include <QPoint>

#include "wgspoint.h"

/**
 * This class contains the data of one flight point. This class
 * is currently not used by Cumulus.
 */

class FlightPoint
{
  public:
  /**
   * The original position of the point. Given in
   * the internal format.
   */
  WGSPoint origP;

  /**
   * The projected position of the point.
   */
  QPoint projP;

  /**
   * The barometrical height, registered by the logger.
   */
  int height;

  /**
   * The gps-height, registered by the logger.
   */
  int gpsHeight;

  /**
   * The time, the point was registered by the logger.
   */
  unsigned int time;

  /**
   * The elevation difference to the previous Point
   */
  int dH;

  /**
   * The time difference to the previous Point
   */
  int dT;

  /**
   * The distance between the Points
   */
  int dS;

  /**
   * The Bearing to the previous Point
   */
  double bearing;

  /**
   * Kreisflug 0 oder Streckenflug 1
   */
  unsigned short f_state;
};

#endif
