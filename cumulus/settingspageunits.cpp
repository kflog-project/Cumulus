/***********************************************************************
 **
 **   settingspageunits.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002      by André Somers
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "speed.h"
#include "altitude.h"
#include "distance.h"
#include "generalconfig.h"
#include "layout.h"
#include "mapcalc.h"
#include "settingspageunits.h"
#include "time_cu.h"
#include "wgspoint.h"

SettingsPageUnits::SettingsPageUnits(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageUnits");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - Units") );

  if( parent )
    {
      resize( parent->size() );
    }

  // Layout used by scroll area
  QHBoxLayout *sal = new QHBoxLayout;

  // new widget used as container for the dialog layout.
  QWidget* sw = new QWidget;

  // Scroll area
  QScrollArea* sa = new QScrollArea;
  sa->setWidgetResizable( true );
  sa->setFrameStyle( QFrame::NoFrame );
  sa->setWidget( sw );

#ifdef QSCROLLER
  QScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( sa->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // Add scroll area to its own layout
  sal->addWidget( sa );

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  // Pass scroll area layout to the content layout.
  contentLayout->addLayout( sal, 10 );

  // The parent of the layout is the scroll widget
  QGridLayout *topLayout = new QGridLayout(sw);

  int row=0;

  QLabel *label = new QLabel(tr("Altitude:"), this);
  topLayout->addWidget(label, row, 0);
  UnitAlt = new QComboBox(this);
  UnitAlt->setObjectName("UnitAlt");
  UnitAlt->setEditable(false);
  topLayout->addWidget(UnitAlt,row++,1);
  UnitAlt->addItem(tr("meters"));
  UnitAlt->addItem(tr("feet"));
  altitudes[0] = int(Altitude::meters);
  altitudes[1] = int(Altitude::feet);

  label = new QLabel(tr("Speed:"), this);
  topLayout->addWidget(label, row, 0);
  UnitSpeed = new QComboBox(this);
  UnitSpeed->setObjectName("UnitSpeed");
  UnitSpeed->setEditable(false);
  topLayout->addWidget(UnitSpeed,row++,1);
  UnitSpeed->addItem(tr("meters per second"));
  UnitSpeed->addItem(tr("kilometers per hour"));
  UnitSpeed->addItem(tr("knots"));
  UnitSpeed->addItem(tr("miles per hour"));
  speeds[0] = Speed::metersPerSecond;
  speeds[1] = Speed::kilometersPerHour;
  speeds[2] = Speed::knots;
  speeds[3] = Speed::milesPerHour;

  label = new QLabel(tr("Distance:"), this);
  topLayout->addWidget(label, row, 0);
  UnitDistance = new QComboBox(this);
  UnitDistance->setObjectName("UnitDistance");
  UnitDistance->setEditable(false);
  topLayout->addWidget(UnitDistance,row++,1);
  UnitDistance->addItem(tr("kilometers"));
  UnitDistance->addItem(tr("statute miles"));
  UnitDistance->addItem(tr("nautical miles"));
  distances[0] = Distance::kilometers;
  distances[1] = Distance::miles;
  distances[2] = Distance::nautmiles;

  label = new QLabel(tr("Vario:"), this);
  topLayout->addWidget(label, row, 0);
  UnitVario = new QComboBox(this);
  UnitVario->setObjectName("UnitVario");
  UnitVario->setEditable(false);
  topLayout->addWidget(UnitVario,row++,1);
  UnitVario->addItem(tr("meters per second"));
  UnitVario->addItem(tr("feet per minute"));
  UnitVario->addItem(tr("knots"));
  varios[0] = Speed::metersPerSecond;
  varios[1] = Speed::feetPerMinute;
  varios[2] = Speed::knots;

  label = new QLabel(tr("Wind:"), this);
  topLayout->addWidget(label, row, 0);
  UnitWind = new QComboBox(this);
  UnitWind->setObjectName("UnitWind");
  UnitWind->setEditable(false);
  topLayout->addWidget(UnitWind,row++,1);
  UnitWind->addItem(tr("meters per second"));
  UnitWind->addItem(tr("kilometers per hour"));
  UnitWind->addItem(tr("knots"));
  UnitWind->addItem(tr("miles per hour"));
  winds[0] = Speed::metersPerSecond;
  winds[1] = Speed::kilometersPerHour;
  winds[2] = Speed::knots;
  winds[3] = Speed::milesPerHour;

  label = new QLabel(tr("Position:"), this);
  topLayout->addWidget(label, row, 0);
  UnitPosition = new QComboBox(this);
  UnitPosition->setObjectName("UnitPosition");
  UnitPosition->setEditable(false);
  topLayout->addWidget(UnitPosition,row++,1);
  UnitPosition->addItem(QString("ddd") + Qt::Key_degree + "mm'ss\"");
  UnitPosition->addItem(QString("ddd") + Qt::Key_degree + "mm.mmm'");
  UnitPosition->addItem(QString("ddd.ddddd") + Qt::Key_degree);
  positions[0] = WGSPoint::DMS;
  positions[1] = WGSPoint::DDM;
  positions[2] = WGSPoint::DDD;

  label = new QLabel(tr("Time:"), this);
  topLayout->addWidget(label, row, 0);
  UnitTime = new QComboBox(this);
  UnitTime->setObjectName("UnitTime");
  UnitTime->setEditable(false);
  topLayout->addWidget(UnitTime,row++,1);
  UnitTime->addItem(tr("UTC"));
  UnitTime->addItem(tr("Local"));
  times[0] = Time::utc;
  times[1] = Time::local;

  label = new QLabel(tr("Temperature:"), this);
  topLayout->addWidget(label, row, 0);
  UnitTemperature = new QComboBox(this);
  UnitTemperature->setObjectName("UnitTemperature");
  UnitTemperature->setEditable(false);
  topLayout->addWidget(UnitTemperature,row++,1);
  UnitTemperature->addItem(tr("Celsius"));
  UnitTemperature->addItem(tr("Fahrenheit"));
  temperature[0] = GeneralConfig::Celsius;
  temperature[1] = GeneralConfig::Fahrenheit;

  label = new QLabel(tr("Air Pressure:"), this);
  topLayout->addWidget(label, row, 0);
  UnitAirPressure = new QComboBox(this);
  UnitAirPressure->setObjectName("UnitAirPressure");
  UnitAirPressure->setEditable(false);
  topLayout->addWidget(UnitAirPressure,row++,1);
  UnitAirPressure->addItem(tr("hPa"));
  UnitAirPressure->addItem(tr("inHg"));
  airPressure[0] = GeneralConfig::hPa;
  airPressure[1] = GeneralConfig::inHg;

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2, 10);

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(IconSize, IconSize));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(IconSize, IconSize));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setPixmap(GeneralConfig::instance()->loadPixmap("setup.png"));

  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);

  load();
}

SettingsPageUnits::~SettingsPageUnits()
{}

void SettingsPageUnits::slotAccept()
{
  if( checkChanges() )
    {
      save();
      GeneralConfig::instance()->save();
      emit settingsChanged();
    }

  QWidget::close();
}

void SettingsPageUnits::slotReject()
{
  QWidget::close();
}

void SettingsPageUnits::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  UnitAlt->setCurrentIndex(searchItem(altitudes, conf->getUnitAlt(), UnitAlt->count()));
  UnitDistance->setCurrentIndex(searchItem(distances, conf->getUnitDist(), UnitDistance->count()));
  UnitSpeed->setCurrentIndex(searchItem(speeds, conf->getUnitSpeed(), UnitSpeed->count()));
  UnitVario->setCurrentIndex(searchItem(varios, conf->getUnitVario(), UnitVario->count()));
  UnitWind->setCurrentIndex(searchItem(winds, conf->getUnitWind(), UnitWind->count()));
  UnitPosition->setCurrentIndex(searchItem(positions, conf->getUnitPos(), UnitPosition->count()));
  UnitTime->setCurrentIndex(searchItem(times, conf->getUnitTime(), UnitTime->count()));
  UnitTemperature->setCurrentIndex(searchItem(temperature, conf->getUnitTemperature(), UnitTemperature->count()));
  UnitAirPressure->setCurrentIndex(searchItem(airPressure, conf->getUnitAirPressure(), UnitAirPressure->count()));
}

/** called to initiate saving to the configuration file. */
void SettingsPageUnits::save()
{
  GeneralConfig *conf = GeneralConfig::instance();
  // save the entries
  conf->setUnitAlt( altitudes[UnitAlt->currentIndex()] );
  conf->setUnitSpeed( speeds[UnitSpeed->currentIndex()] );
  conf->setUnitDist( distances[UnitDistance->currentIndex()] );
  conf->setUnitVario( varios[UnitVario->currentIndex()] );
  conf->setUnitWind( winds[UnitWind->currentIndex()] );
  conf->setUnitPos( positions[UnitPosition->currentIndex()] );
  conf->setUnitTime( times[UnitTime->currentIndex()] );
  conf->setUnitTemperature( temperature[UnitTemperature->currentIndex()] );
  conf->setUnitAirPressure( airPressure[UnitAirPressure->currentIndex()] );

  // Set the static units. A signal that these (may) have changed is emitted
  // by the container, ConfigWidget
  Distance::setUnit(Distance::distanceUnit(distances[UnitDistance->currentIndex()]));
  Speed::setHorizontalUnit(Speed::speedUnit(speeds[UnitSpeed->currentIndex()]));
  Speed::setVerticalUnit(Speed::speedUnit(varios[UnitVario->currentIndex()]));
  Speed::setWindUnit(Speed::speedUnit(winds[UnitWind->currentIndex()]));
  Altitude::setUnit(Altitude::altitudeUnit(altitudes[UnitAlt->currentIndex()]));
  WGSPoint::setFormat(WGSPoint::Format(positions[UnitPosition->currentIndex()]));
  Time::setUnit(Time::timeUnit(times[UnitTime->currentIndex()]));
}

/** This function returns the location of the value in the array. */
int SettingsPageUnits::searchItem(int* p, int value, int max)
{
  for (int i = 0; i < max; i++)
    {
      if (*p == value)
        {
          return i;
        }

      p++;
    }

  return 0;
}

/** Called to ask is confirmation on the close is needed. */
bool SettingsPageUnits::checkChanges()
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed  = conf->getUnitAlt()         != altitudes[UnitAlt->currentIndex()];
  changed |= conf->getUnitSpeed()       != speeds[UnitSpeed->currentIndex()];
  changed |= conf->getUnitDist()        != distances[UnitDistance->currentIndex()];
  changed |= conf->getUnitVario()       != varios[UnitVario->currentIndex()];
  changed |= conf->getUnitWind()        != winds[UnitWind->currentIndex()];
  changed |= conf->getUnitPos()         != positions[UnitPosition->currentIndex()];
  changed |= conf->getUnitTime()        != times[UnitTime->currentIndex()];
  changed |= conf->getUnitTemperature() != temperature[UnitTemperature->currentIndex()];
  changed |= conf->getUnitAirPressure() != airPressure[UnitAirPressure->currentIndex()];

  return changed;
}
