/***********************************************************************
**
**   windstore.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include <QtCore>

#include <distance.h>
#include <mapcalc.h>
#include <speed.h>
#include <WindStore.h>

WindStore::WindStore( QObject* parent ) :
  QObject(parent),
  m_lastRequiredAltitudeInterval(-1)
{
}

/**
 * Called with new measurements. The quality is a measure for how
 * good the measurement is. Higher quality measurements are more
 * important in the end result and stay in the store longer.
 */
void WindStore::slot_Measurement( Vector& windVector,
                                  const Altitude& altitude,
                                  float quality,
                                  int /* numberOfMeasurement */ )
{
  qDebug() << "WindStore::slot_Measurement:"
           << "WD=" << windVector.getAngleDeg()
           << "WS=" << windVector.getSpeed().getKph()
           << "Alt=" << altitude.getMeters()
           << "quality=" << quality;

  WindMeasurement wm;
  wm.vector = windVector;
  wm.quality = quality;

  // reduce altitude to a 100m interval
  wm.altitude = static_cast<int>( altitude.getMeters() );
  wm.altitude = ( wm.altitude / 100 ) * 100;
  wm.time = QTime::currentTime();

  WindMeasurement& oldWm = lastWindMeasurement;

  if( windMap.contains( wm.altitude ) == true )
    {
      oldWm = windMap[wm.altitude];;
    }

  bool takeIt = false;

  // Check, if we have a last wind measurement that fulfills the following
  // conditions:
  // 1. not older as 5 minutes
  // 2. altitude difference <= 300m
  // 3. Speed difference <= 5 Km/h
  // 4. Heading difference <= 10 degrees
  // In this case we apply a low pass filter to the new wind.
  // Otherwise the new original wind measurement is taken.
  if( lastWindMeasurement.vector.isValid() )
    {
      int age = lastWindMeasurement.time.secsTo( wm.time );
      int altDiff = abs( lastWindMeasurement.altitude - wm.altitude );
      double deltaSpeed = fabs( wm.vector.getSpeed().getKph() - lastWindMeasurement.vector.getSpeed().getKph() );
      double angleDiff = fabs( MapCalc::angleDiffDegree( lastWindMeasurement.vector.getAngleDeg(),
                                                         wm.vector.getAngleDeg() ));

      if( age <= 5 * 60 && altDiff <= 300 && deltaSpeed <= 5.0 && angleDiff <= 10.0 )
        {
          filterWindmeasurement( wm, lastWindMeasurement, quality );
          takeIt = true;
        }
    }

  if( takeIt == false )
    {
      if( windMap.contains( wm.altitude ) == false )
        {
          // No wind contained in altitude interval
          takeIt = true;
        }
      else
        {
          // Old wind exists, lower quality over the elapsed time
          double age = double( oldWm.time.secsTo( wm.time ) );
          double oldQuality = oldWm.quality * ( 1 - (age / 3600.) );

          qDebug() << "Age=" << age << "OldQuality=" << oldQuality
                   << "newQuality=" << wm.quality;

          if( wm.quality >= oldQuality )
            {
              takeIt = true;
            }
        }
    }

  if( takeIt == true )
    {
      // insert new or updated measurement into the map.
      windMap.insert( wm.altitude, wm );

      // we may have a new wind value, so make sure it's emitted if needed!
      if( wm.vector != m_lastReportedWind )
        {
          m_lastReportedWind = wm.vector;
          emit newWind( wm.vector );
          qDebug() << "new Wind is reported"
                   << wm.vector.getAngleDeg() << "/"
                   << wm.vector.getSpeed().getKph();
        }
    }

  // Save last stored wind measurement
  lastWindMeasurement = wm;
}

/**
 * Filter wind measurement by using a low pass and by considering the
 * delivered quality.
 */
