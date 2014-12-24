/***********************************************************************
**
**   settingspagepersonal.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2002-2014
 *
 * \version 1.0
 */

#ifndef SETTINGS_PAGE_PERSONAL_H
#define SETTINGS_PAGE_PERSONAL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

#include "altitude.h"
#include "coordeditnumpad.h"

class NumberEditor;

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

private slots:

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

  /** Called to open the directory selection dialog */
  void slot_openDirectoryDialog();

  /** Called, if something has entered in edtHomeCountry. */
  void slot_textEditedCountry( const QString& input );

#ifdef INTERNET
#ifndef ANDROID

  /** Called, if proxy button was pressed. */
  void slot_editProxy();

  /** Called, if proxy data have been modified. */
  void slot_setProxyDisplay();

#endif
#endif

signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  /**
   * Emitted, if the home position was changed.
   */
  void homePositionChanged();

private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  QLineEdit *edtName;
  QComboBox *langBox;
  QLineEdit *edtHomeCountry;
  QLineEdit *edtHomeName;
  QLineEdit *userDataDir;

  NumberEditor    *edtHomeElevation;
  LatEditNumPad   *edtHomeLat;
  LongEditNumPad  *edtHomeLong;

#ifdef INTERNET
#ifndef ANDROID
  QLabel* proxyDisplay;
#endif
#endif

  /**
   * Initial elevation value after load from configuration.
   */
  int m_initalHomeElevationValue;

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit m_altUnit;
};

#endif
