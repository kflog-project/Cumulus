/***********************************************************************
 **
 **   variomodedialog.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004 by Axel Pauli (axel.pauli@onlinehome.de)
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QRadioButton>
#include <QFont>
#include <QGridLayout>

#include "vario.h"
#include "variomodedialog.h"
#include "cucalc.h"
#include "mapconfig.h"
#include "generalconfig.h"

extern MapConfig * _globalMapConfig;

VarioModeDialog::VarioModeDialog (QWidget *parent) :
  QDialog( parent, "VarioModeDialog", true,
           Qt::WStyle_StaysOnTop )
{
  setCaption (tr("Vario"));
  QGridLayout* topLayout = new QGridLayout(this, 8, 3, 5);

  QFont fnt( "Helvetica", 16, QFont::Bold  );
  this->setFont(fnt);
  resize( 200, 240 );

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

  stepGroup = new Q3ButtonGroup(1, Qt::Vertical, tr("Time step width"), this);

  QRadioButton* one  = new QRadioButton(tr("1"), stepGroup);
  QRadioButton* five = new QRadioButton(tr("5"), stepGroup);
  QRadioButton* ten  = new QRadioButton(tr("10"), stepGroup );


  one->setEnabled(true);
  five->setEnabled(true);
  ten->setEnabled(true);

  stepGroup->setButton(_curWidth);

  int row = 0;
  topLayout->addMultiCellWidget(stepGroup, row, row+1, 0, 2, Qt::AlignCenter);
  row+=2;

  QLabel* lbl = new QLabel(tr("Time:"), this);
  topLayout->addWidget(lbl, row, 0);

  spinTime = new QSpinBox( 3, 300, 1, this, "spinTime" );
  spinTime->setButtonSymbols(QSpinBox::PlusMinus);
  // spinTime->setButtonOrientation(Horizontal);
  topLayout->addWidget (spinTime, row, 1);

  QLabel* unit = new QLabel(tr("s"), this);
  topLayout->addWidget (unit, row++, 2);

  topLayout->addRowSpacing(row++, 10);

  QLabel* TekLbl = new QLabel(tr("TEK Mode:"), this);
  topLayout->addWidget(TekLbl, row, 0);

  TEK = new QCheckBox (tr(""), this);
  topLayout->addWidget (TEK, row++, 1);

  TekAdj = new QLabel(tr("TEK Adjust:"), this);
  topLayout->addWidget(TekAdj, row, 0);

  spinTEK = new QSpinBox( -100, 100, 1, this, "spinTEK" );
  spinTEK->setButtonSymbols(QSpinBox::PlusMinus);
  // spinTEK->setButtonOrientation(Horizontal);
  topLayout->addWidget (spinTEK, row++, 1);

  topLayout->addRowSpacing(row++, 10);

  buttonCancel = new QPushButton (tr("Cancel"), this);
  topLayout->addWidget (buttonCancel, row, 0);

  buttonOK = new QPushButton(tr("OK"), this);
  topLayout->addMultiCellWidget( buttonOK, row, row, 1, 2 );
  row++;

  // let us take the user's defined info display time
  _timeout = conf->getInfoDisplayTime();
  timer = new QTimer(this);

  connect(timer, SIGNAL(timeout()),
          this, SLOT(reject()));
  connect(stepGroup, SIGNAL(clicked(int)),
          this, SLOT(change(int)));
  connect(TEK, SIGNAL(toggled(bool)),
          this, SLOT(TekChanged(bool)));
  connect(buttonOK, SIGNAL(clicked()),
          this, SLOT(accept()));
  connect(buttonCancel, SIGNAL(clicked()),
          this, SLOT(reject()));

  // restart timer, if value was changed
  connect(spinTime, SIGNAL(valueChanged(int)),
          this, SLOT(setTimer()));
  connect(spinTEK, SIGNAL(valueChanged(int)),
          this, SLOT(setTimer()));
  connect(stepGroup, SIGNAL(clicked(int)),
          this, SLOT(setTimer()));
}


VarioModeDialog::~VarioModeDialog()
{}


void VarioModeDialog::TekChanged(bool newState )
{
  spinTEK->setEnabled(newState);
}


void VarioModeDialog::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  _intTime = conf->getVarioIntegrationTime();

  if( _intTime < 3 ) // check config value
    {
      _intTime = INT_TIME; // reset to default
      conf->setVarioIntegrationTime(_intTime);
    }

  spinTEK->setEnabled(_TEKComp);
  spinTime->setValue( _intTime );
  spinTEK->setValue( _TEKAdjust );
  TEK->setChecked( _TEKComp );
  spinTime->setFocus();
  change(_curWidth);
  show();
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
}


void VarioModeDialog::accept()
{
  save();
  QDialog::accept();
}


void VarioModeDialog::reject()
{
  QDialog::reject();
}


void VarioModeDialog::setTimer()
{
  if (_timeout > 0)
    timer->start( _timeout * 1000 );
}
