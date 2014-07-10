/***********************************************************************
 **
 **   windanalyser.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by Andre Somers
 **                   2009-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGlobal>

#include "windanalyser.h"
#include "mapcalc.h"
#include "generalconfig.h"

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
  a number of wind measurements and calculates a weighted average based on quality.
*/

WindAnalyser::WindAnalyser(QObject* parent) :
  QObject(parent),
  active(false),
  circleCount(0),
  circleLeft(false),
  circleDegrees(0),
  circleSectors(0),
  lastHeading(-1),
  satCnt(0),
  minSatCnt(4),
  ciclingMode(false),
  gpsStatus(GpsNmea::notConnected)
{
  // Initialization
  minSatCnt = GeneralConfig::instance()->getWindMinSatCount();
}

WindAnalyser::~WindAnalyser()
{}

/** Called if a new sample is available in the sample list. */
void WindAnalyser::slot_newSample()
{
  if( ! active )
    {
      return; // do only work if we are in active mode
    }

  Vector &curVec = calculator->samplelist[0].vector;

  // circle detection
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
  else
    {
      minVector = curVec;
      maxVector = curVec;
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
  qDebug( "curVec: %.3f/%dGrad, minVec: %.3f/%dGrad, maxVec: %.3f/%dGrad",
         curVec.getSpeed().getKph(), curVec.getAngleDeg(),
         minVector.getSpeed().getKph(), minVector.getAngleDeg(),
         maxVector.getSpeed().getKph(), maxVector.getAngleDeg() );
  */

  if( circleDegrees > 360 )
    {
      // full circle made!
      // increase the number of circles flown (used to determine the quality)
      circleCount++;

      // calculate the wind for this circle
      _calcWind();

      circleDegrees = 0;
      circleSectors = 0;

      minVector = curVec;
      maxVector = curVec;
    }
}

/** Called if the flight mode changes */
void WindAnalyser::slot_newFlightMode( Calculator::FlightMode newFlightMode )
{
  // Reset the circle counter for each flight mode change. The important thing
  // to measure is the number of turns in a thermal per turn direction.
  circleCount   = 0;
  circleDegrees = 0;
  circleSectors = 0;
  lastHeading   = -1;

  // We are inactive as default.
  active = false;

  if( newFlightMode == Calculator::circlingL )
    {
      circleLeft = true;
    }
  else if( newFlightMode == Calculator::circlingR )
    {
      circleLeft = false;
    }
  else
    {
      ciclingMode = false;

      // Ok, so we are not circling.
      return;
    }

  // Set circle mode to true.
  ciclingMode = true;

  // Do we have enough satellites in view? The minimum should be four. Otherwise
  // the calculated wind results are very bad.
  if( satCnt < minSatCnt )
    {
      return;
    }

  if( gpsStatus != GpsNmea::validFix )
    {
      // We have not a valid fix.
      return;
    }

  // We are active now.
  active = true;
}

void WindAnalyser::_calcWind()
{
  // int degreePerStep = circleDegrees / circleSectors;

  int aDiff = rint(angleDiff( minVector.getAngleDeg(), maxVector.getAngleDeg() ));

  /*
    Determine quality.

    Currently, we are using the question how well the min and the max vectors
    are on opposing sides of the circle to determine the quality. 140 degrees is
    the minimum separation, 180 is ideal.
    Furthermore, the first two circles are considered to be of lesser quality.
  */

  int quality = 5 - ((180 - abs( aDiff )) / 8);

  if( circleCount < 2 )
    {
      quality--;
    }

  if( circleCount < 1 )
    {
      quality--;
    }

  // qDebug() << "WindQuality=" << quality;

  if( quality < 1 )
    {
      return; // Measurement quality too low
    }

  // 5 is maximum quality, make sure we honor that.
  quality = qMin( quality, 5 );

  // Invert maxVector angle
  maxVector.setAngle( normalize( maxVector.getAngleDeg() + 180 ) );

  // take both directions for min and max vector into account
  /*
  qDebug( "maxAngle=%d/%.0f minAngle=%d/%.0f",
           maxVector.getAngleDeg(), maxVector.getSpeed().getKph(),
           minVector.getAngleDeg(), minVector.getSpeed().getKph() );
  */

  // create a vector object for the resulting wind
  Vector result;

  // the direction of the wind is the direction where the greatest speed occurred
  result.setAngle( ( maxVector.getAngleDeg() + minVector.getAngleDeg() ) / 2);

  // The speed of the wind is half the difference between the minimum and the maximum speeds.
  result.setSpeed( (maxVector.getSpeed().getMps() - minVector.getSpeed().getMps()) / 2.0 );

  // Let the world know about our measurement!
  // qDebug("### ComputedWind: %dGrad/%.0fKm/h", result.getAngleDeg(), result.getSpeed().getKph());

  emit newMeasurement( result, quality );
}

void WindAnalyser::slot_newConstellation( SatInfo& newConstellation )
{
  satCnt = newConstellation.satsInView;

  if( active && (satCnt < minSatCnt) )
    {
      // we are active, but the satellite count drops below minimum
      active = false;
      return;
    }

  if( !active && ciclingMode && satCnt >= minSatCnt )
    {
      // we are not active because we had low satellite count but that has been
      // changed now. So we become active.
      // Initialize analyzer-parameters
      circleCount   = 0;
      circleDegrees = 0;
      circleSectors = 0;
      lastHeading   = -1;
    }
}

void WindAnalyser::slot_gpsStatusChange( GpsNmea::GpsStatus newStatus )
{
  gpsStatus = newStatus;

  if( active && newStatus != GpsNmea::validFix )
    {
      // we are active, but the GPS fix is lost
      active = false;
      return;
    }

  if( !active && ciclingMode )
    {
      // we are not active because we had no GPS fix but that has been
      // changed now. So we become active.
      // Initialize analyzer-parameters
      circleCount   = 0;
      circleDegrees = 0;
      circleSectors = 0;
      lastHeading   = -1;
    }
}
