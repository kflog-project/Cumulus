/***********************************************************************
 **
 **   preflightmiscpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by André Somers
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>

#include <QtGui>

#include "preflightmiscpage.h"

#include "calculator.h"
#include "generalconfig.h"
#include "layout.h"
#include "logbook.h"
#include "igclogger.h"
#include "varspinbox.h"

#ifdef FLARM
#include "flarm.h"
#include "flarmlogbook.h"
#include "gpsnmea.h"
#endif

PreFlightMiscPage::PreFlightMiscPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightMiscPage");

  QGridLayout *topLayout = new QGridLayout(this);
  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"));
  topLayout->addWidget(lbl, row, 0);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();

  VarSpinBox* hspin;

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
  edtMinimalArrival = new QSpinBox;

  if (altUnit == Altitude::meters)
    {
      edtMinimalArrival->setMaximum(1000);
      edtMinimalArrival->setSingleStep(10);
    }
  else
    {
      edtMinimalArrival->setMaximum(3000);
      edtMinimalArrival->setSingleStep(100);
    }

  edtMinimalArrival->setSuffix(" " + Altitude::getUnitText());
  hspin = new VarSpinBox(edtMinimalArrival);
  topLayout->addWidget(hspin, row, 1);
  topLayout->setColumnStretch(2, 2);
  row++;

  lbl = new QLabel(tr("Arrival altitude display:"));
  topLayout->addWidget(lbl, row, 0);
  edtArrivalAltitude = new QComboBox;
  edtArrivalAltitude->addItem( tr("Landing Target"), GeneralConfig::landingTarget );
  edtArrivalAltitude->addItem( tr("Next Target"), GeneralConfig::nextTarget );
  topLayout->addWidget(edtArrivalAltitude, row, 1);
  row++;

  lbl = new QLabel(tr("QNH:"));
  topLayout->addWidget(lbl, row, 0);
  edtQNH = new QSpinBox(this);
  edtQNH->setObjectName("QNH");
  edtQNH->setMaximum(1999);
  edtQNH->setSuffix(" hPa");
  hspin = new VarSpinBox(edtQNH);
  topLayout->addWidget(hspin, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 25);
  row++;

  chkLogAutoStart = new QCheckBox(tr("Autostart IGC logger"));
  topLayout->addWidget(chkLogAutoStart, row, 0 );

  QPushButton* button = new QPushButton( tr("Logbook") );
  topLayout->addWidget(button, row, 1 );
  row++;

  connect(button, SIGNAL(pressed()), SLOT(slotOpenLogbook()));

  lbl = new QLabel(tr("B-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  bRecordInterval = new QSpinBox(this);
  bRecordInterval->setMinimum(1);
  bRecordInterval->setMaximum(60);
  bRecordInterval->setSuffix(" s");
  hspin = new VarSpinBox(bRecordInterval);
  topLayout->addWidget(hspin, row, 1);
  row++;

  lbl = new QLabel(tr("K-Record Interval:"));
  topLayout->addWidget(lbl, row, 0);
  kRecordInterval = new QSpinBox(this);
  kRecordInterval->setMinimum(0);
  kRecordInterval->setMaximum(300);
  kRecordInterval->setSpecialValueText(tr("Off"));
  kRecordInterval->setSuffix(" s");
  hspin = new VarSpinBox(kRecordInterval);
  topLayout->addWidget(hspin, row, 1);
  row++;

#ifdef FLARM

  lbl = new QLabel(tr("Flarm flight book:"));
  topLayout->addWidget(lbl, row, 0);
  button = new QPushButton( tr("Open") );
  topLayout->addWidget(button, row, 1 );
  row++;

  extern Calculator *calculator;

  if( calculator->moving() )
    {
      // Disable Flarm flight downloads if we are moving.
      button->setEnabled( false );
    }
  else
    {
      connect(button, SIGNAL(pressed()), SLOT(slotOpenFlarmFlights()));
    }

#endif

  topLayout->setRowStretch(row, 10);
}

PreFlightMiscPage::~PreFlightMiscPage()
{
  // qDebug("PreFlightMiscPage::~PreFlightMiscPage()");
}

void PreFlightMiscPage::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // @AP: Arrival Altitude is always saved as meter.
  Altitude minArrival = conf->getSafetyAltitude();

  if (altUnit == Altitude::meters) // user wants meters
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getMeters()));
      edtMinimalArrival->setSingleStep(50);
    }
  else // user gets feet
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getFeet()));
      edtMinimalArrival->setSingleStep(100);
    }

  edtArrivalAltitude->setCurrentIndex( conf->getArrivalAltitudeDisplay() );

  edtQNH->setValue( conf->getQNH() );
  bRecordInterval->setValue( conf->getBRecordInterval() );
  kRecordInterval->setValue( conf->getKRecordInterval() );
  chkLogAutoStart->setChecked( conf->getLoggerAutostartMode() );
}

void PreFlightMiscPage::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  IgcLogger     *log  = IgcLogger::instance();

  if( chkLogAutoStart->isChecked() )
    {
      if ( ! log->getIsLogging() )
        {
          // Change logger mode to standby only, if logger is not on.
          log->Standby();
        }
    }
  else
    {
      if (log->getIsStandby())
        {
          log->Stop();
        }
    }

  conf->setLoggerAutostartMode( chkLogAutoStart->isChecked() );

  // @AP: Store altitude always as meter.
  if (altUnit == Altitude::meters)
    {
      conf->setSafetyAltitude(Altitude(edtMinimalArrival->value()));
    }
  else
    {
      Altitude currentAlt; // Altitude will be converted to feet

      currentAlt.setFeet((double) edtMinimalArrival->value());
      conf->setSafetyAltitude(currentAlt);
    }

  conf->setArrivalAltitudeDisplay( (GeneralConfig::ArrivalAltitudeDisplay) edtArrivalAltitude->itemData(edtArrivalAltitude->currentIndex()).toInt() );
  conf->setQNH(edtQNH->value());
  conf->setBRecordInterval(bRecordInterval->value());
  conf->setKRecordInterval(kRecordInterval->value());
}

void PreFlightMiscPage::slotOpenLogbook()
{
  Logbook* lbw = new Logbook( this );
  lbw->setVisible( true );
}

#ifdef FLARM

/**
 * Called to open the Flarm flight download dialog.
 */
void PreFlightMiscPage::slotOpenFlarmFlights()
{
  FlarmLogbook* flb = new FlarmLogbook( this );
  flb->setVisible( true );
}

#endif
