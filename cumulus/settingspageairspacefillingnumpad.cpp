/***********************************************************************
**
**   settingspageairspacefillingnumpad.cpp
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

#include "altitude.h"
#include "generalconfig.h"
#include "numberEditor.h"
#include "mainwindow.h"
#include "map.h"
#include "settingspageairspacefillingnumpad.h"

SettingsPageAirspaceFillingNumPad::SettingsPageAirspaceFillingNumPad(QWidget *parent) :
  QWidget(parent, Qt::WindowStaysOnTopHint)
{
  setObjectName("SettingsPageAirspaceFillingNumPad");
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setWindowTitle(tr("Airspace fill settings"));

  if( MainWindow::mainWindow() )
    {
      // Resize the window to the same size as the main window has. That will
      // completely hide the parent window.
      resize( MainWindow::mainWindow()->size() );
    }

  QVBoxLayout * topLayout = new QVBoxLayout(this);

  QGroupBox* fillGroup = new QGroupBox(this);
  QHBoxLayout* fillLayout = new QHBoxLayout(fillGroup);

  m_enableFilling = new QCheckBox(tr("Enable filling"));
  m_enableFilling->setChecked(true);
  connect(m_enableFilling, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));

  fillLayout->addWidget( m_enableFilling );
  topLayout->addWidget(fillGroup);

  m_distanceGroup = new QGroupBox(tr("Distances"), this);
  topLayout->addWidget(m_distanceGroup);

  int row = 0;
  QGridLayout * mVGroupLayout = new QGridLayout(m_distanceGroup);
  // mVGroupLayout->setContentsMargins(0, 0, 0, 0);
  mVGroupLayout->setRowMinimumHeight ( row++, 8 );

  // row 0
  QLabel* lbl;
  lbl = new QLabel(tr("Not near"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 1);
  lbl = new QLabel(tr("Near"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 2);
  lbl = new QLabel(tr("Very near"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 3);
  lbl = new QLabel(tr("Inside"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 4);
  row++;

  // row 1
  lbl = new QLabel(tr("Vertical"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 0);

  m_verticalNotNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_verticalNotNear, row, 1);

  m_verticalNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_verticalNear, row, 2);

  m_verticalVeryNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_verticalVeryNear, row, 3);

  m_verticalInside = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_verticalInside, row, 4);

  mVGroupLayout->setColumnMinimumWidth( 5, 20 );
  row++;

  // row 2
  lbl = new QLabel(tr("Lateral"), m_distanceGroup);
  mVGroupLayout->addWidget(lbl, row, 0);

  m_lateralNotNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_lateralNotNear, row, 1);

  m_lateralNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_lateralNear, row, 2);

  m_lateralVeryNear = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_lateralVeryNear, row, 3);

  m_lateralInside = createNumEd( m_distanceGroup );
  mVGroupLayout->addWidget(m_lateralInside, row, 4);
  row++;

  topLayout->addSpacing(20);
  topLayout->addStretch(10);

  m_reset    = new QPushButton(tr("Reset"));
  m_defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );

  buttonBox->addButton( m_reset, QDialogButtonBox::ActionRole );
  buttonBox->addButton( m_defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  topLayout->addWidget( buttonBox );

  connect(m_reset,    SIGNAL(clicked()), this, SLOT(slot_reset()));
  connect(m_defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slot_save()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(slot_reject()));
  slot_load();
}

SettingsPageAirspaceFillingNumPad::~SettingsPageAirspaceFillingNumPad()
{
}

NumberEditor* SettingsPageAirspaceFillingNumPad::createNumEd( QWidget* parent )
{
  NumberEditor* numEd = new NumberEditor( parent );
  numEd->setDecimalVisible( false );
  numEd->setPmVisible( false );
  numEd->setMaxLength(3);
  numEd->setSuffix( " %" );
  numEd->setMaximum( 100 );
  numEd->setTitle("0...100");
  QRegExpValidator* eValidator = new QRegExpValidator( QRegExp( "(0|[1-9][0-9]{0,2})" ), this );
  numEd->setValidator( eValidator );

  return numEd;
}

void SettingsPageAirspaceFillingNumPad::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool enabled = conf->getAirspaceFillingEnabled();

  m_enableFilling->setChecked(enabled);
  slot_enabledToggled(enabled);

  m_verticalNotNear->setValue(conf->getAirspaceFillingVertical(Airspace::none));
  m_verticalNear->setValue(conf->getAirspaceFillingVertical(Airspace::near));
  m_verticalVeryNear->setValue(conf->getAirspaceFillingVertical(Airspace::veryNear));
  m_verticalInside->setValue(conf->getAirspaceFillingVertical(Airspace::inside));

  m_lateralNotNear->setValue(conf->getAirspaceFillingLateral(Airspace::none));
  m_lateralNear->setValue(conf->getAirspaceFillingLateral(Airspace::near));
  m_lateralVeryNear->setValue(conf->getAirspaceFillingLateral(Airspace::veryNear));
  m_lateralInside->setValue(conf->getAirspaceFillingLateral(Airspace::inside));
}

/**
 * Called to set all spin boxes to the default value
 */
