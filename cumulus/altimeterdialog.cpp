/***********************************************************************
**
**   altimeterdialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Völlm
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "generalconfig.h"
#include "altitude.h"
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

  m_baro  = new QRadioButton( tr( "Baro" ), altRef );
  m_baro->setFont( font() );
  m_baro->setEnabled( true );
  m_baro->setFocusPolicy( Qt::NoFocus );

  QHBoxLayout* radioLayout2 = new QHBoxLayout( altRef );
  radioLayout2->addWidget( m_gps );
  radioLayout2->addStretch( 5 );
  radioLayout2->addWidget( m_baro );

  QHBoxLayout* urHBLayout = new QHBoxLayout;

  urHBLayout->addWidget( altUnit );
  urHBLayout->addStretch( 10 );
  urHBLayout->addWidget( altRef );

  controlLayout->addLayout( urHBLayout );

  //---------------------------------------------------------------------------

  QGroupBox* altitudeBox = new QGroupBox( this );
  QGridLayout* altitudeLayout = new QGridLayout;
  altitudeLayout->setMargin( 5 );
  altitudeLayout->setSpacing( 15 );
  int row = 0;

  QLabel* lbl = new QLabel( tr( "AltGain:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );

  altitudeGainDisplay = new QLineEdit( this );
  altitudeGainDisplay->setReadOnly( true );
  altitudeGainDisplay->setFont( font() );
  altitudeLayout->addWidget( altitudeGainDisplay, row, 1 );

  setAltitudeGain = new QPushButton( "S", this );
  altitudeLayout->addWidget( setAltitudeGain, row++, 2 );

  connect( setAltitudeGain, SIGNAL(pressed()), SLOT(slotResetGainedAltitude()) );

  connect( calculator, SIGNAL(newGainedAltitude(const Altitude&)),
           this, SLOT( slotAltitudeGain(const Altitude&)) );

  lbl = new QLabel( tr( "Leveling:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );
  spinLeveling = new QSpinBox( this );
  spinLeveling->setMinimum( -1000 );
  spinLeveling->setMaximum( 1000 );
  spinLeveling->setButtonSymbols( QSpinBox::NoButtons );
  spinLeveling->setAlignment( Qt::AlignHCenter );

  connect( spinLeveling, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  altitudeLayout->addWidget( spinLeveling, row, 1 );

  lbl = new QLabel( tr( "Altitude:" ), this );
  altitudeLayout->addWidget( lbl, row, 2 );
  m_altitudeDisplay = new QLabel( "0", this );
  altitudeLayout->addWidget( m_altitudeDisplay, row++, 3 );

  lbl = new QLabel( tr( "QNH:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );
  spinQnh = new QSpinBox( this );
  spinQnh->setRange( 500, 1500 );
  spinQnh->setSingleStep( 1 );
  spinQnh->setSuffix( " hPa" );
  spinQnh->setButtonSymbols( QSpinBox::NoButtons );
  spinQnh->setAlignment( Qt::AlignHCenter );

  connect( spinQnh, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  altitudeLayout->addWidget( spinQnh, row++, 1 );
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
  pmLayout->setSpacing( 5 );
  pmLayout->addWidget( pplus, Qt::AlignLeft );
  pmLayout->addWidget( plus, Qt::AlignLeft );
  pmLayout->addSpacing(20);
  pmLayout->addStretch( 10 );
  pmLayout->addWidget(reset);
  pmLayout->addStretch(10);
  pmLayout->addSpacing(20);
  pmLayout->addWidget( minus, Qt::AlignRight );
  pmLayout->addWidget( mminus, Qt::AlignRight );
  pmLayout->setSpacing( 5 );

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
  mainLayout->addSpacing( 25 );
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
      spinLeveling->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );
      break;
    case Altitude::feet:
      m_feet->setChecked(true);
      m_unit = 1;
      spinLeveling->setValue( (int) conf->getGpsUserAltitudeCorrection().getFeet() );
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
      break;
    case 1:
      m_baro->setChecked(true);
      break;
    default:
      m_gps->setChecked(true);
      break;
  }

  slotAltitudeGain( calculator->getGainedAltitude() );

  m_saveLeveling = spinLeveling->value();
  m_saveQnh = conf->getQNH();
  spinQnh->setValue( m_saveQnh );

  startTimer();
}

void AltimeterDialog::slotModeChanged( int mode )
{
  m_mode = mode;
  GeneralConfig::instance()->setAltimeterMode( m_mode );

  emit newAltimeterMode();     // informs MapView
  emit newAltimeterSettings(); // informs GpsNmea
  m_timeout->stop();
}

void AltimeterDialog::slotUnitChanged( int unit )
{
  m_unit = unit;

  // The new unit is set temporary to see the correct value.
  Altitude::setUnit( (enum Altitude::altitudeUnit) m_unit );
  emit newAltimeterMode(); // informs MapView
  m_timeout->stop();
}

void AltimeterDialog::slotReferenceChanged( int ref )
{
  _ref = ref;
  GeneralConfig::instance()->setGpsAltitude( _ref );

  emit newAltimeterMode();     // informs MapView
  emit newAltimeterSettings(); // informs GpsNmea
  m_timeout->stop();
}

/** This slot is being called if the altitude has been changed. */
void AltimeterDialog::slotAltitudeChanged(const Altitude& altitude )
{
  m_altitudeDisplay->setText( altitude.getText( true, 0 ) );
}

