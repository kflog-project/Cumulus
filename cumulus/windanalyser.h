/***********************************************************************
 **
 **   windanalyser.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by André Somers, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef WINDANALYSER_H
#define WINDANALYSER_H

#include <QObject>

#include "vector.h"
#include "cucalc.h"

/**
 * The windanalyser analyses the list of flightsamples looking
 * for windspeed and direction.
 * @author André Somers
 */
class WindAnalyser : public QObject
{
  Q_OBJECT

    public:
  WindAnalyser(QObject * parent);

  ~WindAnalyser();

 signals: // Signals
  /**
   * Send if a new windmeasurement has been made. The result is included in wind,
   * the quality of the measurement (1-5; 1 is bad, 5 is excellent) in quality.
   */
  void newMeasurement(Vector wind, int quality);

  public slots: // Public slots
  /**
   * Called if the flightmode changes
   */
  void slot_newFlightMode(CuCalc::flightmode, int);

  /**
   * Called if a new sample is available in the samplelist.
   */
  void slot_newSample();

  /**
   * Called if a new satelite constellation has been detected.
   */
  void slot_newConstellation();

 private: // Private attributes
  int circleCount; //we are counting the number of circles, the first onces are probably not very round
  bool circleLeft; //true=left, false=right
  bool active;     //active is set to true or false by the slot_newFlightMode slot
  int startmarker;
  int startheading;
  int circleDeg;
  int lastHeading;
  bool pastHalfway;
  Vector minVector;
  Vector maxVector;
  int satCnt;
  int minSatCnt;
  bool curModeOK;

 private: // Private memberfunctions
  void _calcWind();
};

#endif
