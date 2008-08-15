/***********************************************************************
**
**   settingspageinformation.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003, 2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFileDialog>

#include "generalconfig.h"
#include "mapdefaults.h"
#include "settingspageinformation.h"

SettingsPageInformation::SettingsPageInformation( QWidget *parent ) :
  QWidget(parent), loadConfig(true)
{
  setObjectName("SettingsPageInformation");

  int row=0;
  QGridLayout *topLayout = new QGridLayout( this );

  QHBoxLayout *hBox = new QHBoxLayout();

  QPushButton *soundSelection = new QPushButton( tr("Sound Player"), this );
  soundSelection->setToolTip(tr("Select a sound player, use %s if played file is encloed in command line arguments"));
  hBox->addWidget(soundSelection);

  connect(soundSelection, SIGNAL( clicked()), this, SLOT(slot_openToolDialog()) );

  soundTool = new QLineEdit( this );
  hBox->addWidget(soundTool);

  topLayout->addLayout( hBox, row++, 0, 1, 3 );

  topLayout->setRowMinimumHeight ( row++, 10 );

  topLayout->addWidget(new QLabel(tr("Airfield display time:"), this),row,0);
  spinAirfield = new QSpinBox(this);
  spinAirfield->setObjectName("spinAirfield");
  spinAirfield->setMaximum(60);
  spinAirfield->setButtonSymbols(QSpinBox::PlusMinus);
  spinAirfield->setSuffix( "s" );
  topLayout->addWidget( spinAirfield, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Airspace display time:"), this),row,0);
  spinAirspace = new QSpinBox(this);
  spinAirspace->setObjectName("spinAirspace");
  spinAirspace->setMaximum(60);
  spinAirspace->setButtonSymbols(QSpinBox::PlusMinus);
  spinAirspace->setSuffix( "s" );
  topLayout->addWidget( spinAirspace, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Info display time:"), this),row,0);
  spinInfo = new QSpinBox(this);
  spinInfo->setObjectName("spinInfo");
  spinInfo->setMaximum(60);
  spinInfo->setButtonSymbols(QSpinBox::PlusMinus);
  spinInfo->setSuffix( "s" );
  topLayout->addWidget( spinInfo, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Waypoint display time:"), this),row,0);
  spinWaypoint = new QSpinBox(this);
  spinWaypoint->setObjectName("spinWaypoint");
  spinWaypoint->setMaximum(60);
  spinWaypoint->setButtonSymbols(QSpinBox::PlusMinus);
  spinWaypoint->setSuffix( "s" );
  topLayout->addWidget( spinWaypoint, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Warning display time:"), this),row,0);
  spinWarning = new QSpinBox(this);
  spinWarning->setObjectName("spinWarning");
  spinWarning->setMaximum(60);
  spinWarning->setButtonSymbols(QSpinBox::PlusMinus);
  spinWarning->setSuffix( "s" );
  topLayout->addWidget( spinWarning, row, 1 );
  row++;

  topLayout->addWidget(new QLabel(tr("Warning suppress time:"), this),row,0);
  spinSuppress = new QSpinBox(this);
  spinSuppress->setObjectName("spinSuppress");
  spinSuppress->setMaximum(600);
  spinSuppress->setButtonSymbols(QSpinBox::PlusMinus);
  spinSuppress->setSuffix( "min" );
  topLayout->addWidget( spinSuppress, row, 1 );
  row++;

  checkAlarmSound = new QCheckBox(tr("Alarm Sound"), this);
  checkAlarmSound->setObjectName("checkAlarmSound");
  checkAlarmSound->setChecked(true);
  topLayout->addWidget( checkAlarmSound, row, 0 );
  row++;

  calculateNearestSites = new QCheckBox(tr("Nearest Site Calculator"),this);
  calculateNearestSites->setObjectName("calcNearest");
  calculateNearestSites->setChecked(true);
  topLayout->addWidget( calculateNearestSites, row, 0 );
  row++;

  checkAltimeterToggle = new QCheckBox(tr("Toggle Altimeter on tip"), this);
  checkAltimeterToggle->setObjectName("altimeterToggle");
  checkAltimeterToggle->setChecked(false);
  topLayout->addWidget( checkAltimeterToggle, row, 0 );
  row++,

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 2, 10 );

  buttonReset = new QPushButton (tr("Defaults"), this);
  topLayout->addWidget( buttonReset, row, 2, Qt::AlignRight );
  row++;

  connect( buttonReset, SIGNAL(clicked()),
           this, SLOT(slot_setFactoryDefault()));
}


SettingsPageInformation::~SettingsPageInformation()
{}


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

  soundTool->setText( conf->getSoundPlayer() );
  spinAirfield->setValue( conf->getAirfieldDisplayTime() );
  spinAirspace->setValue( conf->getAirspaceDisplayTime() );
  spinInfo->setValue( conf->getInfoDisplayTime() );
  spinWaypoint->setValue( conf->getWaypointDisplayTime() );
  spinWarning->setValue( conf->getWarningDisplayTime() );
  spinSuppress->setValue( conf->getWarningSuppressTime() );
  checkAlarmSound->setChecked( conf->getAlarmSoundOn() );
  calculateNearestSites->setChecked( conf->getNearestSiteCalculatorSwitch() );
  checkAltimeterToggle->setChecked( conf->getAltimeterToggleMode() );
}


void SettingsPageInformation::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSoundPlayer( soundTool->text() );
  conf->setAirfieldDisplayTime( spinAirfield->value() );
  conf->setAirspaceDisplayTime( spinAirspace->value() );
  conf->setInfoDisplayTime( spinInfo->value() );
  conf->setWaypointDisplayTime( spinWaypoint->value() );
  conf->setWarningDisplayTime( spinWarning->value() );
  conf->setWarningSuppressTime( spinSuppress->value() );
  conf->setAlarmSoundOn( checkAlarmSound->isChecked() );
  conf->setNearestSiteCalculatorSwitch( calculateNearestSites->isChecked() );
  conf->setAltimeterToggleMode( checkAltimeterToggle->isChecked() );
}


void SettingsPageInformation::slot_setFactoryDefault()
{
  soundTool->setText("");
  spinAirfield->setValue(AIRFIELD_DISPLAY_TIME_DEFAULT);
  spinAirspace->setValue(AIRSPACE_DISPLAY_TIME_DEFAULT);
  spinInfo->setValue(INFO_DISPLAY_TIME_DEFAULT);
  spinWaypoint->setValue(WAYPOINT_DISPLAY_TIME_DEFAULT);
  spinWarning->setValue(WARNING_DISPLAY_TIME_DEFAULT);
  spinSuppress->setValue(WARNING_SUPPRESS_TIME_DEFAULT);
  checkAlarmSound->setChecked(ALARM_SOUND_DEFAULT);
  calculateNearestSites->setChecked(NEAREST_SITE_CALCULATOR_DEFAULT);
  checkAltimeterToggle->setChecked( false );
}

void SettingsPageInformation::slot_openToolDialog()
{
  QString file = QFileDialog::getOpenFileName( this,
                                               tr("Please select a sound player"),
                                               QDir::homePath() );

  if( file.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  soundTool->setText( file );
}