void AltimeterDialog::slotChangeSpinValue()
{
  m_timeout->stop();

  // Look which spin box has the focus
  if( QApplication::focusWidget() == spinQnh )
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

  if( QApplication::focusWidget() == spinLeveling )
    {
      int steps = 1;

      if( (Altitude::altitudeUnit) m_unit != Altitude::meters )
        {
          steps = 3;
        }

      if( plus->isDown() || minus->isDown() )
        {
          // + or - is pressed
          spinLeveling->setSingleStep( steps );
        }
      else if( pplus->isDown() || mminus->isDown() )
        {
          // ++ or -- is pressed
          spinLeveling->setSingleStep( steps * 5 );
        }
      else if( reset->isDown() )
        {
          // reset is pressed
          spinLeveling->setValue( 0 );
        }
      else
        {
          return;
        }

      if( plus->isDown() || pplus->isDown() )
        {
          spinLeveling->stepUp();
        }
      else if( minus->isDown() || mminus->isDown() )
        {
          spinLeveling->stepDown();
        }

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slotChangeSpinValue()));

      Altitude::setUnit( (Altitude::altitudeUnit) m_unit );

      GeneralConfig *conf = GeneralConfig::instance();
      Altitude newAlt = Altitude::convertToMeters( spinLeveling->value() );
      conf->setGpsUserAltitudeCorrection( newAlt );

      if( m_baro->isChecked() && m_msl->isChecked() &&
          GpsNmea::gps->getDeliveredAltitude() == GpsNmea::PRESSURE &&
          GpsNmea::gps->baroAltitudeSeen() )
        {
          // http://wolkenschnueffler.de/media//DIR_62701/7c9e0b09d2109871ffff8127ac144233.pdf
          // WE have pressure selected and got a pressure altitude.
          // So we can try to calculate the QNH.
          // The article of link above said, that 8.3m/hPa at MSL is the
          // pressure difference.
          // A common approach is to expect a pressure difference of 1 hPa per
          // 30ft until 18.000ft. 30ft are 9.1437m
          qDebug() << "NewAlt=" << newAlt.getMeters();

          int qnh = (int) rint( 1013.25 + newAlt.getMeters() / 8.3 );
          spinQnh->setValue( qnh );
        }

      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
      return;
    }
}

void AltimeterDialog::slotSpinValueChanged( const QString& text )
{
  Q_UNUSED( text )

  // Stops the timer after a spin box value change
  m_timeout->stop();
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
           m_saveLeveling != spinLeveling->value() ||
           m_saveQnh != spinQnh->value() );
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
      conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(spinLeveling->value()) );

      int qnh = spinQnh->value();

      if( m_baro->isChecked() && m_msl->isChecked() &&
          GpsNmea::gps->getDeliveredAltitude() == GpsNmea::PRESSURE &&
          GpsNmea::gps->baroAltitudeSeen() )
        {
          // Calculate again the QNH to avoid wrong value, if the leveling spin box
          // has not been operated via the plus and minus buttons or a radio button
          // has been pressed as last.

          // We have pressure selected and get pressure altitude. So we can try
          // to calculate the QNH.
          // The common approach is to expect a pressure difference of 1 hPa per
          // 30ft until 18.000ft. 30ft are 9.1437m
          // ### Altitude newAlt = Altitude::convertToMeters( spinLeveling->value() );
          // ### qnh = (int) rint( 1013.25 + newAlt.getFeet() / 30.0 );
          Altitude newAlt = Altitude::convertToMeters( -spinLeveling->value() );
          qnh = getQNH( newAlt );
        }

      conf->setQNH( qnh );
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
      conf->save();

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
  const double power = -9.80665 / ( 287.05 * -0.0065);

  const double k1 = -0.0065 / 288.15;

  double p1 = 1013.25 * pow( 1 + (k1 * altitude.getMeters()), power );

  return static_cast<int>(rint(p1));
}
