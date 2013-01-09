/***********************************************************************
**
**   settingspageairspacewarningsnumpad.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "airspacewarningdistance.h"
#include "altitude.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "numberEditor.h"
#include "settingspageairspacewarningsnumpad.h"

SettingsPageAirspaceWarningsNumPad::SettingsPageAirspaceWarningsNumPad(QWidget *parent) :
  QWidget(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("SettingsPageAirspaceWarningsNumPad");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setWindowTitle(tr("Airspace warning settings"));

  if( MainWindow::mainWindow() )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );
    }

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  m_altUnit = Altitude::getUnit();

  QVBoxLayout *topLayout = new QVBoxLayout(this);

  QGroupBox* warningGroup = new QGroupBox(this);
  QHBoxLayout* warningLayout = new QHBoxLayout(warningGroup);

  m_enableWarning = new QCheckBox(tr("Enable Warnings"));
  m_enableWarning->setChecked(true);
  connect( m_enableWarning, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));

  warningLayout->addWidget( m_enableWarning );
  topLayout->addWidget( warningGroup );

  m_distanceGroup = new QGroupBox(tr("Distances"), this);
  topLayout->addWidget(m_distanceGroup);

  int row = 0;

  QGridLayout* mVGroupLayout = new QGridLayout(m_distanceGroup);
  mVGroupLayout->setHorizontalSpacing(10);
  mVGroupLayout->setVerticalSpacing(10);
  mVGroupLayout->setRowMinimumHeight ( row++, 8 );

  // row 0
  QLabel* lbl;
  lbl = new QLabel(tr("Lateral"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 0 );
  lbl = new QLabel(tr("Above"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 1 );
  lbl = new QLabel(tr("Below"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 2 );
  row++;

  //row 1
  lbl = new QLabel(tr("Near"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 3, Qt::AlignLeft|Qt::AlignVCenter);

  m_horiWarnDist = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_horiWarnDist, row, 0);

  m_aboveWarnDist = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_aboveWarnDist, row, 1);

  m_belowWarnDist = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_belowWarnDist, row, 2);
  row++;

  // row 2
  lbl = new QLabel(tr("Very Near"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 3, Qt::AlignLeft|Qt::AlignVCenter);

  m_horiWarnDistVN = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_horiWarnDistVN, row, 0);

  m_aboveWarnDistVN = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_aboveWarnDistVN, row, 1);

  m_belowWarnDistVN = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_belowWarnDistVN, row, 2);
  row++;

  topLayout->addSpacing(20);
  topLayout->addStretch(10);

  m_defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );
  buttonBox->addButton( m_defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  topLayout->addWidget( buttonBox );

  connect(m_defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slot_save()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(slot_reject()));

  slot_load();
}

SettingsPageAirspaceWarningsNumPad::~SettingsPageAirspaceWarningsNumPad()
{
}

NumberEditor* SettingsPageAirspaceWarningsNumPad::createNumEd( QWidget* parent )
{
  QString unit = (m_altUnit == Altitude::meters) ? " m" : " ft";

  NumberEditor* numEd = new NumberEditor( parent );
  numEd->setDecimalVisible( false );
  numEd->setPmVisible( false );
  numEd->setMaxLength(7);
  numEd->setSuffix( unit );
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "([0-9]{1,7})" ), this );
  numEd->setValidator( eValidator );

  // Sets a minimum width for the widget
  int mw = QFontMetrics(font()).width("9999999 " + unit) + 10;
  numEd->setMinimumWidth( mw );

  return numEd;
}

void SettingsPageAirspaceWarningsNumPad::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd=conf->getAirspaceWarningDistances();
  bool enabled = conf->getAirspaceWarningEnabled();

  m_enableWarning->setChecked(enabled);
  slot_enabledToggled(enabled);

  if( m_altUnit == Altitude::meters )
    { // user wants meters
      m_horiWarnDist->setValue((int) rint(awd.horClose.getMeters()));
      m_horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getMeters()));

      m_aboveWarnDist->setValue((int) rint(awd.verAboveClose.getMeters()));
      m_aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getMeters()));

      m_belowWarnDist->setValue((int) rint(awd.verBelowClose.getMeters()));
      m_belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getMeters()));
    }
  else
    { // user gets feet
      m_horiWarnDist->setValue((int) rint(awd.horClose.getFeet()));
      m_horiWarnDistVN->setValue((int) rint(awd.horVeryClose.getFeet()));

      m_aboveWarnDist->setValue((int) rint(awd.verAboveClose.getFeet()));
      m_aboveWarnDistVN->setValue((int) rint(awd.verAboveVeryClose.getFeet()));

      m_belowWarnDist->setValue((int) rint(awd.verBelowClose.getFeet()));
      m_belowWarnDistVN->setValue((int) rint(awd.verBelowVeryClose.getFeet()));
    }

  // save loaded values for change control
  m_horiWarnDistValue    = m_horiWarnDist->value();
  m_horiWarnDistVNValue  = m_horiWarnDistVN->value();

  m_aboveWarnDistValue   = m_aboveWarnDist->value();
  m_aboveWarnDistVNValue = m_aboveWarnDistVN->value();

  m_belowWarnDistValue   = m_belowWarnDist->value();
  m_belowWarnDistVNValue = m_belowWarnDistVN->value();
}

/**
 * Called to set all spinboxes to the default value
 */
