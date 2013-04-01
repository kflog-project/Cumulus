/***********************************************************************
**
**   wpeditdialogpagegeneral.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli <kflog.cumulus@gmail.com>
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
 * \date 2002-2013
 *
 * \version $Id$
 */

#ifndef WPEDIT_DIALOG_PAGE_GENERAL_H
#define WPEDIT_DIALOG_PAGE_GENERAL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QString>

#include "coordedit.h"
#include "coordeditnumpad.h"
#include "waypoint.h"

class DoubleNumberEditor;
class NumberEditor;

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

private slots:

  /**
   * Called to make all text to upper cases.
   */
  void slot_textEditedName( const QString& text );

  /**
   * Called to make all text to upper cases.
   */
  void slot_textEditedCountry( const QString& text );

  /**
   * Called to make all text to upper cases.
   */
  void slot_textEditedNameFinished();

  /**
   * Called to make all text to upper cases.
   */
  void slot_textEditedCountryFinished();

private:

  QLineEdit *m_edtName;
  QLineEdit *m_edtDescription;
  QLineEdit *m_edtCountry;

  LatEditNumPad  *m_edtLat;
  LongEditNumPad *m_edtLong;
  NumberEditor   *m_edtElev;

  QComboBox *m_cmbType;
  QComboBox *m_cmbImportance;

  // Store loaded values and reuse them, if no coordinates
  // have been changed to avoid rounding errors caused by conversions.
  int m_loadedLat;
  int m_loadedLon;

  int getWaypointType();
  void setWaypointType(int type);
};

#endif
