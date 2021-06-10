/***********************************************************************
 **
 **   WindAnalyser.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Andre Somers
 **                   2009-2021 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <cmath>

#include <QtGlobal>

#include "generalconfig.h"
#include "mapcalc.h"
#include "WindAnalyser.h"

/*
  About Wind analysis

  Currently, the wind is analyzed by finding the minimum and the maximum
  ground speeds measured while flying a circle. The direction of the wind is taken
  to be the direction in which the speed reaches it's maximum value, the speed
  is half the difference between the maximum and the minimum speeds measured.
  A quality parameter, based on the number of circles already flown (the first
  circles are taken to be less accurate) and the angle between the headings at
  minimum and maximum speeds, is calculated in order to be able to weigh the
  resulting measurement. This method do not more work for wind speeds < 5Km/h.

  There are other options for determining the wind speed. You could for instance
  add all the vectors in a circle, and take the resulting vector as the wind speed.
  This is a more complex method, but because it is based on more heading/speed
  measurements by the GPS, it is probably more accurate. If equipped with
  instruments that pass along airspeed, the calculations can be compensated for
  changes in airspeed, resulting in better measurements. We are now assuming
  the pilot flies in perfect circles with constant airspeed, which is of course
  not a safe assumption.
  The quality indication we are calculation can also be approached differently,
  by calculating how constant the speed in the circle would be if corrected for
  the wind speed we just derived. The more constant, the better. This is again
  more CPU intensive, but may produce better results.

  Some of the errors made here will be averaged-out by the WindStore, which keeps
  a number of wind measurements and calculates a weighted average based on quality
  for an given altitude band.
*/

WindAnalyser::WindAnalyser(QObject* parent) :
  QObject(parent),
  circleCount(0),
  circleDegrees(0),
  circleSectors(0),
  lastHeading(-1),
  satCnt(5),
  minSatCnt(5),
  flightMode(Calculator::unknown),
  gpsStatus(GpsNmea::notConnected)
{
  // Initialization
  minSatCnt = GeneralConfig::instance()->getWindMinSatCount();

  // Note: Flarm devivers a new sat count only after a sat count change.
  satCnt = minSatCnt;
}

WindAnalyser::~WindAnalyser()
{}

/** Called if a new sample is available in the sample list. */
void WindAnalyser::slot_newSample()
{
  if( flightMode != Calculator::circlingL &&
      flightMode != Calculator::circlingR )
    {
      // Do not work if we are not in circling mode.
      return;
    }

  if( satCnt < minSatCnt )
    {
      // The satellite count is below minimum.
      return;
    }

  if( gpsStatus != GpsNmea::validFix )
    {
      // No valid GPS fix.
      return;
    }

  // load the last stored flight sample
  Vector curVec = calculator->samplelist[0].vector;

  // Save the last stored altitude
  lastAltitude = calculator->samplelist[0].altitude;

  if( lastHeading != -1 )
    {
      int diff = abs( curVec.getAngleDeg() - lastHeading );

      if( diff > 180 )
        {
          // Correct difference, if current angle has 360 degree passed.
          // In such a case it starts with smaller values again.
          diff = abs(diff - 360 );
        }

      circleDegrees += diff;
      circleSectors++;
    }

  lastHeading = curVec.getAngleDeg();

  if( curVec.getSpeed().getMps() < minVector.getSpeed().getMps() )
    {
      // New minimum speed detected
      minVector = curVec;
    }

  if( curVec.getSpeed().getMps() > maxVector.getSpeed().getMps() )
    {
      // New maximum speed detected
      maxVector = curVec;
    }

  /*
  qDebug( "curVec: %d/%.3f, minVec: %d/%.3f, maxVec: %d/%.3f",
          curVec.getAngleDeg(), curVec.getSpeed().getKph(),
          minVector.getAngleDeg(), minVector.getSpeed().getKph(),
          maxVector.getAngleDeg(), maxVector.getSpeed().getKph() );
  */

  if( circleDegrees > 361 )
    {
      // a bit more than one circle to ensure both ends are in
      // increase the number of circles flown (used to determine the last
      // heading uality
      circleCount++;

      // calculate the wind for this circle
      calcWind();
      restartCycle( false );
    }
}

