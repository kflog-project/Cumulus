/***********************************************************************
**
**   variomodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004-2010 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class VarioModeDialog
 *
 * \author Axel Pauli
 *
 * \brief Dialog for variometer user interaction.
 *
 * This dialog is the user interface for the variometer settings.
 *
 * \date 2004-2010
 *
 */

#ifndef VARIO_MODE_DIALOG_H
#define VARIO_MODE_DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

class VarioModeDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( VarioModeDialog )

public:

  VarioModeDialog(QWidget *parent);

  virtual ~VarioModeDialog();

  /**
   * @return Returns the current number of instances.
   */
  static int getNrOfInstances()
  {
    return noOfInstances;
  };

protected:

  void showEvent(QShowEvent *);

private slots:

  /** This method starts a timer which closes the dialog automatically after
   *  the timer has expired.
   */
  void slot_setTimer();
  void slot_accept();

  /**
   * This method changes the value in the spin box which has the current focus.
   *
   * @param newStep value to be set in spin box
   */
  void slot_change( int newStep );

  /**
   * This method switches on/off the TEK adjustment of the variometer.
   *
   * @param newState activity state of TEL adjustment.
   */
  void slot_tekChanged( bool newState );

  /** Increments spin box value according to set step width. */
  void slot_timePlus();

  /** Decrements spin box value according to set step width. */
  void slot_timeMinus();

  /** Increments spin box value according to set step width. */
  void slot_tekPlus();

  /** Decrements spin box value according to set step width. */
  void slot_tekMinus();

signals:
  /**
   * This signal is emitted, if the integration time has
   * been changed. Passed value unit is seconds.
   *
   * @param newTime new time value in seconds
   */
  void newVarioTime(int newTime);

  /**
   * This signal is emitted, if the TEK Mode has been changed.
   *
   * @param newMode switches on/off the TEK adjustment
   */
  void newTEKMode( bool newMode );

  /**
   * This signal is emitted, if the TEK Mode has been changed.
   *
   * @param newAdjust new adjust value in percent
   */
  void newTEKAdjust(int newAdjust);

private:

  /** Loads the permanent widget data. */
  void load();

  /** Saves the permanent widget data. */
  void save();

  QSpinBox*    spinTime;
  QSpinBox*    spinTEK;
  QCheckBox*   TEK;
  QLabel*      TekAdj;

  QPushButton *plus;
  QPushButton *pplus;
  QPushButton *minus;
  QPushButton *mminus;

  QPushButton *ok;
  QPushButton *cancel;

  QTimer* timer;
  int     _timeout;
  int     _intTime;
  bool    _TEKComp;
  int     _TEKAdjust;

  /** contains the current number of class instances */
  static int noOfInstances;
};

#endif
