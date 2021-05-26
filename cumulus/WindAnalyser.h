/***********************************************************************
 **
 **   WindAnalyser.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002       by André Somers,
 **                   20082-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class WindAnalyser
 *
 * \author André Somers, Axel Pauli
 *
 * \brief wind analyzer
 *
 * The wind analyzer processes the list of flight samples looking
 * for wind speed and direction.
 *
 * \date 2002-2021
 */

#pragma once

#include <QObject>

#include "calculator.h"
#include "gpsnmea.h"
#include "vector.h"

class WindAnalyser : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY( WindAnalyser )

public:

  WindAnalyser(QObject* parent);

  virtual ~WindAnalyser();

signals:

  /**
   * Send if a new wind measurement has been made. The result is included in wind,
   * the quality of the measurement (1-5; 1 is bad, 5 is excellent) in quality.
   */
  void newMeasurement( Vector& wind,
                       const Altitude&,
                       float quality,
                       int numberOfmeasurment );

public slots:
  /**
   * Called if the flight mode changes
   */
  void slot_newFlightMode( Calculator::FlightMode newMode );

  /**
   * Called if a new sample is available in the sample list.
   */
  void slot_newSample();

  /**
   * Called if a new satellite constellation has been detected.
   */
  void slot_newConstellation( SatInfo& newConstellation );

  /**
   * Called, if the GPS status has changed.
   */
  void slot_gpsStatusChange( GpsNmea::GpsStatus newStatus );

private:

  /**
   * After a full circle the wind is calculated by this method.
   */
  void calcWind();

  /**
   * Reset all wind calculation variables to their defaults.
   *
   * @param clean if true, the circleCount is also set to zero.
   */
  void restartCycle( const bool clean );

  int circleCount; // we are counting the number of circles, the first one are probably not very round
  int circleDegrees; // Degrees of current flown circle
  int circleSectors; // Sectors of current flown circle
  int lastHeading; // Last processed heading
  int satCnt; // last reported sat count
  int minSatCnt; // configured minimal sat count
  Calculator::FlightMode flightMode;
  GpsNmea::GpsStatus gpsStatus;
  Vector minVector;
  Vector maxVector;
};
