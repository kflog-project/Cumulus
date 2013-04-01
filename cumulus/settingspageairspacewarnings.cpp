/***********************************************************************
**
**   settingspageairspacewarnings.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "airspacewarningdistance.h"
#include "altitude.h"
#include "generalconfig.h"
#include "settingspageairspacewarnings.h"

SettingsPageAirspaceWarnings::SettingsPageAirspaceWarnings(QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint),
  m_autoSip( true )
{
  setObjectName("SettingsPageAirspaceWarnings");
  setAttribute( Qt::WA_DeleteOnClose );
  setModal(true);
  setSizeGripEnabled(true);
  setWindowTitle(tr("Airspace warning settings"));

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();
  QString unit = (altUnit == Altitude::meters) ? " m" : " ft";

  QVBoxLayout *topLayout = new QVBoxLayout(this);

  enableWarning = new QCheckBox(tr("Enable Warnings"), this);
  enableWarning->setObjectName("EnableWarnings");
  enableWarning->setChecked(true);
  enableWarning->setToolTip(tr("Switch on/off Airspace Warnings"));

  connect( enableWarning, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));
  topLayout->addWidget( enableWarning );

  // make the step width of the spin boxes configurable in different steps
  QGroupBox* stepGroup = new QGroupBox(tr("Spin step width"), this);
  s1 = new QRadioButton("1", stepGroup);
  s2 = new QRadioButton("10", stepGroup);
  s3 = new QRadioButton("100", stepGroup);
  s4 = new QRadioButton("1000", stepGroup);

  s1->setChecked(true);
  s1->setFocusPolicy(Qt::NoFocus);
  s2->setFocusPolicy(Qt::NoFocus);
  s3->setFocusPolicy(Qt::NoFocus);
  s4->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout* radioLayout = new QHBoxLayout(stepGroup);
  radioLayout->addWidget(s1);
  radioLayout->addWidget(s2);
  radioLayout->addWidget(s3);
  radioLayout->addWidget(s4);

  topLayout->addWidget(stepGroup);

#ifndef MAEMO5
  separations = new QGroupBox(tr("Distances"), this);
#else
  // The dialog widget is too small in MAEMO 5 for a group box.
  separations = new QWidget(this);
#endif

  topLayout->addWidget(separations);

  int row = 0;

  QGridLayout* mVGroupLayout = new QGridLayout(separations);
  mVGroupLayout->setRowMinimumHeight ( row++, 8 );

  // row 0
  QLabel* lbl;
  lbl = new QLabel(tr("Lateral"), separations);
  mVGroupLayout->addWidget(lbl, row, 1 );
  lbl = new QLabel(tr("Above"), separations);
  mVGroupLayout->addWidget(lbl, row, 2 );
  lbl = new QLabel(tr("Below"), separations);
  mVGroupLayout->addWidget(lbl, row, 3 );
  row++;

  // take a bold font for the plus and minus sign
  QFont bFont = font();
  bFont.setBold(true);

  // Create a plus and a minus button for spinbox operation
  plus  = new QPushButton("+");
  minus = new QPushButton("-");

  plus->setToolTip( tr("Increase number value") );
  minus->setToolTip( tr("Decrease number value") );

  plus->setFont(bFont);
  minus->setFont(bFont);

  // The buttons have no focus policy to avoid a focus change during click of them.
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);

  //row 1
  lbl = new QLabel(tr("Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  horiWarnDist = new QSpinBox(separations);
  horiWarnDist->setRange(0, 99999);
  horiWarnDist->setButtonSymbols(QSpinBox::NoButtons);
  horiWarnDist->setSuffix( unit );
  horiWarnDist->setWrapping(true);

  mVGroupLayout->addWidget(horiWarnDist, row, 1);

  aboveWarnDist = new QSpinBox(separations);
  aboveWarnDist->setRange(0, 99999);
  aboveWarnDist->setButtonSymbols(QSpinBox::NoButtons);
  aboveWarnDist->setSuffix( unit );
  aboveWarnDist->setWrapping(true);
  mVGroupLayout->addWidget(aboveWarnDist, row, 2);

  belowWarnDist = new QSpinBox(separations);
  belowWarnDist->setRange(0, 99999);
  belowWarnDist->setButtonSymbols(QSpinBox::NoButtons);
  belowWarnDist->setSuffix( unit );
  belowWarnDist->setWrapping(true);
  mVGroupLayout->addWidget(belowWarnDist, row, 3);
  mVGroupLayout->setColumnMinimumWidth( 4, 20 );
  mVGroupLayout->addWidget(plus, row, 5);
  row++;

  // row 2
  lbl = new QLabel(tr("Very Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  horiWarnDistVN = new QSpinBox(separations);
  horiWarnDistVN->setRange(0, 99999);
  horiWarnDistVN->setButtonSymbols(QSpinBox::NoButtons);
  horiWarnDistVN->setSuffix( unit );
  horiWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(horiWarnDistVN, row, 1);

  aboveWarnDistVN = new QSpinBox(separations);
  aboveWarnDistVN->setRange(0, 99999);
  aboveWarnDistVN->setButtonSymbols(QSpinBox::NoButtons);
  aboveWarnDistVN->setSuffix( unit );
  aboveWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(aboveWarnDistVN, row, 2);

  belowWarnDistVN = new QSpinBox(separations);
  belowWarnDistVN->setRange(0, 99999);
  belowWarnDistVN->setButtonSymbols(QSpinBox::NoButtons);
  belowWarnDistVN->setSuffix( unit );
  belowWarnDistVN->setWrapping(true);
  mVGroupLayout->addWidget(belowWarnDistVN, row, 3);
  mVGroupLayout->addWidget(minus, row, 5);
  row++;

  topLayout->addSpacing(20);
  topLayout->addStretch(10);

  defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );
  buttonBox->addButton( defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  topLayout->addWidget( buttonBox );

  connect(defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slot_save()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(s1, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s1, 1);
  connect(s2, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s2, 10);
  connect(s3, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s3, 100);
  connect(s4, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s4, 1000);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_change(int)));

  connect(plus, SIGNAL(pressed()), this, SLOT(slotIncrementBox()));
  connect(minus, SIGNAL(pressed()), this, SLOT(slotDecrementBox()));

  slot_load();

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );
}

SettingsPageAirspaceWarnings::~SettingsPageAirspaceWarnings()
{
  qApp->setAutoSipEnabled( m_autoSip );
}

void SettingsPageAirspaceWarnings::slotIncrementBox()
{
  if( ! plus->isDown() )
    {
      return;
    }

  QAbstractSpinBox* spinBoxList[] = {
      horiWarnDist,
      horiWarnDistVN,
      aboveWarnDist,
      aboveWarnDistVN,
      belowWarnDist,
      belowWarnDistVN
  };

  // Look which spin box has the focus.
  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] )
        {
          spinBoxList[i]->stepUp();
          spinBoxList[i]->setFocus();

          // Start repetition timer, to check, if button is longer pressed.
           QTimer::singleShot(250, this, SLOT(slotIncrementBox()));
          return;
        }
    }
}

void SettingsPageAirspaceWarnings::slotDecrementBox()
{
  if( ! minus->isDown() )
    {
      return;
    }

  QAbstractSpinBox* spinBoxList[] = {
      horiWarnDist,
      horiWarnDistVN,
      aboveWarnDist,
      aboveWarnDistVN,
      belowWarnDist,
      belowWarnDistVN
  };

  // Look which spin box has the focus.
  for( uint i = 0; i < (sizeof(spinBoxList) / sizeof(spinBoxList[0])); i++ )
    {
      if( QApplication::focusWidget() == spinBoxList[i] )
        {
          spinBoxList[i]->stepDown();
          spinBoxList[i]->setFocus();

          // Start repetition timer, to check, if button is longer pressed.
          QTimer::singleShot(250, this, SLOT(slotDecrementBox()));
          return;
        }
    }
}

/**
 * Called to change the step width of the spin boxes.
 */
