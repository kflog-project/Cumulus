/***********************************************************************
**
**   settingspagepersonal.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGEPERSONAL_H
#define SETTINGSPAGEPERSONAL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>

#include "coordedit.h"
#include "altitude.h"

/**
 * This class represents the personal settings page
 */

class SettingsPagePersonal : public QWidget
  {
    Q_OBJECT
  public:
    SettingsPagePersonal(QWidget *parent=0);
    ~SettingsPagePersonal();

  public slots: // Public slots

    /** called to initiate saving to the configurationfile */
    void slot_save();

    /** Called to initiate loading of the configurationfile. */
    void slot_load();

    private slots:

    /** called to open the directory selection dialog */
    void slot_openDirectoryDialog();

  private:
    bool loadConfig; // control loading of config data
    
    QLineEdit *edtName;
    QLineEdit *edtBirth;
    QComboBox *langBox;
    LatEdit   *edtHomeLat;
    LongEdit  *edtHomeLong;
    QLineEdit *edtFrameCol;
    QLineEdit *userDataDir;

  };

#endif
