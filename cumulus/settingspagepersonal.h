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

#include "coordedit.h"
#include "altitude.h"

/**This class represents the personal settings page
  *@author Heiner Lamprecht
  */

class SettingsPagePersonal : public QWidget
  {
    Q_OBJECT
  public:
    SettingsPagePersonal(QWidget *parent=0, const char *name=0);
    ~SettingsPagePersonal();

  protected:
    QLineEdit * edtName;
    QLineEdit * edtBirth;

    LatEdit * edtHomeLat;
    LongEdit * edtHomeLong;

  public slots: // Public slots
    /** called to initiate saving to the configurationfile */
    void slot_save();
  public slots: // Public slots
    /** Called to initiate loading of the configurationfile. */
    void slot_load();

  private:
    bool loadConfig; // control loading of config data
  };

#endif
