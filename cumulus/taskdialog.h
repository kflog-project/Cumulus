/***********************************************************************
**
**   taskdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASK_H
#define TASK_H

#include <QList>
#include <QString>
#include <Q3ListView>
#include <QLineEdit>
#include <QDialog>
#include <QStringList>
#include <QComboBox>

#include "flighttask.h"
#include "waypoint.h"
#include "listviewfilter.h"

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    /** */
  TaskDialog( QWidget* parent, const char* name, QStringList &taskNamesInUse,
              FlightTask* task=0 );

    /** */
    ~TaskDialog();

protected:
    /** */
    virtual void accept();
    virtual void reject();

signals:
    /** */
    void newTask( FlightTask* );

    /** */
    void editedTask( FlightTask* );

private slots:
    /** */
    void slotAddWaypoint();

    /** */
    void slotRemoveWaypoint();

    /** */
    void slotMoveWaypointUp();

    /** */
    void slotMoveWaypointDown();

    /** */
    void slotInvertWaypoints();

    /** Toggle between WP or AF list on user request */
    void slotToggleList(int);

private:

    /** describes the current edit state of object */
    enum EditState { create, edit };

    /** */
    void __showWPList();

    /** */
    void __showAFList();

    /** */
    void __showTask();

    /** create a deep copy of the waypoint list */
    QList<wayPoint*> *copyWpList();

    /** */
    Q3ListView* taskList;
    /** */
    QStringList& taskNamesInUse;
    /** number lists, at the moment waypointlist and airfield list*/
#define NUM_LISTS 2
    /** */
    Q3ListView* waypointList[NUM_LISTS];
    /** */
    QLineEdit* taskName;
    /** */
    QString editedTaskName;
    /** */
    ListViewFilter* filter[NUM_LISTS];
    /** */
    QList<wayPoint*> *wpList;
    /** */
    QList<wayPoint*> taskWPList;
    /** */
    FlightTask* planTask;
    /** Flag for indication of edit state */
    enum  EditState editState;
    /** Flag for indication of last selected item */
    int lastSelectedItem;
    /** which list to show */
    QComboBox * listSelectCB;
    /** the text for the toggle button */
    QString listSelectText[NUM_LISTS];
};

#endif // TASK_H
