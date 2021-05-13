/***********************************************************************
**
**   mapcalc.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2008-2021  by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef MAP_CALC_H
#define MAP_CALC_H

#include <cmath>
#include <QPair>
#include <QRect>

#include "speed.h"
#include "waypoint.h"

#define PI2 M_PI*2

/**
 * The earth's radius used for calculation, given in Meters
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6371000 // FAI Radius, this was the previous radius ->6370290

// WGS84 averrage radius
// #define RADIUS ((6378137 + 6356752) / 2)

// Our nautical mile definition
#define RADIUS_kfl (RADIUS / (360.0 * 600000.0))

// Define nautical mile in meters according to earth radius of KFL
#define MILE_kfl (PI2 * RADIUS / (360.0 * 60.0))

namespace MapCalc
{
  /**
   * Calculates the distance between two given points according to great circle in km.
   */
  double dist( double lat1, double lon1, double lat2, double lon2 );

  double distP( double lat1, double lon1, double lat2, double lon2 );

  double distC1( double lat1, double lon1, double lat2, double lon2 );

  double distC1( QPoint *p1, QPoint *p2 );

  /**
   * Vincentys-formula for DMST distance calculation taken over from:
   *
   * https://github.com/dariusarnold/vincentys-formula
   *
   * http://www.movable-type.co.uk/scripts/latlong-vincenty.html#direct
   *
   * @param lat1 from point
   * @param lon1 from point
   * @param lat2 to point
   * @param lon2 to point
   *
   * @return distance in Kilometers and bearing from/to in radiant.
   */
  QPair<double, double> distVinc(double latp, double longp,
                                 double latc, double longc);

  /**
   * Wrapper function for Vincentys-formula for DMST distance calculation.
   *
   * @param p1 from point in kflog format
   * @param p2 to point in kflog format
   *
   * @return distance in Kilometers and bearing from/to in radiant.
   */
  QPair<double, double> distVinc( QPoint *p1, QPoint *p2 );

  /**
   * Calculates the distance between two given points in km.
   */
  double dist(QPoint* p1, QPoint* p2);

  /**
   * Calculates the distance between two given points in km.
   */
  double dist(Waypoint* wp1, Waypoint* wp2);

  /**
   * Calculates the bearing to the next point
   */
  double getBearing(QPoint p1, QPoint p2);

  /**
   * Calculates the bearing to the next point with coordinates mapped to
   * the current projection
   */
  double getBearing2(QPoint p1, QPoint p2);

  /**
   * Calculates the bearing to the next point with wgs84 coordinates
   */
  double getBearingWgs(QPoint p1, QPoint p2);

  /**
   * Converts a x/y position into a polar-coordinate.
   */
  double polar(double y, double x);

  /**
   * Calculate the position of the target point, described by start point, length
   * and direction angle with the help of the polar coordinate transformation.
   *
   * @param startPos The start point in KFLog coordinate format
   * @param double The distance to the the target point in meters
   * @param direction The course angle to the target point
   * @return The end point in KFLog coordinate format
   */
  QPoint getPosition( QPoint startPos, double distance, int direction );

  /**
   * Calculates the direction of the vector pointing to the outside
   * of the area spanned by the two vectors.
   */
  double outsideVector(QPoint center, QPoint p1, QPoint p2);

  double normalize(double angle);

  int normalize(int angle);

  /**
   * Calculate the smaller bisector value from angles.
   *
   * @param angle as degree 0...359
   * @param average as degree 0...359
   * @return average angle as degree 0...359
   */
  double bisectorOfAngles( double angle1, double angle2 );

  /**
    * Calculates the difference between two angles, returning the smallest
    * angle. It returns an angle between -180 and 180 in degrees. Positive
    * in clockwise direction.
    */
  int angleDiff(int ang1, int ang2);

  /**
    * Calculates the difference between two angles, returning the smallest
    * angle. It returns an angle between -Pi and Pi in rad.
    */
  double angleDiff(double ang1, double ang2);

  /**
   * Calculates a (crude) bounding box that contains the circle of radius @arg r
   * around point @arg center. @arg r is given in kilometers.
   */
  QRect areaBox(QPoint center, double r);

  /**
   * Calculates the bounding box of the given tile number in KFLog coordinates.
   * The returned rectangle used the x-axis as longitude and the y-axis as latitude.
   */
  QRect getTileBox(const ushort tileNo);

  /**
   * Calculates the map tile number from the passed coordinate. The coordinate
   * format is decimal degree. Positive numbers are N and E, negative numbers
   * are W and S.
   *
   * @param lat Latitude in decimal degree. 90...-90
   * @param lon Longitude in decimal degree. -180...180
   * @return map tile number 0...16199
   */
  int mapTileNumber( double lat, double lon );

  /**
   * Calculates ground speed, wca and true heading via the wind triangle.
   * See http://www.delphiforfun.org/programs/math_topics/WindTriangle.htm
   * for more info. Thanks to the publisher.
   *
   * @param trueCourse TC in degree 0...359
   * @param trueAirSpeed TAS, unit must be the same as for wind speed
   * @param windDirection wind from direction in degree 0...359
   * @param windSpeed wind speed, unit must be the same as for true air speed
   * @param groundSpeed calculated ground speed, unit is the same as used for TAS and wind
   * @param wca calculated wind correction angle in degree 0...359
   * @param trueHeading calculated TH in degree 0...359
   * @return true if results could calculated otherwise false, when the wind is too strong
   */
  bool windTriangle( const double trueCourse,
		     const double trueAirSpeed,
		     const double windDirection,
		     const double windSpeed,
		     double &groundSpeed,
		     double &wca,
		     double &trueHeading );

  /**
   * Calculates the estimated true air speed and the true heading. Ground speed
   * and wind speed must used the same units! The ETAS is calculated in these
   * units.
   *
   * \param [in] tk Track, course over ground in degrees
   * \param [in] gs Ground speed
   * \param [in] wd Wind direction in degrees
   * \param [in] ws Wind speed
   * \param [out] etas Estimated true air speed.
   * \param [out] eth Estimated true heading
   */
  bool calcETAS( const int tk,
		 const Speed& gs,
		 const int wd,
		 const Speed& ws,
		 Speed& etas,
		 int& eth );
};

#endif
