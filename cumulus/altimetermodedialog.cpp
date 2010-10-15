/***********************************************************************
**
**   altimetermodedialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Völlm
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "altitude.h"
#include "altimetermodedialog.h"
#include "calculator.h"
#include "glider.h"
#include "mapconfig.h"
#include "layout.h"

// set static member variable
int AltimeterModeDialog::noOfInstances = 0;

extern MapConfig *_globalMapConfig;

AltimeterModeDialog::AltimeterModeDialog (QWidget *parent) :
  QDialog( parent, Qt::WindowStaysOnTopHint ),
  _mode( 0 ),
  _unit( 0 )
{
  noOfInstances++;
  setObjectName( "AltimeterModeDialog" );
  setModal( true );
  setWindowTitle( tr( "Altimeter Settings" ) );
  setAttribute( Qt::WA_DeleteOnClose );

#ifndef MAEMO
  int minFontSize = 14;
#else
  int minFontSize = 20;
#endif

  QFont b = font();
  b.setBold( true );
  setFont( b );

  // set font size to a reasonable and usable value
  if( font().pointSize() < minFontSize )
    {
      QFont cf = font();
      cf.setPointSize( minFontSize );
      this->setFont( cf );
    }

  QGroupBox* altMode = new QGroupBox( this );
  _msl = new QRadioButton( tr( "MSL" ), altMode );
  _std = new QRadioButton( tr( "STD" ), altMode );
  _agl = new QRadioButton( tr( "AGL" ), altMode );
  _ahl = new QRadioButton( tr( "AHL" ), altMode );

  _msl->setChecked( true );
  _msl->setEnabled( true );
  _std->setEnabled( true );
  _agl->setEnabled( true );
  _ahl->setEnabled( true );

  QHBoxLayout* mainLayout = new QHBoxLayout( this );

  QVBoxLayout* controlLayout = new QVBoxLayout;
  controlLayout->addWidget( altMode );

  QHBoxLayout* radioLayout = new QHBoxLayout( altMode );
  radioLayout->addWidget( _msl );
  radioLayout->addWidget( _std );
  radioLayout->addWidget( _agl );
  radioLayout->addWidget( _ahl );

  //---------------------------------------------------------------------------

  QGroupBox* altUnit = new QGroupBox( this );
  _meter = new QRadioButton( tr( "Meters" ), altUnit );
  _feet  = new QRadioButton( tr( "Feet" ), altUnit );

  _meter->setChecked( true );
  _meter->setEnabled( true );
  _feet->setEnabled( true );

  QHBoxLayout* radioLayout1 = new QHBoxLayout( altUnit );
  radioLayout1->addWidget( _meter );
  radioLayout1->addStretch( 5 );
  radioLayout1->addWidget( _feet );

  controlLayout->addWidget( altUnit );

  //---------------------------------------------------------------------------

  QGroupBox* altitudeBox = new QGroupBox( this );
  QGridLayout* altitudeLayout = new QGridLayout;
  altitudeLayout->setMargin( 5 );
  altitudeLayout->setSpacing( 15 );
  int row = 0;

  QLabel* lbl = new QLabel( tr( "Leveling:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );
  spinLeveling = new QSpinBox( this );
  spinLeveling->setMinimum( -1000 );
  spinLeveling->setMaximum( 1000 );
  spinLeveling->setButtonSymbols( QSpinBox::NoButtons );

  connect( spinLeveling, SIGNAL(valueChanged(const QString&)),
           this, SLOT(slotSpinValueChanged(const QString&)));

  altitudeLayout->addWidget( spinLeveling, row, 1 );

  lbl = new QLabel( tr( "Altitude:" ), this );
  altitudeLayout->addWidget( lbl, row, 2 );
  _altitudeDisplay = new QLabel( "0", this );
  altitudeLayout->addWidget( _altitudeDisplay, row++, 3 );

  lbl = new QLabel( tr( "QNH:" ), this );
  altitudeLayout->addWidget( lbl, row, 0 );
  spinQnh = new QSpinBox( this );
  spinQnh->setRange( 500, 1500 );
  spinQnh->setSingleStep( 1 );
  spinQnh->setSuffix( " hPa" );
  spinQnh->setButtonSymbols( QSpinBox::NoButtons );

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

  int size = 40;

#ifdef MAEMO
  size = 80;
#endif

  pplus->setMinimumSize( size, size );
  plus->setMinimumSize( size, size );
  minus->setMinimumSize( size, size );
  mminus->setMinimumSize( size, size );

  pplus->setMaximumSize( size, size );
  plus->setMaximumSize( size, size );
  minus->setMaximumSize( size, size );
  mminus->setMaximumSize( size, size );

  pplus->setFocusPolicy( Qt::NoFocus );
  plus->setFocusPolicy( Qt::NoFocus );
  minus->setFocusPolicy( Qt::NoFocus );
  mminus->setFocusPolicy( Qt::NoFocus );

  QHBoxLayout *pmLayout = new QHBoxLayout;
  pmLayout->setSpacing( 5 );
  pmLayout->addWidget( pplus, Qt::AlignLeft );
  pmLayout->addWidget( plus, Qt::AlignLeft );
  pmLayout->addStretch( 10 );
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
  cancel->setIconSize( QSize( IconSize, IconSize ) );
  cancel->setMinimumSize( size, size );
  cancel->setMaximumSize( size, size );
  cancel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );

  QPushButton *ok = new QPushButton( this );
  ok->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "ok.png" ) ) );
  ok->setIconSize( QSize( IconSize, IconSize ) );
  ok->setMinimumSize( size, size );
  ok->setMaximumSize( size, size );
  ok->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );

  QVBoxLayout *buttonLayout = new QVBoxLayout;
  buttonLayout->addWidget( cancel );
  buttonLayout->addStretch();
  buttonLayout->addWidget( ok );
  buttonLayout->addSpacing( size + 5 );

  mainLayout->addLayout( controlLayout );
  mainLayout->addSpacing( 25 );
  mainLayout->addLayout( buttonLayout );

  timeout = new QTimer( this );
  timeout->setSingleShot( true );

  // Map altitude reference radio buttons
  QSignalMapper* signalMapper = new QSignalMapper( this );
  connect( _msl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( _msl, 0 );
  connect( _std, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( _std, 1 );
  connect( _agl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( _agl, 2 );
  connect( _ahl, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  signalMapper->setMapping( _ahl, 3 );

  // map altitude unit radio buttons
  QSignalMapper* signalMapperUnit = new QSignalMapper( this );
  connect( _meter, SIGNAL(clicked()), signalMapperUnit, SLOT(map()) );
  signalMapperUnit->setMapping( _meter, 0 );
  connect( _feet, SIGNAL(clicked()), signalMapperUnit, SLOT(map()) );
  signalMapperUnit->setMapping( _feet, 1 );

  // Map altitude QNH and leveling buttons
  QSignalMapper* signalMapperButtons = new QSignalMapper( this );
  connect( pplus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()) );
  signalMapperButtons->setMapping( pplus, 0 );
  connect( plus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()) );
  signalMapperButtons->setMapping( plus, 1 );
  connect( minus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()) );
  signalMapperButtons->setMapping( minus, 2 );
  connect( mminus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()) );
  signalMapperButtons->setMapping( mminus, 3 );

  connect( signalMapper, SIGNAL(mapped(int)), this, SLOT(slotModeChanged(int)) );
  connect( signalMapperUnit, SIGNAL(mapped(int)), this, SLOT(slotUnitChanged(int)) );
  connect( signalMapperButtons, SIGNAL(mapped(int)), this, SLOT(slotChangeSpinValue(int)) );

  connect( timeout, SIGNAL(timeout()), this, SLOT(reject()) );
  connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );

  load();
}

AltimeterModeDialog::~AltimeterModeDialog()
{
  noOfInstances--;
}

QString AltimeterModeDialog::mode2String()
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

int AltimeterModeDialog::mode()
{
  return GeneralConfig::instance()->getAltimeterMode();
}

void AltimeterModeDialog::load()
{
  _mode = GeneralConfig::instance()->getAltimeterMode();

  switch( _mode )
  {
    case 0:
      _msl->setChecked(true);
      break;
    case 1:
      _std->setChecked(true);
      break;
    case 2:
      _agl->setChecked(true);
      break;
    case 3:
      _ahl->setChecked(true);
      break;
    default:
      qWarning("AltimeterModeDialog::load(): invalid mode: %d", _mode);
  }

  saveMode = _mode;

  GeneralConfig *conf = GeneralConfig::instance();

  saveUnit = Altitude::getUnit();

  switch( saveUnit )
  {
    case Altitude::meters:
      _meter->setChecked(true);
      _unit = 0;
      spinLeveling->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );
      break;
    case Altitude::feet:
      _feet->setChecked(true);
      _unit = 1;
      spinLeveling->setValue( (int) conf->getGpsUserAltitudeCorrection().getFeet() );
      break;
    default:
      _meter->setChecked(true);
      _unit = 0;
      break;
  }

  saveLeveling = spinLeveling->value();

  saveQnh = conf->getQNH();

  spinQnh->setValue( saveQnh );

  startTimer();
}

void AltimeterModeDialog::slotModeChanged( int mode )
{
  _mode = mode;
  startTimer();
}

void AltimeterModeDialog::slotUnitChanged( int unit )
{
  _unit = unit;
  startTimer();
}

/** This slot is being called if the altitude has been changed. */
void AltimeterModeDialog::slotAltitudeChanged(const Altitude& altitude )
{
  _altitudeDisplay->setText( altitude.getText( true, 0 ) );
}

