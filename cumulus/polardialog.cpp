/***********************************************************************
**
**   polardialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "polardialog.h"
#include "mainwindow.h"

extern MainWindow  *_globalMainWindow;

PolarDialog::PolarDialog( Polar& polar, QWidget* parent) :
  QDialog(parent),
  _polar(polar)
{
  setObjectName("PolarDialog");
  setModal(true);

  if( _globalMainWindow )
    {
      // Resize the dialog to the same size as the main window has. That will
      // completely hide the parent window.
      resize( _globalMainWindow->size() );
    }

  QPalette palette;
  palette.setColor(backgroundRole(), Qt::white);
  setPalette(palette);
  setWindowTitle ( "Polar for " + polar.name() + " - <Esc> or Mouse click to Close");

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

  setVisible(true );
}

PolarDialog::~PolarDialog()
{
}

void PolarDialog::slot_keyup()
{
  lift.setMps( lift.getMps() + 0.1 );
  repaint();
}

void PolarDialog::slot_keydown()
{
  lift.setMps( lift.getMps() - 0.1 );
  repaint();
}

void PolarDialog::slot_shiftkeyup()
{
  mc.setMps( mc.getMps() + 0.5 );
  repaint();
}

void PolarDialog::slot_shiftkeydown()
{
  if( mc.getMps() > 0.01 )
    {
      mc.setMps( mc.getMps() - 0.5 );
      repaint();
    }
}

void PolarDialog::slot_keyleft()
{
  if( wind.getKph() > 100.0 )
    {
      return;
    }

  wind.setKph( wind.getKph() + 5.0 );
  repaint();
}

void PolarDialog::slot_keyright()
{
  if( wind.getKph() < -100.0 )
    {
      return;
    }

  wind.setKph(wind.getKph()-5.0);
  repaint();
}

void PolarDialog::slot_keyhome()
{
  wind.setKph(0.0);
  lift.setMps(0.0);
  repaint();
}

void PolarDialog::paintEvent (QPaintEvent*)
{
  _polar.drawPolar(this, wind, lift, mc);
}

// Close the dialog on mouse press. Needed by Maemo, there is no close
// button in the upper window frame
void PolarDialog::mousePressEvent( QMouseEvent* )
{
  QDialog::accept();
}
