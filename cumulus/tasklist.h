/***********************************************************************
**
**   tasklist.h
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

#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <QList>
#include <QTreeWidget>
#include <QWidget>
#include <QStringList>
#include <QSpinBox>
#include <QCheckBox>
#include <QSplitter>

#include "flighttask.h"
#include "tasklistview.h"

class TaskList : public QWidget
{
    Q_OBJECT

public:
    /** */
    TaskList( QWidget* parent );

    /** */
    ~TaskList();

    /** Takes out the selected task from the task list. */
    FlightTask* takeSelectedTask();

  protected:
    void showEvent(QShowEvent *);

private:
    /** Save task list */
    bool saveTaskList();

    /** Select the last stored task */
    void selectLastTask();

private slots:
    /** show the details of a task */
    void slotTaskDetails();

    /** load tasks from the file */
    bool slotLoadTask();

    /** create a new task */
    void slotNewTask();

    /** edit an existing task */
    void slotEditTask();

    /** remove a task */
    void slotDeleteTask();

    /** overtake a new task item from the editor */
    void slotUpdateTaskList( FlightTask* );

    /** overtake a edited task item from the editor */
    void slotEditTaskList( FlightTask* );

    /** set new value in cruising spin box */
    void slotCruisingSpeedChanged( int value );

private:

    /** splitter widget */
    QSplitter* splitter;
    /** spin box for cruising speed entry */
    QSpinBox* cruisingSpeed;
    /** task list overview */
    QTreeWidget* taskListWidget;
    /** widget with task content in detail */
    TaskListView* taskContent;
    /** list with all defined flight tasks */
    QList<FlightTask*> taskList;
    /** flight task being edited */
    FlightTask* editTask;
    /** names of flight tasks */
    QStringList taskNames;
};

#endif // TASK_LIST_H
