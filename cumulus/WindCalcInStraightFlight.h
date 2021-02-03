/***********************************************************************
 **
 **   WindCalcInStraightFlight.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2021 by Axel Pauli
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

signals:

  /**
   * Send if a new wind measurement has been made. The result is included in wind,
   * the quality of the measurement (1-5; 1 is bad, 5 is excellent) in quality.
   */
  void newMeasurement( const Vector& wind, int quality );

public slots:

  /**
   * Called, if a new true compass heading is available.
   *
   * @param heading in degrees
   */
  void slot_trueCompassHeading( const double& heading );

private:

  void _calcWind();

  uint   nunberOfSamples;    // current number of samples
  QTime measurementDuration; // time measurement in seconds
  double deltaSpeed;         // accepted speed deviation in km/h
  double deltaHeading;       // accepted heading deviation in degrees
  double tas;                // TAS in km/h
  double groundSpeed;        // GS in km/h
  double trueCourse;         // Compass true heading
  double trueHeading;        // GPS heading
  double sumTas;             // TAS in km/h
  double sumGroundSpeed;     // sum of GS in km/h
  double sumTHDeviation;     // sum of Compass true heading deviation
  double sumTCDeviation;     // sum of GPS heading (true course) deviation
  double vMin;               // minimal measured speed
  double vMax;               // maximal measured speed
  double hMin;               // lower limit of heading observation window
  double hMax;               // upper limit of heading observation window
};

