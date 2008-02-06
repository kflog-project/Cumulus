/***********************************************************************
**
**   settingspagegps.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
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

SettingsPageGPS::SettingsPageGPS(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    QGridLayout * topLayout = new QGridLayout(this, 5, 3, 5);
    int row=0;

    topLayout->addWidget(new QLabel(tr("Serial Device:"), this),row,0);
    GpsDev = new QComboBox(true, this, "GPSDevice");
    topLayout->addWidget(GpsDev,row++,2);

    if( HWINFO->getType() == HwInfo::ipaq39xx ) {
        GpsDev->insertItem("/dev/tts/0");   // ipaq 39xx does only support this one
        GpsDev->insertItem("/dev/rfcomm0");
        GpsDev->insertItem("/dev/rfcomm1");
    }
    else if( HWINFO->getType() == HwInfo::ipaq38xx )
        GpsDev->insertItem("/dev/ttySA0");   // ipaq 38xx does only support this one
    else {
        GpsDev->insertItem("/dev/ttyS0");
        GpsDev->insertItem("/dev/ttyS1");
        GpsDev->insertItem("/dev/ttyS2");
        GpsDev->insertItem("/dev/ttyS3");

	// Blue Tooth default devices
        GpsDev->insertItem("/dev/rfcomm0");
        GpsDev->insertItem("/dev/rfcomm1");

	// automatic search for serial compact flash GPS devices
        GpsDev->insertItem("CF");
    }

    GpsDev->insertItem(NMEASIM_DEVICE);

    topLayout->addWidget(new QLabel(tr("Transfer rate (bps):"), this),row,0);
    GpsSpeed = new QComboBox(false, this, "GPSSpeed");
    topLayout->addWidget(GpsSpeed,row++,2);
    GpsSpeed->insertItem("57600");
    GpsSpeed->insertItem("38400");
    GpsSpeed->insertItem("19200");
    GpsSpeed->insertItem("9600");
    GpsSpeed->insertItem("4800");
    GpsSpeed->insertItem("2400");
    GpsSpeed->insertItem("1200");
    GpsSpeed->insertItem("600");

    // @AP: Some GPS CF Cards (e.g. BC-307) deliver only height above the WGS 84
    // ellipsoid in GGA record. This is not deriveable from the received
    // record. Therefore we need an additional configuration entry :(

    topLayout->addWidget(new QLabel(tr("Altitude:"), this),row,0);
    GpsAltitude = new QComboBox(false, this, "GPSAltitude");
    topLayout->addWidget(GpsAltitude,row++,2);
    GpsAltitude->insertItem(tr("MSL"));
    GpsAltitude->insertItem(tr("HAE"));
    GpsAltitude->insertItem(tr("User"));

    //AS: Some GPS units (like the Pretec) don't include any form of HAE correction.
    //For these, the user can manually enter the correction.
    topLayout->addWidget(new QLabel(tr("Altitude Correction:"), this),row,0);
    spinUserCorrection = new QSpinBox(-1000, 1000, 1, this, "GPSAltitudeCorrection");
    spinUserCorrection->setButtonSymbols(QSpinBox::PlusMinus);
    topLayout->addWidget(spinUserCorrection,row++,2);

    topLayout->addRowSpacing(row++,10);

    checkSoftStart = new QCheckBox (tr("Soft start"), this);
    topLayout->addMultiCellWidget(checkSoftStart, row, row, 0, 2);
    row++;

    checkHardStart = new QCheckBox (tr("Hard start"), this);
    topLayout->addMultiCellWidget(checkHardStart, row, row, 0, 2);
    row++;

    checkSyncSystemClock = new QCheckBox (tr("Update system clock"), this);
    topLayout->addMultiCellWidget(checkSyncSystemClock, row, row, 0, 2);
    row++;

    topLayout->setRowStretch(row++,10);
    
    buttonReset = new QPushButton (tr("Reset to factory settings"), this);
    topLayout->addMultiCellWidget (buttonReset, row, row, 0, 2, Qt::AlignRight);
    row++;

    topLayout->setColStretch(1,100);

    GeneralConfig *conf = GeneralConfig::instance();
    QString devText = conf->getGpsDevice();

    // select last saved device, if possible
    for(int i=0; i < GpsDev->count(); i++) {
        if(GpsDev->text(i) == devText) {
            GpsDev->setCurrentItem(i);
            break;
        }
    }

    connect (buttonReset, SIGNAL(clicked()),
             gps, SLOT(sendFactoryReset()));
    connect (GpsAltitude, SIGNAL(activated(int )),
             this, SLOT(slot_altitude_mode(int )));
}


SettingsPageGPS::~SettingsPageGPS()
{}


/** Called to initiate loading of the configurationfile */
void SettingsPageGPS::slot_load()
{
    GeneralConfig *conf = GeneralConfig::instance();

    GpsDev->lineEdit()->setText( conf->getGpsDevice() );

    QString rate = QString::number( conf->getGpsSpeed() );
    for (int i=0;i<GpsSpeed->count();i++) {
        if (GpsSpeed->text(i)==rate) {
            GpsSpeed->setCurrentItem(i);
            break;
        }
    }

    GpsAltitude->setCurrentItem( conf->getGpsAltitude() );
    spinUserCorrection->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );
    slot_altitude_mode( conf->getGpsAltitude() );
    checkSoftStart->setChecked( conf->getGpsSoftStart() );
    checkHardStart->setChecked( conf->getGpsHardStart() );
    checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
}


/** Called to initiate saving to the configurationfile. */
void SettingsPageGPS::slot_save()
{
    GeneralConfig *conf = GeneralConfig::instance();

    conf->setGpsDevice( GpsDev->currentText() );
    conf->setGpsSpeed( GpsSpeed->currentText().toInt() );
    conf->setGpsAltitude( GPSNMEA::DeliveredAltitude(GpsAltitude->currentItem()) );

    if( GpsAltitude->currentItem() == GPSNMEA::USER ) {
        conf->setGpsUserAltitudeCorrection( Altitude(spinUserCorrection->value()) );
    } else {
        conf->setGpsUserAltitudeCorrection( 0 );
    }

    conf->setGpsHardStart( checkHardStart->isChecked() );
    conf->setGpsSoftStart( checkSoftStart->isChecked() );
    conf->setGpsSyncSystemClock( checkSyncSystemClock->isChecked() );

    conf->save();
}


void SettingsPageGPS::slot_altitude_mode(int mode)
{
    spinUserCorrection->setEnabled(mode == GPSNMEA::USER);
}
