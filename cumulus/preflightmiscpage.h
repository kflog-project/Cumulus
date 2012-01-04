/***********************************************************************
**
**   preflightmiscpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers
**                   2008-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightMiscPage
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A widget for preflight miscellaneous settings.
 *
 * \date 2004-2012
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_MISCPAGE_H
#define PREFLIGHT_MISCPAGE_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>

#include "altitude.h"

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

 private slots:

    /**
    * This slot increments the value in the spin box which has the current focus.
    */
    void slotIncrementBox();

    /**
    * This slot decrements the value in the spin box which has the current focus.
    */
    void slotDecrementBox();

    /**
     * This slot is called, when the focus changes to another widget. The old
     * focus widget is saved.
     */
    void slotFocusChanged( QWidget* oldWidget, QWidget* newWidget);

 private:

    QCheckBox*   chkLogAutoStart;
    QSpinBox*    edtMinimalArrival;
    QComboBox*   edtArrivalAltitude;
    QSpinBox*    edtQNH;
    QSpinBox*    bRecordInterval; // B-Record logging interval in seconds
    QSpinBox*    kRecordInterval; // K-Record logging interval in seconds
    QPushButton* plus;
    QPushButton* minus;

    // Widget, that held the last focus.
    QWidget*     lastFocusWidget;

    /** saves altitude unit set during construction of object */
    Altitude::altitudeUnit altUnit;
};

#endif
