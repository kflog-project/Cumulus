/***********************************************************************
**
**   wpeditdialogpageaero.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WpEditDialogPageAero
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This is the general page for the waypoint editor dialog
 *
 * \date 2002-2013
 */

#ifndef WPEDIT_DIALOG_PAGE_AERO_H
#define WPEDIT_DIALOG_PAGE_AERO_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>

#include "waypoint.h"

class DoubleNumberEditor;
class NumberEditor;

class WpEditDialogPageAero : public QWidget
{
  Q_OBJECT

public:

  WpEditDialogPageAero(QWidget *parent=0);

  virtual ~WpEditDialogPageAero();

public slots:

  /**
   * Called if the data needs to be saved.
   */
  void slot_save(Waypoint * wp);

  /**
   * Called if the page needs to load data from the waypoint
   */
  void slot_load(Waypoint * wp);

private:

  QLineEdit* edtICAO;
  QComboBox* edtRunway1;
  QComboBox* edtRunway2;
  QCheckBox* chkLandable;
  QComboBox* cmbSurface;

  DoubleNumberEditor* edtFrequency;
  NumberEditor*       edtLength;

  int getSurface();
  void setSurface(int s);
};

#endif
