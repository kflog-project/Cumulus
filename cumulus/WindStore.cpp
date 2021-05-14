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

#include "WindStore.h"

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
                                  float quality )
{
  qDebug() << "WindStore::slot_Measurement: WS="
           <<  windVector.getSpeed().getKph()
           << "WD=" << windVector.getAngleDeg()
           << "Alt=" << altitude.getMeters()
           << "quality=" << quality;

  WindMeasurement wind;
  wind.vector = windVector;
  wind.quality = quality;

  // reduce altitude to a 100m interval
  wind.altitude = static_cast<int>( altitude.getMeters() );
  wind.altitude = ( wind.altitude / 100 ) * 100;
  wind.time = QTime::currentTime();

  bool takeIt = false;

  if( windMap.contains( wind.altitude ) == false )
    {
      qDebug() << "WindStore: first wind added";
      // insert new measurement into the map.
      takeIt = true;
    }
  else
    {
      // Load existing measurement
      WindMeasurement& oldWind = windMap[wind.altitude];

      if( wind.quality >= oldWind.quality )
        {
          qDebug() << "WindStore: wind replaced due to better quality"
                   << "oldQual=" << oldWind.quality
                   << "newQual=" << wind.quality;

          // The better or younger quality is stored.
          takeIt = true;
        }
      else
        {
          // Evaluate the quality difference
          float qDiff = fabsf( oldWind.quality - wind.quality );

          // calculate age in seconds.
          int age = oldWind.time.secsTo( wind.time );

          qDebug() << "WindStore: qDiff=" << qDiff << "WindAge=" << age;

          if( age >= 60 * 60 && qDiff <= 2.5 )
            {
              // after 60 minutes
              takeIt = true;
              qDebug() << "WindStore: replace wind after 60m";
            }
          else if( age >= 40 * 60 && qDiff <= 2. )
            {
              // after 40 minutes
              takeIt = true;
              qDebug() << "WindStore: replace wind after 40m";
            }
          else if( age >= 20 * 60 && qDiff <= 1. )
            {
              // after 20 minutes
              takeIt = true;
              qDebug() << "WindStore: replace wind after 20m";
             }
          else if( age >= 10 * 60 && qDiff <= 0.5 )
            {
              // after 10 minutes
              takeIt = true;
              qDebug() << "WindStore: replace wind after 10m";
            }
        }
    }

  if( takeIt == true )
    {
      // insert new measurement into the map.
      windMap.insert( wind.altitude, wind );
    }

  // we may have a new wind value, so make sure it's emitted if needed!
  if( wind.vector != m_lastReportedWind )
    {
      m_lastReportedWind = wind.vector;
      emit newWind( wind.vector );
    }
}

/**
 * Called to get the stored wind for the required altitude. If no wind is
 * available, an invalid wind vector is returned.
 */
Vector WindStore::getWind( const Altitude &altitude, bool exact )
{
  qDebug() << "WindStore::getWind(): " << altitude.getMeters();
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
