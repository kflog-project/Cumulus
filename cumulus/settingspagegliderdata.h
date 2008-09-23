/***********************************************************************
**
**   settingspagegliderdata.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**  $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGE_GLIDER_DATA_H
#define SETTINGSPAGE_GLIDER_DATA_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QList>

#include "coordedit.h"
#include "polar.h"
#include "glider.h"

/**
 * This class represents the glider data settings page
 * @author Eggert Ehmke
 */
class SettingsPageGliderData : public QDialog
{
    Q_OBJECT

public:

    SettingsPageGliderData(QWidget *parent=0, Glider * glider=0);
    ~SettingsPageGliderData();
    Polar* getPolar();

private:

    void readPolarData ();
    /**
      * called to initiate saving to the configuration file
      */
    void save();

    /**
      * Called to initiate loading of the configuration file.
      */
    void load();

public slots: // Public slots
    /**
      * called when a glider type has been selected from the combobox
      */
    void slotActivated(const QString&);

    /**
      * called when the show button was pressed
      */
    void slotButtonShow();


signals: // Signals
    /**
      * Send if a glider has been edited.
      */
    void editedGlider(Glider*);
    /**
      * Send if a new glider has been made.
      */
    void newGlider(Glider*);

protected:

    /** overwritten method from QDialog */
    virtual void accept();

    /** overwritten method from QDialog */
    virtual void reject();

private:

    QComboBox* comboType;
    QDoubleSpinBox* spinV1;
    QDoubleSpinBox* spinW1;
    QDoubleSpinBox* spinV2;
    QDoubleSpinBox* spinW2;
    QDoubleSpinBox* spinV3;
    QDoubleSpinBox* spinW3;
    QLineEdit* edtGType;
    QLineEdit* edtGReg;
    QLineEdit* edtGCall;
    QPushButton* buttonShow;
    QSpinBox* emptyWeight;
    QSpinBox* addedLoad;
    QSpinBox* spinWater;
    QComboBox* comboSeats;

    QList<Polar*> _polars;
    Glider * _glider;
    Polar  * _polar;
    bool isNew;
    /** Flag to indicate if a glider object was created by this class */
    bool gliderCreated;
};

#endif
