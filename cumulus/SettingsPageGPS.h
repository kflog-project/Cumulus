/***********************************************************************
**
**   settingspagegps.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageGPS
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for the GPS device.
 *
 * \date 2002-2021
 *
 */

#ifndef SETTINGS_PAGE_GPS_H
#define SETTINGS_PAGE_GPS_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>

class SettingsPageGPS : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageGPS )

 public:

  SettingsPageGPS( QWidget *parent=0 );

  virtual ~SettingsPageGPS();

 private slots:

  /**
   * Called when the GPS device is changed.
   */
  void slotGpsDeviceChanged( const QString& text );

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

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  QComboBox*   GpsSource;
  QComboBox*   PressureDevice;
  QComboBox*   GpsDev;
  QComboBox*   GpsSpeed;
  QComboBox*   GpsAltitude;
  QCheckBox*   checkSyncSystemClock;
  QCheckBox*   saveNmeaData;
};

#endif
