/***********************************************************************
**
**   altimeterdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Völlm
**                   2008-2022 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "generalconfig.h"
#include "altimeterdialog.h"
#include "calculator.h"
#include "glider.h"
#include "gpsnmea.h"
#include "mapconfig.h"
#include "layout.h"

// set static member variable
int AltimeterDialog::noOfInstances = 0;

AltimeterDialog::AltimeterDialog (QWidget *parent) :
  QDialog( parent, Qt::WindowStaysOnTopHint ),
  m_mode( 0 ),
  m_unit( 0 ),
  _ref( 0 ),
  m_autoSip(true)
{
  noOfInstances++;
  setObjectName( "AltimeterModeDialog" );
  setModal( true );
  setWindowTitle( tr( "Altimeter Settings" ) );
  setAttribute( Qt::WA_DeleteOnClose );

  QPalette p = palette();

  if( GeneralConfig::instance()->getBlackBgInfoDisplay() == false )
    {
      p.setColor(QPalette::Base, Qt::white);
      setPalette(p);
      p.setColor(QPalette::Text, Qt::black);
      setPalette(p);
    }
  else
    {
      p.setColor(QPalette::Base, Qt::black);
      setPalette(p);
      p.setColor(QPalette::Text, Qt::white);
      setPalette(p);
    }

  // set font size to a reasonable and usable value
  QFont cf = font();
  cf.setBold( true );
  Layout::fitDialogFont( cf );
  setFont(cf);

  QGroupBox* altMode = new QGroupBox( this );
  m_msl = new QRadioButton( tr( "MSL" ), altMode );
  m_std = new QRadioButton( tr( "STD" ), altMode );
  m_agl = new QRadioButton( tr( "AGL" ), altMode );
  m_ahl = new QRadioButton( tr( "AHL" ), altMode );

  m_msl->setFont( font() );
  m_std->setFont( font() );
  m_agl->setFont( font() );
  m_ahl->setFont( font() );

  m_msl->setChecked( true );
  m_msl->setEnabled( true );
  m_std->setEnabled( true );
  m_agl->setEnabled( true );
  m_ahl->setEnabled( true );

  m_msl->setFocusPolicy( Qt::NoFocus );
  m_std->setFocusPolicy( Qt::NoFocus );
  m_agl->setFocusPolicy( Qt::NoFocus );
  m_ahl->setFocusPolicy( Qt::NoFocus );

  QHBoxLayout* mainLayout = new QHBoxLayout( this );

  QVBoxLayout* controlLayout = new QVBoxLayout;
  controlLayout->addWidget( altMode );

  QHBoxLayout* radioLayout = new QHBoxLayout( altMode );
  radioLayout->addWidget( m_msl );
  radioLayout->addWidget( m_std );
  radioLayout->addWidget( m_agl );
  radioLayout->addWidget( m_ahl );

  //---------------------------------------------------------------------------

  QGroupBox* altUnit = new QGroupBox( this );
  m_meter = new QRadioButton( tr( "Meters" ), altUnit );
  m_meter->setFont( font() );
  m_meter->setChecked( true );
  m_meter->setEnabled( true );
  m_meter->setFocusPolicy( Qt::NoFocus );

  m_feet  = new QRadioButton( tr( "Feet" ), altUnit );
  m_feet->setFont( font() );
  m_feet->setEnabled( true );
  m_feet->setFocusPolicy( Qt::NoFocus );

  QHBoxLayout* radioLayout1 = new QHBoxLayout( altUnit );
  radioLayout1->addWidget( m_meter );
  radioLayout1->addStretch( 5 );
  radioLayout1->addWidget( m_feet );

  QGroupBox* altRef = new QGroupBox( this );
  m_gps = new QRadioButton( tr( "GPS" ), altRef );
  m_gps->setFont( font() );
  m_gps->setChecked( true );
  m_gps->setEnabled( true );
  m_gps->setFocusPolicy( Qt::NoFocus );

  connect( m_gps, SIGNAL(toggled(bool)), SLOT(slotGpsRBToggled(bool)));

  m_baro  = new QRadioButton( tr( "Baro" ), altRef );
  m_baro->setFont( font() );
  m_baro->setEnabled( true );
  m_baro->setFocusPolicy( Qt::NoFocus );

  m_devicesList = new QComboBox();
  m_devicesList->setToolTip( tr("Device which delivers pressure altitude.") );
  m_devicesList->setVisible( false );
  m_devicesList->setObjectName("DeviceSelection");
  m_devicesList->setEditable(false);
  m_devicesList->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  m_devicesList->view()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

#ifdef QSCROLLER
  m_devicesList->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  QScroller::grabGesture( m_devicesList->view()->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  m_devicesList->view()->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  QtScroller::grabGesture( m_devicesList->view()->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  m_devicesList->addItems( GeneralConfig::getPressureDevicesList() );

  QHBoxLayout* radioLayout2 = new QHBoxLayout( altRef );
  radioLayout2->addWidget( m_gps );
  radioLayout2->addStretch( 5 );
  radioLayout2->addWidget( m_baro );
  radioLayout2->addWidget( m_devicesList );

  QHBoxLayout* urHBLayout = new QHBoxLayout;

  urHBLayout->addWidget( altUnit );
  urHBLayout->addStretch( 10 );
  urHBLayout->addWidget( altRef );

  controlLayout->addLayout( urHBLayout );

  //---------------------------------------------------------------------------

  QGroupBox* altitudeBox = new QGroupBox( this );
  QGridLayout* altitudeLayout = new QGridLayout;
  altitudeLayout->setMargin( 5 * Layout::getIntScaledDensity());
  altitudeLayout->setSpacing( 15 * Layout::getIntScaledDensity());
  int row = 0;

  QLabel* lbl = new QLabel( tr( "AltGain:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );

  altitudeGainDisplay = new QLineEdit( this );
  altitudeGainDisplay->setReadOnly( true );
  altitudeGainDisplay->setFont( font() );
  altitudeGainDisplay->setAlignment( Qt::AlignHCenter );
  altitudeLayout->addWidget( altitudeGainDisplay, row, 1 );

  startAltitudeGain = new QPushButton( "S", this );
  startAltitudeGain->setToolTip( tr("Start altitude gain recording") );
  altitudeLayout->addWidget( startAltitudeGain, row++, 2 );

  connect( startAltitudeGain, SIGNAL(pressed()), SLOT(slotResetGainedAltitude()) );

  connect( calculator, SIGNAL(newGainedAltitude(const Altitude&)),
           this, SLOT( slotAltitudeGain(const Altitude&)) );

  lbl = new QLabel( tr( "AF Elevation:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );

  m_afElevationDisplay = new NumberEditor( this );
  m_afElevationDisplay->setSuffix( " " + Altitude::getUnitText() );
  m_afElevationDisplay->setRange( 0, 9999 );
  m_afElevationDisplay->setDecimalVisible( false );
  m_afElevationDisplay->setPmVisible( false );
  m_afElevationDisplay->setTip(tr("Elevation ") + "0...9999 " + Altitude::getUnitText() );
  m_afElevationDisplay->setValue( 0 );
  m_afElevationDisplay->setToolTip( tr("Set your airfield elevation.") );
  altitudeLayout->addWidget( m_afElevationDisplay, row, 1 );

  setQnh = new QPushButton( tr("Align"), this );
  setQnh->setToolTip( tr("Align QNH altitude to airfield elevation") );
  altitudeLayout->addWidget( setQnh, row++, 2 );

  connect( m_afElevationDisplay, SIGNAL(numberPadOpened()), SLOT(slotElevationIsEdited()) );
  connect( setQnh, SIGNAL(pressed()), SLOT(slotSetQnh()) );

  QString tip = tr("Adjust altitude to your airfield elevation.");
  lbl = new QLabel( tr( "QNH Altitude:" ), this );
  lbl->setToolTip( tip );
  altitudeLayout->addWidget( lbl, row, 0 );

  m_altitudeDisplay = new QSpinBox( this );
  m_altitudeDisplay->setToolTip( tip );
  m_altitudeDisplay->setButtonSymbols( QSpinBox::NoButtons );
  m_altitudeDisplay->setMinimum( -1000 );
  m_altitudeDisplay->setMaximum( 1000000 );
  m_altitudeDisplay->setAlignment( Qt::AlignHCenter );
  altitudeLayout->addWidget( m_altitudeDisplay, row, 1 );

  lbl = new QLabel( tr( "Correction:" ), this );
  altitudeLayout->addWidget( lbl, row, 2 );
  levelingDisplay = new QLabel( this );
  altitudeLayout->addWidget( levelingDisplay, row++, 3 );

  lbl = new QLabel( tr( "QNH:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );
  spinQnh = new QSpinBox( this );
  spinQnh->setRange( 500, 1500 );
  spinQnh->setSingleStep( 1 );
  spinQnh->setSuffix( " hPa" );
  spinQnh->setButtonSymbols( QSpinBox::NoButtons );
  spinQnh->setAlignment( Qt::AlignHCenter );
  spinQnh->setEnabled( false );

  altitudeLayout->addWidget( spinQnh, row, 1 );

  autoQnh = new QCheckBox( tr( "auto ONH" ), this );
  autoQnh->setCheckable( true );
  autoQnh->setChecked( true );
  autoQnh->setToolTip( tr("Check box for auto ONH setting") );

  connect( autoQnh, SIGNAL(stateChanged(int)), SLOT(slotQnhAutoChanged(int)) );

  altitudeLayout->addWidget( autoQnh, row++, 2, 1, 2 );

  altitudeLayout->setColumnStretch( 4, 5 );

  altitudeBox->setLayout( altitudeLayout );
  controlLayout->addWidget( altitudeBox );

  //---------------------------------------------------------------------------

  pplus  = new QPushButton( "++", this );
  plus   = new QPushButton( "+", this );
  mminus = new QPushButton( "--", this );
  minus  = new QPushButton( "-", this );
  reset  = new QPushButton("R", this);

  int size = Layout::getButtonSize();

  int buttonSize = Layout::getButtonSize();
  int iconSize   = buttonSize - 5;

  pplus->setMinimumSize( buttonSize, buttonSize );
  plus->setMinimumSize( buttonSize, buttonSize );
  minus->setMinimumSize( buttonSize, buttonSize );
  mminus->setMinimumSize( buttonSize, buttonSize );
  reset->setMinimumSize(buttonSize, buttonSize);

  pplus->setMaximumSize( buttonSize, buttonSize );
  plus->setMaximumSize( buttonSize, buttonSize );
  minus->setMaximumSize( buttonSize, buttonSize );
  mminus->setMaximumSize( buttonSize, buttonSize );
  reset->setMaximumSize(buttonSize, buttonSize);

  pplus->setFocusPolicy( Qt::NoFocus );
  plus->setFocusPolicy( Qt::NoFocus );
  minus->setFocusPolicy( Qt::NoFocus );
  mminus->setFocusPolicy( Qt::NoFocus );
  reset->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout *pmLayout = new QHBoxLayout;
  pmLayout->setSpacing( 10 * Layout::getIntScaledDensity());
  pmLayout->addWidget( pplus, Qt::AlignLeft );
  pmLayout->addWidget( plus, Qt::AlignLeft );
  pmLayout->addSpacing( 20 * Layout::getIntScaledDensity());
  pmLayout->addStretch( 10 );
  pmLayout->addWidget(reset);
  pmLayout->addStretch(10);
  pmLayout->addSpacing( 20 * Layout::getIntScaledDensity());
  pmLayout->addWidget( minus, Qt::AlignRight );
  pmLayout->addWidget( mminus, Qt::AlignRight );

  controlLayout->addLayout( pmLayout );

  //---------------------------------------------------------------------------
  // Align ok and cancel button vertically at the right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  QPushButton *cancel = new QPushButton( this );
  cancel->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "cancel.png" ) ) );
  cancel->setIconSize( QSize( iconSize, iconSize ) );
  cancel->setMinimumSize( buttonSize, buttonSize );
  cancel->setMaximumSize( buttonSize, buttonSize );
  // cancel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );

  QPushButton *ok = new QPushButton( this );
  ok->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "ok.png" ) ) );
  ok->setIconSize( QSize( iconSize, iconSize ) );
  ok->setMinimumSize( buttonSize, buttonSize );
  ok->setMaximumSize( buttonSize, buttonSize );
  // ok->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );

  QVBoxLayout *buttonLayout = new QVBoxLayout;
  buttonLayout->addWidget( cancel );
  buttonLayout->addStretch();
  buttonLayout->addWidget( ok );
  buttonLayout->addSpacing( size + 5 );

  mainLayout->addLayout( controlLayout );
  mainLayout->addSpacing( 25 * Layout::getIntScaledDensity() );
  mainLayout->addLayout( buttonLayout );

  m_timeout = new QTimer( this );
  m_timeout->setSingleShot( true );

  // Map altitude reference radio buttons
  QSignalMapper* signalMapper = new QSignalMapper( this );
  connect( m_msl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( m_msl, 0 );
  connect( m_std, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( m_std, 1 );
  connect( m_agl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( m_agl, 2 );
  connect( m_ahl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( m_ahl, 3 );

  // map altitude unit radio buttons
  QSignalMapper* signalMapperUnit = new QSignalMapper( this );
  connect( m_meter, SIGNAL(clicked()), signalMapperUnit, SLOT(map()) );
  signalMapperUnit->setMapping( m_meter, 0 );
  connect( m_feet, SIGNAL(clicked()), signalMapperUnit, SLOT(map()) );
  signalMapperUnit->setMapping( m_feet, 1 );

  // map altitude reference radio buttons
  QSignalMapper* signalMapperReference = new QSignalMapper( this );
  connect( m_gps, SIGNAL(clicked()), signalMapperReference, SLOT(map()) );
  signalMapperReference->setMapping( m_gps, 0 );
  connect( m_baro, SIGNAL(clicked()), signalMapperReference, SLOT(map()) );
  signalMapperReference->setMapping( m_baro, 1 );

  // Map altitude QNH and leveling buttons
  connect( pplus, SIGNAL(pressed()), this, SLOT(slotChangeSpinValue()) );
  connect( plus, SIGNAL(pressed()), this, SLOT(slotChangeSpinValue()) );
  connect( minus, SIGNAL(pressed()), this, SLOT(slotChangeSpinValue()) );
  connect( mminus, SIGNAL(pressed()), this, SLOT(slotChangeSpinValue()) );
  connect( reset, SIGNAL(pressed()), this, SLOT(slotChangeSpinValue()) );

  connect( signalMapper, SIGNAL(mapped(int)), this, SLOT(slotModeChanged(int)) );
  connect( signalMapperUnit, SIGNAL(mapped(int)), this, SLOT(slotUnitChanged(int)) );
  connect( signalMapperReference, SIGNAL(mapped(int)), this, SLOT(slotReferenceChanged(int)) );

  connect( m_timeout, SIGNAL(timeout()), this, SLOT(reject()) );
  connect( ok, SIGNAL(released()), this, SLOT(accept()) );
  connect( cancel, SIGNAL(released()), this, SLOT(reject()) );

  load();

  // Switch off automatic software input panel popup
  m_autoSip = qApp->autoSipEnabled();
  qApp->setAutoSipEnabled( false );
}

AltimeterDialog::~AltimeterDialog()
{
  noOfInstances--;
  qApp->setAutoSipEnabled( m_autoSip );
}

QString AltimeterDialog::mode2String()
{
  switch( GeneralConfig::instance()->getAltimeterMode() )
    {
    case 0:
      return QString("Msl"); // Mean sea level
    case 1:
      return QString("Std"); // standard pressure
    case 2:
      return QString("Agl"); // above ground level
    case 3:
      return QString("Ahl"); // above home level
    default:
      return QString("Ukn"); // unknown
    }
}

int AltimeterDialog::mode()
{
  return GeneralConfig::instance()->getAltimeterMode();
}

void AltimeterDialog::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  m_mode = conf->getAltimeterMode();
  m_saveMode = m_mode;
  m_savedAltitude = calculator->getlastSTDAltitude();
  m_savedElevation = conf->getHomeElevation();

  // Check, if cumulus is in move. In this case disable setQhn button.
  if( calculator->moving() )
    {
      setQnh->setEnabled( false );
    }

  switch( m_mode )
  {
    case 0:
      m_msl->setChecked(true);
      break;
    case 1:
      m_std->setChecked(true);
      break;
    case 2:
      m_agl->setChecked(true);
      break;
    case 3:
      m_ahl->setChecked(true);
      break;
    default:
      qWarning("AltimeterModeDialog::load(): invalid mode: %d", m_mode);
      break;
  }

  m_saveUnit = Altitude::getUnit();

  switch( m_saveUnit )
  {
    case Altitude::meters:
      m_meter->setChecked(true);
      m_unit = 0;
      levelingDisplay->setText( QString::number((int) rint(conf->getGpsUserAltitudeCorrection().getMeters()) ));
      m_afElevationDisplay->setValue((int) rint(m_savedElevation.getMeters()));

      break;
    case Altitude::feet:
      m_feet->setChecked(true);
      m_unit = 1;
      levelingDisplay->setText( QString::number((int) rint(conf->getGpsUserAltitudeCorrection().getFeet()) ));
      m_afElevationDisplay->setValue((int) rint(m_savedElevation.getFeet()));
      break;
    default:
      m_meter->setChecked(true);
      m_unit = 0;
      break;
  }

  _ref = conf->getGpsAltitude();
  m_saveRef = _ref;

  switch( _ref )
  {
    case 0:
      m_gps->setChecked(true);
      m_devicesList->setVisible( false );
      autoQnh->setVisible( false );
     break;
    case 1:
      m_baro->setChecked(true);
      m_devicesList->setVisible( true );
      autoQnh->setVisible( true );
      break;
    default:
      m_gps->setChecked(true);
      m_devicesList->setVisible( false );
      autoQnh->setVisible( false );
      break;
  }

  m_savePressureDevice = conf->getPressureDevice();
  int idx = m_devicesList->findText( m_savePressureDevice );

  if( idx == -1 )
    {
      idx = 0;
    }

  // select last stored pressure device
  m_devicesList->setCurrentIndex( idx );

  // Activate signal handler, if pressure device is changed.
  connect( m_devicesList, SIGNAL(currentIndexChanged(const QString &)),
           SLOT(slotPressureDeviceChanged(const QString&)));

  slotAltitudeGain( calculator->getGainedAltitude() );

  m_saveLeveling = levelingDisplay->text().toInt();
  m_saveQnh = conf->getQNH();
  spinQnh->setValue( m_saveQnh );

  // Update altitude display
  slotAltitudeChanged() ;

  startTimer();
}

void AltimeterDialog::slotPressureDeviceChanged( const QString& device )
{
  GeneralConfig::instance()->setPressureDevice( device );
  emit newPressureDevice( device ); // informs GpsNmea

  // Update altitude display with a delay to get the new STD altitude
  QTimer::singleShot( 3000, this, SLOT( slotAltitudeChanged() ) );

  m_timeout->stop();
}

void AltimeterDialog::slotModeChanged( int mode )
{
  m_mode = mode;
  GeneralConfig::instance()->setAltimeterMode( m_mode );

  emit newAltimeterMode();     // informs MapView
  emit newAltimeterSettings(); // informs GpsNmea

  // Altitude display is updated to get the last delivered altitude.
  slotAltitudeChanged();

  // The gained altitude display must be updated to the new altitude reference.
  slotAltitudeGain( calculator->getGainedAltitude() );

  m_timeout->stop();
}

void AltimeterDialog::slotUnitChanged( int unit )
{
  m_unit = unit;

  // Save old altitude unit
  enum Altitude::altitudeUnit oldAltUnit = Altitude::getUnit();

  // The new altitude unit is set and posted to the other classes.
  Altitude::setUnit( (enum Altitude::altitudeUnit) m_unit );

  // The leveling value must be adapted to the new unit.
  Altitude newAltLeveling(0);

  if( oldAltUnit == Altitude::meters && m_unit == Altitude::feet )
    {
      newAltLeveling.setMeters(levelingDisplay->text().toInt());
      levelingDisplay->setText( QString::number((int) rint(newAltLeveling.getFeet())) );

      Altitude elevation( m_afElevationDisplay->value() );
      m_afElevationDisplay->setValue( (int) rint(elevation.getFeet()) );
    }
  else if( oldAltUnit == Altitude::feet && m_unit == Altitude::meters )
    {
      newAltLeveling.setFeet(levelingDisplay->text().toInt());
      levelingDisplay->setText( QString::number((int) rint(newAltLeveling.getMeters())) );

      Altitude elevation(0);
      elevation.setFeet( m_afElevationDisplay->value() );
      m_afElevationDisplay->setValue( (int) rint(elevation.getMeters()) );
    }

  m_afElevationDisplay->setSuffix( " " + Altitude::getUnitText() );
  m_afElevationDisplay->setTip(tr("Elevation ") + "0...5000 " + Altitude::getUnitText() );

  // informs MapView
  emit newAltimeterMode();

  // Update altitude display
  slotAltitudeChanged();

  // The gained altitude display must be updated to the new altitude unit.
  slotAltitudeGain( calculator->getGainedAltitude() );

  m_timeout->stop();
}

void AltimeterDialog::slotReferenceChanged( int ref )
{
  _ref = ref;
  GeneralConfig::instance()->setGpsAltitude( _ref );

  switch( _ref )
  {
    case 0:
      m_gps->setChecked(true);
      m_devicesList->setVisible( false );
      break;
    case 1:
      m_baro->setChecked(true);
      m_devicesList->setVisible( true );
      break;
    default:
      m_gps->setChecked(true);
      m_devicesList->setVisible( false );
      break;
  }

  emit newAltimeterMode();     // informs MapView
  emit newAltimeterSettings(); // informs GpsNmea

  // Update altitude display with a delay to get the new STD altitude
  QTimer::singleShot( 3000, this, SLOT( slotAltitudeChanged() ) );

  m_timeout->stop();
}

/** This slot is being called if the altitude has been changed. */
void AltimeterDialog::slotAltitudeChanged()
{
  Altitude altitude = calculator->getlastSTDAltitude() +
      GeneralConfig::instance()->getGpsUserAltitudeCorrection();

  m_altitudeDisplay->setValue( altitude.getText( false, 0 ).toInt() );
  m_altitudeDisplay->setSuffix( " " + altitude.getUnitText() );
}

