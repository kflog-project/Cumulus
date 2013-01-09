/***********************************************************************
**
**   settingspageinformation.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003-2013 by Axel Pauli (axel@kflog.org)
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

#ifdef USE_NUM_PAD
#include "numberEditor.h"
#else
#include "varspinbox.h"
#endif

SettingsPageInformation::SettingsPageInformation( QWidget *parent ) :
  QWidget(parent), m_loadConfig(true)
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

  topLayout->addWidget(new QLabel(tr("Airfield display time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinAirfield = createNumEd( this );
  topLayout->addWidget( spinAirfield, row, 1 );
#else
  spinAirfield = new QSpinBox;
  spinAirfield->setObjectName("spinAirfield");
  spinAirfield->setRange(0, 60);
  spinAirfield->setSuffix( " s" );
  spinAirfield->setSpecialValueText(tr("Off"));
  VarSpinBox* hspin = new VarSpinBox(spinAirfield);
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;
  topLayout->addWidget(new QLabel(tr("Airspace display time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinAirspace = createNumEd( this );
  topLayout->addWidget( spinAirspace, row, 1 );
#else
  spinAirspace = new QSpinBox;
  spinAirspace->setObjectName("spinAirspace");
  spinAirspace->setRange(0, 60);
  spinAirspace->setSuffix( " s" );
  spinAirspace->setSpecialValueText(tr("Off"));
  hspin = new VarSpinBox(spinAirspace);
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;
  topLayout->addWidget(new QLabel(tr("Info display time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinInfo = createNumEd( this );
  topLayout->addWidget( spinInfo, row, 1 );
#else
  spinInfo = new QSpinBox;
  spinInfo->setObjectName("spinInfo");
  spinInfo->setRange(0, 60);
  spinInfo->setSuffix( " s" );
  spinInfo->setSpecialValueText(tr("Off"));
  hspin = new VarSpinBox(spinInfo);
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;
  topLayout->addWidget(new QLabel(tr("Waypoint display time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinWaypoint = createNumEd( this );
  topLayout->addWidget( spinWaypoint, row, 1 );
#else
  spinWaypoint = new QSpinBox;
  spinWaypoint->setObjectName("spinWaypoint");
  spinWaypoint->setRange(0, 60);
  spinWaypoint->setSuffix( " s" );
  spinWaypoint->setSpecialValueText(tr("Off"));
  hspin = new VarSpinBox(spinWaypoint);
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;
  topLayout->addWidget(new QLabel(tr("Warning display time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinWarning = createNumEd( this );
  topLayout->addWidget( spinWarning, row, 1 );
#else
  spinWarning = new QSpinBox;
  spinWarning->setObjectName("spinWarning");
  spinWarning->setRange(0, 60);
  spinWarning->setSuffix( " s" );
  spinWarning->setSpecialValueText(tr("Off"));
  hspin = new VarSpinBox(spinWarning);
  topLayout->addWidget( hspin, row, 1 );
#endif

  row++;
  topLayout->addWidget(new QLabel(tr("Warning suppress time:"), this), row, 0);

#ifdef USE_NUM_PAD
  spinSuppress = new NumberEditor;
  spinSuppress->setDecimalVisible( false );
  spinSuppress->setPmVisible( false );
  spinSuppress->setMaxLength(3);
  spinSuppress->setRange(0, 900);
  spinSuppress->setSpecialValueText(tr("Off"));
  spinSuppress->setSuffix( " min" );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,3})" ), this );
  spinSuppress->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw = QFontMetrics(font()).width(QString("999 min")) + 10;
  spinSuppress->setMinimumWidth( mw );
  topLayout->addWidget( spinSuppress, row, 1 );
#else
  spinSuppress = new QSpinBox;
  spinSuppress->setObjectName("spinSuppress");
  spinSuppress->setMaximum(900);
  spinSuppress->setSuffix( " min" );
  spinSuppress->setSpecialValueText(tr("Off"));
  hspin = new VarSpinBox(spinSuppress);
  topLayout->addWidget( hspin, row, 1 );
#endif

  buttonReset = new QPushButton (tr("Defaults"), this);
  topLayout->addWidget( buttonReset, row, 2, Qt::AlignRight );
  row++;

  checkAlarmSound = new QCheckBox(tr("Alarm Sound"), this);
  checkAlarmSound->setObjectName("checkAlarmSound");
  checkAlarmSound->setChecked(true);
  topLayout->addWidget( checkAlarmSound, row, 0 );


  checkFlarmAlarms = new QCheckBox(tr("Flarm Alarms"), this);
  checkFlarmAlarms->setObjectName("checkFlarmAlarms");
  checkFlarmAlarms->setChecked(true);
  topLayout->addWidget( checkFlarmAlarms, row, 1, 1, 2 );
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

NumberEditor* SettingsPageInformation::createNumEd( QWidget* parent )
{
  NumberEditor* numEd = new NumberEditor( parent );
  numEd->setDecimalVisible( false );
  numEd->setPmVisible( false );
  numEd->setMaxLength(2);
  numEd->setRange(0, 60);
  numEd->setTip("0...60");
  numEd->setSpecialValueText(tr("Off"));
  numEd->setSuffix( " s" );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  numEd->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw = QFontMetrics(font()).width(QString("99 s")) + 10;
  numEd->setMinimumWidth( mw );

  return numEd;
}

void SettingsPageInformation::slot_load()
{
  // block multiple loads to avoid reset of changed values in the spin
  // boxes

  if( m_loadConfig )
    {
      m_loadConfig = false;
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
  checkFlarmAlarms->setChecked( conf->getPopupFlarmAlarms() );
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
  conf->setPopupFlarmAlarms( checkFlarmAlarms->isChecked() );
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
  checkFlarmAlarms->setChecked( true );
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
