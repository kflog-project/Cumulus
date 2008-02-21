/***********************************************************************
**
**   windmeasurementlist.cpp
**
**   This file is part of Cumulus
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

#include <cmath>

#include "windmeasurementlist.h"
#include "altitude.h"
#include "vector.h"
#include "generalconfig.h"

#define MAX_MEASUREMENTS 200 //maximum number of windmeasurements in the list. No idea what a sensible value would be...

WindMeasurementList::WindMeasurementList()
{
    // LimitedList<WindMeasurement>::setLimit(MAX_MEASUREMENTS);
    setLimit(MAX_MEASUREMENTS);
    //qDebug("WindMeasurementList::WindMeasurementList() %x", this);
}


WindMeasurementList::~WindMeasurementList()
{}


/**
 * Returns the weighted mean windvector over the stored values, or 0
 * if no valid vector could be calculated (for instance: too little or
 * too low quality data).
 */
Vector WindMeasurementList::getWind(Altitude alt)
{
    //relative weight for each factor
  #define REL_FACTOR_QUALITY 100
  #define REL_FACTOR_ALTITUDE 100
  #define REL_FACTOR_TIME 200

    GeneralConfig *conf = GeneralConfig::instance();
    int altRange  = conf->getWindAltitudeRange();
    int timeRange = conf->getWindTimeRange();

    Q_UINT32 total_quality=0;
    Q_UINT32 quality=0, q_quality=0, a_quality=0, t_quality=0;
    Vector result;
    WindMeasurement * m;
    QTime now=QTime::currentTime();
    int altdiff=0;
    int timediff=0;

    for(uint i=0;i<LimitedList<WindMeasurement>::count();i++) {
        m=LimitedList<WindMeasurement>::at(i);
        altdiff=(int) rint((alt - m->altitude).getMeters());
        timediff=m->time.secsTo(now);
        if (altdiff > -altRange && altdiff < altRange && timediff < timeRange) {
            q_quality = m->quality * REL_FACTOR_QUALITY / 5; //measurement quality
            a_quality = ((10*altRange) - (altdiff*altdiff/100))  * REL_FACTOR_ALTITUDE / (10*altRange); //factor in altitude difference between current altitude and measurement.  Maximum alt difference is 1000 m.
            //      t_quality = (5184 - ((timediff/100)*(timediff/100)))  * REL_FACTOR_TIME / 5184; //factor in timedifference. Maximum difference is 2 hours.
            timediff=(timeRange-timediff)/10;
            t_quality = (((timediff)*(timediff))) * REL_FACTOR_TIME / (72*timeRange); //factor in timedifference. Maximum difference is 2 hours.
            quality=q_quality * a_quality * t_quality;

            // qDebug("i:%d q:%d w:%d/%f (%d, %d, %d)", i, quality, LimitedList<WindMeasurement>::at(i)->vector.getAngleDeg(),LimitedList<WindMeasurement>::at(i)->vector.getSpeed().getKph(), q_quality, a_quality, t_quality    );
            result.add(LimitedList<WindMeasurement>::at(i)->vector * LimitedList<WindMeasurement>::at(i)->quality );
            total_quality+=LimitedList<WindMeasurement>::at(i)->quality;
            // qDebug("Cur result: %d/%f (tQ: %d)",result.getAngleDeg(),result.getSpeed().getKph(),total_quality );
        }
    }

    // qDebug( "======");

    if (total_quality>0) {
        result=result/(int)total_quality;
    }
    // qDebug("WindMeasurementList::getWind %d/%f ",result.getAngleDeg(),result.getSpeed().getKph() );

    return result;
}


/** Adds the windvector vector with quality quality to the list. */
void WindMeasurementList::addMeasurement(Vector vector, Altitude alt, int quality)
{
    WindMeasurement * wind = new WindMeasurement;
    wind->vector=vector;
    wind->quality=quality;
    wind->altitude=alt;
    wind->time=QTime::currentTime();
    append(wind);
    qSort(first(), last());
}


/**
 * getLeastImportantItem is called to identify the item that should be
 * removed if the list is too full. Reimplemented from LimitedList.
 */
uint WindMeasurementList::getLeastImportantItem()
{
    return LimitedList<WindMeasurement>::count();

    int maxscore=0;
    int score=0;
    uint founditem=LimitedList<WindMeasurement>::count()-1;

    for (int i=founditem;i>=0;i--) {
        //Calculate the score of this item. The item with the highest score is the least important one.
        //We may need to adjust the proportion of the quality and the elapsed time. Currently, one
        //quality-point (scale: 1 to 5) is equal to 10 minutes.
        score=600*(6-LimitedList<WindMeasurement>::at(i)->quality);
        score+=LimitedList<WindMeasurement>::at(i)->time.secsTo(QTime::currentTime());
        if (score>maxscore) {
            maxscore=score;
            founditem=i;
        }
    }
    qDebug("WindMeasurementList::getLeastImportantItem() %d", founditem );
    return founditem;
}


bool WindMeasurement::operator < (const WindMeasurement& other) const
{
    //return the difference between the altitudes in item 1 and item 2
    return (altitude < other.altitude);
}

