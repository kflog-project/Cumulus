/***********************************************************************
**
**   windmeasurementlist.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2007-2010 by Axel Pauli
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
#define MAX_MEASUREMENTS 250

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
Vector WindMeasurementList::getWind( const Altitude& alt )
{
  // relative weight for each factor
#define REL_FACTOR_QUALITY 100
#define REL_FACTOR_ALTITUDE 100
#define REL_FACTOR_TIME 200

  GeneralConfig *conf = GeneralConfig::instance();
  int altRange  = conf->getWindAltitudeRange();
  int timeRange = conf->getWindTimeRange();

  int total_quality = 0;
  int quality = 0, q_quality = 0, a_quality = 0, t_quality = 0;
  Vector result;
  QTime now    = QTime::currentTime();
  int altdiff  = 0;
  int timediff = 0;

  for( int i = 0; i < LimitedList<WindMeasurement>::count(); i++ )
    {
      const WindMeasurement& wm = at( i );

      altdiff = (int) rint( (alt - wm.altitude).getMeters() );
      timediff = wm.time.secsTo( now );

      if (altdiff > -altRange && altdiff < altRange && timediff < timeRange)
        {
          q_quality = wm.quality * REL_FACTOR_QUALITY / 5; //measurement quality

          // factor in altitude difference between current altitude and measurement.
          // Maximum altitude difference is 1000 m.
          a_quality = ((10*altRange) - (altdiff*altdiff/100)) * REL_FACTOR_ALTITUDE / (10*altRange);

          // factor in time difference. Maximum difference is 2 hours.
          timediff = (timeRange - timediff) / 10;

          // factor in time difference. Maximum difference is 2 hours.
          t_quality = (((timediff) * (timediff))) * REL_FACTOR_TIME / (72 * timeRange);

          quality = q_quality * a_quality * t_quality;

          // qDebug("i:%d q:%d w:%d/%f (%d, %d, %d)", i, quality, LimitedList<WindMeasurement>::at(i)->vector.getAngleDeg(),LimitedList<WindMeasurement>::at(i)->vector.getSpeed().getKph(), q_quality, a_quality, t_quality    );
          result.add( LimitedList<WindMeasurement>::value(i).vector * LimitedList<WindMeasurement>::value(i).quality );
          total_quality += LimitedList<WindMeasurement>::at(i).quality;
          // qDebug("Cur result: %d/%f (tQ: %d)",result.getAngleDeg(),result.getSpeed().getKph(),total_quality );
        }
  }

  if( total_quality > 0 )
    {
      result = result / (int) total_quality;
    }

  // qDebug("WindMeasurementList::getWind %d/%f ",result.getAngleDeg(),result.getSpeed().getKph() );

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
