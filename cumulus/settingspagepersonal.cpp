/***********************************************************************
**
**   settingspagepersonal.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
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
#include "settingspagepersonal.h"

SettingsPagePersonal::SettingsPagePersonal(QWidget *parent) :
  QWidget(parent), loadConfig(true)
{
  setObjectName("SettingsPagePersonal");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // save current altitude unit. This unit must be considered during
  // storage. The internal storage is always in meters.
  altUnit = Altitude::getUnit();

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

  lbl = new QLabel(tr("Home site elevation:"), this);
  topLayout->addWidget(lbl, row, 0);
  spinHomeElevation = new QSpinBox(this);
  spinHomeElevation->setSingleStep( 10 );
  spinHomeElevation->setMaximum( 9999 );
  spinHomeElevation->setMinimum( -9999 );
  spinHomeElevation->setButtonSymbols(QSpinBox::PlusMinus);
  spinHomeElevation->setSuffix( " " + Altitude::getUnitText() );

  topLayout->addWidget(spinHomeElevation, row, 1);
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

  if( altUnit == Altitude::meters )
    { // user wants meters
      spinHomeElevation->setValue((int) rint(conf->getHomeElevation().getMeters()));
    }
  else
    { // user get feet
      spinHomeElevation->setValue((int) rint(conf->getHomeElevation().getFeet()));
    }

  // save spinbox value for later change check
  spinHomeElevationValue = spinHomeElevation->value();

  userDataDir->setText( conf->getUserDataDirectory() );

  // Determine user's wanted language
  QString langPath = conf->getInstallRoot() + "/locale";

  // Check for installed languages
  QDir dir( langPath );
  QStringList langs = dir.entryList( QDir::AllDirs | QDir::NoDot | QDir::NoDotDot );

  // Add default language English to the list.
  langs << "en";
  langs.sort();

  langBox->addItems(langs);

  // search item to be selected
  int idx = langBox->findText( conf->getLanguage() );

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

  Distance homeElevation;

  if( altUnit == Altitude::meters )
    {
      homeElevation.setMeters( spinHomeElevation->value() );
    }
  else
    {
      homeElevation.setFeet( spinHomeElevation->value() );
    }

  conf->setHomeElevation(homeElevation);

  conf->setUserDataDirectory( userDataDir->text().trimmed() );

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
  changed |= spinHomeElevationValue != spinHomeElevation->value();
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
