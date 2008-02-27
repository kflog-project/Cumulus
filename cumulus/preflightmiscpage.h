/***********************************************************************
**
**   preflightmiscpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PREFLIGHTMISCPAGE_H
#define PREFLIGHTMISCPAGE_H

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>

#include "altitude.h"

/**
 * @short Miscelanious pre-flight settings
 * @author André Somers, Axel Pauli
 */
class PreFlightMiscPage : public QWidget
{
    Q_OBJECT

public:
    PreFlightMiscPage(QWidget *parent=0);

    virtual ~PreFlightMiscPage();

    void load();

    void save();

private:

    QCheckBox* chkLogAutoStart;

    QSpinBox* edtMinimalArrival;

    QSpinBox* edtQNH;

    QSpinBox* loggerInterval; // logger record interval in seconds

    /** saves altitude unit set during construction of object */
    Altitude::altitude altUnit;
};

#endif
