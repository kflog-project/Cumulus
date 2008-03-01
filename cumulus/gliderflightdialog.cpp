/***********************************************************************
**
**   gliderflightdialog.cpp
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

#include <QLabel>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>

#include "gliderflightdialog.h"

#include "cucalc.h"
#include "glider.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig * _globalMapConfig;

GliderFlightDialog::GliderFlightDialog (QWidget *parent) :
  QDialog(parent, Qt::WStyle_StaysOnTop)
{
  setObjectName("GliderFlightDialog");
  setModal(true);
  setWindowTitle (tr("Set Flight Parameters"));

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(5);

  QFont fnt( "Helvetica", 16, QFont::Bold  );
  this->setFont(fnt);

  int row = 0;
  QLabel* lbl = new QLabel(tr("McCready:"), this);
  topLayout->addWidget(lbl,row,0);
  spinMcCready = new QDoubleSpinBox(this);
  spinMcCready->setRange(0.0, 20.0);
  spinMcCready->setSingleStep(0.5);

  spinMcCready->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget (spinMcCready, row++, 1);
  // spinMcCready->setButtonOrientation(Horizontal);

  lbl = new QLabel(tr("Water:"), this);
  topLayout->addWidget(lbl,row,0);
  spinWater = new QSpinBox (0, 200, 5, this);
  spinWater->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget (spinWater, row, 1);
  spinWater->setMaximumHeight(25);

  buttonDump = new QPushButton (tr("Dump"), this);
  topLayout->addWidget(buttonDump, row++, 2);

  lbl = new QLabel(tr("Bugs:"), this);
  topLayout->addWidget(lbl,row,0);
  spinBugs = new QSpinBox (0, 90, 1, this);
  spinBugs->setButtonSymbols(QSpinBox::PlusMinus);
  spinBugs->setSingleStep(5);
  topLayout->addWidget (spinBugs, row++, 1);

  topLayout->addRowSpacing(row++, 10);

  // Align ok and cancel button at the left and right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  QPushButton *ok = new QPushButton(tr("OK"), this);
  QPushButton *cancel = new QPushButton (tr("Cancel"), this);

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( ok );
  butLayout->addStretch();
  butLayout->addWidget( cancel );

  topLayout->addLayout( butLayout, row++, 0, 1, 3 );

  // @AP: let us take the user's defined info display time
  GeneralConfig *conf = GeneralConfig::instance();
  timeout = new QTimer(this);
  _time = conf->getInfoDisplayTime();

  connect (timeout, SIGNAL(timeout()), this, SLOT(reject()));
  connect (buttonDump, SIGNAL(clicked()), this, SLOT(slotDump()));
  connect (buttonDump, SIGNAL(clicked()), this, SLOT(setTimer()));
  connect (ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancel, SIGNAL(clicked()), this, SLOT(reject()));

  connect (spinMcCready, SIGNAL(valueChanged(double)), this, SLOT(setTimer()));
  connect (spinWater, SIGNAL(valueChanged(int)), this, SLOT(setTimer()));
  connect (spinBugs, SIGNAL(valueChanged(int)), this, SLOT(setTimer()));
}


GliderFlightDialog::~GliderFlightDialog()
{}


void GliderFlightDialog::showEvent(QShowEvent *)
{
  double mc_max, mc_step;
  switch (Speed::getVerticalUnit()) {
  case Speed::knots:
    mc_max=40.0;
    mc_step=0.5;
    break;
  case Speed::feetPerMinute:
    mc_max=4000.0;
    mc_step=50.0;
    break;
  case Speed::metersPerSecond:
    mc_max=20.0;
    mc_step=0.5;
    break;
  default:
    mc_max=20.0;
    mc_step=0.5;
  }

  spinMcCready->setMaximum(mc_max);
  spinMcCready->setSingleStep(mc_step);
}


void GliderFlightDialog::load()
{
  spinMcCready->setEnabled(true);
  spinMcCready->setValue(calculator->getlastMc().getVerticalValue());
  spinWater->setEnabled(true);
  spinBugs->setEnabled(true);
  buttonDump->setEnabled(true);

  Glider * glider=calculator->glider();

  if (glider) {
    spinWater->setMaxValue(glider->maxWater());
    if (glider->maxWater()==0) {
      spinWater->setEnabled(false);
      buttonDump->setEnabled(false);
    }
    spinWater->setValue(glider->polar()->water());
    spinBugs->setValue(glider->polar()->bugs());
  } else {
    spinMcCready->setEnabled(false);
    spinWater->setEnabled(false);
    spinBugs->setEnabled(false);
    buttonDump->setEnabled(false);
  }

  setTimer();
}


void GliderFlightDialog::save()
{
  Glider * glider=calculator->glider();
  if (glider) {
    glider->polar()->setWater(int(spinWater->value()), int(spinBugs->value()));
    Speed new_mc;
    new_mc.setVerticalValue(spinMcCready->value());
    calculator->slot_Mc(new_mc.getMps());
  }
}


void GliderFlightDialog::slotDump()
{
  spinWater->setValue(0);
}


void GliderFlightDialog::accept()
{
  save();
  emit settingsChanged();
  QDialog::accept();
}


void GliderFlightDialog::setTimer()
{
  if (_time>0)
    {
      timeout->start(_time * 1000);
    }
}
