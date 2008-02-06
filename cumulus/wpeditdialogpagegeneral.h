/***********************************************************************
**
**   wpeditdialogpagegeneral.h
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

#ifndef WPEDITDIALOGPAGEGENERAL_H
#define WPEDITDIALOGPAGEGENERAL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include "coordedit.h"
#include "waypoint.h"

/**
 * This is the general page for the Waypoint edit dialog
 * @author André Somers
 */
class WPEditDialogPageGeneral : public QWidget
  {
    Q_OBJECT
  public:
    WPEditDialogPageGeneral(QWidget *parent=0, const char *name=0);

    ~WPEditDialogPageGeneral();

  public slots: // Public slots
    /**
     * called if data needs to be saved
     */
    void slot_save(wayPoint * wp);

    /**
     * called if data needs to be loaded
     */
    void slot_load(wayPoint * wp);

  protected: // Protected attributes
    QLineEdit * edtName;
    QLineEdit * edtDescription;

    LatEdit * edtLat;
    LongEdit * edtLong;
    QLineEdit * edtElev;

    QComboBox * cmbType;
    QComboBox * cmbImportance;

  private:
    int getWaypointType();
    void setWaypointType(int type);

  };

#endif
