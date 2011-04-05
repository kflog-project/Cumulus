/***********************************************************************
**
**   preflighttasklist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightTaskList
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief A widget for pre-flight task settings.
 *
 * \date 2002-2011
 *
 */

#ifndef PRE_FLIGHT_TASK_LIST_H
#define PRE_FLIGHT_TASK_LIST_H

#include <QList>
#include <QTreeWidget>
#include <QWidget>
#include <QStringList>
#include <QSpinBox>
#include <QCheckBox>
#include <QSplitter>

#include "flighttask.h"
#include "tasklistview.h"

class PreFlightTaskList : public QWidget
{
  Q_OBJECT

private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PreFlightTaskList )

public:

  /** */
  PreFlightTaskList( QWidget* parent );

  /** */
  ~PreFlightTaskList();

  /** Takes out the selected task from the task list. */
  FlightTask* takeSelectedTask();

protected:

  void showEvent(QShowEvent *);

private:

  /** Save task list */
  bool saveTaskList();

  /** Select the last stored task */
  void selectLastTask();

  /** load tasks from the file */
  bool loadTaskList();

  /** Creates a task definition file in Flarm format. */
  bool createFlarmTaskList( FlightTask* flightTask );

private slots:
  /** show the details of a task */
  void slotTaskDetails();

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

  /** value in TAS spin box has been changed, do update of task list. */
  void slotTasChanged( int value );

  /** value in wind direction spin box has been changed, do update of task list. */
  void slotWDChanged( int value );

  /** value in wind speed spin box has been changed, do update of task list. */
  void slotWSChanged( int value );

private:

  /** splitter widget */
  QSplitter* splitter;
  /** spin box for TAS entry */
  QSpinBox* tas;
  /** spin box for wind direction entry*/
  QSpinBox* windDirection;
  /** spin box for wind speed entry */
  QSpinBox* windSpeed;
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

#endif
