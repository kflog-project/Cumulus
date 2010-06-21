/***********************************************************************
**
**   altimetermodedialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Voellm
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
#include "altimetermodedialog.h"
#include "calculator.h"
#include "glider.h"
#include "mapconfig.h"

extern MapConfig *_globalMapConfig;

AltimeterModeDialog::AltimeterModeDialog (QWidget *parent) :
  QDialog(parent, Qt::WindowStaysOnTopHint),
  _mode(0),
  _unit(0)
{
  setObjectName("AltimeterModeDialog");
  setModal(true);
  setWindowTitle(tr("Altimeter Settings"));
  setAttribute(Qt::WA_DeleteOnClose);

#ifndef MAEMO
  int minFontSize = 14;
#else
  int minFontSize = 20;
#endif

  QFont b = font();
  b.setBold(true);
  setFont(b);

  // set font size to a reasonable and usable value
  if( font().pointSize() < minFontSize )
    {
      QFont cf = font();
      cf.setPointSize( minFontSize );
      this->setFont(cf);
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

  QHBoxLayout* mainLayout = new QHBoxLayout(this);

  QVBoxLayout* controlLayout = new QVBoxLayout;
  controlLayout->addWidget(altMode);

  QHBoxLayout* radioLayout = new QHBoxLayout(altMode);
  radioLayout->addWidget(_msl);
  radioLayout->addWidget(_std);
  radioLayout->addWidget(_agl);
  radioLayout->addWidget(_ahl);

  //---------------------------------------------------------------------------

  QGroupBox* altUnit = new QGroupBox( this );
  _meter = new QRadioButton( tr( "Meter" ), altUnit );
  _feet  = new QRadioButton( tr( "Feet" ), altUnit );
  _fl    = new QRadioButton( tr( "FL" ), altUnit );

  _meter->setChecked( true );
  _meter->setEnabled( true );
  _feet->setEnabled( true );
  _fl->setEnabled( true );
  _fl->setVisible( false );

  QHBoxLayout* radioLayout1 = new QHBoxLayout(altUnit);
  radioLayout1->addWidget(_meter);
  radioLayout1->addWidget(_feet);
  radioLayout1->addWidget(_fl);

  controlLayout->addWidget(altUnit);

  spinUserCorrection = new QSpinBox(this);
  spinUserCorrection->setMinimum(-1000);
  spinUserCorrection->setMaximum(1000);
  spinUserCorrection->setButtonSymbols(QSpinBox::NoButtons);

  pplus  = new QPushButton("++", this);
  plus   = new QPushButton("+", this);
  mminus = new QPushButton("--", this);
  minus  = new QPushButton("-", this);

  int size = 40;

#ifdef MAEMO
  size = 80;
#endif

  pplus->setMinimumSize(size, size);
  plus->setMinimumSize(size, size);
  minus->setMinimumSize(size, size);
  mminus->setMinimumSize(size, size);

  pplus->setMaximumSize(size, size);
  plus->setMaximumSize(size, size);
  minus->setMaximumSize(size, size);
  mminus->setMaximumSize(size, size);

  pplus->setFocusPolicy(Qt::NoFocus);
  plus->setFocusPolicy(Qt::NoFocus);
  minus->setFocusPolicy(Qt::NoFocus);
  mminus->setFocusPolicy(Qt::NoFocus);

  QHBoxLayout *pmLayout = new QHBoxLayout;
  pmLayout->setSpacing(5);
  pmLayout->addWidget(pplus, Qt::AlignLeft);
  pmLayout->addWidget(plus, Qt::AlignLeft);
  pmLayout->addStretch(10);
  pmLayout->addSpacing(20);
  pmLayout->addWidget( spinUserCorrection );
  pmLayout->addSpacing(20);
  pmLayout->addStretch(10);
  pmLayout->addWidget(minus, Qt::AlignRight);
  pmLayout->addWidget(mminus, Qt::AlignRight);

  controlLayout->addLayout( pmLayout );

  // Align ok and cancel button at the left and right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  size = 40;

#ifdef MAEMO
  size = 80;
#endif

  QPushButton *cancel = new QPushButton(this);
  cancel->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  cancel->setIconSize(QSize(30, 30));
  cancel->setMinimumSize(size, size);
  cancel->setMaximumSize(size, size);
  cancel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QPushButton *ok = new QPushButton(this);
  ok->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("ok.png")));
  ok->setIconSize(QSize(30, 30));
  ok->setMinimumSize(size, size);
  ok->setMaximumSize(size, size);
  ok->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);

  QVBoxLayout *buttonLayout = new QVBoxLayout;
  buttonLayout->addWidget( cancel );
  buttonLayout->addStretch();
  buttonLayout->addWidget( ok );

  mainLayout->addLayout(controlLayout);
  mainLayout->addSpacing(25);
  mainLayout->addLayout(buttonLayout);

  timeout = new QTimer(this);
  timeout->setSingleShot(true);

  // Altitude reference radio buttons
  QSignalMapper* signalMapper = new QSignalMapper(this);
  connect(_msl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_msl, 0);
  connect(_std, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_std, 1);
  connect(_agl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_agl, 2);
  connect(_ahl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_ahl, 3);

  // Altitude unit radio buttons
  QSignalMapper* signalMapperUnit = new QSignalMapper(this);
  connect(_meter, SIGNAL(clicked()), signalMapperUnit, SLOT(map()));
  signalMapperUnit->setMapping(_meter, 0);
  connect(_feet, SIGNAL(clicked()), signalMapperUnit, SLOT(map()));
  signalMapperUnit->setMapping(_feet, 1);
  connect(_fl, SIGNAL(clicked()), signalMapperUnit, SLOT(map()));
  signalMapperUnit->setMapping(_fl, 2);

  // Altitude correction buttons
  QSignalMapper* signalMapperButtons = new QSignalMapper(this);
  connect(pplus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()));
  signalMapperButtons->setMapping(pplus, 0);
  connect(plus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()));
  signalMapperButtons->setMapping(plus, 1);
  connect(minus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()));
  signalMapperButtons->setMapping(minus, 2);
  connect(mminus, SIGNAL(clicked()), signalMapperButtons, SLOT(map()));
  signalMapperButtons->setMapping(mminus, 3);

  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slotChangeMode(int)));
  connect(signalMapperUnit, SIGNAL(mapped(int)), this, SLOT(slotChangeUnit(int)));
  connect(signalMapperButtons, SIGNAL(mapped(int)), this, SLOT(slotChangeValue(int)));

  connect (timeout, SIGNAL(timeout()), this, SLOT(reject()));
  connect (ok, SIGNAL(clicked()), this, SLOT(accept()));
  connect (cancel, SIGNAL(clicked()), this, SLOT(reject()));

  load();
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

AltimeterModeDialog::~AltimeterModeDialog()
{}

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

  GeneralConfig *conf = GeneralConfig::instance();

  switch( Altitude::getUnit() )
  {
    case Altitude::meters:
      _meter->setChecked(true);
      _unit = 0;
      spinUserCorrection->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );
      break;
    case Altitude::feet:
      _feet->setChecked(true);
      _unit = 1;
      spinUserCorrection->setValue( (int) conf->getGpsUserAltitudeCorrection().getFeet() );
      break;
    case Altitude::flightlevel:
      _fl->setChecked(true);
      _unit = 2;
      break;
    default:
      _meter->setChecked(true);
      _unit = 0;
      break;
  }

  slotSetTimer();
}

void AltimeterModeDialog::save( int mode )
{
  _mode = mode;
  Altitude::setUnit( (Altitude::altitudeUnit) _unit );

  GeneralConfig *conf = GeneralConfig::instance();

  conf->setAltimeterMode(_mode);
  conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(spinUserCorrection->value()) );
  conf->save();

  emit newAltimeterMode();
  emit settingsChanged();
  emit newAltimeterSetting();
}

void AltimeterModeDialog::slotChangeMode( int mode )
{
  _mode = mode;
  slotSetTimer();
}

void AltimeterModeDialog::slotChangeUnit( int unit )
{
  _unit = unit;
  slotSetTimer();
}

void AltimeterModeDialog::slotChangeValue( int newValue )
{
  int diff;

  if( (Altitude::altitudeUnit) _unit == Altitude::meters )
    {
      diff = 1;
    }
  else
    {
      diff = 3;
    }

  switch( newValue )
    {
    case 0: // ++ was pressed
      spinUserCorrection->setValue( spinUserCorrection->value() + diff*5 );
      break;
    case 1: // + was pressed
      spinUserCorrection->setValue( spinUserCorrection->value() + diff );
      break;
    case 2: // - was pressed
      spinUserCorrection->setValue( spinUserCorrection->value() - diff );
      break;
    case 3: // -- was pressed
      spinUserCorrection->setValue( spinUserCorrection->value() - diff*5 );
      break;
    }

  Altitude::setUnit( (Altitude::altitudeUnit) _unit );

  GeneralConfig *conf = GeneralConfig::instance();
  conf->setGpsUserAltitudeCorrection( Altitude::convertToMeters(spinUserCorrection->value()) );
  emit newAltimeterSetting();

  slotSetTimer();
}

void AltimeterModeDialog::accept()
{
  save(_mode);
  QDialog::accept();
}

void AltimeterModeDialog::slotSetTimer()
{
  // @AP: let us take the user's defined info display time
  _time = GeneralConfig::instance()->getInfoDisplayTime();

  if( _time > 0 )
    {
      timeout->start (_time * 1000);
    }
}
