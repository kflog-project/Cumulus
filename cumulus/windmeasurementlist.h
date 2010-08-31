/***********************************************************************
**
**   windmeasurementlist.h
**
**   This file is part of Cumulus.
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

#ifndef WIND_MEASUREMENT_LIST_H
#define WIND_MEASUREMENT_LIST_H

#include <QTime>

#include "limitedlist.h"
#include "altitude.h"
#include "vector.h"

/**
 * Contains a single wind measurement
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
 * The WindMeasurementList is a list that can contain and
 * process wind measurements.
 * @author André Somers
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
     */
    Vector getWind( const Altitude& alt );

    /** Adds the wind vector vector with quality quality to the list. */
    void addMeasurement( const Vector& vector, const Altitude& alt, int quality );

protected:
    /**
     * getLeastImportantItem is called to identify the item that should be
     * removed if the list is too full. Reimplemented from LimitedList.
     */
    virtual int getLeastImportantItemIndex() const;
};

#endif
