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
#include "calculator.h"
#include "speed.h"

// TAS +- observation interval in Km/h
#define OI_TAS 10.0

// Heading +- observation interval in degrees
#define OI_HD 5.0

/*
  The Wind analysis in straight flight. The wind direction and speed is
  calculated by using the wind triangle formula and the following parameters:

  -TAS
  -Ground speed
  -Compass true heading
  -GPS true course
 */
WindCalcInStraightFlight::WindCalcInStraightFlight( QObject* parent ) :
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
  sumTHDeviation( 0.0 ),
  sumTCDeviation( 0.0 ),
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
  sumTCDeviation = 0.0;
  sumTHDeviation = 0.0;

  // Define limit of observation window
  if( trueHeading >= (360. - deltaHeading ) )
    {
      hMin = trueHeading - 360. - deltaHeading;
    }

  // Define limit of observation window
  hMax = trueHeading + deltaHeading;

  if( hMax >= 360. )
    {
      hMax -= 360.;
    }
}

void WindCalcInStraightFlight::slot_trueCompassHeading( const double& )
{
  // get current TAS
  double ctas = calculator->getlastTas().getKph();

  // Check, if we have a TAS value > 25 km/h. GS can be nearly zero in the wave.
  // If TAS is to low, the measurement make no sense.
  if( ctas < 25.0 )
    {
      // Stop measurements.
      nunberOfSamples = 0;
      return;
    }

  if( nunberOfSamples == 0 )
    {
      // We start a new measurement cycle.
      start();
      return;
    }

  // check if given TAS deltas are valid.
  if( fabs( tas - ctas ) > deltaSpeed )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // check if given magnetic heading deltas are valid.
  double th = calculator->getLastMagneticTrueHeading();

  if( th >= (360.0 - deltaHeading ) )
    {
      th -= 360.0;
    }

  if( ! ( hMin <= th && th <= hMax ) )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // Take values
  nunberOfSamples++;

  // The given deltas are fulfilled
  sumTas += ctas;
  sumGroundSpeed += calculator->getLastSpeed().getKph();

  // Calculate course deviations
  double deviation = calculator->getLastHeading() - trueCourse;

  if( deviation > 180. ) { deviation -= 360.; }
  if( deviation < 0. ) { deviation += 360.; }

  sumTCDeviation += deviation;

  deviation = calculator->getLastMagneticTrueHeading() - trueHeading;

  if( deviation > 180. ) { deviation -= 360.; }
  if( deviation < 0. ) { deviation += 360.; }

  sumTHDeviation += deviation;

  // store min max values for TAS
  vMin = qMin( vMin, ctas );
  vMax = qMax( vMax, ctas );

  if( measurementDuration.elapsed() >= 10000 )
    {
      /**
       calculate wind by using wind triangle, see more here:
       http://klspublishing.de/downloads/KLSP%20061%20Allgemeine%20Navigation%20DREHMEIER.pdf

       The Wind Correction Angle is the angle between the Heading and the
       Desired Course:

       WCA = Heading - DesiredCourse
       */
      double nos = static_cast<double>(nunberOfSamples);

      double tc = sumTCDeviation / nos;

      // normalize angle
      if (tc >= 360.) { tc -= 360.; }
      if (tc < 0.)    { tc += 360.; }

      tc += trueCourse; // Average of TC

      double th = sumTHDeviation / nos;

      // normalize angle
       if (th >= 360.) { th -= 360.; }
       if (th < 0.)    { th += 360.; }

       th += trueHeading; // Average of TH

      // WCA in radians
      double wca = (( sumTCDeviation - sumTHDeviation ) / nos ) * M_PI / 180.0;
      double tas = sumTas / nos;
      double gs = sumGroundSpeed / nos;

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

      qDebug() << "SF-Wind: Samples=" << nunberOfSamples
               << "MM-Time=" << measurementDuration.elapsed() / 1000
               << "s, WS=" << ws << "Km/h, WD=" << wd << "°";
    }
}
