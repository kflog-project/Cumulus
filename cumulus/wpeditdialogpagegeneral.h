/***********************************************************************
**
**   wpeditdialogpagegeneral.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2011 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WpEditDialogPageGeneral
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This is the general page for the waypoint editor dialog
 *
 * \date 2002-2011
 */

#ifndef WPEDIT_DIALOG_PAGE_GENERAL_H
#define WPEDIT_DIALOG_PAGE_GENERAL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QString>

#include "coordedit.h"
#include "waypoint.h"

class WpEditDialogPageGeneral : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WpEditDialogPageGeneral )

public:

  WpEditDialogPageGeneral(QWidget *parent=0 );

  virtual ~WpEditDialogPageGeneral();

public slots:

  /**
   * called if data needs to be saved
   */
  void slot_save(Waypoint *wp);

  /**
   * called if data needs to be loaded
   */
  void slot_load(Waypoint *wp);

  /**
   * Called to make all text to upper cases.
   */
  void slot_textEdited( const QString& text );

private:

  QLineEdit *edtName;
  QLineEdit *edtDescription;

  LatEdit   *edtLat;
  LongEdit  *edtLong;
  QLineEdit *edtElev;

  QComboBox *cmbType;
  QComboBox *cmbImportance;

  // Store loaded values and reuse them, if no coordinates
  // have been changed to avoid rounding errors caused by conversions.
  int loadedLat;
  int loadedLon;

  int getWaypointType();
  void setWaypointType(int type);

};

#endif
