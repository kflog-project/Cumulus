/***********************************************************************
**
**   settingspagelooknfeel.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGELOOKNFEEL_H
#define SETTINGSPAGELOOKNFEEL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

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

  private:
    bool loadConfig; // control loading of config data
    
    QComboBox *styleBox;
    QSpinBox  *spinFontSize;
    QLineEdit *edtFrameCol;
    QCheckBox *virtualKeybord;
  };

#endif
