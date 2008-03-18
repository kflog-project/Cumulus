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
#include <QShortcut>
#include <QFont>

#include "polardialog.h"

PolarDialog::PolarDialog(const Polar* polar, QWidget* parent) :
  QDialog(parent)
{
    setObjectName("PolarDialog");
    setModal(true);
    
    _polar = const_cast<Polar*>(polar);

    setBackgroundColor (Qt::white);
    setWindowTitle (polar->name());
    setFont(QFont ( "Helvetica", 12, QFont::Bold ));

    QShortcut* rcUp =        new QShortcut(this);
    QShortcut* rcDown =      new QShortcut(this);
    QShortcut* rcShiftUp =   new QShortcut(this);
    QShortcut* rcShiftDown = new QShortcut(this);
    QShortcut* rcLeft =      new QShortcut(this);
    QShortcut* rcRight =     new QShortcut(this);
    QShortcut* rcSpace =     new QShortcut(this);
    rcUp->setKey        (Qt::Key_Up);
    rcDown->setKey      (Qt::Key_Down);
    rcShiftUp->setKey   (Qt::Key_Up + Qt::SHIFT);
    rcShiftDown->setKey (Qt::Key_Down + Qt::SHIFT);
    rcLeft->setKey      (Qt::Key_Left);
    rcRight->setKey     (Qt::Key_Right);
    rcSpace->setKey     (Qt::Key_Space);

    connect(rcUp,       SIGNAL(activated()),
            this,       SLOT(slot_keyup()));
    connect(rcDown,     SIGNAL(activated()),
            this,       SLOT(slot_keydown()));
    connect(rcShiftUp,  SIGNAL(activated()),
            this,       SLOT(slot_shiftkeyup()));
    connect(rcShiftDown,SIGNAL(activated()),
            this,       SLOT(slot_shiftkeydown()));
    connect(rcLeft,     SIGNAL(activated()),
            this,       SLOT(slot_keyleft()));
    connect(rcRight,    SIGNAL(activated()),
            this,       SLOT(slot_keyright()));
    connect(rcSpace,    SIGNAL(activated()),
            this,       SLOT(slot_keyhome()));
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
