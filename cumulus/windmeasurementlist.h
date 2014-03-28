/***********************************************************************
**
**   windmeasurementlist.h
**
**   This file is part of Cumulus.
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

#ifndef WIND_MEASUREMENT_LIST_H
#define WIND_MEASUREMENT_LIST_H

#include <QTime>

#include "limitedlist.h"
#include "altitude.h"
#include "vector.h"

/**
 * \class WindMeasurement
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Data container for a single wind measurement.
 *
 * \date 2002-2010
 */
class WindMeasurement
{

public:

  Vector vector;
  int quality;
  QTime time;
  Altitude altitude;

  bool operator < (const WindMeasurement& other) const;

  static bool lessThan(const WindMeasurement &w1, const WindMeasurement &w2)
  {
    return w1.altitude < w2.altitude;
  };

};

/**
 * \class WindMeasurementList
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A list containing single wind measurements.
 *
 * \see LimitedList
 *
 * The WindMeasurementList is a list that contains and
 * processes wind measurements.
 *
 * \date 2002-2014
 */
class WindMeasurementList : public LimitedList<WindMeasurement>
{

public:

  WindMeasurementList();

  virtual ~WindMeasurementList();

  /**
   * Returns the weighted mean wind vector over the stored values, or 0
   * if no valid vector could be calculated (for instance: too little or
   * too low quality data).
   *
   * \param alt Altitude where wind is requested
   *
   * \param timeWindow Time window in seconds for wind search
   *
   * \param altRange Altitude range to be considered
   *
   * \return Vector containing found wind. Is set to invalid, if no wind was
   *         found.
   */
  Vector getWind( const Altitude& alt, const int timeWindow=0, const int altRange=0 );

  /** Adds the wind vector vector with quality quality to the list. */
  void addMeasurement( const Vector& vector, const Altitude& alt, int quality );

protected:
  /**
   * getLeastImportantItem is called to identify the item that should be
   * removed if the list is too full. Reimplemented from \ref LimitedList.
   */
  virtual int getLeastImportantItemIndex() const;
};

#endif
