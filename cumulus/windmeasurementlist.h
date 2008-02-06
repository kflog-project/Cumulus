/***********************************************************************
**
**   windmeasurementlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2007 Axel Pauli
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
#include <Q3PtrCollection>

#include "limitedlist.h"
#include "altitude.h"
#include "vector.h"

/**
 * Contains a single wind measurement
 */
struct WindMeasurement
{
    Vector vector;
    int quality;
    QTime time;
    Altitude altitude;
};

/**
 * The WindMeasurementList is a list that can contain and
 * process windmeasurements.
 * @author André Somers
 */
class WindMeasurementList : protected LimitedList<WindMeasurement>
{
public:
    WindMeasurementList();
    ~WindMeasurementList();

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
    virtual uint getLeastImportantItem();

    /**
     * Compares two measurements by altitude.
     */
    int compareItems(Q3PtrCollection::Item s1, Q3PtrCollection::Item s2);

};

#endif
