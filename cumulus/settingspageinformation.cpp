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

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "layout.h"
#include "mapdefaults.h"
#include "numberEditor.h"
#include "settingspageinformation.h"

SettingsPageInformation::SettingsPageInformation( QWidget *parent ) :
  QWidget(parent)
{
  setObjectName("SettingsPageInformation");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Information") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  topLayout->setHorizontalSpacing(20);
  topLayout->setVerticalSpacing(10);

  int row=0;

#ifndef ANDROID

  QHBoxLayout *hBox = new QHBoxLayout();

  QPushButton *soundSelection = new QPushButton( tr("Sound Player"), this );
  soundSelection->setToolTip(tr("Select a sound player, use %s if played file is enclosed in command line arguments"));
  hBox->addWidget(soundSelection);

  connect(soundSelection, SIGNAL( clicked()), this, SLOT(slot_openToolDialog()) );

  Qt::InputMethodHints imh;

  soundTool = new QLineEdit( this );
  imh = (soundTool->inputMethodHints() | Qt::ImhNoPredictiveText);
  soundTool->setInputMethodHints(imh);

  hBox->addWidget(soundTool);
  topLayout->addLayout( hBox, row++, 0, 1, 3 );
  topLayout->setRowMinimumHeight( row++, 10 );

#endif

  topLayout->addWidget(new QLabel(tr("Airfield display time:"), this), row, 0);

  spinAirfield = createNumEd( this );
  topLayout->addWidget( spinAirfield, row, 1 );
  row++;
  topLayout->addWidget(new QLabel(tr("Airspace display time:"), this), row, 0);

  spinAirspace = createNumEd( this );
  topLayout->addWidget( spinAirspace, row, 1 );
  row++;
  topLayout->addWidget(new QLabel(tr("Info display time:"), this), row, 0);

  spinInfo = createNumEd( this );
  topLayout->addWidget( spinInfo, row, 1 );
  row++;
  topLayout->addWidget(new QLabel(tr("Waypoint display time:"), this), row, 0);

  spinWaypoint = createNumEd( this );
  topLayout->addWidget( spinWaypoint, row, 1 );
  row++;
  topLayout->addWidget(new QLabel(tr("Warning display time:"), this), row, 0);

  spinWarning = createNumEd( this );
  topLayout->addWidget( spinWarning, row, 1 );
  row++;
  topLayout->addWidget(new QLabel(tr("Warning suppress time:"), this), row, 0);

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

  buttonReset = new QPushButton (tr("Defaults"), this);
  topLayout->addWidget( buttonReset, row, 2, Qt::AlignLeft );
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

  inverseInfoDisplay = new QCheckBox(tr("Black Display"), this);
  inverseInfoDisplay->setObjectName("inverseDisplay");
  inverseInfoDisplay->setChecked(false);
#ifdef ANDROID
  // Dd not work on Android
  inverseInfoDisplay->setVisible(false);
#endif

  topLayout->addWidget( inverseInfoDisplay, row, 1, 1, 2 );
  row++;

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 2, 10 );

  connect( buttonReset, SIGNAL(clicked()), SLOT(slot_setFactoryDefault()) );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
}

SettingsPageInformation::~SettingsPageInformation()
{
}

void SettingsPageInformation::slotAccept()
{
  save();
  GeneralConfig::instance()->save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageInformation::slotReject()
{
  QWidget::close();
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

void SettingsPageInformation::load()
{
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
  inverseInfoDisplay->setChecked( conf->getBlackBgInfoDisplay() );
}

void SettingsPageInformation::save()
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
  conf->setBlackBgInfoDisplay( inverseInfoDisplay->isChecked() );
}

void SettingsPageInformation::slot_setFactoryDefault()
{
#ifndef ANDROID
  soundTool->setText( SoundPlayer );
#endif

  spinAirfield->setValue(AIRFIELD_DISPLAY_TIME_DEFAULT);
  spinAirspace->setValue(AIRSPACE_DISPLAY_TIME_DEFAULT);
  spinInfo->setValue(INFO_DISPLAY_TIME_DEFAULT);
  spinWaypoint->setValue(WAYPOINT_DISPLAY_TIME_DEFAULT);
  spinWarning->setValue(WARNING_DISPLAY_TIME_DEFAULT);
  spinSuppress->setValue(WARNING_SUPPRESS_TIME_DEFAULT);
  checkAlarmSound->setChecked(ALARM_SOUND_DEFAULT);
  checkFlarmAlarms->setChecked( true );
  inverseInfoDisplay->setChecked( false );
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
