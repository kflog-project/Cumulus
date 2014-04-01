/***********************************************************************
**
**   gliderflightdialog.h
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
**   $Id$
**
***********************************************************************/

/**
 * \class GliderFlightDialog
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief This dialog is the user interface for the in flight settings.
 *
 * This dialog handles the Mc, load balance and bug settings. It shall
 * enable a simple change also during flight.
 *
 * \date 2002-2014
 *
 * \version $Id$
 */

#ifndef GLIDER_FLIGHT_DIALOG_H
#define GLIDER_FLIGHT_DIALOG_H

#include <QDialog>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "speed.h"

class GliderFlightDialog : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( GliderFlightDialog )

 public:

  GliderFlightDialog(QWidget *parent);

  virtual ~GliderFlightDialog();

  /**
   * @return Returns the current number of instances.
   */
  static int getNrOfInstances()
  {
    return m_noOfInstances;
  };

 protected:

  virtual void showEvent(QShowEvent *);

  virtual bool eventFilter( QObject *o , QEvent *e );

 private slots:

  /**
  * This method changes the value in the spin box which has the current focus.
  *
  * @param newStep value to be set in spin box
  */
  void slotChange( int newStep );

  /**
   * This slot is called if the user has pressed the dump button.
   */
  void slotDump();

  /** Increments spin box value according to set step width. */
  void slotMcPlus();

  /** Decrements spin box value according to set step width. */
  void slotMcMinus();

  /** Increments spin box value according to set step width. */
  void slotWaterPlus();

  /** Decrements spin box value according to set step width. */
  void slotWaterMinus();

  /** Increments spin box value according to set step width. */
  void slotBugsPlus();

  /** Decrements spin box value according to set step width. */
  void slotBugsMinus();

  /**
   * This slot is called if a value in a spin box has been changed
   * to restart the close timer.
   */
  void slotSpinValueChanged( const QString& text );

  /** Shows the flight time. */
  void slotShowFlightTime();

  /**
   * This slot is called, if the user has pressed the ok button.
   */
  void slotAccept();

  /**
   * This slot is called, if the user has pressed the cancel button.
   */
  void slotReject();

 signals:

  /**
   * This signal is emitted, if water or bugs have been changed.
   */
  void newWaterAndBugs( const int water, const int bugs );

  /**
   * This signal is emitted, if the Mc value has been changed.
   */
  void newMc( const Speed& mc );

  /**
   * This signal is emitted, when the dialog is closed
   */
  void closingWidget();

 private:

  /** Loads the permanent widget data. */
  void load();

  /** Saves the permanent widget data. */
  void save();

  /**
   * This method starts a timer which closes the dialog automatically after
   * the timer has expired.
   */
  void startTimer();

  QDoubleSpinBox* spinMcCready;
  double m_mcSmallStep;
  double m_mcBigStep;
  QSpinBox* spinWater;
  QSpinBox* spinBugs;
  QPushButton* buttonDump;
  QTimer* timer;
  int m_time;

  /** Auto sip flag storage. */
  bool m_autoSip;

  /** MC configuration value */
  double m_mcConfig;

  /** Water configuration value */
  int m_waterConfig;

  /** Bugs configuration value */
  int m_bugsConfig;

  // Flight time display
  QLabel* ftLabel;
  QLabel* ftText;

  QPushButton *plus;
  QPushButton *pplus;
  QPushButton *minus;
  QPushButton *mminus;
  QPushButton *reset;

  QPushButton *ok;
  QPushButton *cancel;

  /** contains the current number of class instances */
  static int m_noOfInstances;
};

#endif
