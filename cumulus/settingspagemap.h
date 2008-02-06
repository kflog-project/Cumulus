/***********************************************************************
**
**   settingspagemap.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGEMAP_H
#define SETTINGSPAGEMAP_H

#include <QWidget>
#include <QDialog>
#include <QCheckBox>
#include <Q3ListView>
#include <Q3CheckListItem>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include <QLineEdit>
#include <QSpinBox>

#include "coordedit.h"
#include "projectionbase.h"
#include "distance.h"

/**contains map-related settings
  *@author André Somers
  */
class SettingsPageMapAdv;

class SettingsPageMap : public QWidget
{
    Q_OBJECT
public:
    SettingsPageMap(QWidget *parent=0, const char *name=0);
    ~SettingsPageMap();

public slots: // Public slots
    /**
     * Called to initiate saving to the configurationfile.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configurationfile
     */
    void slot_load();

    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

protected:
    QCheckBox * chkDrawDirectionLine;

    Q3ListView * lvLoadOptions;

    //listitems in listview
    Q3CheckListItem * liIsolines;
    Q3CheckListItem * liIsolineBorders;
    Q3CheckListItem * liWpLabels;
    Q3CheckListItem * liWpLabelsExtraInfo;
    Q3CheckListItem * liRoads;
    Q3CheckListItem * liHighways;
    Q3CheckListItem * liRailroads;
    Q3CheckListItem * liCities;
    Q3CheckListItem * liWaterways;
    Q3CheckListItem * liForests;  //forests and ice

    QPushButton * cmdAdvanced;

public:

    SettingsPageMapAdv * advancedPage;

private: // Private methods
    /**
     * Fills the list with loadoptions
     */
    void fillLoadOptionList();
};


class SettingsPageMapAdv : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    SettingsPageMapAdv(QWidget *parent=0, const char *name=0);

    /**
     * Destructor
     */
    ~SettingsPageMapAdv();

    /**
     * Checks, if the configuration of the projection has been changed
     */
    bool checkIsProjectionChanged();

    /**
     * Checks, if the configuration of the welt 2000 has been changed
     */
    bool checkIsWelt2000Changed();

    // SettingsPageMap takes care of our loading and saving,
    // so it needs access to our internals
    friend class SettingsPageMap; 

public slots: // Public slots
    /**
     * Called to initiate saving to the configurationfile.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configurationfile
     */
    void slot_load();

    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

private slots: // Private slots

    /**
     * Called if the text of the filter has been changed
     */
    void slot_filterChanged( const QString& text );

protected:

    QCheckBox * chkDeleteAfterCompile;
    QCheckBox * chkUnloadUnneeded;
    QComboBox * cmbProjection;
    LatEdit * edtLat1;
    LatEdit * edtLat2;
    LongEdit * edtLon;

    int cylinPar;
    int lambertV1;
    int lambertV2;
    int lambertOrigin;

    // variable currentProjType is an enum ProjectionBase::ProjectionType
    int currentProjType;

    /** saves distance unit set during construction of object */
    Distance::distanceUnit distUnit;

    // Country filter for welt 2000 data file
    QLineEdit* countryFilter;

    // Radius around home position for welt 2000 data file
    QSpinBox* homeRadius;

    /**
     * Reimplemented from QDialog
     * Reload settings from configurationfile so that changes won't
     * be written to the configuration.
     */
    void reject();

    /**
     * Reimplemented from QDialog. Called if OK button is pressed
     */
    void accept();

protected slots:

    void slotSelectProjection(int);
};
#endif
