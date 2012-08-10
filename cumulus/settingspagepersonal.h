/***********************************************************************
**
**   settingspagepersonal.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPagePersonal
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for personal settings.
 *
 * Configuration settings for personal settings.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef SETTINGS_PAGE_PERSONAL_H
#define SETTINGS_PAGE_PERSONAL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>
#include <QSpinBox>

#include "altitude.h"
#include "coordedit.h"

class SettingsPagePersonal : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPagePersonal )

public:

  SettingsPagePersonal(QWidget *parent=0);

  virtual ~SettingsPagePersonal();

  /** Checks if the home position has been changed */
  bool checkIsHomePositionChanged();

  /** Checks if the home latitude has been changed */
  bool checkIsHomeLatitudeChanged();

  /** Checks if the home longitude has been changed */
  bool checkIsHomeLongitudeChanged();

public slots:

  /** Called to initiate saving to the configuration file */
  void slot_save();

  /** Called to initiate loading of the configuration file. */
  void slot_load();

  /** Called to ask is confirmation on the close is needed. */
  void slot_query_close(bool& warn, QStringList& warnings);

private slots:

  /** Called to open the directory selection dialog */
  void slot_openDirectoryDialog();

  /** Called, if something has entered in edtHomeCountry. */
  void slot_textEditedCountry( const QString& input );

#ifdef INTERNET

  /** Called, if proxy button was pressed. */
  void slot_editProxy();

#endif

private:

  bool loadConfig; // control loading of configuration data

  QLineEdit *edtName;
  QComboBox *langBox;
  QLineEdit *edtHomeCountry;
  QLineEdit *edtHomeName;
  LatEdit   *edtHomeLat;
  LongEdit  *edtHomeLong;
  QSpinBox  *spinHomeElevation;
  QLineEdit *userDataDir;

#ifdef INTERNET
  QLabel* proxyDisplay;
#endif

  int spinHomeElevationValue;

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit altUnit;
};

#endif
