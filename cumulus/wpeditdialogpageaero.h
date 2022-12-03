/***********************************************************************
**
**   wpeditdialogpageaero.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers,
**                   2008-2022 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class WpEditDialogPageAero
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This is the aeronautical page for the waypoint editor dialog
 *
 * \date 2002-2022
 */

#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QLineEdit>
#include <QList>
#include <QComboBox>
#include <QCheckBox>
#include <QSignalMapper>

#include "Frequency.h"
#include "waypoint.h"

class DoubleNumberEditor;
class NumberEditor;

class WpEditDialogPageAero : public QWidget
{
  Q_OBJECT

 public:

  WpEditDialogPageAero(QWidget *parent=0);

  virtual ~WpEditDialogPageAero();

  /**
   * Checks all runway designators and only flags for correctness.
   *
   * @return true in case of ok otherwise false.
   */
  bool checkRunways();

 private:

  /**
   * Check the runway designator for 01...36 [ L, C, R ].
   *
   * Return true in case of success otherwise false.
   */
  bool checkRunwayDesignator( QString id );

  /**
   * Report a runway designator error.
   *
   * @param rwyNo Runway number 1...4
   */
  void reportRwyIdError( short rwyNo );

  /**
   * Report a runway takeoff or landing only error.
   *
   * @param rwyNo Runway number 1...4
   */
  void reportRwyOnlyError( short rwyNo );


 public slots:

  /**
   * Called if the data needs to be saved.
   */
  void slot_save(Waypoint* wp);

  /**
   * Called if the widget needs to load the data from the waypoint.
   */
  void slot_load(Waypoint* wp);

 signals:

  /**
   * Signal for signal mapper
   *
   * \param idx Index 0...3
   */
  void stateChangedRwyEnable( int idx );

  /**
   * Signal for signal mapper
   *
   * \param idx Index 0...3
   */
  void rwyHeadingEditingFinished( int idx );

 private slots:

 /**
  * Called, if runway enable checkbox is selected/unselected.
  *
  * \param idx the index of the check array
  */
 void slot_stateChangedRwyEnable( int idx )
   {
     bool state = chkRwyEnable[idx]->isChecked();
     gboxRunway[idx]->setVisible( state == 0 ? false : true );
   }

 /**
  * Called, if the runway heading editor is closed by the user.
  *
  * \param idx The index of the runway heading editor array
  */
 void slot_rwyHeadingEdited( int idx );

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
  QLineEdit* edtFrequencyType;

  // Frequency list of edited waypoint
  QList<Frequency> fqList;

  // Index of edited frequency record
  int edtFequencyListIndex;

  QCheckBox* chkRwyEnable[4];
  QGroupBox* gboxRunway[4];
  QLineEdit* edtRwyHeading[4];
  QCheckBox* chkRwyBoth[4];
  QCheckBox* chkRwyUsable[4];
  QCheckBox* chkRwyMain[4];
  QCheckBox* chkRwyTakeoffOnly[4];
  QCheckBox* chkRwyLandingOnly[4];
  QComboBox* cmbRwySurface[4];
  NumberEditor* edtRwyLength[4];
  NumberEditor* edtRwyWidth[4];

  QSignalMapper *signalMapperEnable;
  QSignalMapper *signalMapperHeading;
};
