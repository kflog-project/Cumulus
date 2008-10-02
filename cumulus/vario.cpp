/***********************************************************************
**
**   vario.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <stdlib.h>

#include "vario.h"
#include "altitude.h"
#include "calculator.h"
#include "generalconfig.h"

Vario::Vario(QObject* parent)
        :  QObject(parent)
{
    GeneralConfig *conf = GeneralConfig::instance();

    _intTime = conf->getVarioIntegrationTime() * 1000;

    _TEKOn = conf->getVarioTekCompensation();
    _TekAdjust = (100.0 + conf->getVarioTekAdjust()) / 100.0;

    // Timeout supervision of delivery of new altitude values
    connect( &_timeOut, SIGNAL(timeout()),
              this, SLOT(_slotTimeout()) );
}


Vario::~Vario()
{
    _timeOut.stop();
}


void Vario::newAltitude()
{
    // Start or restart the timer to supervise the calling of this
    // method. If timer expired the vario will be set to zero.
    _timeOut.setSingleShot(true);
    _timeOut.start( _intTime+2500 );

    if( calculator->samplelist.count() < 20 ) {
        // to less samples in the list
        return;
    }

    int i      = 1; // index for list access
    int max    = calculator->samplelist.count();
    double sum = 0.0;

    bool resultAvailable = false;

    // Step through the list. Note, the list is invers ordered, last sample at
    // first position.

    QTime startTime = calculator->samplelist.at(0).time;

    while( i < max ) {
        double energyAlt1 = 0.0;
        double energyAlt2 = 0.0;
        const flightSample *sample1 = &calculator->samplelist.at(i-1);
        const flightSample *sample2 = &calculator->samplelist.at(i);
        // calculate energy altitude for both samples
        if( _TEKOn ) {
            energyAlt1 = (sample1->airspeed.getMps() * sample1->airspeed.getMps())/(2*9.81);
            energyAlt2 = (sample2->airspeed.getMps() * sample2->airspeed.getMps())/(2*9.81);
        }

        // if( i == 2 )
        // qDebug("Airspeed %f  EnergyAltitude %f",sample1->airspeed.getKph(), energyAlt1 );

        int timeDist = sample2->time.msecsTo(startTime);

        if( timeDist > _intTime ) {
            // time difference to big
            break;
        }

        i++;
        double diff = (sample1->altitude.getMeters()+ energyAlt1*_TekAdjust) -
                      (sample2->altitude.getMeters()+ energyAlt2*_TekAdjust);

        int elapsed = sample2->time.msecsTo(sample1->time);

        sum += (1000.0 * diff / (double) elapsed);

        resultAvailable = true;

        // qDebug("Vario: max=%d, i=%d, diff=%f, elapsed=%dms, sum=%f",
        //	max, i, diff, elapsed, sum );
    }

    Speed lift(0);

    if( resultAvailable ) {
        lift.setMps( sum / (i-1) );
    }

    // qDebug ("New vario=%s, samples=%d", lift.getTextVertical(true, 3).latin1(), i );

    emit newVario(lift);
}


/** This slot is called by the internal timer, to signal a
    timeout. It resets the vario to initial. */
void Vario::_slotTimeout()
{
    // reset all to defaults, due to no new data have arrived over the
    // whole integration period and the measurement is senseless now.

    Speed lift(0);
    emit newVario(lift);
}


/** This slot is called, if the integration time has been changed. Passes
    value in seconds. */
void Vario::slotNewVarioTime( int newTime )
{
    qDebug("Vario::slotNewTime=%d", newTime );
    _intTime = newTime * 1000;
}


void Vario::slotNewTEKMode( bool newMode )
{
    qDebug("Vario::slotNewTEKMode=%d", newMode );
    _TEKOn = newMode;
}


void Vario::slotNewAirspeed(const Speed& /* airspeed */ )
{
    // qDebug("Vario::slotNewAirspeed");
}


void Vario::slotNewTEKAdjust(int adjust)
{
    qDebug("Vario::slotNewTEKAdjust");
    _TekAdjust = (double)((100.0+adjust)/100.0);
}

