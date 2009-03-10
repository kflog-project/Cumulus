/***********************************************************************
**
**   preflightwidget.h
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

#ifndef _PreFlightWidget_h
#define _PreFlightWidget_h

#include <QTabWidget>

#include "tasklist.h"

class PreFlightGliderPage;
class PreFlightMiscPage;

/**
 * @short widget for pre-flight settings
 * @author André Somers
 *
 * This widget provides an interface to set all the pre-flight settings like
 * glider type, copilot, task, amount of water taken on, etc.
 */
class PreFlightWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @argument parent Pointer to parent widget
     * @argument name Name of the page to be displayed. Current options: "taskselection".
     *                Any other string will select glider page.
     */
    PreFlightWidget(QWidget *parent, const char* name);

    /**
     * Destructor
     */
    ~PreFlightWidget();

signals:
    /**
     * This signal is emitted if the settings are changed
     */
    void settingsChanged();
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint *, bool);
    /**
     * This signal is emitted before the widget is closed.
     * MainWindow will use it to update the current view setting.
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
