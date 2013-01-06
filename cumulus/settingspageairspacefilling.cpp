/***********************************************************************
**
**   settingspageairspacefilling.cpp
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

#include "altitude.h"
#include "generalconfig.h"
#include "map.h"
#include "settingspageairspacefilling.h"

SettingsPageAirspaceFilling::SettingsPageAirspaceFilling(QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint),
  m_autoSip(true)
{
  setObjectName("SettingsPageAirspaceFilling");
  setAttribute( Qt::WA_DeleteOnClose );
  setModal(true);
  setSizeGripEnabled(true);
  setWindowTitle(tr("Airspace fill settings"));

  QVBoxLayout * topLayout = new QVBoxLayout(this);

  enableFilling = new QCheckBox(tr("Enable filling"), this);
  enableFilling->setToolTip(tr("Switch on/off Airspace filling"));

  connect(enableFilling, SIGNAL(toggled(bool)), SLOT(slot_enabledToggled(bool)));
  topLayout->addWidget(enableFilling);

  // make the step width of the spin boxes configurable in fixed steps
  QGroupBox* stepGroup = new QGroupBox(tr("Spin step width"), this);
  s1 = new QRadioButton(tr("1"), stepGroup);
  s2 = new QRadioButton(tr("5"), stepGroup);
  s3 = new QRadioButton(tr("10"), stepGroup);
  s4 = new QRadioButton(tr("20"), stepGroup);

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

  int row=0;
  QGridLayout * mVGroupLayout = new QGridLayout(separations);
  // mVGroupLayout->setContentsMargins(0, 0, 0, 0);
  mVGroupLayout->setRowMinimumHeight ( row++, 8 );

  // suffix % appended in spin boxes as unit
  QString spinboxSuffix = " %";

  // row 0
  QLabel* lbl;
  lbl = new QLabel(tr("Not near"), separations);
  mVGroupLayout->addWidget(lbl, row, 1);
  lbl = new QLabel(tr("Near"), separations);
  mVGroupLayout->addWidget(lbl, row, 2);
  lbl = new QLabel(tr("Very near"), separations);
  mVGroupLayout->addWidget(lbl, row, 3);
  lbl = new QLabel(tr("Inside"), separations);
  mVGroupLayout->addWidget(lbl, row, 4);
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

  // row 1
  lbl = new QLabel(tr("Vertical"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  verticalNotNear = new QSpinBox(separations);
  verticalNotNear->setRange( 0, 100 );
  verticalNotNear->setButtonSymbols(QSpinBox::NoButtons);
  verticalNotNear->setSuffix( spinboxSuffix );
  verticalNotNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalNotNear, row, 1);

  verticalNear = new QSpinBox(separations);
  verticalNear->setRange( 0, 100 );
  verticalNear->setButtonSymbols(QSpinBox::NoButtons);
  verticalNear->setSuffix( spinboxSuffix );
  verticalNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalNear, row, 2);

  verticalVeryNear = new QSpinBox(separations);
  verticalVeryNear->setRange( 0, 100 );
  verticalVeryNear->setButtonSymbols(QSpinBox::NoButtons);
  verticalVeryNear->setSuffix( spinboxSuffix );
  verticalVeryNear->setWrapping(true);
  mVGroupLayout->addWidget(verticalVeryNear, row, 3);

  verticalInside = new QSpinBox(separations);
  verticalInside->setRange( 0, 100 );
  verticalInside->setButtonSymbols(QSpinBox::NoButtons);
  verticalInside->setSuffix( spinboxSuffix );
  verticalInside->setWrapping(true);
  mVGroupLayout->addWidget(verticalInside, row, 4);
  mVGroupLayout->setColumnMinimumWidth( 5, 20 );
  mVGroupLayout->addWidget(plus, row, 6);
  row++;

  // row 2
  lbl = new QLabel(tr("Lateral"), separations);
  mVGroupLayout->addWidget(lbl, row, 0);

  lateralNotNear = new QSpinBox(separations);
  lateralNotNear->setRange( 0, 100 );
  lateralNotNear->setButtonSymbols(QSpinBox::NoButtons);
  lateralNotNear->setSuffix( spinboxSuffix );
  lateralNotNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralNotNear, row, 1);

  lateralNear = new QSpinBox(separations);
  lateralNear->setRange( 0, 100 );
  lateralNear->setButtonSymbols(QSpinBox::NoButtons);
  lateralNear->setSuffix( spinboxSuffix );
  lateralNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralNear, row, 2);

  lateralVeryNear = new QSpinBox(separations);
  lateralVeryNear->setRange( 0, 100 );
  lateralVeryNear->setButtonSymbols(QSpinBox::NoButtons);
  lateralVeryNear->setSuffix( spinboxSuffix );
  lateralVeryNear->setWrapping(true);
  mVGroupLayout->addWidget(lateralVeryNear, row, 3);

  lateralInside = new QSpinBox(separations);
  lateralInside->setRange( 0, 100 );
  lateralInside->setButtonSymbols(QSpinBox::NoButtons);
  lateralInside->setSuffix( spinboxSuffix );
  lateralInside->setWrapping(true);
  mVGroupLayout->addWidget(lateralInside, row, 4);
  mVGroupLayout->addWidget(minus, row, 6);
  row++;

  topLayout->addSpacing(20);
  topLayout->addStretch(10);

  reset    = new QPushButton(tr("Reset"));
  defaults = new QPushButton(tr("Default"));

  QDialogButtonBox* buttonBox = new QDialogButtonBox( Qt::Horizontal );

  buttonBox->addButton( reset, QDialogButtonBox::ActionRole );
  buttonBox->addButton( defaults, QDialogButtonBox::ActionRole );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  topLayout->addWidget( buttonBox );

  connect(reset,    SIGNAL(clicked()), this, SLOT(slot_reset()));
  connect(defaults, SIGNAL(clicked()), this, SLOT(slot_defaults()));

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(slot_save()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QSignalMapper* signalMapper = new QSignalMapper(this);

  connect(s1, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s1, 1);
  connect(s2, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s2, 5);
  connect(s3, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s3, 10);
  connect(s4, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(s4, 20);
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slot_change(int)));

  connect(plus, SIGNAL(pressed()), this, SLOT(slotIncrementBox()));
  connect(minus, SIGNAL(pressed()), this, SLOT(slotDecrementBox()));

  slot_load();

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );
}

SettingsPageAirspaceFilling::~SettingsPageAirspaceFilling()
{
  qApp->setAutoSipEnabled( m_autoSip );
}

void SettingsPageAirspaceFilling::slotIncrementBox()
{
  if( ! plus->isDown() )
    {
      return;
    }

  // Look which spin box has the focus.
  QAbstractSpinBox* spinBoxList[] = {
     verticalNotNear,
     verticalNear,
     verticalVeryNear,
     verticalInside,
     lateralNotNear,
     lateralNear,
     lateralVeryNear,
     lateralInside
  };

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

void SettingsPageAirspaceFilling::slotDecrementBox()
{
  if( ! minus->isDown() )
    {
      return;
    }

  // Look which spin box has the focus.
  QAbstractSpinBox* spinBoxList[] = {
     verticalNotNear,
     verticalNear,
     verticalVeryNear,
     verticalInside,
     lateralNotNear,
     lateralNear,
     lateralVeryNear,
     lateralInside
  };

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
void SettingsPageAirspaceFilling::slot_change(int newStep)
{
  verticalNotNear->setSingleStep(newStep);
  verticalNear->setSingleStep(newStep);
  verticalVeryNear->setSingleStep(newStep);
  verticalInside->setSingleStep(newStep);

  lateralNotNear->setSingleStep(newStep);
  lateralNear->setSingleStep(newStep);
  lateralVeryNear->setSingleStep(newStep);
  lateralInside->setSingleStep(newStep);
}

void SettingsPageAirspaceFilling::slot_load()
{
  GeneralConfig * conf = GeneralConfig::instance();
  bool enabled = conf->getAirspaceFillingEnabled();

  enableFilling->setChecked(enabled);
  slot_enabledToggled(enabled);

  verticalNotNear->setValue(conf->getAirspaceFillingVertical(Airspace::none));
  verticalNear->setValue(conf->getAirspaceFillingVertical(Airspace::near));
  verticalVeryNear->setValue(conf->getAirspaceFillingVertical(Airspace::veryNear));
  verticalInside->setValue(conf->getAirspaceFillingVertical(Airspace::inside));

  lateralNotNear->setValue(conf->getAirspaceFillingLateral(Airspace::none));
  lateralNear->setValue(conf->getAirspaceFillingLateral(Airspace::near));
  lateralVeryNear->setValue(conf->getAirspaceFillingLateral(Airspace::veryNear));
  lateralInside->setValue(conf->getAirspaceFillingLateral(Airspace::inside));
}

/**
 * Called to set all spin boxes to the default value
 */
