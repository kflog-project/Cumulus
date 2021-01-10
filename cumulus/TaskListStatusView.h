/***********************************************************************
**
**   TaskListStatusView.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class TaskListStatusView
 *
 * \author Axel Pauli
 *
 * \brief Presents a view that shows a list of the flight task points.
 *
 * Displays the flown average speed of a task for every leg together
 * with the pass time.
 *
 * \date 2021
 *
 * \version 1.0
 */

#ifndef TaskListStatusView_h
#define TaskListStatusView_h

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QDateTime>

#include "rowdelegate.h"
#include "taskpoint.h"
#include "flighttask.h"

class MainWindow;

class TaskListStatusView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskListStatusView )

public:

  TaskListStatusView( QWidget *parent=0 );

  ~TaskListStatusView();

  /** clears all data of the list */
  void clear();

  /** sets the header of the list */
  void setHeader();

  /**
   * \return The list widget.
   */
  QTreeWidget* getListWidget()
  {
    return list;
  }

private:

  /** Resizes the columns of the task list to their contents. */
  void resizeTaskList();

public slots:

  /**
   * This slot is called if the info button has been clicked,
   * or the user pressed 'i'
   */
  void slot_Help();

  /**
   * This slot is called if the close button has been clicked.
   */
  void slot_Close ();

  /**
   * Retrieves the task points from the task, and fills the list.
   */
  void slot_setTask( FlightTask* task );

  /**
   * Updates the internal task data. Will be called after
   * configuration changes of task sector items
   */
  void slot_updateTask();

signals:

  /**
   * This signal is send if the selection is done, and the
   * screen can be closed.
   */
  void done();

protected:

  void showEvent(QShowEvent *);

private:

  RowDelegate* rowDelegate;

  QTreeWidget     *list;
  QBoxLayout      *buttonrow;

  QLabel          *distance;   // currently flown distance of task
  QLabel          *avSpeed;    // average speed
  QLabel          *flightTime; // current flight time
  QLabel          *startTime;
  QLabel          *endTime;
  FlightTask      *m_task;     // currently activated task
};

#endif
