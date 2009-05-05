/***********************************************************************
**
**   wpeditdialogpageaero.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WPEDIT_DIALOG_PAGE_AERO_H
#define WPEDIT_DIALOG_PAGE_AERO_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "waypoint.h"

/**
 * Provides the WpEditDialog page with aeronautical information
 * @author André Somers
 */
class WpEditDialogPageAero : public QWidget
  {
    Q_OBJECT

  public:
    WpEditDialogPageAero(QWidget *parent=0);

    virtual ~WpEditDialogPageAero();

  public slots: // Public slots
    /**
     * Called if the data needs to be saved.
     */
    void slot_save(wayPoint * wp);

    /**
     * Called if the page needs to load data from the waypoint
     */
    void slot_load(wayPoint * wp);

  private:
    QLineEdit *edtICAO;
    QLineEdit *edtFrequency;
    QComboBox *edtRunway1;
    QComboBox *edtRunway2;
    QLineEdit *edtLength;
    QCheckBox *chkLandable;
    QComboBox *cmbSurface;

    int getSurface();
    void setSurface(int s);
  };

#endif
