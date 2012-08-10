/***********************************************************************
**
**   settingspagepersonal.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2008-2012 by Axel Pauli
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
#include "varspinbox.h"

#ifdef INTERNET
#include "proxydialog.h"
#endif

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
  topLayout->addWidget(langBox, row, 1);
  row++;

  lbl = new QLabel(tr("Home site country:"), this);
  topLayout->addWidget(lbl, row, 0);
  edtHomeCountry = new QLineEdit(this);
  edtHomeCountry->setMaxLength(2);
  QRegExp rx("[A-Za-z]{2}");
  edtHomeCountry->setValidator( new QRegExpValidator(rx, this) );

#ifndef ANDROID
  connect( edtHomeCountry, SIGNAL(textEdited( const QString& )),
           this, SLOT(slot_textEditedCountry( const QString& )) );
#endif

  topLayout->addWidget(edtHomeCountry, row, 1);
  row++;

  lbl = new QLabel(tr("Home site name:"), this);
  topLayout->addWidget(lbl, row, 0);

  edtHomeName = new QLineEdit(this);
  edtHomeName->setMaxLength(8);
  topLayout->addWidget(edtHomeName, row, 1);
  row++;

  lbl = new QLabel(tr("Home site elevation:"), this);
  topLayout->addWidget(lbl, row, 0);

  spinHomeElevation = new QSpinBox;
  spinHomeElevation->setSingleStep( 10 );
  spinHomeElevation->setMaximum( 9999 );
  spinHomeElevation->setMinimum( -999 );
  spinHomeElevation->setSuffix( " " + Altitude::getUnitText() );
  VarSpinBox* hspin = new VarSpinBox( spinHomeElevation );
  topLayout->addWidget(hspin, row, 1);
  row++;

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

#ifdef INTERNET

  QPushButton* editProxy = new QPushButton( tr("Set Proxy") );
  editProxy->setToolTip(tr("Enter Proxy data if needed"));
  topLayout->addWidget(editProxy, row, 0);

  connect( editProxy, SIGNAL( clicked()), this, SLOT(slot_editProxy()) );

  proxyDisplay = new QLabel;
  topLayout->addWidget(proxyDisplay, row, 1, 1, 2);

#endif

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
  edtHomeCountry->setText( conf->getHomeCountryCode() );
  edtHomeName->setText( conf->getHomeName() );
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
  QString langPath = conf->getDataRoot() + "/locale";

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

#ifdef INTERNET

  if( conf->getProxy().isEmpty() )
    {
      proxyDisplay->setText( tr("No proxy defined") );
    }
  else
    {
      proxyDisplay->setText( conf->getProxy() );
    }

#endif

}

/** called to initiate saving to the configuration file */
void SettingsPagePersonal::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setSurname( edtName->text().trimmed() );
  conf->setHomeCountryCode( edtHomeCountry->text().trimmed().toUpper() );

  if( edtHomeName->text().trimmed().isEmpty() )
    {
      conf->setHomeName( tr("Home") );
    }
  else
    {
      conf->setHomeName( edtHomeName->text().trimmed() );
    }

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
  changed |= (edtHomeName->text() != conf->getHomeName());
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
  QString dataDir =
      QFileDialog::getExistingDirectory( this,
                                            tr("Please select your data directory"),
                                            userDataDir->text(),
                                            QFileDialog::ShowDirsOnly );
  if( dataDir.isEmpty() )
    {
      return; // nothing was selected by the user
    }

  userDataDir->setText( dataDir );
}

void SettingsPagePersonal::slot_textEditedCountry( const QString& text )
{
  // Change edited text to upper cases
  edtHomeCountry->setText( text.toUpper() );
}

#ifdef INTERNET

/**
 * Opens proxy dialog on user request.
 */
void SettingsPagePersonal::slot_editProxy()
{
  ProxyDialog *dialog = new ProxyDialog( this );

#ifdef ANDROID

  dialog->show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - dialog->width()/2,
                                   height()/2 - dialog->height()/2 ));
  //dialog->move( pos );

#endif

  if( dialog->exec() == QDialog::Accepted )
    {
      // update proxy display
      if( GeneralConfig::instance()->getProxy().isEmpty() )
        {
          proxyDisplay->setText( tr("No proxy defined") );
        }
      else
        {
          proxyDisplay->setText( GeneralConfig::instance()->getProxy() );
        }
    }
}

#endif
