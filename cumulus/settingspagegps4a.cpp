/***********************************************************************
**
**   settingspagegps4a.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2012-2021 by Axel Pauli
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

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "gpsnmea.h"
#include "helpbrowser.h"
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

  topLayout->addWidget(new QLabel(tr("Pressure Supplier:"), this), row, 0);

  PressureDevice = new QComboBox();
  PressureDevice->setToolTip( tr("Device which delivers pressure altitude.") );
  PressureDevice->setEnabled( false );
  PressureDevice->setObjectName("DeviceSelection");
  PressureDevice->setEditable(false);
  PressureDevice->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  PressureDevice->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifdef QSCROLLER
  PressureDevice->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    QScroller::grabGesture( PressureDevice->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
    PressureDevice->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    QtScroller::grabGesture( PressureDevice->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  PressureDevice->addItems( GeneralConfig::getPressureDevicesList() );
  topLayout->addWidget( PressureDevice, row++, 1);

  topLayout->addWidget(new QLabel(tr("WLAN IP:")), row, 0);
  wlanIpAddress = new QLineEdit;
  topLayout->addWidget(wlanIpAddress, row, 1);
  row++;

  topLayout->addWidget(new QLabel(tr("WLAN Port:")), row, 0);
  wlanPort = new QLineEdit;
  topLayout->addWidget(wlanPort, row, 1);
  row++;

  topLayout->addWidget(new QLabel(tr("WLAN Password:")), row, 0);
  wlanPassword = new QLineEdit;
  topLayout->addWidget(wlanPassword, row, 1);
  row++;

  topLayout->setRowMinimumHeight( row++, 10);

  saveNmeaData = new QCheckBox (tr("Save NMEA Data to file"));
  topLayout->addWidget( saveNmeaData, row++, 0, 1, 2, Qt::AlignLeft );
  topLayout->setRowStretch( row, 10 );

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

void SettingsPageGPS4A::slotHelp()
{
  QString file = "cumulus-settings-gps.html";

  HelpBrowser *hb = new HelpBrowser( this, file );
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->setVisible( true );
}

void SettingsPageGPS4A::slotAccept()
{
  if ( save() == false )
    {
      return;
    }

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
      static_cast<Qt::MatchFlags>(Qt::MatchStartsWith | Qt::MatchCaseSensitive);

  int index = GpsSource->findText( conf->getGpsSource(), flags );

  GpsSource->setCurrentIndex( index );
  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );

  // activate altitude change signal
  connect( GpsAltitude, SIGNAL(currentIndexChanged(int) ),
           this, SLOT(slotGpsAltitudeChanged(int)) );

  int idx = PressureDevice->findText( conf->getPressureDevice() );

  if( idx == -1 )
    {
      idx = 0;
    }

  // select last stored pressure device
  PressureDevice->setCurrentIndex( idx );

  if( conf->getGpsAltitude() == GpsNmea::PRESSURE )
    {
      PressureDevice->setEnabled( true );
    }
  else
    {
      PressureDevice->setEnabled( false );
    }

  wlanIpAddress->setText( conf->getGpsWlanIp() );
  wlanPort->setText( conf->getGpsWlanPort() );
  wlanPassword->setText( conf->getGpsWlanPassword() );
  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

bool SettingsPageGPS4A::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsSource( GpsSource->currentText() );
  conf->setGpsAltitude( GpsNmea::DeliveredAltitude( GpsAltitude->currentIndex()) );

  if( PressureDevice->isEnabled() == true )
    {
      conf->setPressureDevice( PressureDevice->currentText() );
      emit newPressureDevice( PressureDevice->currentText() ); // informs GpsNmea
    }

  // Save old settings to check for done updates
  QString oldIp = conf->getGpsWlanIp();
  QString oldPort = conf->getGpsWlanPort();
  QString oldPassword = conf->getGpsWlanPassword();

  // Save current settings.
  QString curIp = wlanIpAddress->text().trimmed();
  QString curPort = wlanPort->text().trimmed();
  QString curPassword = wlanPassword->text().trimmed();

  // Check, if port is a number.
  bool ok = false;
  ushort portNum = wlanPort->text().trimmed().toUShort(&ok, 10);

  if( wlanPort->text().trimmed().isEmpty() || ok == false || portNum == 0 )
    {
      QMessageBox mb( QMessageBox::Warning,
                      tr("Port settings invalid!"),
                      tr("Port must be a number and > 0!"),
                      QMessageBox::Ok,
                      this );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                       height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif
      mb.exec();
      return false;
    }

  conf->setGpsWlanIp( curIp );
  conf->setGpsWlanPort( curPort );
  conf->setGpsWlanPassword( curPassword );

  if( oldIp != curIp || oldPort != curPort || oldPassword != curPassword )
    {
      emit ipSettingsChanged();
    }

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

  return true;
}

/**
 * Called when the GPS altitude reference is changed.
 */
void SettingsPageGPS4A::slotGpsAltitudeChanged( int index )
{
  if( index == 1 )
    {
      // Altitude reference Pressure is selected, enable device selection.
      PressureDevice->setEnabled( true );
    }
  else
    {
      // Altitude reference GPS is selected, disable device selection.
      PressureDevice->setEnabled( false );
    }
}