void SettingsPageAirspaceFillingNumPad::slot_defaults()
{
  if( ! m_enableFilling->isChecked() )
    {
      // spin boxes are insensitive, do nothing
      return;
    }

  m_verticalNotNear->setValue(AS_FILL_NOT_NEAR);
  m_verticalNear->setValue(AS_FILL_NEAR);
  m_verticalVeryNear->setValue(AS_FILL_VERY_NEAR);
  m_verticalInside->setValue(AS_FILL_INSIDE);

  m_lateralNotNear->setValue(AS_FILL_NOT_NEAR);
  m_lateralNear->setValue(AS_FILL_NEAR);
  m_lateralVeryNear->setValue(AS_FILL_VERY_NEAR);
  m_lateralInside->setValue(AS_FILL_INSIDE);
}

/**
 * Called to reset all input fields to zero
 */
void SettingsPageAirspaceFillingNumPad::slot_reset()
{
  if( ! m_enableFilling->isChecked() )
    {
      // input fields are insensitive, do nothing
      return;
    }

  m_verticalNotNear->setValue(0);
  m_verticalNear->setValue(0);
  m_verticalVeryNear->setValue(0);
  m_verticalInside->setValue(0);

  m_lateralNotNear->setValue(0);
  m_lateralNear->setValue(0);
  m_lateralVeryNear->setValue(0);
  m_lateralInside->setValue(0);
}

void SettingsPageAirspaceFillingNumPad::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceFillingEnabled(m_enableFilling->isChecked());

  conf->setAirspaceFillingVertical(Airspace::none,     m_verticalNotNear->value());
  conf->setAirspaceFillingVertical(Airspace::near,     m_verticalNear->value());
  conf->setAirspaceFillingVertical(Airspace::veryNear, m_verticalVeryNear->value());
  conf->setAirspaceFillingVertical(Airspace::inside,   m_verticalInside->value());
  conf->setAirspaceFillingLateral(Airspace::none,      m_lateralNotNear->value());
  conf->setAirspaceFillingLateral(Airspace::near,      m_lateralNear->value());
  conf->setAirspaceFillingLateral(Airspace::veryNear,  m_lateralVeryNear->value());
  conf->setAirspaceFillingLateral(Airspace::inside,    m_lateralInside->value());

  // @AP: initiate a redraw of airspaces on the map due to color modifications.
  //      Not the best solution but it is working ;-)
  Map::getInstance()->scheduleRedraw(Map::airspaces);
  close();
}

void SettingsPageAirspaceFillingNumPad::slot_reject()
{
  close();
}

void SettingsPageAirspaceFillingNumPad::slot_enabledToggled(bool enabled)
{
  m_distanceGroup->setEnabled(enabled);
  m_reset->setEnabled(enabled);
  m_defaults->setEnabled(enabled);
}
