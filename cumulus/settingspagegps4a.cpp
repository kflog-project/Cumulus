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
#include "gpsnmea.h"
#include "settingspagegps4a.h"

SettingsPageGPS4A::SettingsPageGPS4A(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS4A");

  QHBoxLayout* topLayout = new QHBoxLayout(this);

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  GpsAltitude = new QComboBox;
  GpsAltitude->setEditable(false);
  GpsAltitude->addItem(tr("GPS"));
  GpsAltitude->addItem(tr("Pressure"));

  QFormLayout* fl = new QFormLayout;
  fl->addRow( tr("Altitude Reference:"), GpsAltitude );
  topLayout->addLayout( fl );

  saveNmeaData = new QCheckBox (tr("Save NMEA Data to file"), this);
  topLayout->addWidget( saveNmeaData );
  topLayout->addStretch( 10 );
}

SettingsPageGPS4A::~SettingsPageGPS4A()
{
}

void SettingsPageGPS4A::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );
  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

void SettingsPageGPS4A::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsAltitude( GpsNmea::DeliveredAltitude(GpsAltitude->currentIndex()) );

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
