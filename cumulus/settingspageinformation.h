/***********************************************************************
**
**   settingspageinformation.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003, 2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef  SettingsPageInformation_H
#define  SettingsPageInformation_H

#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

/**
 * The class manages the pop up window stay times and the alarm sound.
 * @author Axel.Pauli@onlinehome.de
 */
class SettingsPageInformation : public QWidget
{
    Q_OBJECT
public:

    SettingsPageInformation(QWidget *parent=0, const char *name=0);

    virtual ~SettingsPageInformation();


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
     * Called to restore the factory settings
     */
    void slot_setFactoryDefault();

private:

    QSpinBox*    spinAirfield;
    QSpinBox*    spinAirspace;
    QSpinBox*    spinWaypoint;
    QSpinBox*    spinWarning;
    QSpinBox*    spinInfo;
    QSpinBox*    spinSuppress;
    QCheckBox*   checkAlarmSound;
    QCheckBox*   calculateNearestSites;
    QCheckBox*   checkAltimeterToggle;
    QPushButton* buttonReset;

    bool loadConfig; // control loading of config data

};

#endif // SettingsPageInformation_h
