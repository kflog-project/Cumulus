/***********************************************************************
**
**   settingspagegps4a.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2012-2026 by Axel Pauli
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
#include "MainWindow.h"
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
  GpsSource = Layout::getComboBox();
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

  //----------------------------------------------------------------------------
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin( 0 );

  WiFi1CB = new QCheckBox( tr( "WiFi-1 IP : Port" ) );
  topLayout->addWidget( WiFi1CB, row, 0 );

  WiFi1_IP = new NumberEditor( this );
  WiFi1_IP->disableNumberCheck( true );
  WiFi1_IP->allowEmptyResult( true );
  WiFi1_IP->setDecimalVisible( true );
  WiFi1_IP->setPmVisible( false );
  WiFi1_IP->setMaxLength( 15 );
  WiFi1_IP->setAlignment( Qt::AlignLeft );
  WiFi1_IP->setTitle( tr( "Enter a IP address" ) );
  WiFi1_IP->setTip( tr( "Enter a IP address xxx.xxx.xxx.xxx )" ) );
  WiFi1_IP->setText( "" );
  WiFi1_IP->setValidator( new QRegExpValidator( QRegExp( "([0-9]{1,3}\\.){3}[0-9]{1,3}" ), this ) );

  hbox->addWidget( WiFi1_IP, 3 );

  label1 = new QLabel(" : ", this );
  hbox->addWidget( label1 );

  WiFi1_Port = new NumberEditor( this );
  WiFi1_Port->allowEmptyResult( true );
  WiFi1_Port->setDecimalVisible( false );
  WiFi1_Port->setPmVisible( false );
  WiFi1_Port->setMaxLength( 5 );
  WiFi1_Port->setAlignment( Qt::AlignLeft );
  WiFi1_Port->setRange( 1, 65535 );
  WiFi1_Port->setTitle( tr( "Enter a TCP port" ) );
  WiFi1_Port->setTip( tr( "Enter a TCP port 1...65535)" ) );
  WiFi1_Port->setText( "" );

  hbox->addWidget( WiFi1_Port, 1 );
  topLayout->addLayout( hbox, row++, 1 );

  //----------------------------------------------------------------------------
  WiFi2CB = new QCheckBox( tr( "WiFi-2 IP : Port" ) );
  topLayout->addWidget( WiFi2CB, row, 0 );

  hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

  WiFi2_IP = new NumberEditor( this );
  WiFi2_IP->disableNumberCheck( true );
  WiFi2_IP->allowEmptyResult( true );
  WiFi2_IP->setDecimalVisible( true );
  WiFi2_IP->setPmVisible( false );
  WiFi2_IP->setMaxLength( 15 );
  WiFi2_IP->setAlignment( Qt::AlignLeft );
  WiFi2_IP->setTitle( tr( "Enter a IP address" ) );
  WiFi2_IP->setTip( tr( "Enter a IP address xxx.xxx.xxx.xxx )" ) );
  WiFi2_IP->setText( "" );
  WiFi2_IP->setValidator( new QRegExpValidator( QRegExp( "([0-9]{1,3}\\.){3}[0-9]{1,3}" ), this ) );

  hbox->addWidget( WiFi2_IP, 3 );

  label2 = new QLabel(" : ", this );
  hbox->addWidget( label2 );

  WiFi2_Port = new NumberEditor( this );
  WiFi2_Port->allowEmptyResult( true );
  WiFi2_Port->setDecimalVisible( false );
  WiFi2_Port->setPmVisible( false );
  WiFi2_Port->setMaxLength( 5 );
  WiFi2_Port->setAlignment( Qt::AlignLeft );
  WiFi2_Port->setRange( 1, 65535 );
  WiFi2_Port->setTitle( tr( "Enter a TCP port" ) );
  WiFi2_Port->setTip( tr( "Enter a TCP port 1...65535)" ) );
  WiFi2_Port->setText( "" );

  hbox->addWidget( WiFi2_Port, 1 );
  topLayout->addLayout( hbox, row++, 1 );

  //----------------------------------------------------------------------------
  WiFi3CB = new QCheckBox( tr( "WiFi-KRT2 IP : Port" ) );
  topLayout->addWidget( WiFi3CB, row, 0 );

  hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

  WiFi3_IP = new NumberEditor( this );
  WiFi3_IP->disableNumberCheck( true );
  WiFi3_IP->allowEmptyResult( true );
  WiFi3_IP->setDecimalVisible( true );
  WiFi3_IP->setPmVisible( false );
  WiFi3_IP->setMaxLength( 15 );
  WiFi3_IP->setAlignment( Qt::AlignLeft );
  WiFi3_IP->setTitle( tr( "Enter a IP address" ) );
  WiFi3_IP->setTip( tr( "Enter a IP address xxx.xxx.xxx.xxx )" ) );
  WiFi3_IP->setText( "" );
  WiFi3_IP->setValidator( new QRegExpValidator( QRegExp( "([0-9]{1,3}\\.){3}[0-9]{1,3}" ), this ) );

  hbox->addWidget( WiFi3_IP, 3 );

  label3 = new QLabel(" : ", this );
  hbox->addWidget( label3 );

  WiFi3_Port = new NumberEditor( this );
  WiFi3_Port->allowEmptyResult( true );
  WiFi3_Port->setDecimalVisible( false );
  WiFi3_Port->setPmVisible( false );
  WiFi3_Port->setMaxLength( 5 );
  WiFi3_Port->setAlignment( Qt::AlignLeft );
  WiFi3_Port->setRange( 1, 65535 );
  WiFi3_Port->setTitle( tr( "Enter a TCP port" ) );
  WiFi3_Port->setTip( tr( "Enter a TCP port 1...65535)" ) );
  WiFi3_Port->setText( "" );

  hbox->addWidget( WiFi3_Port, 1 );
  topLayout->addLayout( hbox, row++, 1 );

  //----------------------------------------------------------------------------

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

  WiFi1_IP->setText( conf->getGpsWlanIp1() );
  WiFi1_Port->setText( conf->getGpsWlanPort1() );
  WiFi1CB->setChecked( conf->getGpsWlanCB1() );
  WiFi2_IP->setText( conf->getGpsWlanIp2() );
  WiFi2_Port->setText( conf->getGpsWlanPort2() );
  WiFi2CB->setChecked( conf->getGpsWlanCB2() );
  WiFi3_IP->setText( conf->getGpsWlanIp3() );
  WiFi3_Port->setText( conf->getGpsWlanPort3() );
  WiFi3CB->setChecked( conf->getGpsWlanCB3() );
  //toggleWiFiMenu();

  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );
}

/** Called to toggle the WiFi menu lines. */
void SettingsPageGPS4A::toggleWiFiMenu()
{
#if 0
  if( GpsDev->currentText().startsWith( "WiFi" ) == true )
    {
      WiFi1_IP->show();
      WiFi1_Port->show();
      WiFi1CB->show();
      WiFi2_IP->show();
      WiFi2_Port->show();
      WiFi2CB->show();
      WiFi3_IP->show();
      WiFi3_Port->show();
      WiFi3CB->show();
      label1->show();
      label2->show();
      label3->show();
    }
  else
    {
      WiFi1_IP->hide();
      WiFi1_Port->hide();
      WiFi1CB->hide();
      WiFi2_IP->hide();
      WiFi2_Port->hide();
      WiFi2CB->hide();
      WiFi3_IP->hide();
      WiFi3_Port->hide();
      WiFi3CB->hide();
      label1->hide();
      label2->hide();
      label3->hide();
    }
#endif
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

  QString ip1 = conf->getGpsWlanIp1();
  QString port1 = conf->getGpsWlanPort1();
  bool cb1 = conf->getGpsWlanCB1();
  QString ip2 = conf->getGpsWlanIp2();
  QString port2 = conf->getGpsWlanPort2();
  bool cb2 = conf->getGpsWlanCB2();
  QString ip3 = conf->getGpsWlanIp3();
  QString port3 = conf->getGpsWlanPort3();
  bool cb3 = conf->getGpsWlanCB3();

  QString ip1n = WiFi1_IP->text();
  QString port1n = WiFi1_Port->text();
  bool cb1n = WiFi1CB->isChecked();
  QString ip2n = WiFi2_IP->text();
  QString port2n = WiFi2_Port->text();
  bool cb2n = WiFi2CB->isChecked();
  QString ip3n = WiFi3_IP->text();
  QString port3n = WiFi3_Port->text();
  bool cb3n = WiFi3CB->isChecked();

  conf->setGpsWlanIp2( WiFi2_IP->text() );
  conf->setGpsWlanPort2( WiFi2_Port->text() );
  conf->setGpsWlanCB2( WiFi2CB->isChecked() );

  conf->setGpsWlanIp3( WiFi3_IP->text() );
  conf->setGpsWlanPort3( WiFi3_Port->text() );
  conf->setGpsWlanCB3( WiFi3CB->isChecked() );

  if( (WiFi1CB->isChecked() && (WiFi1_IP->text().isEmpty() || WiFi1_Port->text().isEmpty())) ||
      (WiFi2CB->isChecked() && (WiFi2_IP->text().isEmpty() || WiFi2_Port->text().isEmpty())) ||
      (WiFi3CB->isChecked() && (WiFi3_IP->text().isEmpty() || WiFi3_Port->text().isEmpty())) )
    {
      qDebug() << "WiFi Fehlerbox sollte gezeigt werden.";

      // IP address and port are required, when service is switched on!
      QString info = QString(
          tr( "<html>WiFi entry is activated but IP or Port are not set!"
              "<br><br>Please add the missing items.</html>" ) );

      Layout::messageBox( QMessageBox::Critical,
                          tr("IP items missing"),
                          info,
                          QMessageBox::Ok,
                          QMessageBox::Ok,
                          MainWindow::mainWindow() );

      return false;
    }

  conf->setGpsWlanIp1( WiFi1_IP->text() );
  conf->setGpsWlanPort1( WiFi1_Port->text() );
  conf->setGpsWlanCB1( WiFi1CB->isChecked() );

  conf->setGpsWlanIp2( WiFi2_IP->text() );
  conf->setGpsWlanPort2( WiFi2_Port->text() );
  conf->setGpsWlanCB2( WiFi2CB->isChecked() );

  conf->setGpsWlanIp3( WiFi3_IP->text() );
  conf->setGpsWlanPort3( WiFi3_Port->text() );
  conf->setGpsWlanCB3( WiFi3CB->isChecked() );

  if( ip1 != ip1n || ip2 != ip2n || ip3 != ip3n ||
      port1 != port1n || port2 != port2n || port3 != port3n ||
      cb1 != cb1n || cb2 != cb2n || cb3 != cb3n )
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
