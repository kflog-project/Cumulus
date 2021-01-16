/***********************************************************************
**
**   VarioModeDialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004-2021 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2004-2021
 *
 * \version 1.2
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

  virtual void showEvent(QShowEvent *);

  virtual void closeEvent( QCloseEvent *event );

private slots:

  /** This method starts a timer which closes the dialog automatically after
   *  the timer has expired.
   */
  void slot_setTimer();
  void slot_accept();
  void slot_reject();

  /**
   * This method changes the value in the spin box which has the current focus.
   *
   * @param newStep value to be set in spin box
   */
  void slot_change( int newStep );

  /**
   * This method switches on/off the tek adjustment of the variometer.
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
   * This signal is emitted, if the tek Mode has been changed.
   *
   * @param newMode switches on/off the tek adjustment
   */
  void newTEKMode( bool newMode );

  /**
   * This signal is emitted, if the tek Mode has been changed.
   *
   * @param newAdjust new adjust value in percent
   */
  void newTEKAdjust(int newAdjust);

  /**
   * This signal is emitted, when the dialog is closed
   */
  void closingWidget();

private:

  /** Loads the permanent widget data. */
  void load();

  /** Saves the permanent widget data. */
  void save();

  QSpinBox*    spinTime;
  QSpinBox*    spinTEK;
  QCheckBox*   tek;
  QLabel*      TekAdj;

  QPushButton *plus;
  QPushButton *pplus;
  QPushButton *minus;
  QPushButton *mminus;

  QPushButton *ok;
  QPushButton *cancel;

  QTimer* timer;
  int     m_timeout;
  int     m_intTime;
  bool    m_TEKComp;
  int     m_TEKAdjust;

  /** Auto sip flag storage. */
  bool m_autoSip;

  /** contains the current number of class instances */
  static int noOfInstances;
};

#endif
