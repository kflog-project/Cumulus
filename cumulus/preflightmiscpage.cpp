/***********************************************************************
 **
 **   preflightmiscpage.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by André Somers
 **                   2008-2010 by Axel Pauli
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
#include "igclogger.h"
#include "generalconfig.h"

PreFlightMiscPage::PreFlightMiscPage(QWidget *parent) :
  QWidget(parent)
{
  setObjectName("PreFlightMiscPage");

  QGridLayout *topLayout = new QGridLayout(this);
  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"), this);
  topLayout->addWidget(lbl, row, 0);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();

  const char *unit = "";

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
  edtMinimalArrival = new QSpinBox(this);
  edtMinimalArrival->setObjectName("MinArr");

  if (altUnit == Altitude::meters)
    {
      edtMinimalArrival->setMaximum(1000);
      unit = "m";
    }
  else
    {
      edtMinimalArrival->setMaximum(3000);
      unit = "ft";
    }

  edtMinimalArrival->setButtonSymbols(QSpinBox::PlusMinus);
  edtMinimalArrival->setSuffix(unit);
  topLayout->addWidget(edtMinimalArrival, row, 1);
  topLayout->setColumnStretch(2, 2);
  row++;

  lbl = new QLabel(tr("QNH:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtQNH = new QSpinBox(this);
  edtQNH->setObjectName("QNH");
  edtQNH->setMaximum(1999);
  edtQNH->setButtonSymbols(QSpinBox::PlusMinus);
  edtQNH->setSuffix("hPa");
  topLayout->addWidget(edtQNH, row, 1);
  row++;

  topLayout->setRowMinimumHeight(row, 25);
  row++;

  chkLogAutoStart = new QCheckBox(tr("Autostart IGC logger"), this);
  topLayout->addWidget(chkLogAutoStart, row, 0, 1, 3);
  row++;

  lbl = new QLabel(tr("B-Record Interval:"), this);
  topLayout->addWidget(lbl, row, 0);
  bRecordInterval = new QSpinBox(this);
  bRecordInterval->setMinimum(1);
  bRecordInterval->setMaximum(60);
  bRecordInterval->setButtonSymbols(QSpinBox::PlusMinus);
  bRecordInterval->setSuffix("s");
  topLayout->addWidget(bRecordInterval, row, 1);
  row++;

  lbl = new QLabel(tr("K-Record Interval:"), this);
  topLayout->addWidget(lbl, row, 0);
  kRecordInterval = new QSpinBox(this);
  kRecordInterval->setMinimum(0);
  kRecordInterval->setMaximum(300);
  kRecordInterval->setButtonSymbols(QSpinBox::PlusMinus);
  kRecordInterval->setSpecialValueText(tr("None"));
  kRecordInterval->setSuffix("s");
  topLayout->addWidget(kRecordInterval, row, 1);
  row++;

  topLayout->setRowStretch(row, 10);
}

PreFlightMiscPage::~PreFlightMiscPage()
{
  // qDebug("PreFlightMiscPage::~PreFlightMiscPage()");
}

void PreFlightMiscPage::load()
{
  // qDebug("PreFlightMiscPage::load()");

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

  edtQNH->setValue( conf->getQNH() );
  bRecordInterval->setValue( conf->getBRecordInterval() );
  kRecordInterval->setValue( conf->getKRecordInterval() );
  chkLogAutoStart->setChecked( conf->getLoggerAutostartMode() );
}

void PreFlightMiscPage::save()
{
  //qDebug("PreFlightMiscPage::save()");

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

  conf->setQNH(edtQNH->value());
  conf->setBRecordInterval(bRecordInterval->value());
  conf->setKRecordInterval(kRecordInterval->value());
}