void SettingsPageAirspaceWarningsNumPad::slot_defaults()
{
  if( ! m_enableWarning->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  if( m_altUnit == Altitude::meters )
    { // user wants meters
      m_horiWarnDist->setValue( 2000 );
      m_horiWarnDistVN->setValue( 1000 );
      m_aboveWarnDist->setValue( 200 );
      m_aboveWarnDistVN->setValue( 100 );
      m_belowWarnDist->setValue( 200 );
      m_belowWarnDistVN->setValue( 100 );
    }
  else
    { // user gets feet
      m_horiWarnDist->setValue( 7000 );
      m_horiWarnDistVN->setValue( 3500 );
      m_aboveWarnDist->setValue( 700 );
      m_aboveWarnDistVN->setValue( 350 );
      m_belowWarnDist->setValue( 700 );
      m_belowWarnDistVN->setValue( 350 );
    }
}

void SettingsPageAirspaceWarningsNumPad::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();
  AirspaceWarningDistance awd;

  conf->setAirspaceWarningEnabled(m_enableWarning->isChecked());

  // @AP: Store warning distances always as meters
  if( m_altUnit == Altitude::meters )
    {
      awd.horClose.setMeters( m_horiWarnDist->value() );
      awd.horVeryClose.setMeters( m_horiWarnDistVN->value() );
      awd.verAboveClose.setMeters( m_aboveWarnDist->value() );
      awd.verAboveVeryClose.setMeters( m_aboveWarnDistVN->value() );
      awd.verBelowClose.setMeters( m_belowWarnDist->value() );
      awd.verBelowVeryClose.setMeters( m_belowWarnDistVN->value() );
    }
  else
    {
      awd.horClose.setFeet( m_horiWarnDist->value() );
      awd.horVeryClose.setFeet( m_horiWarnDistVN->value() );
      awd.verAboveClose.setFeet( m_aboveWarnDist->value() );
      awd.verAboveVeryClose.setFeet( m_aboveWarnDistVN->value() );
      awd.verBelowClose.setFeet( m_belowWarnDist->value() );
      awd.verBelowVeryClose.setFeet( m_belowWarnDistVN->value() );
    }

  conf->setAirspaceWarningDistances( awd );
  close();
}

void SettingsPageAirspaceWarningsNumPad::slot_reject()
{
  close();
}

void SettingsPageAirspaceWarningsNumPad::slot_enabledToggled( bool enabled )
{
  m_distanceGroup->setEnabled( enabled );
  m_defaults->setEnabled( enabled );
}
