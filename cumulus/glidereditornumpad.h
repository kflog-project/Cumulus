/***********************************************************************
**
**   glidereditornumpad.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**  $Id$
**
***********************************************************************/

/**
 * \class GliderEditorNumPad
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief This widget provides a glider editor dialog.
 *
 * \date 2002-2014
 *
 * \version $Id$
 */

#ifndef GLIDER_EDITOR_NUMPAD_H
#define GLIDER_EDITOR_NUMPAD_H

#include <QWidget>

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QList>

#include "polar.h"
#include "glider.h"

class DoubleNumberEditor;
class NumberEditor;

class GliderEditorNumPad : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( GliderEditorNumPad )

 public:

  GliderEditorNumPad(QWidget* parent=0, Glider* glider=0);

  virtual ~GliderEditorNumPad();

  /**
   * @return The currently selected polar is returned.
   */
  Polar* getPolar();

 protected:

  void showEvent( QShowEvent *event );

 private:

#if 0
  /**
   * Reads in the data from the Cumulus polar file.
   */
  void readPolarData ();
#endif

  /**
   * Reads in the LK8000 polar data files.
   */
  void readLK8000PolarData();

  /**
   * Reads the data from a single LK8000 polar file.
   *
   * \param fileName Name of the polar file
   *
   * \param polar Polar instance to be filled
   *
   * \return true in case of success otherwise false
   */
  bool readLK8000PolarFile( const QString& fileName, Polar& polar );

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

  /** Called, when the seat button is pressed. */
  void slot_changeSeats();

  /** Called when Ok button is pressed */
  void accept();

  /** Called when Cancel button is pressed */
  void reject();

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

  DoubleNumberEditor* m_dneV1;
  DoubleNumberEditor* m_dneW1;
  DoubleNumberEditor* m_dneV2;
  DoubleNumberEditor* m_dneW2;
  DoubleNumberEditor* m_dneV3;
  DoubleNumberEditor* m_dneW3;
  DoubleNumberEditor* m_dneWingArea;

  QLineEdit* edtGType;
  QLineEdit* edtGReg;
  QLineEdit* edtGCall;

  QPushButton*  buttonShow;
  NumberEditor* grossWeight;
  NumberEditor* addedLoad;
  NumberEditor* addedWater;
  QPushButton*  m_seats;

  QList<Polar> m_polars;
  Glider*      m_glider;
  Polar*       m_polar;
  bool         m_isNew;

  /** Flag to indicate if a glider object was created by this class */
  bool m_gliderCreated;

  /**
   * saves current horizontal/vertical speed unit during construction of object
   */
  Speed::speedUnit m_currHSpeedUnit;
  Speed::speedUnit m_currVSpeedUnit;

  /**
   * Loaded speed values.
   */
  double m_currV1;
  double m_currV2;
  double m_currV3;
  double m_currW1;
  double m_currW2;
  double m_currW3;
};

#endif
