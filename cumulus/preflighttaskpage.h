/***********************************************************************
**
**   preflighttaskpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreFlightTaskPage
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief A widget for pre-flight task settings.
 *
 * \date 2002-2015
 *
 * \version 1.3
 *
 */

#ifndef PRE_FLIGHT_TASK_PAGE_H
#define PRE_FLIGHT_TASK_PAGE_H

#include <QList>
#include <QTreeWidget>
#include <QWidget>
#include <QStringList>
#include <QSpinBox>
#include <QCheckBox>

#include "flighttask.h"
#include "tasklistview.h"

class NumberEditor;

class PreFlightTaskPage : public QWidget
{
  Q_OBJECT

 private:

  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PreFlightTaskPage )

 public:

  PreFlightTaskPage( QWidget* parent );

  virtual ~PreFlightTaskPage();

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

  /**
   * This method is called to update the the way time in the task overview list,
   * if wind or tas have been changed by the user.
   */
  void updateWayTime();

 signals:

   /**
    * Emitted, if settings have been changed.
    */
   void newTaskSelected();

   /**
    * Emitted, if a new waypoint was selected.
    *
    * @param newWaypoint new selected waypoint.
    */
   void newWaypoint( Waypoint* newWaypoint, bool userAction );

   /**
    * Emitted, if the wind parameters have been changed.
    */
   void manualWindStateChange( bool newEnableState );

   /**
    * Emitted, if the widget is closed.
    */
   void closingWidget();

 private slots:

  /** Called, if TAS or wind have been updated. */
  void slotNumberEdited( const QString& number );

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

  /**
   * Makes the task list widget visible.
   */
  void slotShowTaskListWidget();

  /**
   * Makes the task view widget visible.
   */
  void slotShowTaskViewWidget();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

 private:

  /** task list widget */
  QWidget *m_taskListWidget;
  /** task view widget */
  QWidget *m_taskViewWidget;

  /** editor box for TAS entry */
  NumberEditor* m_tas;
  /** editor box for wind direction entry*/
  NumberEditor* m_windDirection;
  /** editor box for wind speed entry */
  NumberEditor* m_windSpeed;
  /** task list overview */
  QTreeWidget* m_taskList;
  /** widget with task content in detail */
  TaskListView* m_taskContent;
  /** list with all defined flight tasks */
  QList<FlightTask*> m_flightTaskList;
  /** flight task being edited */
  FlightTask* m_editTask;
  /** names of flight tasks */
  QStringList m_taskNames;
};

#endif
