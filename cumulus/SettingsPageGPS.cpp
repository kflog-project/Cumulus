/***********************************************************************
**
**   SettingsPageGPS.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2002      by Andrè Somers,
**                 2007-2025 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * This widget is used to define the GPS/KRT2 interface parameters.
 * The user can select different source devices and some special
 * GPS parameters.
 */

#include <HelpBrowser.h>
#include <QtWidgets>

#include "generalconfig.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "hwinfo.h"
#include "layout.h"
#include "SettingsPageGPS.h"

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

#ifdef BLUEZ
  btsAgent = nullptr;
  btdAgent = nullptr;
#endif

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

  GpsDev->addItem( "/dev/ttyS0" );
  GpsDev->addItem( "/dev/ttyS1" );
  GpsDev->addItem( "/dev/ttyS2" );
  GpsDev->addItem( "/dev/ttyS3" );
  GpsDev->addItem( "/dev/ttyUSB0" ); // external USB device
  GpsDev->addItem( "/dev/ttyUSB1" ); // external USB device
#ifdef BLUEZ
  // Bluetooth adapter
  GpsDev->addItem( BT_ADAPTER );
#endif
  GpsDev->addItem( "WiFi" ); // WiFi interface shall be used

  // add entry for NMEA simulator choice
  GpsDev->addItem( NMEASIM_DEVICE );

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

  //----------------------------------------------------------------------------
  BtListLabel = new QLabel( tr( "BT Devices:" ), this );
  topLayout->addWidget( BtListLabel, row, 0 );
  BtList= new QComboBox( this );
  BtList->setEditable( false );
  searchBts = new QPushButton( tr("Scan") );

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setMargin( 0 );
  hbox->addWidget( BtList, 3 );
  hbox->addWidget( searchBts );
  topLayout->addLayout( hbox, row++, 1 );

  // Switch BT menu line off
  toggleBtMenu( false );

  connect( searchBts, SIGNAL(pressed()), SLOT(slotSearchBtDevices()) );

  //----------------------------------------------------------------------------
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

  PressureDevice = new QComboBox( this );
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

  WiFi1CB = new QCheckBox( tr( "WiFi-1 IP : Port" ) );
  topLayout->addWidget( WiFi1CB, row, 0 );

  //----------------------------------------------------------------------------
  hbox = new QHBoxLayout();
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
      GpsDev->addItem( devText );
      GpsDev->setCurrentIndex(GpsDev->findText( devText ));
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

  if( GpsDev->currentText().startsWith( BT_ADAPTER ) == true )
    {
      toggleBtMenu( true );
      loadBtDeviceList();
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
  toggleWiFiMenu();

  saveNmeaData->setChecked( conf->getGpsNmeaLogState() );

  updateGpsToggle();
}

