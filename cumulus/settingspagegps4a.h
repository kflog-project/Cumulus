/***********************************************************************
**
**   settingspagegps4a.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageGPS4A
 *
 * \author Axel Pauli
 *
 * \brief GPS Configuration settings for Android.
 *
 * \date 2012-2018
 *
 */

#ifndef SETTINGS_PAGE_GPS4A_H
#define SETTINGS_PAGE_GPS4A_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

class SettingsPageGPS4A : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageGPS4A )

 public:

  SettingsPageGPS4A( QWidget *parent=0 );

  virtual ~SettingsPageGPS4A();

 private slots:

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 signals:

  /**
   * Emitted when the NMEA logging into a file shall be started.
   */
  void startNmeaLog();

  /**
   * Emitted when the NMEA logging into a file shall be ended.
   */
  void endNmeaLog();

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  /**
   * Emitted, if IP settings have been changed.
   */
  void ipSettingsChanged();

 private:

  /** Called to load the configuration file data. */
  void load();

  /**
   * Called to save the configuration file data.
   * Returns false, if parameters are not acceptable.
   */
  bool save();

  QComboBox* GpsSource;
  QComboBox* GpsAltitude;
  QCheckBox* saveNmeaData;
  QLineEdit* wlanIpAddress;
  QLineEdit* wlanPort;
  QLineEdit* wlanPassword;
};

#endif
