/***********************************************************************
**
**   settingspageterraincolors.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGE_TERRAIN_COLOR_H
#define SETTINGSPAGE_TERRAIN_COLOR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QColor>

#include "elevationimage.h"

/**
 * @author Axel Pauli
 */
class SettingsPageTerrainColors : public QWidget
  {
    Q_OBJECT

  public:

    SettingsPageTerrainColors(QWidget *parent=0);
    ~SettingsPageTerrainColors();

  public slots: // Public slots
    /**
     * Called to initiate saving to the configuration file.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configuration file
     */
    void slot_load();
    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

  private slots:

    /**
     * Called to edit the color of the combo box selection
     */
    void slot_editColor();

    /**
     * Called to set all colors to their default value.
     */
    void slot_setColorDefaults();

  private:

    // modification color flag
    bool colorsChanged;

    // widget which shows the elevation color bar
    ElevationImage *elevationImage;

    // internal color working list
    QColor terrainColor[51];

    // brings up the color chooser dialog
    QPushButton* editColorButton;

    // set all colors back to the defaults.
    QPushButton* defaultColorButton;

    // Selects the altitude level to be modified
    QComboBox* altitudeBox;
};

#endif
