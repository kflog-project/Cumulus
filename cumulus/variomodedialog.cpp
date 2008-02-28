/***********************************************************************
 **
 **   variomodedialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004, 2008 by Axel Pauli (axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QDialogButtonBox>
#include <QFont>
#include <QGridLayout>
#include <QSignalMapper>

#include "vario.h"
#include "variomodedialog.h"
#include "cucalc.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig *_globalMapConfig;

VarioModeDialog::VarioModeDialog(QWidget *parent) :
  QDialog( parent, Qt::WStyle_StaysOnTop )
{
  setObjectName("VarioModeDialog");
  setModal(true);
  setWindowTitle (tr("Vario"));

  setFont(QFont ( "Helvetica", 16, QFont::Bold ));

  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setMargin(10);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  stepGroup = new QGroupBox(tr("Time step width"), this);

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

  gridLayout->addWidget(stepGroup, 0, 0, 1, 3);

  QLabel* label = new QLabel(tr("Time:"), this);
  gridLayout->addWidget(label, 1, 0, 1, 1);

  spinTime = new QSpinBox(this);
  spinTime->setButtonSymbols(QSpinBox::PlusMinus);
  gridLayout->addWidget(spinTime, 1, 1, 1, 1);

  QLabel* unit = new QLabel(tr("s"), this);
  gridLayout->addWidget(unit, 1, 2, 1, 1);

  QLabel* TekLbl = new QLabel(tr("TEK Mode:"), this);
  gridLayout->addWidget(TekLbl, 2, 0, 1, 1);

  TEK = new QCheckBox (tr(""), this);
  gridLayout->addWidget(TEK, 2, 1, 1, 1);

  TekAdj = new QLabel(tr("TEK Adjust:"), this);
  gridLayout->addWidget(TekAdj, 3, 0, 1, 1);

  spinTEK = new QSpinBox( -100, 100, 1, this, "spinTEK" );
  spinTEK->setButtonSymbols(QSpinBox::PlusMinus);
  gridLayout->addWidget(spinTEK, 3, 1, 1, 1);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                 | QDialogButtonBox::Cancel);
  gridLayout->addWidget(buttonBox, 4, 0, 1, 3);

  setLayout (gridLayout);

  timer = new QTimer(this);

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(timer, SIGNAL(timeout()),
          this, SLOT(reject()));
  connect(TEK, SIGNAL(toggled(bool)),
          this, SLOT(TekChanged(bool)));
  connect (buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect (buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  connect(one, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(one, 0);
  connect(five, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(five, 1);
  connect(ten, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(ten, 2);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(change(int)));

  // restart timer, if value was changed
  connect(spinTime, SIGNAL(valueChanged(int)),
          this, SLOT(setTimer()));
  connect(spinTEK, SIGNAL(valueChanged(int)),
          this, SLOT(setTimer()));
  load();
}


VarioModeDialog::~VarioModeDialog()
{}


void VarioModeDialog::TekChanged(bool newState )
{
  spinTEK->setEnabled(newState);
}


void VarioModeDialog::load()
{
  qDebug ("VarioModeDialog::load()");
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

  switch (_curWidth) {
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

  if( TEK->isChecked() != _TEKComp ) {
    _TEKComp = TEK->isChecked();
    // qDebug("TEK Mode changed new val: %d", _TEKComp );
    conf->setVarioTekCompensation( _TEKComp );
    emit newTEKMode( _TEKComp );
  }

  if( spinTEK->value() != _TEKAdjust ) {
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

  switch(newStep) {
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

  spinTime->setLineStep(step);
  spinTime->setMinValue(min);
  setTimer();
}


void VarioModeDialog::accept()
{
  save();
  QDialog::accept();
}


void VarioModeDialog::setTimer()
{
  if (_timeout > 0)
    timer->start( _timeout * 1000 );
}
