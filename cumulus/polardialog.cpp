/***********************************************************************
**
**   polardialog.cpp
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

#include <QKeySequence>

#include "polardialog.h"

PolarDialog::PolarDialog(const Polar* polar, QWidget* parent)
        : QDialog (parent, "polardialog", true, Qt::WStyle_StaysOnTop)
{
    _polar = const_cast<Polar*>(polar);

    setBackgroundColor (Qt::white);
    setWindowTitle (polar->name());

    acc = new Q3Accel(this);

    acc->connectItem(acc->insertItem(Qt::Key_Up),
                     this, SLOT(slot_keyup()));
    acc->connectItem(acc->insertItem(Qt::Key_Down),
                     this, SLOT(slot_keydown()));
    acc->connectItem(acc->insertItem(Qt::Key_Up + Qt::SHIFT),
                     this, SLOT(slot_shiftkeyup()));
    acc->connectItem(acc->insertItem(Qt::Key_Down + Qt::SHIFT),
                     this, SLOT(slot_shiftkeydown()));
    acc->connectItem(acc->insertItem(Qt::Key_Left),
                     this, SLOT(slot_keyleft()));
    acc->connectItem(acc->insertItem(Qt::Key_Right),
                     this, SLOT(slot_keyright()));
    acc->connectItem(acc->insertItem(Qt::Key_Space),
                     this, SLOT(slot_keyhome()));
    show();
}


PolarDialog::~PolarDialog()
{}


void PolarDialog::slot_keyup()
{
    lift.setMps(lift.getMps()-0.1);
    repaint ();
}


void PolarDialog::slot_keydown()
{
    lift.setMps(lift.getMps()+0.1);
    repaint ();
}


void PolarDialog::slot_shiftkeyup()
{
    mc.setMps(mc.getMps()+0.5);
    repaint ();
}


void PolarDialog::slot_shiftkeydown()
{
    if (mc.getMps() > 0.01) {
        mc.setMps(mc.getMps()-0.5);
        repaint ();
    }
}


void PolarDialog::slot_keyleft()
{
    wind.setKph(wind.getKph()+5.0);
    repaint ();
}


void PolarDialog::slot_keyright()
{
    wind.setKph(wind.getKph()-5.0);
    repaint ();
}


void PolarDialog::slot_keyhome()
{
    wind.setKph(0.0);
    lift.setMps(0.0);
    repaint ();
}


void PolarDialog::paintEvent (QPaintEvent*)
{
    _polar->drawPolar(this, wind, lift, mc);
}
