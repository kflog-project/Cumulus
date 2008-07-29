/***********************************************************************
**
**   preflightdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PREFLIGHTDIALOG_H
#define PREFLIGHTDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QLabel>

#include "tasklist.h"

class PreFlightGliderPage;
class PreFlightMiscPage;

/**
 * @short dialog for pre-flight settings
 * @author André Somers
 *
 * This dialog provides an interface to set all the pre-flight settings like
 * glidertype, co-pilot, task, amount of water taken on, etc.
 */
class PreFlightDialog : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @argument parent Pointer to parentwidget
     * @argument name Name of the page to be displayed. Current options: "taskselection".
     *                Any other string will select glider page.
     */
    PreFlightDialog(QWidget *parent, const char* name);

    /**
     * Destructor
     */
    ~PreFlightDialog();

signals:
    /**
     * Not documented
     */
    void settingsChanged();
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint *, bool);
    /**
     * This signal is emitted when the "dialog" should close. CumulusApp will subsequently
     * delete it
     */
    void closeConfig();


protected slots:
    /**
     * Called if dialog is accepted (OK button is clicked)
     */
//    virtual void accept();
    void accept();

    /**
     * Called if dialog is rejected (X button is clicked)
     */
//    virtual void reject();
    void reject();

private slots:
    void keyLeft();
    void keyRight();

protected:

    virtual void resizeEvent(QResizeEvent*);

private:

    TaskList *taskpage;
    PreFlightGliderPage *gliderpage;
    PreFlightMiscPage *miscpage;
    QTabWidget* tabWidget;
    QLabel* title;
};

#endif
