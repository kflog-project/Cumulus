/***********************************************************************
 **
 **   variomodedialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2009 by Axel Pauli (axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QPushButton>

#include "vario.h"
#include "variomodedialog.h"
#include "calculator.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig *_globalMapConfig;

VarioModeDialog::VarioModeDialog(QWidget *parent) :
    QDialog( parent, Qt::WindowStaysOnTopHint )
{
  setObjectName("VarioModeDialog");
  setModal(true);
  setWindowTitle (tr("Vario"));

#ifndef MAEMO
  int minFontSize = 14;
#else
  int minFontSize = 20;
#endif

  QFont b = font();
  b.setBold(true);
  setFont(b);

  // set font size to a reasonable and useable value
  if( font().pointSize() < minFontSize )
    {
      QFont cf = font();
      cf.setPointSize( minFontSize );
      this->setFont(cf);
    }

  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setMargin(5);
  gridLayout->setHorizontalSpacing(10);

  stepGroup = new QGroupBox(tr("Time spin step width"), this);
  one = new QRadioButton(tr("1"), stepGroup);
  five = new QRadioButton(tr("5"), stepGroup);
  ten = new QRadioButton(tr("10"), stepGroup);
  one->setEnabled(true);
  five->setEnabled(true);
  ten->setEnabled(true);

  QHBoxLayout* radioLayout = new QHBoxLayout(stepGroup);
  radioLayout->addWidget(one);
  radioLayout->addWidget(five);
  radioLayout->addWidget(ten);

  int row = 0;
  gridLayout->addWidget(stepGroup, row++, 0, 1, 3);

  //---------------------------------------------------------------------

  QLabel* label = new QLabel(tr("Time:"), this);
  gridLayout->addWidget(label, row, 0);

  spinTime = new QSpinBox(this);
  spinTime->setButtonSymbols(QSpinBox::NoButtons);
  spinTime->setRange(5, 150);

  QPushButton *timePlus  = new QPushButton("+", this);
  timePlus->setMaximumWidth( timePlus->size().height() );
  timePlus->setMinimumWidth( timePlus->size().height() );
  connect(timePlus, SIGNAL(clicked()), this, SLOT(slot_timePlus()));

  QPushButton *timeMinus = new QPushButton("-", this);
  timeMinus->setMaximumWidth( timeMinus->size().height() );
  timeMinus->setMinimumWidth( timeMinus->size().height() );
  connect(timeMinus, SIGNAL(clicked()), this, SLOT(slot_timeMinus()));

  QHBoxLayout *tSpinLayout = new QHBoxLayout;
  tSpinLayout->setSpacing(0);
  tSpinLayout->addWidget(timePlus);
  tSpinLayout->addWidget(spinTime);
  tSpinLayout->addWidget(timeMinus);
  gridLayout->addLayout(tSpinLayout, row, 1);

  QLabel* unit = new QLabel(tr("s"), this);
  gridLayout->addWidget(unit, row++, 2);

  //---------------------------------------------------------------------

  QLabel* TekLbl = new QLabel(tr("TEK Mode:"), this);
  gridLayout->addWidget(TekLbl, row, 0);

  TEK = new QCheckBox (tr(""), this);
  gridLayout->addWidget(TEK, row++, 1);

  //---------------------------------------------------------------------

  TekAdj = new QLabel(tr("TEK Adjust:"), this);
  gridLayout->addWidget(TekAdj, row, 0);

  spinTEK = new QSpinBox( this );
  spinTEK->setRange( -100, 100 );
  spinTEK->setSingleStep( 1 );
  spinTEK->setButtonSymbols(QSpinBox::NoButtons);

  tekPlus  = new QPushButton("+", this);
  tekPlus->setMaximumWidth( tekPlus->size().height() );
  tekPlus->setMinimumWidth( tekPlus->size().height() );
  connect(tekPlus, SIGNAL(clicked()), this, SLOT(slot_tekPlus()));

  tekMinus = new QPushButton("-", this);
  tekMinus->setMaximumWidth( tekMinus->size().height() );
  tekMinus->setMinimumWidth( tekMinus->size().height() );
  connect(tekMinus, SIGNAL(clicked()), this, SLOT(slot_tekMinus()));

  QHBoxLayout *tekSpinLayout = new QHBoxLayout;
  tekSpinLayout->setSpacing(0);
  tekSpinLayout->addWidget(tekPlus);
  tekSpinLayout->addWidget(spinTEK);
  tekSpinLayout->addWidget(tekMinus);
  gridLayout->addLayout(tekSpinLayout, row, 1);

  QLabel* unit1 = new QLabel(tr("%"), this);
  gridLayout->addWidget(unit1, row++, 2);

  //---------------------------------------------------------------------

  // Align ok and cancel button at the left and right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  QPushButton *ok = new QPushButton(tr("OK"), this);
  QPushButton *cancel = new QPushButton (tr("Cancel"), this);

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( ok );
  butLayout->addStretch();
  butLayout->addWidget( cancel );

  gridLayout->addLayout(butLayout, row, 0, 1, 3);
  setLayout(gridLayout);

  timer = new QTimer(this);
  timer->setSingleShot(true);

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(timer, SIGNAL(timeout()), this, SLOT(reject()));
  connect(TEK, SIGNAL(toggled(bool)), this, SLOT(TekChanged(bool)));

  connect (ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancel, SIGNAL(clicked()), this, SLOT(reject()));

  connect(one, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(one, 0);
  connect(five, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(five, 1);
  connect(ten, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(ten, 2);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(change(int)));

  // restart timer, if value was changed
  connect(spinTime, SIGNAL(valueChanged(int)), this, SLOT(setTimer()));
  connect(spinTEK, SIGNAL(valueChanged(int)), this, SLOT(setTimer()));

  load();
}


VarioModeDialog::~VarioModeDialog()
{}


void VarioModeDialog::TekChanged(bool newState )
{
  TekAdj->setEnabled(newState);
  spinTEK->setEnabled(newState);
  tekPlus->setEnabled(newState);
  tekMinus->setEnabled(newState);
}


void VarioModeDialog::load()
{
  // qDebug ("VarioModeDialog::load()");
  GeneralConfig *conf = GeneralConfig::instance();

  _intTime = conf->getVarioIntegrationTime();

  if( _intTime < 3 ) // check config value
    {
      _intTime = INT_TIME; // reset to default
      conf->setVarioIntegrationTime(_intTime);
    }

  _TEKComp = conf->getVarioTekCompensation();
  emit newTEKMode( _TEKComp );

  _TEKAdjust = conf->getVarioTekAdjust();
  emit newTEKAdjust( _TEKAdjust );

  _curWidth = conf->getVarioStepWidth();
  // let us take the user's defined info display time
  _timeout = conf->getInfoDisplayTime();

  spinTEK->setEnabled(_TEKComp);
  spinTime->setValue( _intTime );
  spinTEK->setValue( _TEKAdjust );
  TEK->setChecked( _TEKComp );
  spinTime->setFocus();
  change(_curWidth);

  switch (_curWidth)
    {
    case 0:
      one->setChecked(true);
      break;
    case 1:
      five->setChecked(true);
      break;
    case 2:
      ten->setChecked(true);
      break;
    default:
      qFatal ("VarioModeDialog::load(): invalid width: %d", _curWidth);
    }

  setTimer();
}


void VarioModeDialog::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setVarioIntegrationTime( spinTime->value() );
  conf->setVarioStepWidth( _curWidth );
  // qDebug("Save new IntegrationTime %d", spinTime->value() );

  if( TEK->isChecked() != _TEKComp )
    {
      _TEKComp = TEK->isChecked();
      // qDebug("TEK Mode changed new val: %d", _TEKComp );
      conf->setVarioTekCompensation( _TEKComp );
      emit newTEKMode( _TEKComp );
    }

  if( spinTEK->value() != _TEKAdjust )
    {
      _TEKAdjust = spinTEK->value();
      // qDebug("TEK Adjust changed new val: %d", _TEKAdjust );
      conf->setVarioTekAdjust( _TEKAdjust );
      emit newTEKAdjust( _TEKAdjust );
    }

  emit newVarioTime( spinTime->value() );
  conf->save();
}


// adjust spin box the the new step width and the related minima.
void VarioModeDialog::change( int newStep )
{
  // qDebug("change to %d", newStep);
  _curWidth = newStep;

  int step = 1;
  int min = 0;

  switch(newStep)
    {
    case 0:
      step = 1;
      min = 3;
      break;
    case 1:
      step = 5;
      min = 5;
      break;
    case 2:
      step = 10;
      min = 10;
      break;
    default:
      step = 5;
      min = 5;
      break;
    }

  spinTime->setSingleStep(step);
  spinTime->setMinimum(min);
  setTimer();
}


void  VarioModeDialog::slot_timePlus()
{
  spinTime->setValue( spinTime->value() + spinTime->singleStep() );
}

void  VarioModeDialog::slot_timeMinus()
{
  spinTime->setValue( spinTime->value() - spinTime->singleStep() );
}

void  VarioModeDialog::slot_tekPlus()
{
  spinTEK->setValue( spinTEK->value() + spinTEK->singleStep() );
}

void  VarioModeDialog::slot_tekMinus()
{
  spinTEK->setValue( spinTEK->value() - spinTEK->singleStep() );
}

void VarioModeDialog::accept()
{
  save();
  QDialog::accept();
}


void VarioModeDialog::setTimer()
{
  if (_timeout > 0)
    {
      timer->start( _timeout * 1000 );
    }
}
