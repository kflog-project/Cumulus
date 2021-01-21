/***********************************************************************
**
**   SettingsPageGPS.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2002      by Andrè Somers,
**                 2007-2021 by Axel Pauli
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
#include "SettingsPageGPS.h"

SettingsPageGPS::SettingsPageGPS(QWidget *parent) : QWidget(parent),
WiFi1_PasswordIsHidden( true ),
WiFi2_PasswordIsHidden( true )
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

  QPixmap gps = GeneralConfig::instance()->loadPixmap("gps.png");
  int frame = 40;

  gpsOn = QPixmap( gps.width() + frame, gps.height() + frame );
  gpsOn.fill( Qt::green );

  gpsOff = QPixmap( gps.width() + frame, gps.height() + frame );
  gpsOff.fill( Qt::red );

  QPainter painter;
  painter.begin(&gpsOn);
  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  painter.drawPixmap( frame/2, frame/2, gps );
  painter.end();

  QPainter painterOff;
  painter.begin( &gpsOff );

  painter.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  painter.drawPixmap( frame/2, frame/2, gps );
  painter.end();

  QHBoxLayout *contentLayout = new QHBoxLayout;
  setLayout(contentLayout);

  QGridLayout *topLayout = new QGridLayout;
  topLayout->setHorizontalSpacing(20 * Layout::getIntScaledDensity() );
  topLayout->setVerticalSpacing(10 * Layout::getIntScaledDensity() );

  contentLayout->addLayout( topLayout, 10 );
  contentLayout->addSpacing( 25 * Layout::getIntScaledDensity() );

  int row=0;

  topLayout->addWidget( new QLabel( tr( "GPS Source:" ), this ), row, 0 );
  GpsSource = new QComboBox( this );
  topLayout->addWidget( GpsSource, row++, 1 );
  GpsSource->setEditable( false );
  GpsSource->addItem( tr( "$GP GPS (USA)" ) );
  GpsSource->addItem( tr( "$BD Beidou GPS (China)" ) );
  GpsSource->addItem( tr( "$GA Gallileo GPS (Europe)" ) );
  GpsSource->addItem( tr( "$GL Glonass GPS (Russia)" ) );
  GpsSource->addItem( tr( "$GN Combined GPS Systems" ) );

  topLayout->addWidget(new QLabel(tr("GPS Device:"), this), row, 0);
  GpsDev = new QComboBox(this);
  topLayout->addWidget(GpsDev, row++, 1);
  GpsDev->setEditable(true);
  GpsDev->setToolTip( tr("You can adapt a device entry by editing to your own needs.") );

#ifdef TOMTOMQLineEdit
  GpsDev->addItem(TOMTOM_DEVICE);
#endif

#ifndef MAEMO
  GpsDev->addItem( "/dev/ttyS0" );
  GpsDev->addItem( "/dev/ttyS1" );
  GpsDev->addItem( "/dev/ttyS2" );
  GpsDev->addItem( "/dev/ttyS3" );
  GpsDev->addItem( "/dev/ttyUSB0" ); // external USB device
#ifdef BLUEZ
  // Bluetooth adapter
  GpsDev->addItem( BT_ADAPTER );
#endif
  GpsDev->addItem( WIFI_1 ); // WiFi 1
  GpsDev->addItem( WIFI_2 ); // WiFi 2
  GpsDev->addItem( WIFI_1_2 ); // WiFi 1+2

  // add entry for NMEA simulator choice
  GpsDev->addItem( NMEASIM_DEVICE );
#else
  // Under Maemo there are only three predefined sources.
  GpsDev->addItem(MAEMO_LOCATION_SERVICE); // Maemo GPS Location Service
  // Bluetooth adapter
  GpsDev->addItem(BT_ADAPTER);
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  GpsDev->addItem(WIFI_1); // WiFi 1
  GpsDev->addItem(WIFI_2); // WiFi 2
  GpsDev->addItem(WIFI_1_2); // WiFi 1+2
  GpsDev->addItem(NMEASIM_DEVICE); // Cumulus NMEA simulator
#endif

  // catch selection changes of the GPS device combo box
  connect( GpsDev, SIGNAL( activated(const QString &) ), this,
           SLOT( slotGpsDeviceChanged(const QString&) ) );

  //----------------------------------------------------------------------------
  GpsSpeedLabel = new QLabel( tr( "Speed (bps):" ), this );
  topLayout->addWidget( GpsSpeedLabel, row, 0 );
  GpsSpeed = new QComboBox( this );
  GpsSpeed->setEditable( false );
  GpsSpeed->addItem( "230400" );
  GpsSpeed->addItem( "115200" );
  GpsSpeed->addItem( "57600" );
  GpsSpeed->addItem( "38400" );
  GpsSpeed->addItem( "19200" );
  GpsSpeed->addItem( "9600" );
  GpsSpeed->addItem( "4800" );
  GpsSpeed->addItem( "2400" );
  GpsSpeed->addItem( "1200" );
  GpsSpeed->addItem( "600" );
  topLayout->addWidget( GpsSpeed, row++, 1 );

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  topLayout->addWidget( new QLabel( tr( "Altitude Reference:" ), this ), row, 0 );
  GpsAltitude = new QComboBox( this );
  GpsAltitude->setEditable( false );
  topLayout->addWidget( GpsAltitude, row++, 1 );
  GpsAltitude->addItem( tr( "GPS" ) );
  GpsAltitude->addItem( tr( "Pressure" ) );

  //----------------------------------------------------------------------------
  topLayout->addWidget( new QLabel( tr( "Pressure Supplier:" ), this ), row, 0 );

  PressureDevice = new QComboBox();
  PressureDevice->setToolTip( tr("Device which delivers pressure altitude") );
  PressureDevice->setEnabled( false );
  PressureDevice->setObjectName("DeviceSelection");
  PressureDevice->setEditable(false);
  PressureDevice->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  PressureDevice->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifdef QSCROLLER
    m_cmbType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    QScroller::grabGesture( m_cmbType->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
    m_cmbType->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    QtScroller::grabGesture( m_cmbType->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  PressureDevice->addItems( GeneralConfig::getPressureDevicesList() );
  topLayout->addWidget( PressureDevice, row++, 1);

  QLabel *label = new QLabel( tr( "WiFi-1 IP : Port" ) );
  topLayout->addWidget( label, row, 0 );

  //----------------------------------------------------------------------------
  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

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

  label = new QLabel(" : ", this );
  hbox->addWidget( label );

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
#if 0
  label = new QLabel( tr( "WiFi-1 Password" ) );
  topLayout->addWidget( label, row, 0 );

  hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

  Qt::InputMethodHints imh = Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText;

  WiFi1_Password = new QLineEdit( this );
  WiFi1_Password->setInputMethodHints( imh | WiFi1_Password->inputMethodHints() );
  WiFi1_Password->setEchoMode( QLineEdit::Password );
  hbox->addWidget( WiFi1_Password );

  WiFi1_PwToggle = new QPushButton( tr("Show") );
  hbox->addWidget( WiFi1_PwToggle );
  topLayout->addLayout( hbox, row++, 1 );

  connect( WiFi1_PwToggle, SIGNAL(clicked() ), SLOT(slotTogglePw1() ) );
#endif

  //----------------------------------------------------------------------------
  label = new QLabel( tr( "WiFi-2 IP : Port" ), this );
  topLayout->addWidget( label, row, 0 );

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

  label = new QLabel(" : ", this );
  hbox->addWidget( label );

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
#if 0
  label = new QLabel( tr( "WiFi-2 Password" ) );
  topLayout->addWidget( label, row, 0 );

  hbox = new QHBoxLayout();
  hbox->setMargin( 0 );

  imh = Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText;

  WiFi2_Password = new QLineEdit( this );
  WiFi2_Password->setInputMethodHints( imh | WiFi2_Password->inputMethodHints() );
  WiFi2_Password->setEchoMode( QLineEdit::Password );
  hbox->addWidget( WiFi2_Password );

  WiFi2_PwToggle = new QPushButton( tr("Show") );
  hbox->addWidget( WiFi2_PwToggle );
  topLayout->addLayout( hbox, row++, 1 );

  connect( WiFi2_PwToggle, SIGNAL(clicked() ), SLOT(slotTogglePw2() ) );
#endif

#ifndef MAEMO
  topLayout->setRowMinimumHeight( row++, 10);

  checkSyncSystemClock = new QCheckBox (tr("Sync Clock"), this);
  topLayout->addWidget(checkSyncSystemClock, row, 0 );
  row++;
#endif

  saveNmeaData = new QCheckBox (tr("Save NMEA Data"), this);
  topLayout->addWidget(saveNmeaData, row, 0 );
  row++;

  topLayout->setRowStretch( row++, 10 );
  topLayout->setColumnStretch( 1, 10 );

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

  GpsToggle = new QPushButton(this);
  GpsToggle->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("gps.png")));
  GpsToggle->setIconSize(QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)));
  GpsToggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

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
  connect(GpsToggle, SIGNAL(pressed()), this, SLOT(slotToggleGps()));
  connect(ok, SIGNAL(pressed()), this, SLOT(slotAccept()));
  connect(cancel, SIGNAL(pressed()), this, SLOT(slotReject()));

  QVBoxLayout *buttonBox = new QVBoxLayout;
  buttonBox->setSpacing( 0 );
  buttonBox->addWidget( help, 1 );
  buttonBox->addStretch( 2 );
  buttonBox->addWidget( cancel, 1 );
  buttonBox->addSpacing( 30 );
  buttonBox->addWidget( ok, 1 );
  buttonBox->addStretch( 2 );
  buttonBox->addWidget( GpsToggle, 1 );
  buttonBox->addSpacing( 10 );
  buttonBox->addWidget( titlePix );
  contentLayout->addLayout( buttonBox );
  load();
}

SettingsPageGPS::~SettingsPageGPS()
{
}

/**
 * Called, if the GPS toggle button is pressed.
 */
