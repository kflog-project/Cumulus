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

#include <QTabWidget>

#include "tasklist.h"

class PreFlightGliderPage;
class PreFlightMiscPage;

/**
 * @short widget for pre-flight settings
 * @author André Somers
 *
 * This widget provides an interface to set all the pre-flight settings like
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
    void slot_accept();

    /**
     * Called if dialog is rejected (X button is clicked)
     */
    void slot_reject();

private slots:

    // shortcuts for switching between tabulators
    void slot_keyLeft();
    void slot_keyRight();

private:

    TaskList *taskpage;
    PreFlightGliderPage *gliderpage;
    PreFlightMiscPage *miscpage;
    QTabWidget* tabWidget;
};

#endif
