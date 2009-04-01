/***********************************************************************
**
**   settingspagelooknfeel.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGS_PAGE_LOOKNFEEL_H
#define SETTINGS_PAGE_LOOKNFEEL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "speed.h"

/**
 * This class represents the personal style settings.
 */

class SettingsPageLookNFeel : public QWidget
  {
    Q_OBJECT

  public:

    SettingsPageLookNFeel(QWidget *parent=0);
    ~SettingsPageLookNFeel();

  public slots: // Public slots

    /** called to initiate saving to the configurationfile */
    void slot_save();

    /** Called to initiate loading of the configurationfile. */
    void slot_load();

    /**
     * Called to ask is confirmation on close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

  private slots:

    /** Called to open the font dialog */
    void slot_openFontDialog();

    /** Called to open the color dialog */
    void slot_openColorDialog();

  private:

    bool loadConfig; // control loading of config data
    QString currentFont; // current selected font is saved here
    QColor  currentMapFrameColor; // current color of map frame

    QComboBox      *styleBox;
    QPushButton    *fontDialog;
    QPushButton    *editMapFrameColor;
    QDoubleSpinBox *screenSaverSpeedLimit;
    QCheckBox      *virtualKeybord;

    /** saves horizontal speed unit during construction of object */
    Speed::speedUnit unit;
    /** loaded speed for change control */
    double loadedSpeed;
  };

#endif
