/***********************************************************************
**
**   settingspagegps4a.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2012-2018 by Axel Pauli
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

  int row = 0;

  QGridLayout* topLayout = new QGridLayout;
  topLayout->setMargin(10 * Layout::getIntScaledDensity());
  topLayout->setSpacing(15 * Layout::getIntScaledDensity());
  topLayout->setRowMinimumHeight( row, Layout::getIntScaledDensity() * 20);
  topLayout->setColumnStretch( 1, 5 );
  row++;
  contentLayout->addLayout(topLayout, 10);

  topLayout->addWidget(new QLabel(tr("GPS Source:")), row, 0);
  GpsSource = new QComboBox(this);
  topLayout->addWidget(GpsSource, row, 1);

  QListView* qlv = new QListView();
  QScrollBar* qsb = qlv->verticalScrollBar();
  qsb->setFixedWidth(100);
  GpsSource->setView( qlv );

  GpsSource->setEditable(false);
  GpsSource->addItem( tr("$GP GPS (USA)") );
  GpsSource->addItem( tr("$BD Beidou GPS (China)") );
  GpsSource->addItem( tr("$GA Gallileo GPS (Europe)") );
  GpsSource->addItem( tr("$GL Glonass GPS (Russia)") );
  GpsSource->addItem( tr("$GN Combined GPS Systems") );
  row++;

  // Try to make bigger the vertical scrollbar
  //QAbstractItemView *qv = GpsSource->view();
  //QScrollBar *vsb = qv->verticalScrollBar();
  //vsb->setStyleSheet( Layout::getCbSbStyle() );

  topLayout->addWidget(new QLabel(tr("Altitude Reference:")), row, 0);

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  GpsAltitude = new QComboBox;
  GpsAltitude->setEditable(false);
  GpsAltitude->addItem(tr("GPS"));
  GpsAltitude->addItem(tr("Pressure"));
  topLayout->addWidget(GpsAltitude, row, 1);
  row++;

  topLayout->setRowMinimumHeight( row++, 10);

  saveNmeaData = new QCheckBox (tr("Save NMEA Data to file"));
  topLayout->addWidget( saveNmeaData, row++, 0, 1, 2, Qt::AlignLeft );
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
  buttonBox->addStretch(2);
  buttonBox->addWidget(ok, 1);
  buttonBox->addSpacing(30);
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

  Qt::MatchFlags flags =
      static_cast<Qt::MatchFlags>(Qt::MatchStartsWith|Qt::MatchCaseSensitive);

  int index = GpsSource->findText( conf->getGpsSource(), flags );

  GpsSource->setCurrentIndex( index );
  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );
  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

void SettingsPageGPS4A::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsSource( GpsSource->currentText() );
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
