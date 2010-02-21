/***********************************************************************
**
**   preflightmiscpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers
**                   2008-2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PREFLIGHT_MISCPAGE_H
#define PREFLIGHT_MISCPAGE_H

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

  private:

  Q_DISABLE_COPY ( PreFlightMiscPage )

public:

    PreFlightMiscPage(QWidget *parent=0);

    virtual ~PreFlightMiscPage();

    void load();

    void save();

private:

    QCheckBox* chkLogAutoStart;
    QSpinBox* edtMinimalArrival;
    QSpinBox* edtQNH;
    QSpinBox* bRecordInterval; // B-Record logging interval in seconds
    QSpinBox* kRecordInterval; // K-Record logging interval in seconds

    /** saves altitude unit set during construction of object */
    Altitude::altitude altUnit;
};

#endif