/** Called to toggle the WiFi menu lines. */
void SettingsPageGPS::toggleWiFiMenu()
{
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

#ifdef BLUEZ

  if( GpsDev->currentText() == BT_ADAPTER )
    {
      // Save current selected BT device from combo box, when BT is selected
      // as GPS source.
      QPair<QString, QString> btDev;
      btDev.first = BtList->currentText();
      btDev.second = BtList->currentData().toString();
      conf->setGpsBtDevice( btDev );

      // Save BT list
      QMap<QString, QVariant> btDevList;

      for( int i = 0; i < BtList->count(); i++ )
        {
          BtList->setCurrentIndex(i);
          btDevList.insert( BtList->currentText(), BtList->currentData() );
        }

      conf->setGpsBtDeviceList( btDevList );

      if( btDev.first.size() == 0 )
        {
          QString info = QString(
              tr( "<html>no BT device is defined!"
                  "<br><br>Please make a scan or<br>select another service.</html>" ) );

           messageBox( QMessageBox::Warning,
                       tr( "BT device data missing" ),
                       info );

           return false;
        }
    }

#endif

  if( gpsDevice == "WiFi" )
    {
      if( (! WiFi1CB->isChecked() && ! WiFi2CB->isChecked() && ! WiFi3CB->isChecked()) ||
          (WiFi1CB->isChecked() && (WiFi1_IP->text().isEmpty() || WiFi1_Port->text().isEmpty())) ||
          (WiFi2CB->isChecked() && (WiFi2_IP->text().isEmpty() || WiFi2_Port->text().isEmpty())) ||
          (WiFi3CB->isChecked() && (WiFi3_IP->text().isEmpty() || WiFi3_Port->text().isEmpty())) )
        {
          // IP address and port are required, when service is switched on!
          QString info = QString(
              tr( "<html>WiFi IPs or Ports entries are missing!"
                  "<br><br>Please add the missing items.</html>" ) );

           messageBox( QMessageBox::Warning,
                       tr( "WiFi data missing" ),
                       info );

           return false;
        }
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
    }
  else
    {
      GpsSpeed->setVisible( false );
      GpsSpeedLabel->setVisible( false );
    }

  if( text.startsWith( BT_ADAPTER ) == false )
    {
      toggleBtMenu( false );
    }
  else
    {
      toggleBtMenu( true );
      // load BT device list
      loadBtDeviceList();
    }

  toggleWiFiMenu();
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

/** Loads a stored BT device list. */
void SettingsPageGPS::loadBtDeviceList()
{
#ifdef BLUEZ

  GeneralConfig *conf = GeneralConfig::instance();

  // Load BT device list
  QMap<QString, QVariant>& btDevList = conf->getGpsBtDeviceList();

  if( btDevList.size() == 0 )
    {
      // empty list --> start BT scan
      slotSearchBtDevices();
    }
  else
    {
      BtList->clear();
      QList<QString> keys = btDevList.keys();

      for( int i = 0; i < keys.size(); i++ )
        {
          const QString& key = keys.at(i);
          BtList->addItem( key, btDevList.value(key).toString() );
        }

      BtList->model()->sort(0, Qt::AscendingOrder);

      // Select last known device
      QPair<QString, QString>& btDev = conf->getGpsBtDevice();

      int idx = BtList->findText( btDev.first);

      if( idx == -1 )
        {
          // BT device not found
          BtList->setCurrentIndex( 0 );
        }
      else
        {
          BtList->setCurrentIndex( BtList->findText( btDev.first) );
        }
    }

#endif
}

/**
 * Called, when the BT search button is pressed.
 */
void SettingsPageGPS::slotSearchBtDevices()
{
#ifdef BLUEZ
  searchBts->setEnabled( false );
  BtList->clear();
  BtList->addItem( tr("Searching BT devices") );
  // request BT Service list
  if( btdAgent == nullptr )
    {
      btdAgent = new BluetoothDeviceDiscovery( this );
      connect( btdAgent, SIGNAL(foundBtDevices(bool, QString&, QList<QBluetoothDeviceInfo>&)),
               this, SLOT(slotFoundBtDevices(bool, QString&, QList<QBluetoothDeviceInfo>&) ) );
    }

  btdAgent->start();

#endif
}

#ifdef BLUEZ

/**
 * Called by the BT scanner to transmit the found BTs.
 */
void SettingsPageGPS::slotFoundBtServices( bool ok,
                                           QString& error,
                                           QList<QBluetoothServiceInfo>& btsi )
{
  Q_UNUSED(ok)
  Q_UNUSED(error)
  Q_UNUSED(btsi)
  qDebug() << "SettingsPageGPS::slotFoundBtServices()";
}

/**
 * Called by the BT scanner to transmit the found BT devices.
 */
void SettingsPageGPS::slotFoundBtDevices( bool ok,
                                          QString& error,
                                          QList<QBluetoothDeviceInfo>& btdi )
{
  BtList->clear();

  if( ok == true )
    {
      for( int i=0; i < btdi.size(); i++ )
        {
          const QBluetoothDeviceInfo& di = btdi.at(i);
          BtList->addItem( di.name(), di.address().toString() );
        }

      BtList->model()->sort(0, Qt::AscendingOrder);

      // Select last known device
      QPair<QString, QString>& btDev = GeneralConfig::instance()->getGpsBtDevice();

      int idx = BtList->findText( btDev.first);

      if( idx == -1 )
        {
          // BT device not found
          BtList->setCurrentIndex( 0 );
        }
      else
        {
          BtList->setCurrentIndex( BtList->findText( btDev.first) );
        }
    }
  else
    {
      QString text = tr("BT search error!");
      QString info = error;
      messageBox( QMessageBox::Warning, text, info );
    }

  searchBts->setEnabled( true );
}

#endif

/**
 * Message box with text and info text.
 *
 * @param icon
 * @param text
 * @param infoText
 */
void SettingsPageGPS::messageBox( QMessageBox::Icon icon,
                                  QString text,
                                  QString infoText )
{
  QMessageBox msgBox( this );
  msgBox.setText( text );
  msgBox.setIcon( icon );
  msgBox.setInformativeText( infoText );
  msgBox.setStandardButtons( QMessageBox::Ok );
  msgBox.setDefaultButton( QMessageBox::Ok );
  msgBox.exec();
}
