/***********************************************************************
**
**   settingspagegps4a.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * GPS Configuration settings for Android.
 */

#include <QtGui>

#include "generalconfig.h"
#include "settingspagegps4a.h"

SettingsPageGPS4A::SettingsPageGPS4A(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS4A");

  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  saveNmeaData = new QCheckBox (tr("Save NMEA Data to file"), this);
  topLayout->addWidget(saveNmeaData, row, 0 );
  row++;

  topLayout->setRowStretch( row++, 10 );
  topLayout->setColumnStretch( 2, 10 );
}

SettingsPageGPS4A::~SettingsPageGPS4A()
{
}

void SettingsPageGPS4A::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

void SettingsPageGPS4A::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  bool oldNmeaLogState = conf->getGpsNmeaLogState();

  conf->setGpsNmeaLogState( saveNmeaData->isChecked() );

  if( oldNmeaLogState != saveNmeaData->isChecked() )
    {
      if( saveNmeaData->isChecked() )
        {
          emit startNmeaLog();
        }
      else
        {
          emit endNmeaLog();
        }
    }
}