/**
 * This slot is called if the auto ONH checkbox has changed its state.
 */
void AltimeterDialog::slotQnhAutoChanged( int state )
{
  if( state == Qt::Unchecked )
    {
      spinQnh->setEnabled( true );
    }
  else
    {
      spinQnh->setEnabled( false );
    }
}

void AltimeterDialog::slotChangeSpinValue()
{
  m_timeout->stop();

  // Look which spin box has the focus
  if( autoQnh->isChecked() == false && QApplication::focusWidget() == spinQnh )
    {
      if( reset->isDown() )
        {
          spinQnh->setValue( 1013 );
          return;
        }

      if( plus->isDown() || minus->isDown() )
        {
          // + or - is pressed
          spinQnh->setSingleStep( 1 );
        }
      else if( pplus->isDown() || mminus->isDown() )
        {
          // ++ or -- is pressed
          spinQnh->setSingleStep( 5 );
        }
      else
        {
          return;
        }

      if( plus->isDown() || pplus->isDown() )
        {
          spinQnh->stepUp();
        }
      else
        {
          spinQnh->stepDown();
        }

      // Start repetition timer, to check, if button is longer pressed.
       QTimer::singleShot(300, this, SLOT(slotChangeSpinValue()));
      return;
    }

  if( QApplication::focusWidget() == m_altitudeDisplay )
    {
      int steps = 1;

      if( (Altitude::altitudeUnit) m_unit != Altitude::meters )
        {
          steps = 3;
        }

      if( plus->isDown() )
        {
          // + is pressed
          levelingDisplay->setText( QString::number( levelingDisplay->text().toInt() + steps ) );
        }
      else if( minus->isDown() )
        {
          // - is pressed
          levelingDisplay->setText( QString::number( levelingDisplay->text().toInt() - steps ) );
        }
      else if( pplus->isDown() )
        {
          // ++ is pressed
          levelingDisplay->setText( QString::number( levelingDisplay->text().toInt() + (steps * 5) ) );
        }
      else if( mminus->isDown() )
        {
          // -- is pressed
          levelingDisplay->setText( QString::number( levelingDisplay->text().toInt() - (steps * 5) ) );
        }
      else if( reset->isDown() )
        {
          // reset is pressed
          levelingDisplay->setText( "0" );

          if( autoQnh->isChecked() == true )
            {
              spinQnh->setValue( 1013 );
            }
        }
      else
        {
          return;
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotChangeSpinValue()));

      Altitude::setUnit( (Altitude::altitudeUnit) m_unit );

      GeneralConfig *conf = GeneralConfig::instance();

      Altitude altCorrection;

      // Get last know altitude
      m_savedAltitude = calculator->getlastSTDAltitude();

      switch( m_unit )
      {
        case Altitude::meters:
          altCorrection.setMeters( levelingDisplay->text().toInt() );
          break;
        case Altitude::feet:
          altCorrection.setFeet( levelingDisplay->text().toInt() );
          break;
        default:
          break;
      }

      // Save altitude correction value.
      conf->setGpsUserAltitudeCorrection( altCorrection );

      // Update altitude display
      slotAltitudeChanged();

      if( autoQnh->isVisible() == true && autoQnh->isChecked() == true )
        {
          // http://wolkenschnueffler.de/media//DIR_62701/7c9e0b09d2109871ffff8127ac144233.pdf
          // WE have pressure selected and got a pressure altitude.
          // So we can try to calculate the QNH.
          // The article of link above said, that 8.3m/hPa at MSL is the
          // pressure difference.
          // A common approach is to expect a pressure difference of 1 hPa per
          // 30ft until 18.000ft. 30ft are 9.1437m
          Altitude newAltQnh(Altitude::convertToMeters( - levelingDisplay->text().toInt() ));
          int qnh = getQNH( newAltQnh );
          spinQnh->setValue( qnh );
        }
    }
}

