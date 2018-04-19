/***********************************************************************
**
**   settingspagegps.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2002      by Andrè Somers,
**                 2007-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * This widget is used to define the GPS interface parameters.
 * The user can select different source devices and some special
 * GPS parameters.
 *
 * There is a difference in the provided options between normal Desktop
 * and Maemo. Under Maemo RS232 devices are supported only via USB.
 */

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "generalconfig.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "helpbrowser.h"
#include "hwinfo.h"
#include "layout.h"
#include "settingspagegps.h"

SettingsPageGPS::SettingsPageGPS(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS");
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
  contentLayout->addLayout(topLayout);

  int row=0;

  topLayout->addWidget(new QLabel(tr("GPS Source:"), this), row, 0);
  GpsSource = new QComboBox(this);
  topLayout->addWidget(GpsSource, row++, 1);
  GpsSource->setEditable(false);
  GpsSource->addItem( tr("$GP GPS (USA)") );
  GpsSource->addItem( tr("$BD Beidou GPS (China)") );
  GpsSource->addItem( tr("$GA Gallileo GPS (Europe)") );
  GpsSource->addItem( tr("$GL Glonass GPS (Russia)") );
  GpsSource->addItem( tr("$GN Combined GPS Systems") );

  topLayout->setColumnStretch(2, 10);

  topLayout->addWidget(new QLabel(tr("GPS Device:"), this), row, 0);
  GpsDev = new QComboBox(this);
  topLayout->addWidget(GpsDev, row++, 1);
  GpsDev->setEditable(true);

#ifdef TOMTOM
  GpsDev->addItem(TOMTOM_DEVICE);
#endif

#ifndef MAEMO
  GpsDev->addItem("/dev/ttyS0");
  GpsDev->addItem("/dev/ttyS1");
  GpsDev->addItem("/dev/ttyS2");
  GpsDev->addItem("/dev/ttyS3");
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
#ifdef BLUEZ
  // Bluetooth default devices
  GpsDev->addItem("/dev/rfcomm0");
  GpsDev->addItem("/dev/rfcomm1");
#endif
  // add entry for NMEA simulator choice
  GpsDev->addItem(NMEASIM_DEVICE);
#else
  // Under Maemo there are only three predefined sources.
  GpsDev->addItem(MAEMO_LOCATION_SERVICE); // Maemo GPS Location Service
  // Bluetooth default adapter
  GpsDev->addItem(BT_ADAPTER);
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  GpsDev->addItem(NMEASIM_DEVICE); // Cumulus NMEA simulator
#endif

  // catch selection changes of the GPS device combo box
  connect( GpsDev, SIGNAL(activated(const QString &)),
           this, SLOT(slot_gpsDeviceChanged(const QString&)) );

  topLayout->addWidget(new QLabel(tr("Speed (bps):"), this), row, 0);
  GpsSpeed = new QComboBox(this);
  GpsSpeed->setEditable(false);
  GpsSpeed->addItem("230400");
  GpsSpeed->addItem("115200");
  GpsSpeed->addItem("57600");
  GpsSpeed->addItem("38400");
  GpsSpeed->addItem("19200");
  GpsSpeed->addItem("9600");
  GpsSpeed->addItem("4800");
  GpsSpeed->addItem("2400");
  GpsSpeed->addItem("1200");
  GpsSpeed->addItem("600");
  topLayout->addWidget(GpsSpeed, row++, 1);

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  topLayout->addWidget(new QLabel(tr("Altitude Reference:"), this),row,0);
  GpsAltitude = new QComboBox(this);
  GpsAltitude->setEditable(false);
  topLayout->addWidget(GpsAltitude,row++,1);
  GpsAltitude->addItem(tr("GPS"));
  GpsAltitude->addItem(tr("Pressure"));

#ifndef MAEMO
  topLayout->setRowMinimumHeight( row++, 10);

  checkSyncSystemClock = new QCheckBox (tr("Sync Clock"), this);
  topLayout->addWidget(checkSyncSystemClock, row, 0 );
  row++;
#endif

  saveNmeaData = new QCheckBox (tr("Save NMEA Data"), this);
  topLayout->addWidget(saveNmeaData, row, 0 );
  row++;

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2,10);

  // search for GPS device to be selected
  bool found = false;

  QString devText = GeneralConfig::instance()->getGpsDevice();

  // select last saved device, if possible
  for (int i=0; i < GpsDev->count(); i++)
    {
      if (GpsDev->itemText(i) == devText)
        {
          GpsDev->setCurrentIndex(i);
          found = true;
          break;
        }
    }

  // Stored device not found, we assume, it was added by hand.
  // Therefore we do add it to the list too.
  if ( found == false )
    {
#ifndef MAEMO
      GpsDev->addItem( devText );
      GpsDev->setCurrentIndex(GpsDev->findText( devText ));
#else
      // On Maemo we select the first entry, the Maemo GPS daemon as default
      GpsDev->setCurrentIndex(0);
#endif
    }

  QPushButton *help = new QPushButton(this);
  help->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("help32.png")));
  help->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  help->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

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

  connect(help, SIGNAL(pressed()), this, SLOT(slotHelp()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing(0);
  buttonBox->addWidget(help, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(cancel, 1);
  buttonBox->addSpacing(30);
  buttonBox->addWidget(ok, 1);
  buttonBox->addStretch(2);
  buttonBox->addWidget(titlePix);
  contentLayout->addLayout(buttonBox);
  load();
}

SettingsPageGPS::~SettingsPageGPS()
{
}

void SettingsPageGPS::slotHelp()
{
  QString file = "cumulus-settings-gps.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageGPS::slotAccept()
{
  save();
  emit settingsChanged();
  QWidget::close();
}

void SettingsPageGPS::slotReject()
{
  QWidget::close();
}

/** Called to initiate loading of the configuration file */
void SettingsPageGPS::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  Qt::MatchFlags flags =
      static_cast<Qt::MatchFlags>(Qt::MatchStartsWith|Qt::MatchCaseSensitive);

  int index = GpsSource->findText( conf->getGpsSource(), flags );

  GpsSource->setCurrentIndex( index );
  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );

  QString rate = QString::number( conf->getGpsSpeed() );

  for (int i=0; i < GpsSpeed->count(); i++)
    {
      if (GpsSpeed->itemText(i) == rate)
        {
          GpsSpeed->setCurrentIndex(i);
          break;
        }
    }

#ifdef MAEMO
  if( GpsDev->currentText() != "/dev/ttyUSB0" )
    {
      // switch off access to speed box, when USB is not selected
      GpsSpeed->setEnabled( false );
    }
#endif

  if( GpsDev->currentText() == NMEASIM_DEVICE ||
      GpsDev->currentText() == TOMTOM_DEVICE ||
      GpsDev->currentText() == BT_ADAPTER )
    {
      // switch off access to speed box, when NMEA Simulator, TomTom or
      // BT adapter are selected
      GpsSpeed->setEnabled( false );
    }

#ifndef MAEMO
  checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
#endif

  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

/** Called to initiate saving to the configuration file. */
void SettingsPageGPS::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsSource( GpsSource->currentText() );
  conf->setGpsDevice( GpsDev->currentText() );
  conf->setGpsAltitude( GpsNmea::DeliveredAltitude(GpsAltitude->currentIndex()) );
  conf->setGpsSpeed( GpsSpeed->currentText().toInt() );

#ifndef MAEMO
  conf->setGpsSyncSystemClock( checkSyncSystemClock->isChecked() );
#endif

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

  conf->save();
}

/**
 * Called when the GPS device selection is changed to toggle the access
 * to the GPS speed box in dependency of the necessity.
 */
void SettingsPageGPS::slot_gpsDeviceChanged( const QString& text )
{
  // qDebug("text=%s", text.toLatin1().data());

  if( text == NMEASIM_DEVICE || text == TOMTOM_DEVICE || text == BT_ADAPTER )
    {
      // Switch off access to speed box, when a named pipe is used.
      GpsSpeed->setEnabled( false );
      return;
    }

#ifdef MAEMO
  if( ! text.startsWith("/dev/ttyUSB") )
    {
      // Switch off access to speed box, when USB is not selected.
      // That is done only for Maemo because in those cases a speed
      // entry is not necessary.
      GpsSpeed->setEnabled( false );
      return;
    }
#endif

  GpsSpeed->setEnabled( true );
}
