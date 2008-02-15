/***********************************************************************
**
**   settingspagepolar.h
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

#ifndef SETTINGSPAGEPOLAR_H
#define SETTINGSPAGEPOLAR_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QList>
#include <QGroupBox>

#include "coordedit.h"
#include "polar.h"
#include "glider.h"

/**
 * This class represents the polar settings page
 * @author Eggert Ehmke
 */
class SettingsPagePolar : public QDialog
{
    Q_OBJECT
public:
    SettingsPagePolar(QWidget *parent=0, const char *name=0, Glider * glider=0);
    ~SettingsPagePolar();
    Polar* getPolar();


public slots: // Public slots
    /**
      * called to initiate saving to the configurationfile
      */
    void slot_save();

    /**
      * Called to initiate loading of the configurationfile.
      */
    void slot_load();

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
    QGroupBox* bgSeats;
    QRadioButton* seatsOne;
    QRadioButton* seatsTwo;

    virtual void accept();


private:
    void readPolarData ();
    QList<Polar*> _polars;
    Glider * _glider;
    Polar  * _polar;
    bool isNew;
};

#endif