void SettingsPageGPS::slotToggleGps()
{
  bool gpsSwitchState = GeneralConfig::instance()->getGpsSwitchState();

  GeneralConfig::instance()->setGpsSwitchState( ! gpsSwitchState );
  updateGpsToggle();

  // Disable GPS switch button for some seconds
  GpsToggle->setEnabled( false );
  QTimer::singleShot( 10000, this, SLOT( slotEnableGpsToggle() ) );

  emit userGpsSwitchRequest();
}

void SettingsPageGPS::updateGpsToggle()
{
  bool gpsSwitchState = GeneralConfig::instance()->getGpsSwitchState();

  if( gpsSwitchState == true )
    {
      // Switch on connection to GPS receiver
      GpsToggle->setIcon( QIcon( gpsOff ) );
      GpsToggle->setToolTip( tr( "Press button to disconnect from GPS" ) );
    }
  else
    {
      // Switch off connection to GPS receiver
      GpsToggle->setIcon( QIcon( gpsOn ) );
      GpsToggle->setToolTip( tr( "Press button to connect to GPS" ) );
    }
}

void SettingsPageGPS::slotEnableGpsToggle()
{
  GpsToggle->setEnabled( true );
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
  if( save() == false )
    {
      return;
    }

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

  QString rate = QString::number( conf->getGpsSpeed() );

  for (int i=0; i < GpsSpeed->count(); i++)
    {
      if (GpsSpeed->itemText(i) == rate)
        {
          GpsSpeed->setCurrentIndex(i);
          break;
        }
    }

  if( GpsDev->currentText().startsWith( "/dev/tty" ) == false )
    {
      // switch off access to speed box, when no tty is selected
      GpsSpeed->setVisible( false );
      GpsSpeedLabel->setVisible( false );
    }

  WiFi1_IP->setText( conf->getGpsWlanIp1() );
  WiFi1_Port->setText( conf->getGpsWlanPort1() );
#if 0
  WiFi1_Password->setText( conf->getGpsWlanPassword1() );
#endif
  WiFi2_IP->setText( conf->getGpsWlanIp2() );
  WiFi2_Port->setText( conf->getGpsWlanPort2() );
#if 0
  WiFi2_Password->setText( conf->getGpsWlanPassword2() );
#endif

#ifndef MAEMO
  checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
#endif

  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );

  updateGpsToggle();
}