void SettingsPageAirspaceFilling::slot_defaults()
{
  if( ! enableFilling->isChecked() )
    {
      // spin boxes are insensitive, do nothing
      return;
    }

  verticalNotNear->setValue(AS_FILL_NOT_NEAR);
  verticalNear->setValue(AS_FILL_NEAR);
  verticalVeryNear->setValue(AS_FILL_VERY_NEAR);
  verticalInside->setValue(AS_FILL_INSIDE);

  lateralNotNear->setValue(AS_FILL_NOT_NEAR);
  lateralNear->setValue(AS_FILL_NEAR);
  lateralVeryNear->setValue(AS_FILL_VERY_NEAR);
  lateralInside->setValue(AS_FILL_INSIDE);
}

/**
 * Called to reset all spinboxes to zero
 */
void SettingsPageAirspaceFilling::slot_reset()
{
  if( ! enableFilling->isChecked() )
    {
      // spinboxes are insensitive, do nothing
      return;
    }

  verticalNotNear->setValue(0);
  verticalNear->setValue(0);
  verticalVeryNear->setValue(0);
  verticalInside->setValue(0);

  lateralNotNear->setValue(0);
  lateralNear->setValue(0);
  lateralVeryNear->setValue(0);
  lateralInside->setValue(0);
}

