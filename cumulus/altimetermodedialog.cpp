/***********************************************************************
**
**   altimetermodedialog.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Voellm 
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QLabel>
#include <QFont>
#include <QRadioButton>
#include <QGridLayout>

#include "generalconfig.h"
#include "altimetermodedialog.h"
#include "cucalc.h"
#include "glider.h"
#include "mapconfig.h"

extern MapConfig * _globalMapConfig;

int AltimeterModeDialog::_mode = 0; // MSL mode

AltimeterModeDialog::AltimeterModeDialog (QWidget *parent)
    : QDialog(parent, "altimetermodedialog", true, Qt::WStyle_StaysOnTop)
{
  setCaption (tr("Altimeter"));
  QGridLayout* topLayout = new QGridLayout(this, 3,3,5);

  QFont fnt( "Helvetica", 16, QFont::Bold  );
  this->setFont(fnt);

  GeneralConfig *conf = GeneralConfig::instance();

  _mode = conf->getAltimeterMode();
  _toggling_mode = conf->getAltimeterToggleMode();

  int row = 0;

  altMode = new Q3ButtonGroup(1, Qt::Vertical, tr("Altimeter Mode"),this);
  QRadioButton * msl=new QRadioButton(tr("MSL"),altMode);
  QRadioButton * gnd=new QRadioButton(tr("AGL"),altMode);
  QRadioButton * std=new QRadioButton(tr("STD"),altMode);
  msl->setChecked(true);
  topLayout->addMultiCellWidget(altMode,row,row,0,2, Qt::AlignCenter);
  row++;
  msl->setEnabled(true);
  gnd->setEnabled(true);
  std->setEnabled(true);
  altMode->setButton(_mode);

  buttonOK = new QPushButton(tr("OK"), this);
  // buttonOK->setFont(fnt1);
  topLayout->addWidget (buttonOK, row, 2);
  buttonCancel = new QPushButton (tr("Cancel"), this);
  // buttonCancel->setFont(fnt1);
  topLayout->addWidget (buttonCancel, row++, 0);

  timeout = new QTimer(this);
  connect (timeout, SIGNAL(timeout()), this, SLOT(reject()));
  connect (buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
  connect (buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

QString AltimeterModeDialog::Pretext()
{
  switch( _mode )
    {
    case 0:
      return QString("Msl");
    case 1:
      return QString("Agl");
    case 2:
    default:
      return QString("Std");
    }
}

AltimeterModeDialog::~AltimeterModeDialog()
{}

void AltimeterModeDialog::load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  _mode = conf->getAltimeterMode();
  _toggling_mode = conf->getAltimeterToggleMode();

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
        default:
          save( 0 );
          break;
        }
    }
  else
    {
      load();
      show();
    }
}

void AltimeterModeDialog::save(int mode)
{
  _mode = mode;
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setAltimeterMode(_mode);
  conf->save();
  // qDebug("Save new val %d", _mode );
  emit newAltimeterMode();
  emit settingsChanged();
}

void AltimeterModeDialog::accept()
{
  int selected_mode = altMode->id(altMode->selected());

  if(  selected_mode != _mode )
    {
      _mode = selected_mode;
      // qDebug("New Altimeter Mode: %d", _mode );
      save(_mode);
    }

  QDialog::accept();
}

void AltimeterModeDialog::setTimer()
{
  // @AP: let us take the user's defined info display time
  GeneralConfig *conf = GeneralConfig::instance();

  _time = conf->getInfoDisplayTime();

  timeout->start (_time * 1000);
}
