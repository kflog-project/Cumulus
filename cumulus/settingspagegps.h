/***********************************************************************
**
**   settingspagegps.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGS_PAGE_GPS_H
#define SETTINGS_PAGE_GPS_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>

/**
 * This class represents the GPS settings page.
 * @author André Somers
 */
class SettingsPageGPS : public QWidget
  {
    Q_OBJECT

  public:

    SettingsPageGPS(QWidget *parent=0);
    ~SettingsPageGPS();

  private:

    QComboBox* GpsDev;
    QComboBox* GpsSpeed;
    QComboBox* GpsAltitude;
    QCheckBox* checkSoftStart;
    QCheckBox* checkHardStart;
    QCheckBox* checkSyncSystemClock;
    QPushButton* buttonReset;
    QSpinBox* spinUserCorrection;

  public slots: // Public slots

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
   * Called when the GPS altitude mode is changed.
   */
  void slot_altitude_mode(int);

  /**
   * Called when the GPS device is changed.
   */
  void slot_gpsDeviceChanged( const QString& text );

  /**
   * Called when the user correction value is changed.
   */
  void slot_spinUserCorrectionChanged( int newValue );

  };

#endif
