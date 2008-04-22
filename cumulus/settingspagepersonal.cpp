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
#include <QPushButton>
#include <QToolTip>
#include <QDir>
#include <QFileDialog>

#include "generalconfig.h"
#include "altitude.h"
#include "settingspagepersonal.h"

SettingsPagePersonal::SettingsPagePersonal(QWidget *parent) :
    QWidget(parent), loadConfig(true)
{
  setObjectName("SettingsPagePersonal");
  
  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  QLabel * lbl = new QLabel(tr("Pilot name:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtName = new QLineEdit(this);
  topLayout->addWidget(edtName, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Date of birth:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtBirth = new QLineEdit(this);
  topLayout->addWidget(edtBirth, row, 1, 1, 2);
  row++;
  
  lbl = new QLabel(tr("Language:"), this);
  topLayout->addWidget(lbl, row, 0);
  langBox = new QComboBox(this);
  // langBox->setMaximumWidth(30);
  langBox->addItem("de");
  langBox->addItem("en");
  topLayout->addWidget(langBox, row, 1, 1, 1);
  row++;

  topLayout->setRowMinimumHeight(row++, 10);

  lbl = new QLabel(tr("Home site lat.:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLat = new LatEdit(this);
  topLayout->addWidget(edtHomeLat, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Home site lon.:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLong = new LongEdit(this);
  topLayout->addWidget(edtHomeLong, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Sidebar frame col.:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtFrameCol = new QLineEdit(this);
  topLayout->addWidget(edtFrameCol, row, 1, 1, 2);
  row++;

  QPushButton* userDirSelection = new QPushButton( tr("Data Directory"), this );
  userDirSelection->setToolTip(tr("Select your personal data directory"));
  topLayout->addWidget(userDirSelection, row, 0);

  connect(userDirSelection, SIGNAL( clicked()), this, SLOT(slot_openDirectoryDialog()) );

  userDataDir = new QLineEdit(this);
  topLayout->addWidget(userDataDir, row, 1, 1, 2);
  row++;

  topLayout->setRowStretch(row,10);
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
  
  edtFrameCol->setText( conf->getFrameCol() );

  userDataDir->setText( conf->getUserDataDirectory() );

  // search item to be selected
  int idx = langBox->findText(conf->getLanguage());
  
  if( idx != -1 )
    {
      langBox->setCurrentIndex(idx);
    }
}

/** called to initiate saving to the configurationfile */
void SettingsPagePersonal::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSurname( edtName->text() );
  conf->setBirthday( edtBirth->text() );
  conf->setLanguage( langBox->currentText() );
  conf->setFrameCol( edtFrameCol->text() );
  conf->setUserDataDirectory( userDataDir->text() );

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

/** called to open the directory selection dialog */
void SettingsPagePersonal::slot_openDirectoryDialog()
{
  QString dataDir = QFileDialog::getExistingDirectory( this,
                                                      tr("Please select your data directory"),
                                                      userDataDir->text(),
                                                      QFileDialog::ShowDirsOnly );
  if( dataDir.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  userDataDir->setText( dataDir.remove(  dataDir.size()-1, 1 ) );
}
