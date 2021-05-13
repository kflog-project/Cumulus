/***********************************************************************
 **
 **   WindCalcInStraightFlight.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class WindCalcInStraightFlight
 *
 * \author Axel Pauli
 *
 * \brief Wind calculation in straight flight by using true compass heading.
 *
 * Wind calculator in straight flight by using true compass heading, TAS,
 * GPS heading and ground speed.
 *
 * \date 2021
 */

#pragma once

#include <QObject>
#include <QTime>

#include "vector.h"

class WindCalcInStraightFlight : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY( WindCalcInStraightFlight )

public:

  WindCalcInStraightFlight( QObject* parent) ;

  virtual ~WindCalcInStraightFlight();

  /**
   * Starts a new measurement.
   */
  void start();

  /**
   * Normalize angels into range 0...359 degrees.
   * @param angle to be normalized
   * @return normalized angle
   */
  static double normAngle( double angle )
  {
    while( angle < 0.0 )
      angle += 360.0;
    while( angle >= 360.0 )
      angle -= 360.0;
    return angle;
  }

signals:

  /**
   * Send if a new wind measurement has been made. The result is included in wind,
   * the quality of the measurement (1-5; 1 is bad, 5 is excellent) in quality.
   */
  void newMeasurement( const Vector& wind, float quality );

public slots:

  /**
   * Called, if a new true compass heading is available.
   *
   * @param heading in degrees
   */
  void slot_trueCompassHeading( const double& heading );

private:

  /**
   * Calculate smaller bisector value from angles.
   *
   * @param angle as degree 0...359
   * @param average as degree 0...359
   * @return average angle as degree 0...359
   */
  double meanAngle( double angle, double average );

  uint   nunberOfSamples;    // current number of samples
  int    deliverWind;        // time in seconds for next wind delivery
  QTime measurementStart;    // time measurement in seconds
  double minimumAirSpeed;    // minimum air speed to start calculation
  double deltaSpeed;         // accepted speed deviation in km/h
  double deltaHeading;       // accepted heading deviation in degrees
  double tas;                // TAS in km/h
  double groundSpeed;        // GS in km/h
  double trueCourse;         // Compass true heading
  double trueHeading;        // GPS heading
  double sumTas;             // TAS in km/h
  double sumGroundSpeed;     // sum of GS in km/h
  double meanTH;             // mean of Compass true heading
  double meanTC;             // mean of GPS heading (true course)
  double tcMin;              // lower limit of true course observation window
  double tcMax;              // upper limit of true course observation window
  double thMin;              // lower limit of true heading observation window
  double thMax;              // upper limit of true heading observation window
};