void AltimeterModeDialog::slotChangeSpinValue( int button )
{
  // Look which spin box has the focus
  if( QApplication::focusWidget() == spinQnh )
    {
      // qDebug() << "spinQnh has focus";
      switch( button )
        {
        case 0: // ++ was pressed
          spinQnh->setValue( spinQnh->value() + 5 );
          break;
        case 1: // + was pressed
          spinQnh->setValue( spinQnh->value() + 1 );
          break;
        case 2: // - was pressed
          spinQnh->setValue( spinQnh->value() - 1 );
          break;
        case 3: // -- was pressed
          spinQnh->setValue( spinQnh->value() - 5 );
          break;
        }

      return;
    }

  if( QApplication::focusWidget() == spinLeveling )
    {

      int steps;

      if( (Altitude::altitudeUnit) _unit == Altitude::meters )
        {
          steps = 1;
        }
      else
        {
          steps = 3;
        }

      switch( button )
        {
        case 0: // ++ was pressed
          spinLeveling->setValue( spinLeveling->value() + steps*5 );
          break;
        case 1: // + was pressed
          spinLeveling->setValue( spinLeveling->value() + steps );
          break;
        case 2: // - was pressed
          spinLeveling->setValue( spinLeveling->value() - steps );
          break;
        case 3: // -- was pressed
          spinLeveling->setValue( spinLeveling->value() - steps*5 );
          break;
        }

      Altitude::setUnit( (Altitude::altitudeUnit) _unit );

      GeneralConfig *conf = GeneralConfig::instance();
      conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters( spinLeveling->value()) );

      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
      return;
    }
}

