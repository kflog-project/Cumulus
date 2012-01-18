/***********************************************************************
**
**   glidereditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**  $Id$
**
***********************************************************************/

/**
 * \class GliderEditor
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief This widget provides a glider editor dialog.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef GLIDER_EDITOR_H
#define GLIDER_EDITOR_H

#include <QWidget>

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QList>

#include "coordedit.h"
#include "polar.h"
#include "glider.h"

class GliderEditor : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( GliderEditor )

public:

  GliderEditor(QWidget* parent=0, Glider* glider=0);

  virtual ~GliderEditor();

  /**
   * @return The currently selected polar is returned.
   */
  Polar* getPolar();

protected:

  void showEvent( QShowEvent *event );

private:

  /**
   * Reads in the data from the Cumulus polar file.
   */
  void readPolarData ();

  /**
    * Called to initiate saving to the configuration file.
    */
  void save();

  /**
    * Called to initiate loading of the configuration file.
    */
  void load();

public slots:

  /**
    * Called when a glider type has been selected in the combo box.
    */
  void slotActivated(const QString&);

  /**
    * Called when the show button was pressed to draw the polar.
    */
  void slotButtonShow();

private slots:

  /** Called when Ok button is pressed */
  void accept();

  /** Called when Cancel button is pressed */
  void reject();

  /**
  * This slot increments the value in the spin box which has the current focus.
  */
  void slotIncrementBox();

  /**
  * This slot decrements the value in the spin box which has the current focus.
  */
  void slotDecrementBox();

signals:

  /**
    * Send if a glider has been edited.
    */
  void editedGlider(Glider*);

  /**
    * Send if a new glider has been made.
    */
  void newGlider(Glider*);

private:

  QComboBox* comboType;
  QDoubleSpinBox* spinV1;
  QDoubleSpinBox* spinW1;
  QDoubleSpinBox* spinV2;
  QDoubleSpinBox* spinW2;
  QDoubleSpinBox* spinV3;
  QDoubleSpinBox* spinW3;
  QDoubleSpinBox* spinWingArea;
  QLineEdit* edtGType;
  QLineEdit* edtGReg;
  QLineEdit* edtGCall;
  QPushButton* buttonShow;
  QSpinBox* emptyWeight;
  QSpinBox* addedLoad;
  QSpinBox* spinWater;
  QComboBox* comboSeats;

  /** Button to increase spinbox value. */
  QPushButton *plus;
  /** Button to decrease spinbox value. */
  QPushButton *minus;

  QList<Polar> _polars;
  Glider * _glider;
  Polar  * _polar;
  bool isNew;
  /** Flag to indicate if a glider object was created by this class */
  bool gliderCreated;

  /**
   * saves current horizontal/vertical speed unit during construction of object
   */
  Speed::speedUnit currHSpeedUnit;
  Speed::speedUnit currVSpeedUnit;

  /**
   * Loaded values in spin boxes.
   */
  double currV1;
  double currV2;
  double currV3;
  double currW1;
  double currW2;
  double currW3;
};

#endif
