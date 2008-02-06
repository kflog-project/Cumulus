/***********************************************************************
**
**   vario.h
**
**   This file is part of Cumulus.
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

#ifndef VARIO_H
#define VARIO_H

#include <QObject>
#include <QTimer>

#include "speed.h"

/**
 * @author Eggert Ehmke
 */
// default integration time in s for vario calculation

#define INT_TIME 20

class Vario: public QObject
{
    Q_OBJECT
public:

    Vario(QObject*);

    virtual ~Vario();

    void newAltitude();


public slots:
    /**
     * This slot is called, if the integration time has
     * been changed. Passes value in seconds.
     */
    void slotNewVarioTime(int newTime);

    void slotNewTEKMode(bool newMode);

    void slotNewTEKAdjust(int newAdjust);

    void slotNewAirspeed(const Speed& airspeed);


signals:
    void newVario(const Speed&);


private:
    QTimer _timeOut; // calling supervision timer
    int    _intTime; // integration time
    bool   _TEKOn;   // TEK compensated Mode
    double  _energyAlt; // v*v/2g
    double  _TekAdjust; // adjust TEK Compensation


private slots: // Private slots
    /**
     * This slot is called by the internal timer, to signal a
     * timeout. It resets the vario to initial.
     */
    void _slotTimeout();
};

#endif
