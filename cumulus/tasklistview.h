/***********************************************************************
**
**   tasklistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASK_LIST_VIEW_H
#define TASK_LIST_VIEW_H

#include <QWidget>
#include <QTreeWidget>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>

#include "rowdelegate.h"
#include "taskpoint.h"
#include "flighttask.h"

class MainWindow;

/**
 * \author André Somers, Axel Pauli
 *
 * \brief Presents a view that holds a list of the task points of a flight task.
 *
 * Displays all points of a task as list with different buttons for
 * actions. Can be used in two modes, as display only, buttons for
 * actions are not visible or with command buttons.
 *
 */

class TaskListView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskListView )

public:

  TaskListView( QWidget *parent=0, bool showButtons=true );

  ~TaskListView();

  /**
   * @Returns a pointer to the currently high lighted waypoint.
   */
  wayPoint *getSelectedWaypoint();

  /** clears all data of the list */
  void clear();

  /** sets the header of the list */
  void setHeader();

private:

  /** Resizes the columns of the task list to their contents. */
  void resizeTaskList();

public slots:

  /**
   * This signal is called to indicate that a selection has been made.
   */
  void slot_Select();

  /**
   * This slot is called if the start button has been clicked.
   */
  void slot_Start();

  /**
   * This slot is called if the info button has been clicked,
   * or the user pressed 'i'
   */
  void slot_Info();

  /**
   * Called when the listview should be closed without selection
   */
  void slot_Close ();

  /**
   * Retrieves the task points from the task, and fills the list.
   */
  void slot_setTask(const FlightTask *);

  /**
   * Updates the internal task data. Will be called after
   * configuration changes of task sector items
   */
  void slot_updateTask();

signals:

  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(wayPoint*, bool);

  /**
   * This signal is send if the selection is done, and the
   * screen can be closed.
   */
  void done();

  /**
   * Emitted if the user clicks the Info button.
   */
  void info(wayPoint*);

protected:

  void showEvent(QShowEvent *);

private:

  QTreeWidget     *list;
  MainWindow      *par;
  QBoxLayout      *buttonrow;
  bool            _outlandShow;
  QPushButton     *cmdShowOl;
  QPushButton     *cmdHideOl;
  QPushButton     *cmdSelect;
  QLabel          *wind;
  QLabel          *distTotal;
  QLabel          *speedTotal;
  QLabel          *timeTotal;
  QPixmap         _arrows;
  FlightTask      *_task;
  TaskPoint       *_selectedTp;
  QTreeWidgetItem * _currSelectedTp;
  QTreeWidgetItem * _newSelectedTp;
  QString         _selectText;
  QString         _unselectText;

  // flag for showing buttons or not
  bool _showButtons;

  RowDelegate* rowDelegate;

private slots:
  /**
   * This slot is called if the user selects a task point in the task
   */
  void slot_Selected();

private:

  /**
   * Extension of QTreeWidgetItem to store additional information of a single
   * task point.
   */
  class _TaskPointItem : public QTreeWidgetItem
    {
      public:

        /**
         * This class constructor sets all data of a QTreeWidgetItem.
         *
         * @param tpList Parent of QTreeWidget.
         * @param point Data of task point to be set.
         * @param wtCalcFlag Flag to indicate if wind triangle calculation
         *        was successful for all task legs or not.
         */
        _TaskPointItem( QTreeWidget* tpList, TaskPoint* point, bool wtCalcFlag );

        /** Returns the task point of this item. */
        TaskPoint *getTaskPoint() const
        {
          return tp;
        };

      private:

        /** Passed task point instance in constructor. */
        TaskPoint *tp;
    };
};

#endif
