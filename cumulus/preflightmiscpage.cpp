/***********************************************************************
**
**   preflightmiscpage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QLabel>
#include <QMessageBox>
#include <QValidator>
#include <QGridLayout>

#include "preflightmiscpage.h"
#include "igclogger.h"
#include "generalconfig.h"


PreFlightMiscPage::PreFlightMiscPage(QWidget *parent, const char *name ) : QWidget(parent,name)
{
  QGridLayout *topLayout = new QGridLayout(this, 6, 3, 5);
  int row = 0;

  QLabel *lbl = new QLabel(tr("Minimal arrival altitude:"), this);
  topLayout->addWidget(lbl, row, 0);
  topLayout->addRowSpacing(row, 25);

  // get current set altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();

  const char *unit = "";

  // Input accept only feet and meters all other make no sense. Therefore all
  // other (FL) is treated as feet.
  if( altUnit == Altitude::meters )
    {
      edtMinimalArrival = new QSpinBox(0, 1000, 10, this, "MinArr");
      unit = "m";
    }
  else
    {
      edtMinimalArrival = new QSpinBox(0, 3000, 10, this, "MinArr");
      unit = "ft";
    }

  edtMinimalArrival->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(edtMinimalArrival, row, 1);
  topLayout->addWidget(new QLabel(unit, this), row, 2);
  topLayout->setColStretch(2,2);
  row++;

  lbl = new QLabel(tr("QNH:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtQNH = new QSpinBox(0, 1999, 1, this, "QNH");
  edtQNH->setButtonSymbols(QSpinBox::PlusMinus);

  topLayout->addWidget(edtQNH, row, 1);
  topLayout->addWidget(new QLabel( "hPa", this), row, 2);
  row++;
  
  lbl = new QLabel(tr("Logger Interval:"), this);
  topLayout->addWidget(lbl, row, 0);
  loggerInterval = new QSpinBox(1, 60, 1, this, "LoggerInterval");
  loggerInterval->setButtonSymbols(QSpinBox::PlusMinus);

  topLayout->addWidget(loggerInterval, row, 1);
  topLayout->addWidget(new QLabel( "s", this), row, 2);
  row++;

  topLayout->addRowSpacing(row, 50);

  chkLogAutoStart = new QCheckBox(tr("Autostart logging"),this,"log_autostart");
  topLayout->addMultiCellWidget( chkLogAutoStart, row, row, 0, 2 );
  chkLogAutoStart->setChecked(IgcLogger::instance()->getisStandby());
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

  if( altUnit == Altitude::meters ) // user wants meters
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getMeters()));
    }
  else // user gets feet
    {
      edtMinimalArrival->setValue((int) rint(minArrival.getFeet()) );
    }

  edtQNH->setValue( conf->getQNH() );

  loggerInterval->setValue( conf->getLoggerInterval() );
}


void PreFlightMiscPage::save()
{
  //qDebug("PreFlightMiscPage::save()");

  IgcLogger *log = IgcLogger::instance();

  if (chkLogAutoStart->isChecked())
    {
      if (log->getisLogging())
        {
          int answer= QMessageBox::warning(this,tr("Restart Logging?"),
                                           tr("Cumulus is currently\nlogging.\nDo you want\nto close this logfile\nand start a new log?"),
                                           QMessageBox::Yes,
                                           QMessageBox::No | QMessageBox::Escape | QMessageBox::Default);

          if (answer==QMessageBox::Yes)
            {
              log->Standby();
            }
        }
      else
        {
          log->Standby();
        }
    }
  else
    {
      if (log->getisStandby())
        {
          log->Stop();
        }
    }

  GeneralConfig *conf = GeneralConfig::instance();

  // @AP: Store altitude always as meter.
  if( altUnit == Altitude::meters )
    {
      conf->setSafetyAltitude( Altitude(edtMinimalArrival->value()) );
    }
  else
    {
      Altitude currentAlt; // Altitude will be converted to feet

      currentAlt.setFeet( (double) edtMinimalArrival->value() );
      conf->setSafetyAltitude( currentAlt );
    }

  conf->setQNH( edtQNH->value() );

  conf->setLoggerInterval( loggerInterval->value() );
}