void SettingsPageAirspaceWarnings::slot_change(int newStep)
{
  horiWarnDist->setSingleStep(newStep);
  horiWarnDistVN->setSingleStep(newStep);

  aboveWarnDist->setSingleStep(newStep);
  aboveWarnDistVN->setSingleStep(newStep);

  belowWarnDist->setSingleStep(newStep);
  belowWarnDistVN->setSingleStep(newStep);
}

void SettingsPageAirspaceWarnings::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd=conf->getAirspaceWarningDistances();
  bool enabled = conf->getAirspaceWarningEnabled();

  enableWarning->setChecked(enabled);
  slot_enabledToggled(enabled);

  if( altUnit == Altitude::meters )
    { // user wants meters
      horiWarnDist->setValue((int) rint(awd.horClose.getMeters()));
      horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getMeters()));

      aboveWarnDist->setValue((int) rint(awd.verAboveClose.getMeters()));
      aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getMeters()));

      belowWarnDist->setValue((int) rint(awd.verBelowClose.getMeters()));
      belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getMeters()));
    }
  else
    { // user gets feet
      horiWarnDist->setValue((int) rint(awd.horClose.getFeet()));
      horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getFeet()));

      aboveWarnDist->setValue((int) rint(awd.verAboveClose.getFeet()));
      aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getFeet()));

      belowWarnDist->setValue((int) rint(awd.verBelowClose.getFeet()));
      belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getFeet()));
    }

  // save loaded values for change control
  horiWarnDistValue = horiWarnDist->value();
  horiWarnDistVNValue = horiWarnDistVN->value();

  aboveWarnDistValue = aboveWarnDist->value();
  aboveWarnDistVNValue = aboveWarnDistVN->value();

  belowWarnDistValue = belowWarnDist->value();
  belowWarnDistVNValue = belowWarnDistVN->value();
}

