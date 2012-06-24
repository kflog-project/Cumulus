/***********************************************************************
**
**   settingspagegps4a.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageGPS4A
 *
 * \author Axel Pauli
 *
 * \brief GPS Configuration settings for Android.
 *
 * \date 2012
 *
 */

#ifndef SETTINGS_PAGE_GPS4A_H
#define SETTINGS_PAGE_GPS4A_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>

class SettingsPageGPS4A : public QWidget
{
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( SettingsPageGPS4A )

  public:

    SettingsPageGPS4A( QWidget *parent=0 );

    virtual ~SettingsPageGPS4A();

  private:

    QComboBox*   GpsAltitude;
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
