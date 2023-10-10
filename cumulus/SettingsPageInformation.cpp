/***********************************************************************
**
**   settingspageinformation.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2003-2023 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>

#include "generalconfig.h"
#include "HelpBrowser.h"
#include "SettingsPageInformation.h"
#include "layout.h"
#include "MainWindow.h"
#include "mapdefaults.h"
#include "numberEditor.h"

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
  QScroller::grabGesture( sa->viewport(), QScroller::LeftMouseButtonGesture );

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  topLayout->setHorizontalSpacing(20 * Layout::getIntScaledDensity() );
  topLayout->setVerticalSpacing(10 * Layout::getIntScaledDensity() );

  int row=0;
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
  spinSuppress->setRange(0, 60);
  spinSuppress->setSpecialValueText(tr("Off"));
  spinSuppress->setSuffix( " min" );
  spinSuppress->setTip("0...60 min");
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  spinSuppress->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw = QFontMetrics(font()).horizontalAdvance(QString("999 min")) + 10;
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

  topLayout->addWidget( inverseInfoDisplay, row, 1, 1, 2 );
  row++;

  topLayout->setRowStretch ( row, 10 );
  topLayout->setColumnStretch( 2, 10 );

  connect( buttonReset, SIGNAL(clicked()), SLOT(slot_setFactoryDefault()) );

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
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

void SettingsPageInformation::slotHelp()
{
  QString file = "cumulus-settings-information.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
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
  numEd->setTip("0...60 s");
  numEd->setSpecialValueText(tr("Off"));
  numEd->setSuffix( " s" );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,2})" ), this );
  numEd->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw = QFontMetrics(font()).horizontalAdvance(QString("99 s")) + 10;
  numEd->setMinimumWidth( mw );

  return numEd;
}

void SettingsPageInformation::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

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
