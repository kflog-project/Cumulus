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
#include "gpsnmea.h"
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
  resulting measurement.

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
  circleDeg(0),
  lastHeading(-1),
  satCnt(0),
  minSatCnt(4),
  ciclingMode(false),
  minVector(0.0, 0.0),
  maxVector(0.0, 0.0)
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
      // qDebug("diff: %d", diff );
      circleDeg += diff;
    }
  else
    {
      minVector = curVec.clone();
      maxVector = curVec.clone();
    }

  lastHeading = curVec.getAngleDeg();

  if( circleDeg > 360 )
    {
      // full circle made!
      circleDeg = 0;

      // increase the number of circles flown (used to determine the quality)
      circleCount++;

      // calculate the wind for this circle
      _calcWind();

      minVector = curVec.clone();
      maxVector = curVec.clone();
    }
  else
    {
      // qDebug("curVec: %f/%f", (float)curVec.getKph(), (float)curVec.getAngleDeg() );
      if( curVec.getSpeed().getMps() < minVector.getSpeed().getMps() )
        {
          minVector = curVec.clone();
        }

      if( curVec.getSpeed().getMps() > maxVector.getSpeed().getMps() )
        {
          maxVector = curVec.clone();
        }

      qDebug( "minVec: %f/%d, maxVec: %f/%d",
              (float) minVector.getSpeed().getKph(), minVector.getAngleDeg(),
              (float) maxVector.getSpeed().getKph(), maxVector.getAngleDeg() );
    }
}

/** Called if the flight mode changes */
void WindAnalyser::slot_newFlightMode( Calculator::flightmode newFlightMode )
{
  // Reset the circle counter for each flight mode change. The important thing
  // to measure is the number of turns in a thermal per turn direction.
  circleCount = 0;
  circleDeg   = 0;
  lastHeading = -1;
  minVector   = Vector( 0.0, 0.0 );
  maxVector   = Vector( 0.0, 0.0 );

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
      return; // ok, so we are not circling. Exit function.
    }

  // Set circle mode to true.
  ciclingMode = true;

  // Do we have enough satellites in view? The minimum should be four. Otherwise
  // the calculated wind results are very bad.
  if( satCnt < minSatCnt )
    {
      return;
    }

  // We are active now.
  active = true;
}


void WindAnalyser::_calcWind()
{
  int aDiff = angleDiff( minVector.getAngleDeg(), maxVector.getAngleDeg() );

  qDebug(" _calcWind diff=%d min=%d max=%d\n\n", aDiff, minVector.getAngleDeg(), maxVector.getAngleDeg());

  /*determine quality.
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

  qDebug() << "WindQuality=" << quality;

  if( quality < 1 )
    {
      return; // Measurement quality too low
    }

  // 5 is maximum quality, make sure we honor that.
  quality = qMin( quality, 5 );

  // change to work with radian, that's faster because it is the internal format.
  int ang = maxVector.getAngleDeg();
  maxVector.setAngle( normalize( ang + 180 ) );
  Vector a = maxVector.clone();
  a.add( minVector );

  // take both directions for min and max vector into account
  qDebug("maxAngle=%d/%f minAngle=%d/%f avr=%d/%f",
          maxVector.getAngleDeg(), maxVector.getSpeed().getKph(),
          minVector.getAngleDeg(), minVector.getSpeed().getKph(),
          a.getAngleDeg(), a.getSpeed().getKph() );

  // create a vector object for the resulting wind
  Vector result;

  // the direction of the wind is the direction where the greatest speed occurred
  result.setAngle( a.getAngleDeg() );

  // The speed of the wind is half the difference between the minimum and the maximum speeds.
  result.setSpeed( (maxVector.getSpeed().getMps() - minVector.getSpeed().getMps()) / 2 );

  // Let the world know about our measurement!
  //qDebug("Wind: %d/%f\n", (int)result.getAngleDeg(),(float)result.getSpeed().getKph());
  emit newMeasurement( result, quality );
}

void WindAnalyser::slot_newConstellation( SatInfo& newConstellation )
{
  satCnt = newConstellation.satsInUse;

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
      circleCount = 0;
      circleDeg   = 0;
      lastHeading = -1;
    }
}