void AltimeterModeDialog::slotSpinValueChanged( const QString& text )
{
  Q_UNUSED( text )
  // Restarts the timer after a spin box value change
  startTimer();
}

bool AltimeterModeDialog::changesDone()
{
  if( _unit != saveUnit ||
      _mode != saveMode ||
      saveLeveling != spinLeveling->value() ||
      saveQnh != spinQnh->value() )
    {
      return true;
    }

  return false;
}

void AltimeterModeDialog::accept()
{
  if( changesDone() )
    {
      Altitude::setUnit( (Altitude::altitudeUnit) _unit );

      GeneralConfig *conf = GeneralConfig::instance();

      conf->setUnitAlt( _unit );
      conf->setAltimeterMode( _mode );
      conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(spinLeveling->value()) );
      conf->setQNH( spinQnh->value() );
      conf->save();

      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
    }

  QDialog::accept();
}

void AltimeterModeDialog::reject()
{
  if( changesDone() )
    {
      Altitude::setUnit( (Altitude::altitudeUnit) saveUnit );

      GeneralConfig *conf = GeneralConfig::instance();

      conf->setUnitAlt( saveUnit );
      conf->setAltimeterMode( saveMode );
      conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(saveLeveling) );
      conf->setQNH( saveQnh );
      conf->save();

      emit newAltimeterMode();     // informs MapView
      emit newAltimeterSettings(); // informs GpsNmea
    }

  QDialog::reject();
}

void AltimeterModeDialog::startTimer()
{
  // @AP: let us take the user's defined info display time
  int time = GeneralConfig::instance()->getInfoDisplayTime();

  if( time > 0 )
    {
      timeout->start( time * 1000 ); // milli seconds have to be passed
    }
}
