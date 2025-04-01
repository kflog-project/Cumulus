/***********************************************************************
**
**   settingspagegps4a.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012-2025 by Axel Pauli
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
 * \date 2012-2025
 *
 */

#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

#include "numberEditor.h"

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
  * Called when the GPS altitude reference is changed.
  */
 void slotGpsAltitudeChanged( int index );

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
   * Emitted, if the pressure device is changed.
   * */
  void newPressureDevice( const QString& device );

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

  /** Called to toggle the WiFi menu lines. */
  void toggleWiFiMenu();

  QComboBox* GpsSource;
  QComboBox* PressureDevice;
  QComboBox* GpsAltitude;
  QCheckBox* saveNmeaData;
  NumberEditor*  WiFi1_IP;
  NumberEditor*  WiFi1_Port;
  NumberEditor*  WiFi2_IP;
  NumberEditor*  WiFi2_Port;
  NumberEditor*  WiFi3_IP;
  NumberEditor*  WiFi3_Port;
  QCheckBox*     WiFi1CB;
  QCheckBox*     WiFi2CB;
  QCheckBox*     WiFi3CB;
  QLabel*        label1;
  QLabel*        label2;
  QLabel*        label3;
};
