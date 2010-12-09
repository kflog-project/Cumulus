/***********************************************************************
**
**   settingspageterraincolors.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageTerrainColors
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for terrain colors.
 *
 * This configuration widget shows the terrain colors used for drawing of contour areas.
 * The user can modify the assigned color via a color chooser dialog, if he wants that.
 * The widget shows the altitudes in the user selected unit (meter/feed).
 *
 * \date 2002-2010
 */

#ifndef SETTINGS_PAGE_TERRAIN_COLOR_H
#define SETTINGS_PAGE_TERRAIN_COLOR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QColor>
#include <QSize>

#include "elevationcolorimage.h"

class SettingsPageTerrainColors : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageTerrainColors )

public:

  SettingsPageTerrainColors(QWidget *parent=0);

  virtual ~SettingsPageTerrainColors();

public slots:

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
   * Called to edit the ground color
   */
  void slot_editGroundColor();

  /**
   * Called to set all colors to their default value.
   */
  void slot_setColorDefaults();

private:

  // size of the color pixmaps used in icons
  QSize pixmapSize;

  // modification color flag
  bool colorsChanged;

  // widget which shows the elevation color bar
  ElevationColorImage *elevationImage;

  // Internal temporary color working list. Will be saved as new colors,
  // when the method slot_save() is called.
  QColor terrainColor[51];

  // temporary storage of ground color
  QColor groundColor;

  // brings up the color chooser dialog
  QPushButton* editColorButton;

  // brings up the color chooser dialog for the grund color
  QPushButton* groundColorButton;

  // set all colors back to the defaults.
  QPushButton* defaultColorButton;

  // Selects the elevation level to be modified
  QComboBox* elevationBox;
};

#endif
