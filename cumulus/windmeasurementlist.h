/***********************************************************************
**
**   windmeasurementlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andrï¿½ Somers, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WINDMEASUREMENTLIST_H
#define WINDMEASUREMENTLIST_H

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
 * process windmeasurements.
 * @author André Somers
 */
class WindMeasurementList : public LimitedList<WindMeasurement>
{
public:
    WindMeasurementList();
    virtual ~WindMeasurementList();

    /**
     * Returns the weighted mean windvector over the stored values, or 0
     * if no valid vector could be calculated (for instance: too little or
     * too low quality data).
     */
    Vector getWind(Altitude alt);

    /** Adds the windvector vector with quality quality to the list. */
    void addMeasurement(Vector vector, Altitude alt, int quality);

protected:
    /**
     * getLeastImportantItem is called to identify the item that should be
     * removed if the list is too full. Reimplemented from LimitedList.
     */
    virtual int getLeastImportantItemIndex() const;
};

#endif
