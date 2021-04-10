/***********************************************************************
 **
 **   WindCalcInStraightFlight.cpp
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

#include <cmath>
#include <QtGlobal>

#include "WindCalcInStraightFlight.h"
#include "generalconfig.h"
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
  -GPS Ground speed
  -Compass true heading
  -GPS true course

  TAS, GS and magnetic heading must fulfill the given time limits and deltas
  before the wind calculation is executed. Only one wind calculation is done
  per second.
 */
WindCalcInStraightFlight::WindCalcInStraightFlight( QObject* parent ) :
  QObject(parent),
  nunberOfSamples( 0 ),
  deliverWind( 10 ),
  deltaSpeed( OI_TAS ), // +- 10 kmph
  deltaHeading( OI_HD ), // +- 5 degree
  tas( 0.0 ),
  groundSpeed( 0.0 ),
  trueCourse( 0.0 ),
  trueHeading( 0.0 ),
  sumTas( 0.0 ),
  sumGroundSpeed( 0.0 ),
  sumTHDeviation( 0.0 ),
  sumTCDeviation( 0.0 ),
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
  deliverWind = GeneralConfig::instance()->getStartWindCalcInStraightFlight();
  measurementDuration.start();
  tas = calculator->getlastTas().getKph();
  groundSpeed = calculator->getLastSpeed().getKph();
  trueCourse = calculator->getLastHeading();
  trueHeading = calculator->getLastMagneticTrueHeading();
  sumTas = tas;
  sumGroundSpeed = groundSpeed;
  sumTCDeviation = 0.0;
  sumTHDeviation = 0.0;

  // Define limit of observation window
  hMin = trueHeading - deltaHeading;

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
  // get current ground speed.
  double cgs = calculator->getLastSpeed().getKph();

  // Check, if we have a GS value > 25 km/h. GS can be nearly zero in the wave.
  // If GS is to low, the measurement make no sense.
  if( cgs < 25.0 )
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

  // check, if given ground speed deltas are valid.
  if( fabs( groundSpeed - cgs ) > deltaSpeed )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // get current TAS
  double ctas = calculator->getlastTas().getKph();

  // check if given TAS deltas are valid.
  if( fabs( tas - ctas ) > deltaSpeed )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // get true magnetic heading
  double cth = calculator->getLastMagneticTrueHeading();

  if( cth >= (360.0 - deltaHeading ) )
    {
      cth -= 360.0;
    }

  // check if given magnetic heading deltas are valid.
  if( ! ( cth >= hMin && cth <= hMax ) )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // get true course
  double ctc = calculator->getLastMagneticTrueHeading();

  if( ctc >= (360.0 - deltaHeading ) )
    {
      ctc -= 360.0;
    }

  // check if given true course deltas are valid.
  if( ! ( ctc >= hMin && ctc <= hMax ) )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // Take values
  nunberOfSamples++;

  // The given deltas are fulfilled
  sumTas += ctas;
  sumGroundSpeed += cgs;

  // Calculate true course deviations
  double deviation = ctc - trueCourse;

  if( deviation < -180. ) { deviation += 360.; }

  sumTCDeviation += deviation;

  // Calculate true magnetic heading deviations
  deviation = cth - trueHeading;

  if( deviation < -180. ) { deviation += 360.; }

  sumTHDeviation += deviation;

  if( measurementDuration.elapsed() >= deliverWind * 1000 )
    {
      /**
       calculate wind by using wind triangle, see more here:
       http://klspublishing.de/downloads/KLSP%20061%20Allgemeine%20Navigation%20DREHMEIER.pdf

       The Wind Correction Angle is the angle between the Heading and the
       Desired Course:

       WCA = Heading - DesiredCourse
       */

      // A new wind is only delivered after one second has elapsed in minimum.
      deliverWind++;

      double nos = double( nunberOfSamples );

      double tc = sumTCDeviation / nos;

      // normalize angle
      if (tc >= 360.) { tc -= 360.; }
      else if (tc < 0.) { tc += 360.; }

      tc += trueCourse; // Average of TC

      double th = sumTHDeviation / nos;

      // normalize angle
       if (th >= 360.) { th -= 360.; }
       else if (th < 0.) { th += 360.; }

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
      double wa = asin( tas / term ) * 180.0 / M_PI;

      // Wind direction: W = TC - WA
      double wd = tc - wa;

      if( wd < 0.0 )
        {
          wd += 360.0;
        }
      else if( wd >= 360.0 )
        {
          wd -= 360.0;
        }

      Speed WS;
      WS.setKph( ws );
      Vector wind;
      wind.setSpeed ( WS );
      wind.setAngle( wd );
      emit newMeasurement( wind, 5 );

      qDebug() << "SF-Wind: Samples=" << nunberOfSamples
               << "MM-Time=" << measurementDuration.elapsed() / 1000
               << "s, WS=" << ws << "Km/h, WD=" << wd << "°";
    }
}