void AltimeterDialog::slotAltitudeGain(const Altitude& altitudeGainIn )
{
  altitudeGainDisplay->setText( Altitude::getText(altitudeGainIn.getMeters(), true, 0) );
}

void AltimeterDialog::slotResetGainedAltitude()
{
  Altitude alt(0);
  slotAltitudeGain(alt);
  calculator->slot_setMinimumAltitude();
  m_timeout->stop();
}

bool AltimeterDialog::changesDone()
{
  return( m_unit != m_saveUnit ||
           m_mode != m_saveMode ||
           _ref != m_saveRef ||
           m_saveLeveling != levelingDisplay->text().toInt() ||
           m_saveQnh != spinQnh->value() ||
           m_savePressureDevice != m_devicesList->currentText() );
}

void AltimeterDialog::accept()
{
  if( changesDone() )
    {
      Altitude::setUnit( (Altitude::altitudeUnit) m_unit );
      GeneralConfig *conf = GeneralConfig::instance();

      conf->setUnitAlt( m_unit );
      conf->setAltimeterMode( m_mode );
      conf->setGpsAltitude( _ref );
      conf->setQNH( spinQnh->value() );

      Altitude correction;
      Distance elevation;

      switch( Altitude::getUnit() )
        {
        case Altitude::feet:
          correction.setFeet( levelingDisplay->text().toInt() );
          elevation.setFeet( m_afElevationDisplay->value() );
          break;
        case Altitude::meters:
        default:
          correction.setMeters( levelingDisplay->text().toInt() );
          elevation.setMeters( m_afElevationDisplay->value() );
          break;
        }

      conf->setGpsUserAltitudeCorrection( correction );
      conf->setHomeElevation( elevation );
      conf->save();

      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
    }

  emit closingWidget();
  QDialog::accept();
}

