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

#include "generalconfig.h"
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
  minimumAirSpeed( 0 ),
  deltaSpeed( OI_TAS ), // +- 10 kmph
  deltaHeading( OI_HD ), // +- 5 degree
  tas( 0.0 ),
  groundSpeed( 0.0 ),
  trueCourse( 0.0 ),
  trueHeading( 0.0 ),
  sumTas( 0.0 ),
  sumGroundSpeed( 0.0 ),
  meanTH( 0.0 ),
  meanTC( 0.0 ),
  tcMin( 0.0 ),
  tcMax( 0.0 ),
  thMin( 0.0 ),
  thMax( 0.0 )
{
}

WindCalcInStraightFlight::~WindCalcInStraightFlight()
{
}

/**
 * Starts a new measurement cycle.
 */
void WindCalcInStraightFlight::start()
{
  GeneralConfig *conf = GeneralConfig::instance();

  nunberOfSamples = 1;
  deliverWind = conf->getStartWindCalcInStraightFlight();
  minimumAirSpeed = conf->getMinimumAirSpeed4WC().getKph();
  deltaSpeed = conf->getSpeedTolerance4WC().getKph();
  deltaHeading = conf->getHeadingTolerance4WC();
  measurementStart.start();
  tas = calculator->getlastTas().getKph();
  groundSpeed = calculator->getLastSpeed().getKph();
  trueCourse = calculator->getLastHeading();
  trueHeading = calculator->getLastMagneticTrueHeading();
  sumTas = tas;
  sumGroundSpeed = groundSpeed;
  meanTC = 0.0;
  meanTH = 0.0;

  // Define limits of course observation window
  tcMin = normAngle( trueCourse - deltaHeading );
  tcMax = normAngle( trueCourse + deltaHeading );

  // Define limits of heading observation window
  thMin = normAngle( trueHeading - deltaHeading );
  thMax = normAngle( trueHeading + deltaHeading );
}

void WindCalcInStraightFlight::slot_trueCompassHeading( const double& )
{
  if( GeneralConfig::instance()->isSfWCEnabled() == false )
    {
      // Wind calculation is disabled.
      nunberOfSamples = 0;
      return;
    }

  // get current TAS
  double ctas = calculator->getlastTas().getKph();

  if( ctas < minimumAirSpeed )
    {
      // We are not in flight
      nunberOfSamples = 0;
      return;
    }

  // get current ground speed.
  double cgs = calculator->getLastSpeed().getKph();

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

  // check if given TAS deltas are valid.
  if( fabs( tas - ctas ) > deltaSpeed )
    {
      // Condition violated, start a new measurements cycle.
      start();
      return;
    }

  // get true magnetic heading
  double cth = calculator->getLastMagneticTrueHeading();

  bool ok = true;

  // Check if given magnetic heading deltas are valid.
  if( thMin < thMax && ( cth < thMin || cth > thMax ) )
    {
      // Heading outside of observation window
      ok = false;
    }
  else if( thMin > thMax && cth < thMin && cth > thMax )
    {
      // Heading outside of observation window
      ok = false;
    }

 if( ok == false )
   {
     // Condition violated, start a new measurements cycle.
     start();
     return;
   }

  // get true course (GPS heading)
  double ctc = calculator->getLastHeading();

  if( cgs >= 5 )
    {
      // The ground course check is only done, if the ground speed is >=5 Km/h.
      // Near speed zero, the ground course is not stable in its direction.
      // Check if given true course deltas are valid.
      if( tcMin < tcMax && ( ctc < tcMin || ctc > tcMax ) )
        {
          // Heading outside of observation window
          ok = false;
        }
      else if( tcMin > tcMax && ctc < tcMin && ctc > tcMax )
        {
          // Heading outside of observation window
          ok = false;
        }

     if( ok == false )
       {
         // Condition violated, start a new measurements cycle.
         start();
         return;
       }
    }

  // Take values
  nunberOfSamples++;

  // The given deltas are fulfilled
  sumTas += ctas;
  sumGroundSpeed += cgs;

  // Calculate true course mean
  meanTC = meanAngle( ctc, meanTC );

  // Calculate true magnetic heading mean
  meanTH = meanAngle( cth, meanTH );

  if( measurementStart.elapsed() >= deliverWind * 1000 )
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

      double wca = 0.0;
      double ws = 0.0;
      double wd = 0.0;
      double nos = double( nunberOfSamples );
      double tas = sumTas / nos;
      double gs = sumGroundSpeed / nos;

      if( gs < 2.0 )
        {
          // In the wave the ground speed can be nearly zero. In this case
          // the GPS can deliver a wrong true course. Therefore the wind
          // direction is set to true heading and the wind speed is set to the
          // tas.
          wd = meanTH;
          ws = tas;
        }
      else
        {
          // WCA in radians
          wca = ( meanTC - meanTH ) * M_PI / 180.0;

          // Apply the Cosinus sentence: c^2 = a^2 + b^2 − 2 * a * b * cos( α )
          // to calculate the WS (wind speed)
          ws = sqrt( (tas * tas) + (gs * gs ) - ( 2 * tas * gs * cos( wca ) ) );

          // wind direction calculation taken from here:
          // view-source:http://www.owoba.de/fliegerei/flugrechner.html
          double tcrad = meanTC * M_PI / 180.0;
          double thrad = meanTH * M_PI / 180.0;

          // wind direction formula to calculate wd
          wd = tcrad + atan2( tas * sin( thrad - tcrad ),
                              tas * cos( thrad - tcrad ) - gs );

          // transfer radian to degree
          wd = wd * 180.0 / M_PI;
          wd = normAngle( wd );
        }

      Speed WS;
      WS.setKph( ws );
      Vector wind;
      wind.setSpeed ( WS );
      wind.setAngle( wd + 0.5 );
      emit newMeasurement( wind, lastAltitude, 5, nunberOfSamples );

      qDebug() << "SF-Wind: Samples=" << nunberOfSamples
               << "MM-Time=" << measurementStart.elapsed() / 1000
               << "s, WCA=" << ( meanTC - meanTH ) << "WS="
               << ws << "Km/h, WD=" << wd;
    }
}

/**
 * Calculate the smaller bisector value from angles.
 *
 * @param angle as degree 0...359
 * @param average as degree 0...359
 * @return average angle as degree 0...359
 */
double WindCalcInStraightFlight::meanAngle( double angle, double average )
{
  double bisector = 0.0;
  double result = 0.0;
  double absDiff = fabs( angle - average );

  if( absDiff > 180.0 )
    {
      bisector = ( 360.0 - absDiff ) / 2.0;

      if( angle <= average )
        {
          result = average + bisector;
        }
      else
        {
          result = average - bisector;
        }
    }
  else
    {
      bisector = absDiff / 2.0;

      if( angle <= average )
        {
          result = angle + bisector;
        }
      else
        {
          result = angle - bisector;
        }
   }

  return normAngle( result );
}