/**
 * Called to set all spinboxes to the default value
 */
void SettingsPageAirspaceWarnings::slot_defaults()
{
  if( ! enableWarning->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  if( altUnit == Altitude::meters )
    { // user wants meters
      horiWarnDist->setValue( 2000 );
      horiWarnDistVN->setValue( 1000 );
      aboveWarnDist->setValue( 200 );
      aboveWarnDistVN->setValue( 100 );
      belowWarnDist->setValue( 200 );
      belowWarnDistVN->setValue( 100 );
    }
  else
    { // user gets feet
      horiWarnDist->setValue( 7000 );
      horiWarnDistVN->setValue( 3500 );
      aboveWarnDist->setValue( 700 );
      aboveWarnDistVN->setValue( 350 );
      belowWarnDist->setValue( 700 );
      belowWarnDistVN->setValue( 350 );
    }
}

void SettingsPageAirspaceWarnings::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd;

  conf->setAirspaceWarningEnabled(enableWarning->isChecked());

  // @AP: Store warning distances always as meters
  if( altUnit == Altitude::meters )
    {
      awd.horClose.setMeters( horiWarnDist->value() );
      awd.horVeryClose.setMeters( horiWarnDistVN->value() );
      awd.verAboveClose.setMeters( aboveWarnDist->value() );
      awd.verAboveVeryClose.setMeters( aboveWarnDistVN->value() );
      awd.verBelowClose.setMeters( belowWarnDist->value() );
      awd.verBelowVeryClose.setMeters( belowWarnDistVN->value() );
    }
  else
    {
      awd.horClose.setFeet( horiWarnDist->value() );
      awd.horVeryClose.setFeet( horiWarnDistVN->value() );
      awd.verAboveClose.setFeet( aboveWarnDist->value() );
      awd.verAboveVeryClose.setFeet( aboveWarnDistVN->value() );
      awd.verBelowClose.setFeet( belowWarnDist->value() );
      awd.verBelowVeryClose.setFeet( belowWarnDistVN->value() );
    }

  conf->setAirspaceWarningDistances( awd );

  accept();
}

void SettingsPageAirspaceWarnings::slot_enabledToggled( bool enabled )
{
  separations->setEnabled( enabled );
  defaults->setEnabled( enabled );
}
