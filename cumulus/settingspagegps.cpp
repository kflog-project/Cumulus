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
    GpsDev = new QComboBox(this);
    GpsDev->setObjectName ("GPSDevice");
    GpsDev->setEditable (true);
    topLayout->addWidget(GpsDev,row++,2);

    if( HWINFO->getType() == HwInfo::ipaq39xx ) {
        GpsDev->addItem("/dev/tts/0");   // ipaq 39xx does only support this one
        GpsDev->addItem("/dev/rfcomm0");
        GpsDev->addItem("/dev/rfcomm1");
    }
    else if( HWINFO->getType() == HwInfo::ipaq38xx )
        GpsDev->addItem("/dev/ttySA0");   // ipaq 38xx does only support this one
    else {
        GpsDev->addItem("/dev/ttyS0");
        GpsDev->addItem("/dev/ttyS1");
        GpsDev->addItem("/dev/ttyS2");
        GpsDev->addItem("/dev/ttyS3");

	// Blue Tooth default devices
        GpsDev->addItem("/dev/rfcomm0");
        GpsDev->addItem("/dev/rfcomm1");

	// automatic search for serial compact flash GPS devices
        GpsDev->addItem("CF");
    }

    GpsDev->addItem(NMEASIM_DEVICE);

    topLayout->addWidget(new QLabel(tr("Transfer rate (bps):"), this),row,0);
    GpsSpeed = new QComboBox(this);
    GpsSpeed->setObjectName("GPSSpeed");
    GpsSpeed->setEditable(false);
    topLayout->addWidget(GpsSpeed,row++,2);
    GpsSpeed->addItem("57600");
    GpsSpeed->addItem("38400");
    GpsSpeed->addItem("19200");
    GpsSpeed->addItem("9600");
    GpsSpeed->addItem("4800");
    GpsSpeed->addItem("2400");
    GpsSpeed->addItem("1200");
    GpsSpeed->addItem("600");

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
    conf->setGpsAltitude( GPSNMEA::DeliveredAltitude(GpsAltitude->currentIndex()) );

    if( GpsAltitude->currentIndex() == GPSNMEA::USER ) {
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