void AltimeterDialog::reject()
{
  if( changesDone() )
    {
      Altitude::setUnit( (Altitude::altitudeUnit) m_saveUnit );

      GeneralConfig *conf = GeneralConfig::instance();

      conf->setUnitAlt( m_saveUnit );
      conf->setAltimeterMode( m_saveMode );
      conf->setGpsAltitude( m_saveRef );
      conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(m_saveLeveling) );
      conf->setQNH( m_saveQnh );
      conf->setPressureDevice( m_savePressureDevice );
      conf->save();

      emit newPressureDevice( m_savePressureDevice ); // informs GpsNmea
      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
    }

  emit closingWidget();
  QDialog::reject();
}

void AltimeterDialog::startTimer()
{
  // @AP: let us take the user's defined info display time
  int time = GeneralConfig::instance()->getInfoDisplayTime();

  if( time > 0 )
    {
      m_timeout->start( time * 1000 ); // milli seconds have to be passed
    }
}

int AltimeterDialog::getQNH( const Altitude& altitude )
{
  // Under this link I found the formula for QNH calculation.
  // http://www.wolkenschnueffler.de/media//DIR_62701/7c9e0b09d2109871ffff8127ac144233.pdf
  const double power = -9.80665 / (287.05 * -0.0065);

  const double k1 = -0.0065 / 288.15;

  double p1 = 1013.25 * pow( 1 + (k1 * altitude.getMeters()), power );

  return static_cast<int>(rint(p1));
}

