/***********************************************************************
**
**   windmeasurementlist.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2007-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtCore>

#include "windmeasurementlist.h"
#include "altitude.h"
#include "vector.h"
#include "generalconfig.h"

// Maximum number of wind measurements in the list.
// No idea what a sensible value would be...
#define MAX_MEASUREMENTS 1800

WindMeasurementList::WindMeasurementList() :
  LimitedList<WindMeasurement>( MAX_MEASUREMENTS )
{
}

WindMeasurementList::~WindMeasurementList()
{
}

/**
 * Returns the weighted mean wind vector over the stored values, or 0
 * if no valid vector could be calculated (for instance: too little or
 * too low quality data).
 */
Vector WindMeasurementList::getWind( const Altitude& alt,
                                     const int timeWindow,
                                     const int altRange )
{
// relative weight for each factor in percent
#define REL_FACTOR_QUALITY 100
#define REL_FACTOR_ALTITUDE 100
#define REL_FACTOR_TIME 200

  // counts the number of calls to prevent endless recursion.
  static short entry = 0;

  Vector result;

  if( size() == 0 )
    {
      // Measurement list is empty.
      return result;
    }

  entry++;

  QTime dauer;
  dauer.start();

  GeneralConfig *conf = GeneralConfig::instance();

  double usedAltRange = 0;

  if( altRange == 0 )
    {
      // Take the default altitude range from the configuration
      usedAltRange  = static_cast<double>(conf->getWindAltitudeRange()) / 2.0;  // 1000m
    }
  else
    {
      usedAltRange = altRange / 2.0;
    }

  int timeRange = timeWindow;

  if( timeRange == 0 )
    {
      // If no time range has been passed, take default one from configuration.
      // The default is set to 10 minutes to get the last current wind.
      timeRange = conf->getWindTimeRange(); // 600s
    }

  int total_quality = 0;
  int quality = 0, q_quality = 0, a_quality = 0, t_quality = 0;
  QTime now = QTime::currentTime();

  double altDiff  = 0.0;
  double timeDiff = 0.0;

  for( int i = 0; i < count(); i++ )
    {
      const WindMeasurement& wm = at( i );

      altDiff = (alt - wm.altitude).getMeters() / usedAltRange;
      timeDiff = fabs( (double) wm.time.secsTo(now) / (double) timeRange );

      if( (fabs(altDiff) < 1.0) && (timeDiff < 1.0) )
        {
          // Measurement quality range is 1...5, 5 is the best quality
          // Maximum quality is 5*100/5 = 100%
          // Minimum quality is 1*100/5 = 20%
          q_quality = qMin(5, wm.quality) * REL_FACTOR_QUALITY / 5;

          // Factor in altitude difference between current altitude and measurement.
          // altRange is 1000 m, altDiff = 0 -> 100%, altDiff = 1 -> 0%
          a_quality = (int) rint(((2.0 / (altDiff * altDiff + 1.0)) - 1.0) * REL_FACTOR_ALTITUDE );

          // Factor in time difference. Default timeRange is 600s.
          const double k = 0.75;

          // timeDiff = 0 -> 200%, timeDiff = 1 -> 0%
          t_quality = (int) rint((k * (1.0 - timeDiff) / (timeDiff * timeDiff + k)) * REL_FACTOR_TIME);

          quality = q_quality * a_quality * t_quality;

          /*
          qDebug("i=%d alt=%f qual=%d wind=%d/%f (%d, %d, %d)",
                  i, wm.altitude.getMeters(), quality,
                  value(i).vector.getAngleDeg(),
                  value(i).vector.getSpeed().getKph(),
                  q_quality, a_quality, t_quality );
          */

          if( quality == 0 )
            {
              continue;
            }

          result.add( value(i).vector * quality );
          total_quality += quality;

          /*
          qDebug( "Cur result=%d/%f (tQ=%d)",
                  result.getAngleDeg(),
                  result.getSpeed().getKph() / total_quality,
                  total_quality );
          */
        }
  }

  if( total_quality > 0 )
    {
      result = result / total_quality;
    }

  /*
  qDebug( "Round=%d, Alt=%f, WindResult=%d/%f",
          entry, alt.getMeters(),
          result.getAngleDeg(), result.getSpeed().getKph() );
  */

  if( ! result.isValid() && entry == 1 && timeWindow < 3600 )
    {
      // If there is no younger wind available make a second round with a time
      // window of one hour.
      result = getWind( alt, 3600 );

      if( ! result.isValid() )
        {
          // If there is no younger wind available make a second round with a time
          // window of two hour.
          result = getWind( alt, 7200 );
        }
    }

  entry--;

  return result;
}

/** Adds the wind vector vector with quality quality to the list. */
void WindMeasurementList::addMeasurement( const Vector& vector,
                                          const Altitude& alt,
                                          int quality )
{
  WindMeasurement wind;
  wind.vector = vector;
  wind.quality = quality;
  wind.altitude = alt;
  wind.time = QTime::currentTime();

  // Add item to limited list.
  add( wind );

  qSort( begin(), end(), WindMeasurement::lessThan );
}

/**
 * getLeastImportantItem is called to identify the item that should be
 * removed if the list is too full. Reimplemented from LimitedList.
 */
int WindMeasurementList::getLeastImportantItemIndex() const
{
  int maxscore = 0;
  int score = 0;
  int foundItem = LimitedList<WindMeasurement>::size() - 1;

  for( int i = foundItem; i >= 0; i-- )
    {
      // Calculate the score of this item. The item with the highest score is the
      // least important one. We may need to adjust the proportion of the quality
      // and the elapsed time. Currently one quality-point (scale: 0 to 5) and
      // the elapsed time are used for the core value.
      score = 6 - LimitedList<WindMeasurement>::at( i ).quality;
      score *= LimitedList<WindMeasurement>::at( i ).time.secsTo( QTime::currentTime() );

      if( score > maxscore )
        {
          maxscore = score;
          foundItem = i;
        }
  }

  return foundItem;
}

bool WindMeasurement::operator < (const WindMeasurement& other) const
{
  // return the difference between the altitudes in item 1 and item 2
  return( altitude < other.altitude );
}