void WindAnalyser::calcWind()
{
  int degreePerStep = circleDegrees / circleSectors;

  double aDiff = MapCalc::angleDiff( minVector.getAngleDeg(), maxVector.getAngleDeg() );

  /*
    Determine quality.

    Currently, we are using the question how well the min and the max vectors
    are on opposing sides of the circle to determine the quality. 140 degrees is
    the minimum separation, 180 is ideal.
    Furthermore, the first circle IS considered to be of lesser quality.
    5 is maximum quality, make sure we honor that.
  */
  float quality = 5 - ((180.0 - fabsf( aDiff )) / 8.0 );

  qDebug() << "calcWind(): circles=" << circleCount
           << "altitude(m)=" << lastAltitude.getMeters()
           << "degree/Step=" << degreePerStep
           << "angleDiff=" << aDiff
           << "quality=" << quality;

  if( quality < 1 )
    {
      return; // Measurement quality too low, aDiff > 32°
    }

  // take both directions for min and max vector into account
  int maxAngle = maxVector.getAngleDeg();
  int minAngle = minVector.getAngleDeg();
  double maxSpeed = maxVector.getSpeed().getKph();
  double minSpeed = minVector.getSpeed().getKph();

  // Invert maxVector angle
  maxVector.setAngle( MapCalc::normalize( maxVector.getAngleDeg() + 180 ) );

  // create a vector object for the resulting wind
  Vector result;

  // Calculate bisector between the two angles.
  double bisector = MapCalc::bisectorOfAngles( maxVector.getAngleDegDouble(),
                                               minVector.getAngleDegDouble() );

  // the direction of the wind is the direction where the greatest speed occurred
  result.setAngle( bisector );

  // The speed of the wind is half the difference between the minimum and the maximum speeds.
  result.setSpeed( (maxVector.getSpeed().getMps() - minVector.getSpeed().getMps()) / 2.0 );

  // Let the world know about our measurement!
  qDebug( "### Circle-Wind: %d/%.0fKm/h, maxAngle=%d/%.0f minAngle=%d/%.0f",
          result.getAngleDeg(),
          result.getSpeed().getKph(),
          maxAngle, maxSpeed,
          minAngle, minSpeed );

  emit newMeasurement( result,
                       calculator->samplelist[0].altitude,
                       quality,
                       circleCount );
}

/**
 * Reset all wind calculation variables to their defaults.
 *
 * @param clean if true, the circleCount is also set to zero.
 */
void WindAnalyser::restartCycle( const bool clean )
{
  if( clean == true )
    {
      circleCount = 0;
    }

  circleDegrees = 0;
  circleSectors = 0;
  lastHeading   = -1;

  // set start limits in m/s
  minVector.setSpeed( 100.0 );
  maxVector.setSpeed( 0.0 );
}

/** Called if the flight mode changes */
void WindAnalyser::slot_newFlightMode( Calculator::FlightMode newFlightMode )
{
  // Store new flight mode
  flightMode = newFlightMode;

  // Restart wind analysis for each flight mode change. The important thing
  // to measure is the number of turns in a thermal per turn direction.
  restartCycle( true );
}

void WindAnalyser::slot_newConstellation( SatInfo& newConstellation )
{
  satCnt = newConstellation.satsInView;

  if( satCnt < minSatCnt )
    {
      // We are active, but the satellite count drops below minimum. A new
      // wind analysis is started.
      restartCycle( true );
    }
}

void WindAnalyser::slot_gpsStatusChange( GpsNmea::GpsStatus newStatus )
{
  gpsStatus = newStatus;

  if( newStatus != GpsNmea::validFix )
    {
      // GPS fix is lost, start a new wind analysis.
      restartCycle( true );
    }
}
