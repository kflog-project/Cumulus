/***********************************************************************
**
**   taskeditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2016 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class TaskEditor
 *
 * \author Heiner Lampbrecht, Axel Pauli
 *
 * \brief Flight Task Editor
 *
 * This class handles creation and modification of flight tasks in a
 * simple editor. The editor is realized as an own modal window.
 *
 * \date 2002-2018
 *
 * \version 1.4
 */

#ifndef TaskEditor_H
#define TaskEditor_H

#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QWidget>

#include "flighttask.h"
#include "listviewfilter.h"
#include "listwidgetparent.h"
#include "singlepoint.h"
#include "taskpoint.h"
#include "waypoint.h"
#include "TaskPointSelectionList.h"

class TaskEditor : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskEditor )

public:

  /** Constructor */
  TaskEditor( QWidget* parent, QStringList &taskNamesInUse, FlightTask* task=0 );

  /** Destructor */
  virtual ~TaskEditor();

  /**
   * Sets the default task point figure schemas. That must be done after an
   * insert, move and remove action on the task point list.
   *
   * @param tpList A list containing the task points.
   *
   * @param setDefaultFigure Assign the default figure to the task point
   *                         if set to true.
   */
  static void setTaskPointFigureSchemas( QList<TaskPoint *>& tpList,
					 const bool setDefaultFigure );

signals:

  /** Signals that a new flight task is ready for take over. */
  void newTask( FlightTask* );

  /** Signals that an edited flight task is ready for take over. */
  void editedTask( FlightTask* );

 private slots:

  void openAfSelectionList();

  /** Handles the addition of a taskpoint to the list. */
  void slotAddTaskpoint( SinglePoint* sp );

  /** Handles the remove of a taskpoint from the list. */
  void slotRemoveTaskpoint();

  /** Handles moving up of a taskpoint in the list. */
  void slotMoveTaskpointUp();

  /** Handles moving down of a taskpoint in the list. */
  void slotMoveTaskpointDown();

  /** Creates the list in reverse order. */
  void slotInvertTaskpoints();

  /** Edit taskpoint (define sector etc) */
  void slotEditTaskPoint ();

  /** Called, if a task point has been edited. */
  void slotTaskPointEdited( TaskPoint* editedTaskPoint );

  /** Called to reset all task points to their task figure default schema. */
  void slotSetTaskPointsDefaultSchema();

  /** Toggle between WP or AF list on user request */
  void slotToggleList( int );

  /**
   * Called to check the item selection. If item Total is called, the selection
   * is reset to the previous row.
   */
  void slotCurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);

  /**
   * Called, if an edited waypoint is saved.
   */
  void slotWpEdited( Waypoint &editedWp );

  /** Handles button press ok. */
  void slotAccept();

  /** Handles button press cancel. */
  void slotReject();

 private:

   /** describes the current edit state of the object */
   enum EditState
   {
     create, edit
   };

   /**
    * aligns the task list columns to their contents
    */
   void resizeTaskListColumns();

   /**
    * Enables/disables the command buttons of the editor in dependency
    * of contained task points.
    */
   void enableCommandButtons();

  /** shows the updated task */
  void showTask();

  /** creates a deep copy of the waypoint list */
  QList<Waypoint*> *copyWpList();

  /**
   * Swap the task point schema data between the two task points.
   */
  void swapTaskPointSchemas( TaskPoint* tp1, TaskPoint* tp2 );

  //----------------- data items -----------------------------------------------

  /** list containing defined tasks */
  QTreeWidget* taskList;

  /** list with all defined task names */
  QStringList& taskNamesInUse;

  /** selection lists with point data */
  QList<ListWidgetParent *> pointDataList;

  /** name of current task */
  QLineEdit* taskName;

  /** name of current edited task */
  QString editedTaskName;

  /** */
  QList<ListViewFilter *> filter;

  /** Task point list of flight task */
  QList<TaskPoint *> tpList;

  /** Flight task to be edited */
  FlightTask* task2Edit;

  /** Flag for indication of edit state */
  enum EditState editState;

  /** Flag for indication of last selected item */
  int lastSelectedItem;

  /** Editor command buttons. */
  QPushButton* upButton;
  QPushButton* downButton;
  QPushButton* invertButton;
  QPushButton* addButton;
  QPushButton* delButton;
  QPushButton* editButton;
  QPushButton* defaultButton;

  TaskPointSelectionList* afSelectionList;

  /** The list index of the last edited taskpoint. */
  int m_lastEditedTP;
};

#endif // TaskEditor_H
