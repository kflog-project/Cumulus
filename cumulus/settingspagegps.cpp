/***********************************************************************
**
**   settingspagegps.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

#include "generalconfig.h"
#include "settingspagegps.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "hwinfo.h"

SettingsPageGPS::SettingsPageGPS(QWidget *parent) : QWidget(parent)
{
  qDebug("SettingsPageGPS::SettingsPageGPS Enter");
    setObjectName("SettingsPageGPS");
    QGridLayout * topLayout = new QGridLayout(this);

    int row=0;

    topLayout->addWidget(new QLabel(tr("GPS Device:"), this), row, 0);
    GpsDev = new QComboBox(this);
    GpsDev->setObjectName ("GPSDevice");
    topLayout->addWidget(GpsDev, row++, 2);

#ifndef MAEMO
    GpsDev->setEditable(true);
    GpsDev->addItem("/dev/ttyS0");
    GpsDev->addItem("/dev/ttyS1");
    GpsDev->addItem("/dev/ttyS2");
    GpsDev->addItem("/dev/ttyS3");
    // Bluetooth default devices
    GpsDev->addItem("/dev/rfcomm0");
    GpsDev->addItem("/dev/rfcomm1");
    // add entry for NMEA simulator choise
    GpsDev->addItem(NMEASIM_DEVICE);
#else
    // Under Maemo these settings are fixed.
    GpsDev->setEditable(false);
    GpsDev->addItem("GPS Daemon");
    GpsDev->addItem(NMEASIM_DEVICE);
#endif

#ifndef MAEMO
    topLayout->addWidget(new QLabel(tr("Transfer rate (bps):"), this),row,0);
    GpsSpeed = new QComboBox(this);
    GpsSpeed->setObjectName("GPSSpeed");
    GpsSpeed->setEditable(false);
    topLayout->addWidget(GpsSpeed,row++,2);
    GpsSpeed->addItem("115200");
    GpsSpeed->addItem("57600");
    GpsSpeed->addItem("38400");
    GpsSpeed->addItem("19200");
    GpsSpeed->addItem("9600");
    GpsSpeed->addItem("4800");
    GpsSpeed->addItem("2400");
    GpsSpeed->addItem("1200");
    GpsSpeed->addItem("600");
#endif

    // @AP: Some GPS CF Cards (e.g. BC-307) deliver only height above the WGS 84
    // ellipsoid in GGA record. This is not deriveable from the received
    // record. Therefore we need an additional configuration entry :(
    topLayout->addWidget(new QLabel(tr("Altitude:"), this),row,0);
    GpsAltitude = new QComboBox(this);
    GpsAltitude->setObjectName("GPSAltitude");
    GpsAltitude->setEditable(false);
    topLayout->addWidget(GpsAltitude,row++,2);
    GpsAltitude->addItem(tr("MSL"));
    GpsAltitude->addItem(tr("HAE"));
    GpsAltitude->addItem(tr("User"));

    connect (GpsAltitude, SIGNAL(activated(int )),
             this, SLOT(slot_altitude_mode(int )));

    //AS: Some GPS units (like the Pretec) don't include any form of HAE correction.
    //For these, the user can manually enter the correction.
    topLayout->addWidget(new QLabel(tr("Altitude Correction:"), this),row,0);
    spinUserCorrection = new QSpinBox(this);
    spinUserCorrection->setObjectName("GPSAltitudeCorrection");
    spinUserCorrection->setMinimum(-1000);
    spinUserCorrection->setMaximum(1000);
    spinUserCorrection->setButtonSymbols(QSpinBox::PlusMinus);
    topLayout->addWidget(spinUserCorrection,row++,2);

#ifndef MAEMO
    topLayout->setRowMinimumHeight( row++, 10);

    checkSoftStart = new QCheckBox (tr("Soft start"), this);
    topLayout->addWidget(checkSoftStart, row, 0, 1, 3);
    row++;

    checkHardStart = new QCheckBox (tr("Hard start"), this);
    topLayout->addWidget(checkHardStart, row, 0, 1, 3);
    row++;

    checkSyncSystemClock = new QCheckBox (tr("Update system clock"), this);
    topLayout->addWidget(checkSyncSystemClock, row, 0, 1, 3);
    row++;

    topLayout->setRowStretch(row++,10);

    buttonReset = new QPushButton (tr("Reset to factory settings"), this);
    topLayout->addWidget(buttonReset, row, 0, 1, 3, Qt::AlignRight);
    row++;

    connect (buttonReset, SIGNAL(clicked()),
             gps, SLOT(sendFactoryReset()));

    topLayout->setColumnStretch(1,100);
#else
    topLayout->setRowStretch(row++,10);
#endif

  // select first device in list as default
  GpsDev->setCurrentIndex(0);
}

SettingsPageGPS::~SettingsPageGPS()
{
  return;
}


/** Called to initiate loading of the configurationfile */
void SettingsPageGPS::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();
  QString devText = conf->getGpsDevice();
  
  // select last saved device, if possible
  for(int i=0; i < GpsDev->count(); i++) {
    if(GpsDev->itemText(i) == devText) {
      GpsDev->setCurrentIndex(i);
      break;
    }
  }

  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );

  spinUserCorrection->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );

  slot_altitude_mode( conf->getGpsAltitude() );

#ifndef MAEMO
  QString rate = QString::number( conf->getGpsSpeed() );
  
  for (int i=0;i<GpsSpeed->count();i++) {
      if (GpsSpeed->itemText(i)==rate) {
          GpsSpeed->setCurrentIndex(i);
          break;
      }
  }

  checkSoftStart->setChecked( conf->getGpsSoftStart() );
  checkHardStart->setChecked( conf->getGpsHardStart() );
  checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
#endif
}


/** Called to initiate saving to the configurationfile. */
void SettingsPageGPS::slot_save()
{
    GeneralConfig *conf = GeneralConfig::instance();

    conf->setGpsDevice( GpsDev->currentText() );
    conf->setGpsAltitude( GPSNMEA::DeliveredAltitude(GpsAltitude->currentIndex()) );

    if( GpsAltitude->currentIndex() == GPSNMEA::USER ) {
        conf->setGpsUserAltitudeCorrection( Altitude(spinUserCorrection->value()) );
    } else {
        conf->setGpsUserAltitudeCorrection( 0 );
    }

#ifndef MAEMO    
    conf->setGpsSpeed( GpsSpeed->currentText().toInt() );
    conf->setGpsHardStart( checkHardStart->isChecked() );
    conf->setGpsSoftStart( checkSoftStart->isChecked() );
    conf->setGpsSyncSystemClock( checkSyncSystemClock->isChecked() );
#endif    

    conf->save();
}


void SettingsPageGPS::slot_altitude_mode(int mode)
{
    spinUserCorrection->setEnabled(mode == GPSNMEA::USER);
}
