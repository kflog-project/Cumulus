/***********************************************************************
**
**   variomodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004-2009 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef VARIO_MODE_DIALOG_H
#define VARIO_MODE_DIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QGroupBox>
#include <QTimer>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

/**
 * @author Axel Pauli
 */
class VarioModeDialog : public QDialog
{
  Q_OBJECT

public:

  VarioModeDialog(QWidget *parent);

  ~VarioModeDialog();

  static int getNrOfInstances()
  {
    return noOfInstances;
  };

protected:

  void showEvent(QShowEvent *);

public slots:

  void load();

  void save();

signals:
  /**
   * This slot is called, if the integration time has
   * been changed. Passed value in seconds
   */
  void newVarioTime(int newTime);

  /**
   * This slot is called, if the TEK Mode has been changed
   */
  void newTEKMode(bool newMode);

  /**
   * This slot is called, if the TEK Mode has been changed
   */
  void newTEKAdjust(int newAdjust);

private:

  bool hildonStyle;
  QRadioButton* one;
  QRadioButton* five;
  QRadioButton* ten;

  QGroupBox*   stepGroup;
  QSpinBox*    spinTime;
  QSpinBox*    spinTEK;
  QCheckBox*   TEK;
  QLabel*      TekAdj;
  QPushButton *tekPlus;
  QPushButton *tekMinus;

  QPushButton *ok;
  QPushButton *cancel;

  QTimer* timer;
  int     _timeout;
  int     _intTime;
  int     _curWidth;
  bool    _TEKComp;
  int     _TEKAdjust;

  /** contains the current number of class instances */
  static int noOfInstances;

private slots:

  void setTimer();
  void accept();
  void change(int newStep);
  void TekChanged( bool newState );

  void slot_timePlus();
  void slot_timeMinus();

  void slot_tekPlus();
  void slot_tekMinus();
};

#endif
