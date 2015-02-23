/***********************************************************************
**
**   settingspagegps4a.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2012-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * GPS Configuration settings for Android.
 */

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "generalconfig.h"
#include "gpsnmea.h"
#include "layout.h"
#include "settingspagegps4a.h"

SettingsPageGPS4A::SettingsPageGPS4A(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS4A");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Settings - GPS") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  QGridLayout* topLayout = new QGridLayout;
  contentLayout->addLayout(topLayout, 10);

  int row = 0;

  topLayout->setRowMinimumHeight( row++, 20);

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  GpsAltitude = new QComboBox;
  GpsAltitude->setEditable(false);
  GpsAltitude->addItem(tr("GPS"));
  GpsAltitude->addItem(tr("Pressure"));

  QFormLayout* fl = new QFormLayout;
  fl->addRow( tr("Altitude Reference:"), GpsAltitude );
  topLayout->addLayout( fl, row++, 0, Qt::AlignLeft );
  topLayout->setRowMinimumHeight( row++, 10);

  saveNmeaData = new QCheckBox (tr("Save NMEA Data to file"));
  topLayout->addWidget( saveNmeaData, row++, 0, Qt::AlignLeft );

  topLayout->setRowStretch( row, 10 );

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QLabel *titlePix = new QLabel(this);
  titlePix->setAlignment( Qt::AlignCenter );
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

SettingsPageGPS4A::~SettingsPageGPS4A()
{
}

void SettingsPageGPS4A::slotAccept()
{
  save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageGPS4A::slotReject()
{
  QWidget::close();
}

void SettingsPageGPS4A::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );
  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

void SettingsPageGPS4A::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsAltitude( GpsNmea::DeliveredAltitude( GpsAltitude->currentIndex()) );

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