void WindStore::filterWindmeasurement( WindMeasurement& newWm,
                                       WindMeasurement& oldWm,
                                       float quality )
{
  float kq = quality / 20.0;
  double a1 = oldWm.vector.getAngleDegDouble();
  double a2 = newWm.vector.getAngleDegDouble();

  double wdDelta = MapCalc::angleDiffDegree( a1, a2) * kq;
  double wsDelta = ( newWm.vector.getSpeed().getMps() - oldWm.vector.getSpeed().getMps() ) * kq;

  qDebug() << "w1=" << a1 << "w2=" << a2
           << "w1-w2=" << MapCalc::angleDiffDegree( a1, a2)
           << "wdDelta=" << wdDelta << "wsDelta" << wsDelta;

  newWm.vector.setAngle( oldWm.vector.getAngleDegDouble() + wdDelta );
  newWm.vector.setSpeed( oldWm.vector.getSpeed().getMps() + wsDelta );

  // average quality.
  newWm.quality = ( newWm.quality + oldWm.quality ) / 2.0;

  qDebug() << "WindStore: wind filtered by kq=" << kq
           << "newWD=" << newWm.vector.getAngleDeg()
           << "newWS=" <<  newWm.vector.getSpeed().getKph()
           << "Alt=" << newWm.altitude
           << "newQuality=" << newWm.quality;
}

/**
 * Called to get the stored wind for the required altitude. If no wind is
 * available, an invalid wind vector is returned.
 */
Vector WindStore::getWind( const Altitude &altitude, bool exact )
{
  // qDebug() << "WindStore::getWind(): " << altitude.getMeters();
  Vector wind;

  if( windMap.isEmpty() == true )
    {
      // no wind info available
      return wind;
    }

  // reduce altitude to a 100m interval
  int altInterval = static_cast<int>( altitude.getMeters() );
  altInterval = ( altInterval / 100 ) * 100;

  // Check, if a wind is available for the required altitude. We always deliver
  // the up to date wind as first.
  if( windMap.contains( altInterval ) == true )
    {
      wind = windMap[altInterval].vector;

      // Reset older search results.
      m_lastRequiredAltitudeInterval = -1;

      return wind;
    }

  if( exact == true )
    {
      // The altitude band should not be used, to search the next closed wind.
      return wind;
    }

  // Look, if old wind data can be reused
  if( m_lastRequiredAltitudeInterval == altInterval )
    {
      return m_lastRequiredWind;
    }

  // Now we try to search a wind in the near of the requested altitude.
  QList<int> keys = windMap.keys();

  int altMinKey = keys.first();
  int altMaxKey = keys.last();

  // Search next minimum altitude
  for( int i=0; i < keys.size(); i++ )
    {
      if( keys.at(i) < altInterval )
        {
          altMinKey = keys.at(i);
          continue;
        }

      break;
    }

  // Search next maximum altitude
  for( int i = keys.size() - 1; i > 0; i-- )
    {
      if( keys.at(i) > altInterval )
        {
          altMaxKey = keys.at(i);
          continue;
        }

      break;
    }

  // Look which altitude is closer to the required altitude
  int minDiff = abs( altInterval - altMinKey );
  int maxDiff = abs( altMaxKey - altInterval );

  if( minDiff <= maxDiff )
    {
      wind = windMap[altMinKey].vector;
    }
  else
    {
      wind = windMap[altMaxKey].vector;
    }

  // Store wind data for reusage.
  m_lastRequiredAltitudeInterval = altInterval;
  m_lastRequiredWind = wind;

  return wind;
}

/**
 * Returns the stored wind measurement from the required altitude from the
 * wind map. If the measurement is not available, the wind vector is set
 * to invalid.
 */
WindMeasurement WindStore::getWindMeasurement( const Altitude &altitude )
{
  WindMeasurement wm;

  if( windMap.isEmpty() == true )
    {
      return wm;
    }

  // reduce altitude to a 100m interval
  int altInterval = static_cast<int>( altitude.getMeters() );
  altInterval = ( altInterval / 100 ) * 100;

  // Check, if a wind is available for the required altitude. We always deliver
  // the up to date wind as first.
  if( windMap.contains( altInterval ) == true )
    {
      return windMap[altInterval];
    }

  return wm;
}
