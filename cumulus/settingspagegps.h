/***********************************************************************
**
**   settingspagegps.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageGPS
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for the GPS device.
 *
 * \date 2002-2010
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

  private:

    QComboBox*   GpsDev;
    QComboBox*   GpsSpeed;
    QComboBox*   GpsAltitude;
    QCheckBox*   checkSyncSystemClock;
    QCheckBox*   saveNmeaData;

  public slots:

    /**
     * Called to initiate saving to the configuration file.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configuration file
     */
    void slot_load();

  private slots:

  /**
   * Called when the GPS device is changed.
   */
  void slot_gpsDeviceChanged( const QString& text );

  signals:

  /**
   * Emitted when the NMEA logging into a file shall be started.
   */
  void startNmeaLog();

  /**
   * Emitted when the NMEA logging into a file shall be ended.
   */
  void endNmeaLog();
};

#endif
