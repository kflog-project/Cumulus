/***********************************************************************
**
**   settingspageunits.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QGridLayout>

#include "speed.h"
#include "altitude.h"
#include "distance.h"
#include "wgspoint.h"
#include "mapcalc.h"
#include "settingspageunits.h"
#include "generalconfig.h"

SettingsPageUnits::SettingsPageUnits(QWidget *parent, const char *name ) : QWidget(parent,name)
{
    QGridLayout * topLayout = new QGridLayout(this, 6,2,5);
    int row=0;

    QLabel * lblAlt = new QLabel(tr("Altitude:"), this);
    topLayout->addWidget(lblAlt,row,0);
    UnitAlt = new QComboBox(false, this, "UnitAlt");
    topLayout->addWidget(UnitAlt,row++,1);
    UnitAlt->insertItem(tr("meters"));
    UnitAlt->insertItem(tr("feet"));
    altitudes[0]=int(Altitude::meters);
    altitudes[1]=int(Altitude::feet);

    QLabel * lblSpeed = new QLabel(tr("Speed:"), this);
    topLayout->addWidget(lblSpeed,row,0);
    UnitSpeed = new QComboBox(false, this, "UnitSpeed");
    topLayout->addWidget(UnitSpeed,row++,1);
    UnitSpeed->insertItem(tr("meters per second"));
    UnitSpeed->insertItem(tr("kilometers per hour"));
    UnitSpeed->insertItem(tr("knots"));
    UnitSpeed->insertItem(tr("miles per hour"));
    speeds[0]=Speed::metersPerSecond;
    speeds[1]=Speed::kilometersPerHour;
    speeds[2]=Speed::knots;
    speeds[3]=Speed::milesPerHour;

    QLabel * lblDistance = new QLabel(tr("Distance:"), this);
    topLayout->addWidget(lblDistance,row,0);
    UnitDistance = new QComboBox(false, this, "UnitDistance");
    topLayout->addWidget(UnitDistance,row++,1);
    UnitDistance->insertItem(tr("kilometers"));
    UnitDistance->insertItem(tr("miles"));
    UnitDistance->insertItem(tr("nautical miles"));
    distances[0]=Distance::kilometers;
    distances[1]=Distance::miles;
    distances[2]=Distance::nautmiles;

    QLabel * lblVario = new QLabel(tr("Vario:"), this);
    topLayout->addWidget(lblVario,row,0);
    UnitVario = new QComboBox(false, this, "UnitVario");
    topLayout->addWidget(UnitVario,row++,1);
    UnitVario->insertItem(tr("meters per second"));
    UnitVario->insertItem(tr("feet per minute"));
    UnitVario->insertItem(tr("knots"));
    varios[0]=Speed::metersPerSecond;
    varios[1]=Speed::feetPerMinute;
    varios[2]=Speed::knots;

    QLabel * lblWind = new QLabel(tr("Wind:"), this);
    topLayout->addWidget(lblWind,row,0);
    UnitWind = new QComboBox(false, this, "UnitWind");
    topLayout->addWidget(UnitWind,row++,1);
#warning FIXME We need some changes in the Speed object to make it possible to get a separate windspeed unit.

    UnitWind->insertItem(tr("Same as Speed"));
    UnitWind->insertItem(tr("meters per second"));
    UnitWind->insertItem(tr("kilometers per hour"));
    UnitWind->insertItem(tr("knots"));
    UnitWind->insertItem(tr("miles per hour"));
    //UnitWind->insertItem(tr("beaufort"));
    winds[0]=Speed::metersPerSecond;
    winds[1]=Speed::kilometersPerHour;
    winds[2]=Speed::knots;
    winds[3]=Speed::milesPerHour;
    UnitWind->setEnabled(false);

    QLabel * lblDegrees = new QLabel(tr("Position:"), this);
    topLayout->addWidget(lblDegrees,row,0);
    UnitPosition = new QComboBox(false, this, "");
    topLayout->addWidget(UnitPosition,row++,1);
    UnitPosition->insertItem(tr("ddd°mm'ss\""));
    UnitPosition->insertItem(tr("ddd°mm.mmm\'"));
    positions[0]=WGSPoint::DMS;
    positions[1]=WGSPoint::DDM;

    topLayout->setRowStretch(row++,10);

    connect (UnitAlt,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
    connect (UnitDistance,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
    connect (UnitSpeed,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
    connect (UnitVario,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
    connect (UnitWind,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
    connect (UnitPosition,SIGNAL(activated(int)),
             this,SLOT(slotUnitChanged()));
}


SettingsPageUnits::~SettingsPageUnits()
{}


/** Called to initiate loading of the configurationfile */
void SettingsPageUnits::slot_load()
{
    GeneralConfig *conf = GeneralConfig::instance();

    UnitAlt->setCurrentItem(searchItem(altitudes, conf->getUnitAlt(), UnitAlt->count()));
    UnitDistance->setCurrentItem(searchItem(distances, conf->getUnitDist(), UnitDistance->count()));
    UnitSpeed->setCurrentItem(searchItem(speeds, conf->getUnitSpeed(), UnitSpeed->count()));
    UnitVario->setCurrentItem(searchItem(varios, conf->getUnitVario(), UnitVario->count()));
    UnitWind->setCurrentItem(searchItem(winds, conf->getUnitWind(), UnitWind->count()));
    UnitPosition->setCurrentItem(searchItem(positions, conf->getUnitPos(), UnitPosition->count()));

    //set the static units for distances, speeds and altitudes. A signal that these (may) have
    //changed is emitted by the container, ConfigDialog
    Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentItem()]));
    Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentItem()]));
    Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentItem()]));
    Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentItem()]));
    WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentItem()]));
}


/** called to initiate saving to the configurationfile. */
void SettingsPageUnits::slot_save()
{
    GeneralConfig *conf = GeneralConfig::instance();
    // set the entries
    conf->setUnitAlt( altitudes[UnitAlt->currentItem()] );
    conf->setUnitSpeed( speeds[UnitSpeed->currentItem()] );
    conf->setUnitDist( distances[UnitDistance->currentItem()] );
    conf->setUnitVario( varios[UnitVario->currentItem()] );
    conf->setUnitWind( winds[UnitWind->currentItem()] );
    conf->setUnitPos( positions[UnitPosition->currentItem()] );
    conf->save();

    //set the static units for distances, speeds and altitudes. A signal that these (may) have
    //changed is emitted by the container, ConfigDialog
    Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentItem()]));
    Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentItem()]));
    Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentItem()]));
    Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentItem()]));
    WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentItem()]));
}


/** This function returns the location of the value in the array. */
int SettingsPageUnits::searchItem(int * p, int value, int max)
{
    int i;

    for (i=1; i<=max; i++) {
        if (*p==value)
            return i-1;
        p++;
    }
    return 0;
}


/** this slot is called when a unit has been changed, to make sure this unit is in effect immediatly. */
void SettingsPageUnits::slotUnitChanged()
{
    Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentItem()]));
    Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentItem()]));
    Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentItem()]));
    Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentItem()]));
    WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentItem()]));
}
