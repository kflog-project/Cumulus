/***********************************************************************
**
**   settingspageterraincolors.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009-2013 by Axel Pauli
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
 * \date 2002-2013
 *
 * \version $Id$
 *
 */

#ifndef SETTINGS_PAGE_TERRAIN_COLOR_H
#define SETTINGS_PAGE_TERRAIN_COLOR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QColor>
#include <QSize>
#include <QSpinBox>

#include "elevationcolorimage.h"

class SettingsPageTerrainColors : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageTerrainColors )

 public:

  SettingsPageTerrainColors(QWidget *parent=0);

  virtual ~SettingsPageTerrainColors();

 protected:

  virtual void showEvent(QShowEvent *);

  virtual void hideEvent(QHideEvent *);

 signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

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

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();


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

  // provides an positive or negative elevation offset index.
  QSpinBox* elevationOffset;

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif
