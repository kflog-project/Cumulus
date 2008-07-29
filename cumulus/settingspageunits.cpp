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

SettingsPageUnits::SettingsPageUnits(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageUnits");
  
  QGridLayout *topLayout = new QGridLayout(this);
  int row=0;

  QLabel * lblAlt = new QLabel(tr("Altitude:"), this);
  topLayout->addWidget(lblAlt,row,0);
  UnitAlt = new QComboBox(this);
  UnitAlt->setObjectName("UnitAlt");
  UnitAlt->setEditable(false);
  topLayout->addWidget(UnitAlt,row++,1);
  UnitAlt->addItem(tr("meters"));
  UnitAlt->addItem(tr("feet"));
  altitudes[0]=int(Altitude::meters);
  altitudes[1]=int(Altitude::feet);

  QLabel * lblSpeed = new QLabel(tr("Speed:"), this);
  topLayout->addWidget(lblSpeed,row,0);
  UnitSpeed = new QComboBox(this);
  UnitSpeed->setObjectName("UnitSpeed");
  UnitSpeed->setEditable(false);
  topLayout->addWidget(UnitSpeed,row++,1);
  UnitSpeed->addItem(tr("meters per second"));
  UnitSpeed->addItem(tr("kilometers per hour"));
  UnitSpeed->addItem(tr("knots"));
  UnitSpeed->addItem(tr("miles per hour"));
  speeds[0]=Speed::metersPerSecond;
  speeds[1]=Speed::kilometersPerHour;
  speeds[2]=Speed::knots;
  speeds[3]=Speed::milesPerHour;

  QLabel * lblDistance = new QLabel(tr("Distance:"), this);
  topLayout->addWidget(lblDistance,row,0);
  UnitDistance = new QComboBox(this);
  UnitDistance->setObjectName("UnitDistance");
  UnitDistance->setEditable(false);
  topLayout->addWidget(UnitDistance,row++,1);
  UnitDistance->addItem(tr("kilometers"));
  UnitDistance->addItem(tr("miles"));
  UnitDistance->addItem(tr("nautical miles"));
  distances[0]=Distance::kilometers;
  distances[1]=Distance::miles;
  distances[2]=Distance::nautmiles;

  QLabel * lblVario = new QLabel(tr("Vario:"), this);
  topLayout->addWidget(lblVario,row,0);
  UnitVario = new QComboBox(this);
  UnitVario->setObjectName("UnitVario");
  UnitVario->setEditable(false);
  topLayout->addWidget(UnitVario,row++,1);
  UnitVario->addItem(tr("meters per second"));
  UnitVario->addItem(tr("feet per minute"));
  UnitVario->addItem(tr("knots"));
  varios[0]=Speed::metersPerSecond;
  varios[1]=Speed::feetPerMinute;
  varios[2]=Speed::knots;

  QLabel * lblWind = new QLabel(tr("Wind:"), this);
  topLayout->addWidget(lblWind,row,0);
  UnitWind = new QComboBox(this);
  UnitWind->setObjectName("UnitWind");
  UnitWind->setEditable(false);
  topLayout->addWidget(UnitWind,row++,1);
#warning FIXME We need some changes in the Speed object to make it possible to get a separate windspeed unit.

  UnitWind->addItem(tr("Same as Speed"));
  UnitWind->addItem(tr("meters per second"));
  UnitWind->addItem(tr("kilometers per hour"));
  UnitWind->addItem(tr("knots"));
  UnitWind->addItem(tr("miles per hour"));
  //UnitWind->addItem(tr("beaufort"));
  winds[0]=Speed::metersPerSecond;
  winds[1]=Speed::kilometersPerHour;
  winds[2]=Speed::knots;
  winds[3]=Speed::milesPerHour;
  UnitWind->setEnabled(false);

  QLabel * lblDegrees = new QLabel(tr("Position:"), this);
  topLayout->addWidget(lblDegrees,row,0);
  UnitPosition = new QComboBox(this);
  UnitPosition->setObjectName("UnitPosition");
  UnitPosition->setEditable(false);
  topLayout->addWidget(UnitPosition,row++,1);
  UnitPosition->addItem(tr("ddd°mm'ss\""));
  UnitPosition->addItem(tr("ddd°mm.mmm\'"));
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

  UnitAlt->setCurrentIndex(searchItem(altitudes, conf->getUnitAlt(), UnitAlt->count()));
  UnitDistance->setCurrentIndex(searchItem(distances, conf->getUnitDist(), UnitDistance->count()));
  UnitSpeed->setCurrentIndex(searchItem(speeds, conf->getUnitSpeed(), UnitSpeed->count()));
  UnitVario->setCurrentIndex(searchItem(varios, conf->getUnitVario(), UnitVario->count()));
  UnitWind->setCurrentIndex(searchItem(winds, conf->getUnitWind(), UnitWind->count()));
  UnitPosition->setCurrentIndex(searchItem(positions, conf->getUnitPos(), UnitPosition->count()));

  //set the static units for distances, speeds and altitudes. A signal that these (may) have
  //changed is emitted by the container, ConfigDialog
  Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentIndex()]));
  Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentIndex()]));
  Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentIndex()]));
  Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentIndex()]));
  WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentIndex()]));
}


/** called to initiate saving to the configurationfile. */
void SettingsPageUnits::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  // set the entries
  conf->setUnitAlt( altitudes[UnitAlt->currentIndex()] );
  conf->setUnitSpeed( speeds[UnitSpeed->currentIndex()] );
  conf->setUnitDist( distances[UnitDistance->currentIndex()] );
  conf->setUnitVario( varios[UnitVario->currentIndex()] );
  conf->setUnitWind( winds[UnitWind->currentIndex()] );
  conf->setUnitPos( positions[UnitPosition->currentIndex()] );
  conf->save();

  //set the static units for distances, speeds and altitudes. A signal that these (may) have
  //changed is emitted by the container, ConfigDialog
  Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentIndex()]));
  Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentIndex()]));
  Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentIndex()]));
  Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentIndex()]));
  WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentIndex()]));
}


/** This function returns the location of the value in the array. */
int SettingsPageUnits::searchItem(int * p, int value, int max)
{
  int i;

  for (i=1; i<=max; i++)
    {
      if (*p==value)
        {
          return i-1;
        }
      p++;
    }
  
  return 0;
}


/** this slot is called when a unit has been changed, to make sure this unit is in effect immediatly. */
void SettingsPageUnits::slotUnitChanged()
{
  Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentIndex()]));
  Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentIndex()]));
  Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentIndex()]));
  Altitude::setUnit(Altitude::altitude(altitudes[UnitAlt->currentIndex()]));
  WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentIndex()]));
}
