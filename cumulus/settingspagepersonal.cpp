/***********************************************************************
**
**   settingspagepersonal.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include <QLabel>
#include <QGridLayout>

#include "generalconfig.h"
#include "altitude.h"
#include "settingspagepersonal.h"

SettingsPagePersonal::SettingsPagePersonal(QWidget *parent, const char *name ) :
    QWidget(parent,name), loadConfig(true)
{
  QGridLayout * topLayout = new QGridLayout(this, 8, 3, 5);
  int row=0;

  QLabel * lbl = new QLabel(tr("Pilot name:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtName = new QLineEdit(this);
  topLayout->addMultiCellWidget(edtName, row, row, 1, 2);
  row++;

  lbl = new QLabel(tr("Date of birth:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtBirth = new QLineEdit(this);
  topLayout->addMultiCellWidget(edtBirth, row, row, 1, 2);

  topLayout->addRowSpacing(row++, 15);

  lbl = new QLabel(tr("Home site lat.:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLat = new LatEdit(this, "HomeLat");
  topLayout->addMultiCellWidget(edtHomeLat, row, row, 1, 2);
  row++;

  lbl = new QLabel(tr("Home site lon.:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLong = new LongEdit(this, "HomeLong");
  topLayout->addMultiCellWidget(edtHomeLong, row, row, 1, 2);
  row++;

  //topLayout->addRowSpacing(row++,10);
}

SettingsPagePersonal::~SettingsPagePersonal()
{}

/** Called to initiate loading of the configurationfile. */
void SettingsPagePersonal::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  edtName->setText( conf->getSurname() );
  edtBirth->setText( conf->getBirthday() );

  edtHomeLat->setKFLogDegree(conf->getHomeLat());
  edtHomeLong->setKFLogDegree(conf->getHomeLon());
}

/** called to initiate saving to the configurationfile */
void SettingsPagePersonal::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSurname( edtName->text() );
  conf->setBirthday( edtBirth->text() );

  // Check, if string input values have been changed. If not, no
  // storage is done to avoid roundings errors. They can appear if the
  // position formats will be changed between DMS <-> DDM vise versa.

  if( edtHomeLat->isInputChanged() )
    {
      conf->setHomeLat( edtHomeLat->KFLogDegree() );
    }

  if( edtHomeLong->isInputChanged() )
    {
      conf->setHomeLon( edtHomeLong->KFLogDegree() );
    }

  conf->save();
}
