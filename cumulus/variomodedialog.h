/***********************************************************************
**
**   variomodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004, 2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef VARIOMODEDIALOG_H
#define VARIOMODEDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QGroupBox>
#include <QTimer>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>

/**
 * @author Axel Pauli
 */
class VarioModeDialog : public QDialog
  {
    Q_OBJECT
  public:

    VarioModeDialog(QWidget *parent);
    ~VarioModeDialog();

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
     * This slot is called, if thei TEK Mode has been changed
     */
    void newTEKMode(bool newMode);

    /**
     * This slot is called, if thei TEK Mode has been changed
     */
    void newTEKAdjust(int newAdjust);


  private:
    QRadioButton* one;
    QRadioButton* five;
    QRadioButton* ten;

    QGroupBox*   stepGroup;
    QSpinBox*    spinTime;
    QSpinBox*    spinTEK;
    QCheckBox*   TEK;
    QLabel*      TekAdj;

    QTimer* timer;
    int     _timeout;
    int     _intTime;
    int     _curWidth;
    bool    _TEKComp;
    int     _TEKAdjust;

  private slots:
    void setTimer();
    void accept();
    void change(int newStep);
    void TekChanged( bool newState );
  };

#endif