void SettingsPageAirspaceFilling::slot_save()
{
  GeneralConfig * conf = GeneralConfig::instance();

  conf->setAirspaceFillingEnabled(enableFilling->isChecked());

  conf->setAirspaceFillingVertical(Airspace::none,     verticalNotNear->value());
  conf->setAirspaceFillingVertical(Airspace::near,     verticalNear->value());
  conf->setAirspaceFillingVertical(Airspace::veryNear, verticalVeryNear->value());
  conf->setAirspaceFillingVertical(Airspace::inside,   verticalInside->value());
  conf->setAirspaceFillingLateral(Airspace::none,      lateralNotNear->value());
  conf->setAirspaceFillingLateral(Airspace::near,      lateralNear->value());
  conf->setAirspaceFillingLateral(Airspace::veryNear,  lateralVeryNear->value());
  conf->setAirspaceFillingLateral(Airspace::inside,    lateralInside->value());

  // @AP: initiate a redraw of airspaces on the map due to color modifications.
  //      Not the best solution but it is working ;-)
  Map::getInstance()->scheduleRedraw(Map::airspaces);

  accept();
}

void SettingsPageAirspaceFilling::slot_enabledToggled(bool enabled)
{
  separations->setEnabled(enabled);
  reset->setEnabled(enabled);
  defaults->setEnabled(enabled);
}
