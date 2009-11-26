/***********************************************************************
**
**   gliderflightdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>

#include "gliderflightdialog.h"

#include "calculator.h"
#include "glider.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig* _globalMapConfig;

// set static member variable
int GliderFlightDialog::noOfInstances = 0;

GliderFlightDialog::GliderFlightDialog (QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint)
{
  noOfInstances++;
  setObjectName("GliderFlightDialog");
  setModal(true);
  setWindowTitle (tr("Set Flight Parameters"));

  QGridLayout* topLayout = new QGridLayout(this);
  topLayout->setMargin(20);
  topLayout->setSpacing(25);

  hildonStyle = false;

  if( GeneralConfig::instance()->getGuiStyle() == "Hildon" )
    {
      // Maemo Qt has a special style, that required to use the normal spin boxes.
      hildonStyle = true;
    }

#ifndef MAEMO
  int minFontSize = 14;
#else
  int minFontSize = 20;
#endif

  QFont b = font();
  b.setBold(true);
  setFont(b);

  // set font size to a reasonable and usable value
  if( font().pointSize() < minFontSize )
    {
      QFont cf = font();
      cf.setPointSize( minFontSize );
      this->setFont(cf);
    }

  int row = 0;
  QLabel* lbl = new QLabel(tr("McCready:"), this);
  topLayout->addWidget(lbl,row,0);
  spinMcCready = new QDoubleSpinBox(this);
  spinMcCready->setRange(0.0, 20.0);
  spinMcCready->setSingleStep(0.5);
  spinMcCready->setSuffix(QString(" ") + Speed::getUnitText(Speed::getVerticalUnit()));

  if( hildonStyle == false )
    {
      spinMcCready->setButtonSymbols(QSpinBox::NoButtons);

      mcPlus  = new QPushButton("+", this);
      mcPlus->setMaximumWidth( mcPlus->size().height() );
      mcPlus->setMinimumWidth( mcPlus->size().height() );
      connect(mcPlus, SIGNAL(clicked()), this, SLOT(slotMcPlus()));

      mcMinus = new QPushButton("-", this);
      mcMinus->setMaximumWidth( mcMinus->size().height() );
      mcMinus->setMinimumWidth( mcMinus->size().height() );
      connect(mcMinus, SIGNAL(clicked()), this, SLOT(slotMcMinus()));

      QHBoxLayout *mcSpinLayout = new QHBoxLayout;
      mcSpinLayout->setSpacing(0);
      mcSpinLayout->addWidget(mcPlus);
      mcSpinLayout->addWidget(spinMcCready);
      mcSpinLayout->addWidget(mcMinus);

      topLayout->addLayout(mcSpinLayout, row++, 1);
    }
  else
    {
      topLayout->addWidget(spinMcCready, row++, 1);
    }

  //---------------------------------------------------------------------

  lbl = new QLabel(tr("Water:"), this);
  topLayout->addWidget(lbl,row,0);
  spinWater = new QSpinBox (this);
  spinWater->setRange(0, 200);
  spinWater->setSingleStep(5);
  spinWater->setSuffix( " l" );

  if( hildonStyle == false )
    {
      spinWater->setButtonSymbols(QSpinBox::NoButtons);

      waterPlus  = new QPushButton("+", this);
      waterPlus->setMaximumWidth( waterPlus->size().height() );
      waterPlus->setMinimumWidth( waterPlus->size().height() );
      connect(waterPlus, SIGNAL(clicked()), this, SLOT(slotWaterPlus()));

      waterMinus = new QPushButton("-", this);
      waterMinus->setMaximumWidth( waterMinus->size().height() );
      waterMinus->setMinimumWidth( waterMinus->size().height() );

      connect(waterMinus, SIGNAL(clicked()), this, SLOT(slotWaterMinus()));

      QHBoxLayout *waterSpinLayout = new QHBoxLayout;
      waterSpinLayout->setSpacing(0);
      waterSpinLayout->addWidget(waterPlus);
      waterSpinLayout->addWidget(spinWater);
      waterSpinLayout->addWidget(waterMinus);

      topLayout->addLayout(waterSpinLayout, row, 1);
    }
  else
    {
      topLayout->addWidget(spinWater, row, 1);
    }

  buttonDump = new QPushButton (tr("Dump"), this);
  topLayout->addWidget(buttonDump, row++, 2);

  //---------------------------------------------------------------------

  lbl = new QLabel(tr("Bugs:"), this);
  topLayout->addWidget(lbl,row, 0);
  spinBugs = new QSpinBox (this);
  spinBugs->setRange(0, 90);
  spinBugs->setSingleStep(1);
  spinBugs->setSuffix( " %" );

  if( hildonStyle == false )
    {
      spinBugs->setButtonSymbols(QSpinBox::NoButtons);

      bugsPlus  = new QPushButton("+", this);
      bugsPlus->setMaximumWidth( bugsPlus->size().height() );
      bugsPlus->setMinimumWidth( bugsPlus->size().height() );
      connect(bugsPlus, SIGNAL(clicked()), this, SLOT(slotBugsPlus()));

      bugsMinus = new QPushButton("-", this);
      bugsMinus->setMaximumWidth( bugsMinus->size().height() );
      bugsMinus->setMinimumWidth( bugsMinus->size().height() );
      connect(bugsMinus, SIGNAL(clicked()), this, SLOT(slotBugsMinus()));

      QHBoxLayout *bugsSpinLayout = new QHBoxLayout;
      bugsSpinLayout->setSpacing(0);
      bugsSpinLayout->addWidget(bugsPlus);
      bugsSpinLayout->addWidget(spinBugs);
      bugsSpinLayout->addWidget(bugsMinus);

      topLayout->addLayout(bugsSpinLayout, row++, 1);
    }
  else
    {
      topLayout->addWidget(spinBugs, row++, 1);
    }

  //---------------------------------------------------------------------

  topLayout->setRowMinimumHeight( row++, 20 );

  // Align ok and cancel button at the left and right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  ok = new QPushButton( tr("  OK  "), this);
  cancel = new QPushButton (tr("Cancel"), this);

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
{
  noOfInstances--;
}

void GliderFlightDialog::showEvent(QShowEvent *)
{
  double mc_max, mc_step;

  switch (Speed::getVerticalUnit())
    {
    case Speed::knots:
      mc_max = 40.0;
      mc_step = 0.5;
      break;
    case Speed::feetPerMinute:
      mc_max = 4000.0;
      mc_step = 50.0;
      break;
    case Speed::metersPerSecond:
      mc_max = 20.0;
      mc_step = 0.5;
      break;
    default:
      mc_max = 20.0;
      mc_step = 0.5;
    }

  spinMcCready->setMaximum(mc_max);
  spinMcCready->setSingleStep(mc_step);

  QSize sizeOk = ok->size();
  QSize sizeCancel = cancel->size();

  if( sizeCancel.width() > sizeOk.width() )
    {
      ok->resize( sizeCancel );
    }
  else if( sizeCancel.width() < sizeOk.width() )
    {
      cancel->resize( sizeCancel );
    }
}

void GliderFlightDialog::load()
{
  Glider * glider = calculator->glider();

  if (glider)
    {
      spinMcCready->setEnabled(true);
      spinWater->setEnabled(true);
      spinBugs->setEnabled(true);
      buttonDump->setEnabled(true);

      if( hildonStyle == false )
        {
          mcPlus->setEnabled(true);
          mcMinus->setEnabled(true);

          waterPlus->setEnabled(true);
          waterMinus->setEnabled(true);

          bugsPlus->setEnabled(true);
          bugsMinus->setEnabled(true);
        }

      spinWater->setMaximum(glider->maxWater());

      if (glider->maxWater() == 0)
        {
          spinWater->setEnabled(false);
          buttonDump->setEnabled(false);
        }

      spinMcCready->setValue(calculator->getlastMc().getVerticalValue());
      spinWater->setValue(glider->polar()->water());
      spinBugs->setValue(glider->polar()->bugs());
    }
  else
    {
      spinMcCready->setEnabled(false);
      spinWater->setEnabled(false);
      spinBugs->setEnabled(false);
      buttonDump->setEnabled(false);

      if( hildonStyle == false )
        {
          mcPlus->setEnabled(false);
          mcMinus->setEnabled(false);

          waterPlus->setEnabled(false);
          waterMinus->setEnabled(false);

          bugsPlus->setEnabled(false);
          bugsMinus->setEnabled(false);
        }
    }

  setTimer();
}


void GliderFlightDialog::save()
{
  Glider* glider = calculator->glider();

  if (glider)
    {
      glider->polar()->setWater(int(spinWater->value()), int(spinBugs->value()));
      Speed new_mc;
      new_mc.setVerticalValue(spinMcCready->value());
      calculator->slot_Mc(new_mc.getMps());
    }
}

void GliderFlightDialog::slotMcPlus()
{
  spinMcCready->setValue( spinMcCready->value() + spinMcCready->singleStep() );
}

void GliderFlightDialog::slotMcMinus()
{
  spinMcCready->setValue( spinMcCready->value() - spinMcCready->singleStep() );
}

void GliderFlightDialog::slotWaterPlus()
{
  spinWater->setValue( spinWater->value() + spinWater->singleStep() );
}

void GliderFlightDialog::slotWaterMinus()
{
  spinWater->setValue( spinWater->value() - spinWater->singleStep() );
}

void GliderFlightDialog::slotBugsPlus()
{
  spinBugs->setValue( spinBugs->value() + spinBugs->singleStep() );
}

void GliderFlightDialog::slotBugsMinus()
{
  spinBugs->setValue( spinBugs->value() - spinBugs->singleStep() );
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
  if ( _time > 0 )
    {
      timeout->start(_time * 1000);
    }
}
