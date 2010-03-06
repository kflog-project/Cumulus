/***********************************************************************
**
**   gliderflightdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GLIDER_FLIGHT_DIALOG_H
#define GLIDER_FLIGHT_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QTimer>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "speed.h"

/**
 * @author Eggert Ehmke
 */

class GliderFlightDialog : public QDialog
{
    Q_OBJECT

private:

  Q_DISABLE_COPY ( GliderFlightDialog )

public:

    GliderFlightDialog(QWidget *parent);
    ~GliderFlightDialog();
    void load();

    static int getNrOfInstances()
    {
      return noOfInstances;
    };

protected:

    void accept();
    void showEvent(QShowEvent *);

private:

    void save();

    bool hildonStyle;
    QDoubleSpinBox* spinMcCready;
    QSpinBox* spinWater;
    QSpinBox* spinBugs;
    QPushButton* buttonDump;
    QTimer* timeout;
    int _time;

    QPushButton* mcPlus;
    QPushButton* mcMinus;

    QPushButton* waterPlus;
    QPushButton* waterMinus;

    QPushButton* bugsPlus;
    QPushButton* bugsMinus;

    QPushButton *ok;
    QPushButton *cancel;

    /** contains the current number of class instances */
    static int noOfInstances;

private slots:

    void slotDump();
    void setTimer();

    void slotMcPlus();
    void slotMcMinus();

    void slotWaterPlus();
    void slotWaterMinus();

    void slotBugsPlus();
    void slotBugsMinus();

signals:

    void settingsChanged();
};

#endif
