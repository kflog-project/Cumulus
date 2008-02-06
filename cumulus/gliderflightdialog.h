/***********************************************************************
**
**   gliderflightdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GLIDERFLIGHTDIALOG_H
#define GLIDERFLIGHTDIALOG_H

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
public:
    GliderFlightDialog(QWidget *parent);
    ~GliderFlightDialog();
    void load ();

protected:
    void accept ();
    void showEvent(QShowEvent *);

private:
    void save ();

    QDoubleSpinBox* spinMcCready;
    QSpinBox* spinWater;
    QSpinBox* spinBugs;
    QPushButton* buttonDump;
    QPushButton* buttonOK;
    QPushButton* buttonCancel;
    QTimer* timeout;
    int _time;

private slots:
    void slotDump();
    void setTimer();

signals:
    void settingsChanged();
};

#endif
