/***********************************************************************
**
**   settingspagelooknfeel.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageLookNFeel
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for personal look and feel.
 *
 * \date 2008-2010
 *
 */

#ifndef SETTINGS_PAGE_LOOKNFEEL_H
#define SETTINGS_PAGE_LOOKNFEEL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "speed.h"

class SettingsPageLookNFeel : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageLookNFeel )

public:

  SettingsPageLookNFeel(QWidget *parent=0);

  virtual ~SettingsPageLookNFeel();

protected:

  virtual void showEvent( QShowEvent *event );

  virtual void hideEvent( QHideEvent *event );

public slots:

  /** called to initiate saving to the configuration file */
  void slot_save();

  /** Called to initiate loading of the configuration file. */
  void slot_load();

  /**
   * Called to ask is confirmation on close is needed.
   */
  void slot_query_close(bool& warn, QStringList& warnings);

private slots:

  /** Called to open the font dialog */
  void slot_openFontDialog();

  /** Called to open the menu font dialog */
  void slot_openMenuFontDialog();

  /** Called to open the color dialog */
  void slot_openColorDialog();

private:

  bool    m_loadConfig; // control loading of configuration data
  QString m_currentFont; // current selected font is saved here
  QString m_currentMenuFont; // current selected menu font is saved here
  QColor  m_currentMapFrameColor; // current color of map frame

  QComboBox      *styleBox;
  QPushButton    *fontDialog;
  QPushButton    *menuFontDialog;
  QPushButton    *editMapFrameColor;
  QDoubleSpinBox *screenSaverSpeedLimit;
  QCheckBox      *virtualKeybord;

  /** saves horizontal speed m_unit during construction of object */
  Speed::speedUnit m_unit;

  /** loaded speed for change control */
  double m_loadedSpeed;

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif
