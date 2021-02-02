/***********************************************************************
 **
 **   WindCalcInStraightFlight.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2021 by Axel PauliWindCalcInStraightFlight.h
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <cmath>
#include <QtGlobal>

#include "WindCalcInStraightFlight.h"
#include "generalconfig.h"
#include "speed.h"
#include "vector.h"

// Observation interval TAS in Km/h
#define OI_TAS 10.0

// Observation interval heading in degrees
#define OI_HD 5.0

/*
  The Wind analysis in straight flight. The wind direction and speed is
  calculated by using the wind triangle formula and the following parameters:

  -TAS
  -Ground speed
  -Compass true heading
  -GPS true headingWindCalcInStraightFlight.h
 */
WindCalcInStraightFlight::WindCalcInStraightFlight(QObject* parent) :
  QObject(parent),
  nunberOfSamples(0),
  deltaSpeed( OI_TAS ), // +- 10 kmph
  deltaHeading( OI_HD ), // +- 5 degree
  tas( 0.0 ),
  groundSpeed( 0.0 ),
  trueCourse( 0.0),
  trueHeading( 0.0 ),
  sumTas( 0.0 ),
  sumGroundSpeed( 0.0 ),
  sumTrueCourse( 0.0 ),
  sumTrueHeading( 0.0 ),
  vMin( 0.0 ),
  vMax( 0.0 ),
  hMin( 0.0 ),
  hMax( 0.0 )
{
}

WindCalcInStraightFlight::~WindCalcInStraightFlight()
{
}

/**
 * Starts a new measurment cycle.
 */
void WindCalcInStraightFlight::start()
{
  nunberOfSamples = 1;
  measurementDuration.start();
  vMin = vMax = tas = calculator->getlastTas().getKph();
  groundSpeed = calculator->getLastSpeed().getKph();
  trueCourse = calculator->getLastHeading();
  trueHeading = calculator->getLastMagneticTrueHeading();
  sumTas = tas;
  sumGroundSpeed = groundSpeed;
  sumTrueCourse = trueCourse;
  sumTrueHeading = trueHeading;

  if( trueHeading >= (360. - deltaHeading ) )
    {
      hMin = trueHeading - 360. - deltaHeading;
    }

  hMax = trueHeading + deltaHeading;

  if( hMax >= 360. )
    {
      hMax -= 360.;
    }
}

void WindCalcInStraightFlight::slot_trueCompassHeading( const double& heading )
{
  // get current TAS
  double ctas = calculator->getlastTas().getKph();

  // Check, if we have a TAS value > 20 km/h. GS can be nearly zero in the wave.
  // If TAS is to low, the measurement make no sense.
  if( ctas < 20.0 )
    {
      if( nunberOfSamples > 0 )
        {
          // Reset measurements.
          nunberOfSamples = 0;
          return;
        }
    }

  if( nunberOfSamples == 0 )
    {
      // We start a new measurement cycle.
      start();
      return;
    }

  // check if given deltas are valid.
  if( fabs( tas - ctas ) > deltaSpeed )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // check if given deltas are valid.
  double th = calculator->getLastMagneticTrueHeading();

  if( th >= (360.0 - deltaHeading ) )
    {
      th -= 360.0;
    }

  if( th < hMin || th > hMax )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // Take values
  nunberOfSamples++;

  // The given deltas are fulfilled
  sumTas += calculator->getlastTas().getKph();
  sumGroundSpeed += calculator->getLastSpeed().getKph();
  sumTrueCourse += calculator->getLastHeading();
  sumTrueHeading += calculator->getLastMagneticTrueHeading();

  // store min max values for spped and heading
  vMin = qMin( vMin, ctas );
  vMax = qMax( vMax, ctas );

  hMin = qMin( hMin, th );
  hMax = qMax( hMax, th );

  if( measurementDuration.elapsed() >= 10000 )
    {
      // calculate wind by using wind triangle, see more here:
      // http://klspublishing.de/downloads/KLSP%20061%20Allgemeine%20Navigation%20DREHMEIER.pdf
      /*
       By definition the Wind Correction Angle is the angle between the Heading
       and the Desired Course:
       WCA = Heading - DesiredCourse which leads directly to the above equation.
       */
      double nos = static_cast<double>(nunberOfSamples);

      // WCA in radians
      double wca = (( sumTrueCourse - sumTrueHeading ) / nos ) * M_PI / 180.0;
      double tas = sumTas / nos;
      double gs = sumGroundSpeed / nos;
      double tc = sumTrueCourse / nos;
      double th = sumTrueHeading / nos;

      // Apply the Cosinus sentence: c^2 = a^2 + b^2 − 2 * a * b * cos( α )
      // to calculate the WS (wind speed)
      double ws = sqrt( (tas * tas) + (gs * gs ) - ( 2 * tas * gs * cos( wca ) ) );

      // WS / sin(WCA)
      double term = ws / sin( wca );

      // calculate WA (wind angle) in degree
      double wa = asin( tas / term ) * 180. / M_PI;

      // Wind direction: W = TC - WA
      double wd = tc - wa;

      if( wd < 0 )
        {
          // correct negative angels
          wd += 360.;
        }

      Speed WS;
      WS.setMph( ws );
      Vector wind;
      wind.setSpeed ( WS );
      wind.setAngle( wd );
      emit newMeasurement( wind, 5 );
    }
}
