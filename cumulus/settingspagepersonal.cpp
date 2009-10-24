/***********************************************************************
**
**   settingspagepersonal.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2008-2009 by Axel Pauli
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
#include "settingspagepersonal.h"

SettingsPagePersonal::SettingsPagePersonal(QWidget *parent) :
  QWidget(parent), loadConfig(true)
{
  setObjectName("SettingsPagePersonal");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  GeneralConfig *conf = GeneralConfig::instance();
  QGridLayout* topLayout = new QGridLayout(this);
  int row=0;

  QLabel * lbl = new QLabel(tr("Pilot name:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtName = new QLineEdit(this);
  topLayout->addWidget(edtName, row, 1, 1, 2);
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

  lbl = new QLabel(tr("Home site latitude:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLat = new LatEdit(this, conf->getHomeLat());
  topLayout->addWidget(edtHomeLat, row, 1, 1, 2);
  row++;

  lbl = new QLabel(tr("Home site longitude:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeLong = new LongEdit(this, conf->getHomeLon());
  topLayout->addWidget(edtHomeLong, row, 1, 1, 2);
  row++;

  QPushButton* userDirSelection = new QPushButton( tr("Data Directory"), this );
  userDirSelection->setToolTip(tr("Select your personal data directory"));
  topLayout->addWidget(userDirSelection, row, 0);

  connect(userDirSelection, SIGNAL( clicked()), this, SLOT(slot_openDirectoryDialog()) );

  userDataDir = new QLineEdit(this);
  topLayout->addWidget(userDataDir, row, 1, 1, 2);
  row++;

  topLayout->setRowStretch(row, 10);
  topLayout->setColumnStretch( 2, 10 );
}

SettingsPagePersonal::~SettingsPagePersonal()
{}

/** Called to initiate loading of the configuration file. */
void SettingsPagePersonal::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  edtName->setText( conf->getSurname() );

  edtHomeLat->setKFLogDegree(conf->getHomeLat());
  edtHomeLong->setKFLogDegree(conf->getHomeLon());

  userDataDir->setText( conf->getUserDataDirectory() );

  // search item to be selected
  int idx = langBox->findText(conf->getLanguage());

  if( idx != -1 )
    {
      langBox->setCurrentIndex(idx);
    }
}

/** called to initiate saving to the configuration file */
void SettingsPagePersonal::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSurname( edtName->text() );
  conf->setLanguage( langBox->currentText() );
  conf->setUserDataDirectory( userDataDir->text() );

  // Check, if string input values have been changed. If not, no
  // storage is done to avoid rounding errors. They can appear if the
  // position formats will be changed between DMS <-> DDM vice versa.
  if( edtHomeLat->isInputChanged() )
    {
      conf->setHomeLat( edtHomeLat->KFLogDegree() );
    }

  if( edtHomeLong->isInputChanged() )
    {
      conf->setHomeLon( edtHomeLong->KFLogDegree() );
    }
}

/**
 * Checks if the home position has been changed.
 */
bool SettingsPagePersonal::checkIsHomePositionChanged()
{
  bool changed = false;

  if( edtHomeLat->isInputChanged() || edtHomeLong->isInputChanged() )
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomePositionChanged(): %d", changed );
  return changed;
}

/** Checks if the home latitude has been changed */
bool SettingsPagePersonal::checkIsHomeLatitudeChanged()
{
  bool changed = false;

  if (edtHomeLat->isInputChanged())
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomeLatitudeChanged(): %d", changed );
  return changed;
}

/** Checks if the home longitude has been changed */
bool SettingsPagePersonal::checkIsHomeLongitudeChanged()
{
  bool changed = false;

  if( edtHomeLong->isInputChanged() )
    {
      changed = true;
    }

  // qDebug( "SettingsPagePersonal::checkIsHomeLongitudeChanged(): %d", changed );
  return changed;
}

/** Called to ask is confirmation on the close is needed. */
void SettingsPagePersonal::slot_query_close( bool& warn, QStringList& warnings )
{
  bool changed = false;
  GeneralConfig *conf = GeneralConfig::instance();

  changed  = (edtName->text() != conf->getSurname());
  changed |= (langBox->currentText() != conf->getLanguage());
  changed |= checkIsHomePositionChanged();
  changed |= (userDataDir->text() != conf->getUserDataDirectory());

  if (changed)
    {
    // set warn to 'true' if the data has been changed.
      warn = true;
      warnings.append(tr("The Personal settings"));
    }
}

/** Called to open the directory selection dialog */
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

  userDataDir->setText( dataDir );
}
