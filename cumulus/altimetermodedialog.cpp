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

AltimeterModeDialog::AltimeterModeDialog (QWidget *parent)
  : QDialog(parent, Qt::WindowStaysOnTopHint),
    _mode(0)
{
  setObjectName("AltimeterModeDialog");
  setModal(true);
  setWindowTitle(tr("Set Altimeter"));
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

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setObjectName("mainlayout");

  QHBoxLayout* modeLayout = new QHBoxLayout();
  modeLayout->setObjectName("modelayout");
  modeLayout->addWidget(altMode);

  QHBoxLayout* radioLayout = new QHBoxLayout(altMode);
  radioLayout->setObjectName("radiolayout");
  radioLayout->addWidget(_msl);
  radioLayout->addWidget(_std);
  radioLayout->addWidget(_agl);
  radioLayout->addWidget(_ahl);

  // Align ok and cancel button at the left and right side of the
  // widget to have enough space between them. That shall avoid wrong
  // button pressing in turbulent air.
  int size = 40;

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

  QHBoxLayout *butLayout = new QHBoxLayout;
  butLayout->addWidget( ok );
  butLayout->addStretch();
  butLayout->addWidget( cancel );

  mainLayout->addLayout(modeLayout);
  mainLayout->addSpacing(25);
  mainLayout->addLayout(butLayout);

  timeout = new QTimer(this);
  timeout->setSingleShot(true);

  QSignalMapper* signalMapper = new QSignalMapper(this);
  connect(_msl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_msl, 0);
  connect(_std, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_std, 1);
  connect(_agl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_agl, 2);
  connect(_ahl, SIGNAL(clicked()), signalMapper, SLOT(map()));
  signalMapper->setMapping(_ahl, 3);

  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(change_mode(int)));
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
  GeneralConfig *conf = GeneralConfig::instance();

  _mode = conf->getAltimeterMode();

  switch (_mode) {
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

  setTimer();
}

void AltimeterModeDialog::work()
{
  // qDebug("_toggling_mode %d", _toggling_mode );

  GeneralConfig *conf = GeneralConfig::instance();

  _mode = conf->getAltimeterMode();
  _toggling_mode = conf->getAltimeterToggleMode();

  if( _toggling_mode )
    {
      // qDebug("_mode %d", _mode );

      switch( _mode )
        {
        case 0:
          save( 1 );
          break;
        case 1:
          save( 2 );
          break;
        case 2:
          save( 3 );
          break;
        case 3:
        default:
          save( 0 );
          break;
        }
    }
  else
    {
      load();
    }
}

void AltimeterModeDialog::save( int mode )
{
  _mode = mode;
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setAltimeterMode(_mode);
  conf->save();
  // qDebug("Save new val %d", _mode );
  emit newAltimeterMode();
  emit settingsChanged();
}

void AltimeterModeDialog::change_mode( int mode )
{
  _mode = mode;
}

void AltimeterModeDialog::accept()
{
  save(_mode);
  QDialog::accept();
}

void AltimeterModeDialog::setTimer()
{
  // @AP: let us take the user's defined info display time
  GeneralConfig *conf = GeneralConfig::instance();

  _time = conf->getInfoDisplayTime();

  if( _time > 0 )
    {
      timeout->start (_time * 1000);
    }
}