/** Called to initiate saving to the configuration file. */
bool SettingsPageGPS::save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsSource( GpsSource->currentText() );
  conf->setGpsDevice( GpsDev->currentText() );
  conf->setGpsAltitude( GpsNmea::DeliveredAltitude(GpsAltitude->currentIndex()) );
  conf->setGpsSpeed( GpsSpeed->currentText().toInt() );

  if( PressureDevice->isEnabled() == true )
    {
      conf->setPressureDevice( PressureDevice->currentText() );
      emit newPressureDevice( PressureDevice->currentText() ); // informs GpsNmea
    }

  QString gpsDevice = GpsDev->currentText();

  if( gpsDevice == WIFI_1 || gpsDevice == WIFI_1_2 )
    {
      if( WiFi1_IP->text().isEmpty() || WiFi1_Port->text().isEmpty() )
        {
          // IP address and port are required, when service is switched on!
          QString msg = QString(
              tr( "<html>WiFi-1 IP or Port entry are missing!"
                  "<br><br>Please add the missing items.</html>" ) );

           QMessageBox mb( QMessageBox::Warning,
                           tr( "WiFi-1 data missing" ),
                           msg,
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
    }

  if( gpsDevice == WIFI_2 || gpsDevice == WIFI_1_2 )
    {
      if( WiFi2_IP->text().isEmpty() || WiFi2_Port->text().isEmpty() )
        {
          // IP address and port are required, when service is switched on!
          QString msg = QString(
              tr( "<html>WiFi-2 IP or Port entry are missing!"
                  "<br><br>Please add the missing items.</html>" ) );

           QMessageBox mb( QMessageBox::Warning,
                           tr( "WiFi-2 data missing" ),
                           msg,
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
    }

  conf->setGpsWlanIp1( WiFi1_IP->text() );
  conf->setGpsWlanPort1( WiFi1_Port->text() );
#if 0
  conf->setGpsWlanPassword1( WiFi1_Password->text() );
#endif

  conf->setGpsWlanIp2( WiFi2_IP->text() );
  conf->setGpsWlanPort2( WiFi2_Port->text() );
#if 0
  conf->setGpsWlanPassword2( WiFi2_Password->text() );
#endif

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
  return( true );
}

/**
 * Called when the GPS device selection is changed to toggle the visibility
 * to the GPS speed box in dependency of the necessity.
 */
void SettingsPageGPS::slotGpsDeviceChanged( const QString &text )
{
  qDebug() << "SettingsPageGPS::slotGpsDeviceChanged()" << text;

  if( text.startsWith( "/dev/tty" ) == true )
    {
      // Switch off visibility to speed box, when no tty is selected.
      GpsSpeed->setVisible( true );
      GpsSpeedLabel->setVisible( true );
      return;
    }

  GpsSpeed->setVisible( false );
  GpsSpeedLabel->setVisible( false );
}

/**
 * Called when the GPS altitude reference is changed.
 */
void SettingsPageGPS::slotGpsAltitudeChanged( int index )
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

/**
 * Called, if the password toggle button 1 is pressed.
 */
void SettingsPageGPS::slotTogglePw1()
{
  if( WiFi1_PasswordIsHidden == true )
    {
      WiFi1_PasswordIsHidden = false;
      WiFi1_Password->setEchoMode( QLineEdit::Normal );
      WiFi1_PwToggle->setText( tr( "Hide" ) );
    }
  else
    {
      WiFi1_PasswordIsHidden = true;
      WiFi1_Password->setEchoMode( QLineEdit::Password );
      WiFi1_PwToggle->setText( tr( "Show" ) );
    }
}

/**
 * Called, if the password toggle button 1 is pressed.
 */
void SettingsPageGPS::slotTogglePw2()
{
  if( WiFi2_PasswordIsHidden == true )
    {
      WiFi2_PasswordIsHidden = false;
      WiFi2_Password->setEchoMode( QLineEdit::Normal );
      WiFi2_PwToggle->setText( tr( "Hide" ) );
    }
  else
    {
      WiFi2_PasswordIsHidden = true;
      WiFi2_Password->setEchoMode( QLineEdit::Password );
      WiFi2_PwToggle->setText( tr( "Show" ) );
    }
}
