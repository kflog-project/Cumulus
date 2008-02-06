/***********************************************************************
**
**   settingspagegps.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somer, 2008 Axel paulis
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGEGPS_H
#define SETTINGSPAGEGPS_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>

/**
 * This class represents the GPS settings page
 * @author Heiner Lamprecht
 */
class SettingsPageGPS : public QWidget
{
    Q_OBJECT
public:
    SettingsPageGPS(QWidget *parent=0, const char *name=0);
    ~SettingsPageGPS();

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
     * Called to initiate saving to the configurationfile.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configurationfile
     */
    void slot_load();

private slots:
    /**
     * called when the GPS altitude mode is changed:
     */
    void slot_altitude_mode(int);
};

#endif