/** This slot is being called if the setQhn button is pressed. */
void AltimeterDialog::slotSetQnh()
{
  // Check, if cumulus is in move. In this case disable action.
  if( calculator->moving() )
    {
      return;
    }

  m_timeout->stop();

  GeneralConfig *conf = GeneralConfig::instance();
  Altitude elevation;

  switch( Altitude::getUnit() )
    {
    case Altitude::feet:
      elevation.setFeet( m_afElevationDisplay->value() );
      break;
    case Altitude::meters:
    default:
      elevation.setMeters( m_afElevationDisplay->value() );
      break;
    }

  Altitude altCorrection = elevation - calculator->getlastSTDAltitude();

  switch( Altitude::getUnit() )
    {
    case Altitude::feet:
      levelingDisplay->setText( QString::number( (int) rint(altCorrection.getFeet() )));
      break;
    case Altitude::meters:
    default:
      levelingDisplay->setText( QString::number( (int) rint(altCorrection.getMeters() )));
      break;
    }

  // Save altitude correction value.
  conf->setGpsUserAltitudeCorrection( altCorrection );

  // Update altitude display
  slotAltitudeChanged();

  if( autoQnh->isVisible() == true && autoQnh->isChecked() == true )
    {
      Altitude newAltQnh(Altitude::convertToMeters( - levelingDisplay->text().toInt() ));
      int qnh = getQNH( newAltQnh );
      spinQnh->setValue( qnh );
    }
}

/** This slot is being called if the elevation is being edited. */
void AltimeterDialog::slotElevationIsEdited()
{
  m_timeout->stop();
}
