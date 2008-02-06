/***********************************************************************
**
**   wpeditdialogpageaero.h
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

#ifndef WPEDITDIALOGPAGEAERO_H
#define WPEDITDIALOGPAGEAERO_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "degreespinbox.h"
#include "waypoint.h"

/**
 * Provides the WPEditDialog page with aeronautical information
 * @author André Somers
 */
class WPEditDialogPageAero : public QWidget
  {
    Q_OBJECT

  public:
    WPEditDialogPageAero(QWidget *parent=0, const char *name=0);

    ~WPEditDialogPageAero();

  public slots: // Public slots
    /**
     * Called if the data needs to be saved.
     */
    void slot_save(wayPoint * wp);

    /**
     * Called if the page needs to load data from the waypoint
     */
    void slot_load(wayPoint * wp);

  protected:
    QLineEdit * edtICAO;
    QLineEdit * edtFrequency;
    DegreeSpinBox * edtRunway;
    QLineEdit * edtLength;
    QCheckBox * chkLandable;
    QComboBox * cmbSurface;

  private:
    int getSurface();
    void setSurface(int s);
  };

#endif
