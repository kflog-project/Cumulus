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
#include <QGroupBox>
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
  void slot_save(Waypoint* wp);

  /**
   * Called if the widget needs to load the data from the waypoint.
   */
  void slot_load(Waypoint* wp);

 private slots:

 /**
  * Called, if runway enable checkbox is selected/unselected.
  *
  * \param state The new state of check box.
  */
 void slot_stateChangedRwy1Enable( int state )
   {
     gboxRunway1->setEnabled( state == 0 ? false : true );
   };

 /**
  * Called, if runway enable checkbox is selected/unselected.
  *
  * \param state The new state of check box.
  */
 void slot_stateChangedRwy2Enable( int state )
   {
     gboxRunway2->setEnabled( state == 0 ? false : true );
   };

 /**
  * Called, if the runway heading editor is closed by the user.
  *
  * \param value New runway heading
  */
 void slot_rwy1HeadingEdited( const QString& value );

 /**
  * Called, if the runway heading editor is closed by the user.
  *
  * \param value New runway heading
  */
 void slot_rwy2HeadingEdited( const QString& value );

 private:

  NumberEditor* createRunwayHeadingEditor( QWidget* parent=0 );

  NumberEditor* createRunwayLengthEditor( QWidget* parent=0 );

  /**
   * Gets the runway surface as enumration.
   *
   * \param cbox Combobox to be queried
   *
   * \return The internal used surface type as integer enumeration.
   */
  int getSurface( QComboBox* cbox );

  /**
   * Sets the runway surface text of the combobox at the given index.
   *
   * \param cbox Combobox to be used
   *
   * \param s The combobox index where is to set the translated surface text.
   */
  void setSurface( QComboBox* cbox, int s );

  QLineEdit* edtICAO;
  DoubleNumberEditor* edtFrequency;

  QGroupBox* gboxRunway1;
  QGroupBox* gboxRunway2;

  QCheckBox* chkRwy1Enable;
  QCheckBox* chkRwy2Enable;

  NumberEditor* edtRwy1Heading;
  NumberEditor* edtRwy2Heading;

  QCheckBox* chkRwy1Both;
  QCheckBox* chkRwy2Both;

  QCheckBox* chkRwy1Usable;
  QCheckBox* chkRwy2Usable;

  QComboBox* cmbRwy1Surface;
  QComboBox* cmbRwy2Surface;

  NumberEditor* edtRwy1Length;
  NumberEditor* edtRwy2Length;
};

#endif
