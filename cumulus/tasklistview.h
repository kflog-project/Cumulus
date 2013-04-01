/***********************************************************************
**
**   tasklistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class TaskListView
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Presents a view that shows a list of the flight task points.
 *
 * Displays all points of a task as list with different buttons for
 * actions. Can be used in two modes, as display only, buttons for
 * actions are not visible or with command buttons.
 *
 * \date 2004-2013
 *
 * \version $Id$
 */

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

class TaskListView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskListView )

public:

  TaskListView( QWidget *parent=0, bool showButtons=true );

  ~TaskListView();

  /**
   * @return A pointer to the currently high lighted waypoint.
   */
  Waypoint *getSelectedWaypoint();

  /** clears all data of the list */
  void clear();

  /** sets the header of the list */
  void setHeader();

  /**
   * Change the visibility of the headline according to the passed flag.
   *
   * \param flag The visibility attribute of the headline.
   */
  void setHeadlineVisible( bool flag )
  {
    headline->setVisible( flag );
  };

  /**
   * \return The visibility of the headline.
   */
  bool headlineIsVisible() const
  {
    return headline->isVisible();
  };

private:

  /** Resizes the columns of the task list to their contents. */
  void resizeTaskList();

public slots:

  /**
   * This slot is called if the select button has been clicked.
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
   * This slot is called if the close button has been clicked.
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

private slots:
  /**
   * This slot is called if the user changes the selection in the
   * task list.
   */
  void slot_Selected();

signals:

  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(Waypoint*, bool);

  /**
   * This signal is send if the selection is done, and the
   * screen can be closed.
   */
  void done();

  /**
   * Emitted if the user clicks the Info button.
   */
  void info(Waypoint*);

protected:

  void showEvent(QShowEvent *);

private:

  RowDelegate* rowDelegate;

  // flag for showing buttons or not
  bool _showButtons;

  QWidget         *headline;
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
         * @param showTpIcon Flag to indicate that the turnpoint icon should be set
         *        was successful for all task legs or not.
         */
        _TaskPointItem( QTreeWidget* tpList,
                        TaskPoint* point,
                        bool wtCalcFlag,
                        bool showTpIcon );

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
