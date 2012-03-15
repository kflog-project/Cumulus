/***********************************************************************
**
**   settingspageinformation.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003-2012 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "mapdefaults.h"
#include "settingspageinformation.h"
#include "varspinbox.h"

SettingsPageInformation::SettingsPageInformation( QWidget *parent ) :
  QWidget(parent), loadConfig(true)
{
  setObjectName("SettingsPageInformation");

  int row=0;
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setHorizontalSpacing(10);
  topLayout->setVerticalSpacing(10);

#ifndef ANDROID

  QHBoxLayout *hBox = new QHBoxLayout();

  QPushButton *soundSelection = new QPushButton( tr("Sound Player"), this );
  soundSelection->setToolTip(tr("Select a sound player, use %s if played file is enclosed in command line arguments"));
  hBox->addWidget(soundSelection);

  connect(soundSelection, SIGNAL( clicked()), this, SLOT(slot_openToolDialog()) );

  soundTool = new QLineEdit( this );
  hBox->addWidget(soundTool);
  topLayout->addLayout( hBox, row++, 0, 1, 3 );
  topLayout->setRowMinimumHeight( row++, 10 );

#endif

  VarSpinBox* hspin;

  topLayout->addWidget(new QLabel(tr("Airfield display time:"), this), row, 0);
  spinAirfield = new QSpinBox;
  spinAirfield->setObjectName("spinAirfield");
  spinAirfield->setMaximum(60);
  spinAirfield->setSuffix( " s" );
  hspin = new VarSpinBox(spinAirfield);
  topLayout->addWidget( hspin, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Airspace display time:"), this), row, 0);
  spinAirspace = new QSpinBox;
  spinAirspace->setObjectName("spinAirspace");
  spinAirspace->setMaximum(60);
  spinAirspace->setSuffix( " s" );
  hspin = new VarSpinBox(spinAirspace);
  topLayout->addWidget( hspin, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Info display time:"), this), row, 0);
  spinInfo = new QSpinBox;
  spinInfo->setObjectName("spinInfo");
  spinInfo->setMaximum(60);
  spinInfo->setSuffix( " s" );
  hspin = new VarSpinBox(spinInfo);
  topLayout->addWidget( hspin, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Waypoint display time:"), this), row, 0);
  spinWaypoint = new QSpinBox;
  spinWaypoint->setObjectName("spinWaypoint");
  spinWaypoint->setMaximum(60);
  spinWaypoint->setSuffix( " s" );
  hspin = new VarSpinBox(spinWaypoint);
  topLayout->addWidget( hspin, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Warning display time:"), this), row, 0);
  spinWarning = new QSpinBox;
  spinWarning->setObjectName("spinWarning");
  spinWarning->setMaximum(60);
  spinWarning->setSuffix( " s" );
  hspin = new VarSpinBox(spinWarning);
  topLayout->addWidget( hspin, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Warning suppress time:"), this), row, 0);
  spinSuppress = new QSpinBox;
  spinSuppress->setObjectName("spinSuppress");
  spinSuppress->setMaximum(600);
  spinSuppress->setSuffix( " min" );
  hspin = new VarSpinBox(spinSuppress);
  topLayout->addWidget( hspin, row, 1 );

  buttonReset = new QPushButton (tr("Defaults"), this);
  topLayout->addWidget( buttonReset, row, 2, Qt::AlignRight );
  row++;

  checkAlarmSound = new QCheckBox(tr("Alarm Sound"), this);
  checkAlarmSound->setObjectName("checkAlarmSound");
  checkAlarmSound->setChecked(true);
  topLayout->addWidget( checkAlarmSound, row, 0 );
  row++;

  calculateNearestSites = new QCheckBox(tr("Nearest Site Calculator"), this);
  calculateNearestSites->setObjectName("calcNearest");
  calculateNearestSites->setChecked(true);
  topLayout->addWidget( calculateNearestSites, row, 0 );
  row++;

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 2, 10 );

  connect( buttonReset, SIGNAL(clicked()),
           this, SLOT(slot_setFactoryDefault()));
}

SettingsPageInformation::~SettingsPageInformation()
{
}

void SettingsPageInformation::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)

  qApp->setAutoSipEnabled( false );
}

void SettingsPageInformation::hideEvent( QHideEvent *event )
{
  Q_UNUSED(event)

  qApp->setAutoSipEnabled( true );
}

void SettingsPageInformation::slot_load()
{
  // block multiple loads to avoid reset of changed values in the spin
  // boxes

  if( loadConfig )
    {
      loadConfig = false;
    }
  else
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

#ifndef ANDROID
  soundTool->setText( conf->getSoundPlayer() );
#endif

  spinAirfield->setValue( conf->getAirfieldDisplayTime() );
  spinAirspace->setValue( conf->getAirspaceDisplayTime() );
  spinInfo->setValue( conf->getInfoDisplayTime() );
  spinWaypoint->setValue( conf->getWaypointDisplayTime() );
  spinWarning->setValue( conf->getWarningDisplayTime() );
  spinSuppress->setValue( conf->getWarningSuppressTime() );
  checkAlarmSound->setChecked( conf->getAlarmSoundOn() );
  calculateNearestSites->setChecked( conf->getNearestSiteCalculatorSwitch() );
}

void SettingsPageInformation::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

#ifndef ANDROID
  conf->setSoundPlayer( soundTool->text() );
#endif

  conf->setAirfieldDisplayTime( spinAirfield->value() );
  conf->setAirspaceDisplayTime( spinAirspace->value() );
  conf->setInfoDisplayTime( spinInfo->value() );
  conf->setWaypointDisplayTime( spinWaypoint->value() );
  conf->setWarningDisplayTime( spinWarning->value() );
  conf->setWarningSuppressTime( spinSuppress->value() );
  conf->setAlarmSoundOn( checkAlarmSound->isChecked() );
  conf->setNearestSiteCalculatorSwitch( calculateNearestSites->isChecked() );
}

void SettingsPageInformation::slot_setFactoryDefault()
{
#ifndef ANDROID
  soundTool->setText("");
#endif

  spinAirfield->setValue(AIRFIELD_DISPLAY_TIME_DEFAULT);
  spinAirspace->setValue(AIRSPACE_DISPLAY_TIME_DEFAULT);
  spinInfo->setValue(INFO_DISPLAY_TIME_DEFAULT);
  spinWaypoint->setValue(WAYPOINT_DISPLAY_TIME_DEFAULT);
  spinWarning->setValue(WARNING_DISPLAY_TIME_DEFAULT);
  spinSuppress->setValue(WARNING_SUPPRESS_TIME_DEFAULT);
  checkAlarmSound->setChecked(ALARM_SOUND_DEFAULT);
  calculateNearestSites->setChecked(NEAREST_SITE_CALCULATOR_DEFAULT);
}

#ifndef ANDROID

void SettingsPageInformation::slot_openToolDialog()
{
  QString player = GeneralConfig::instance()->getSoundPlayer();

  if( player.isEmpty() )
    {
      player = QDir::homePath();
    }

  QString file = QFileDialog::getOpenFileName( this,
                                               tr("Please select a sound player"),
                                               player );
  if( file.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  soundTool->setText( file );
}

#endif
