/***********************************************************************
**
**   settingspagelooknfeel.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.

**
***********************************************************************/

/**
 * \class SettingsPageLookNFeel
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for personal look and feel.
 *
 * \date 2008-2018
 *
 * \version 1.1
 *
 */

#ifndef SETTINGS_PAGE_LOOKNFEEL_H
#define SETTINGS_PAGE_LOOKNFEEL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSize>

#include "speed.h"

class DoubleNumberEditor;

class SettingsPageLookNFeel : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageLookNFeel )

public:

  SettingsPageLookNFeel(QWidget *parent=0);

  virtual ~SettingsPageLookNFeel();

signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

private slots:

  /** Called to open the font dialog */
  void slot_openFontDialog();

  /** Called to open the menu font dialog */
  void slot_openMenuFontDialog();

  /** Called to open the color dialog */
  void slot_openColorDialog();

  /** Called to set the default color. */
  void slot_defaultColor();

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

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

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  bool    m_loadConfig; // control loading of configuration data
  QString m_currentFont; // current selected font is saved here
  QString m_currentMenuFont; // current selected menu font is saved here
  QColor  m_currentMapFrameColor; // current color of map frame

  QComboBox      *m_styleBox;
  QPushButton    *m_fontDialog;
  QPushButton    *m_menuFontDialog;
  QPushButton    *m_editMapFrameColor;
  QCheckBox      *m_virtualKeybord;

  DoubleNumberEditor *m_screenSaverSpeedLimit;

  /** saves horizontal speed m_unit during construction of object */
  Speed::speedUnit m_unit;

  /** loaded speed for change control */
  double m_loadedSpeed;

  // size of the color pixmap used in icons
  QSize m_pixmapSize;
};

#endif
