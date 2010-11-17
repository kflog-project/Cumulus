/***********************************************************************
**
**   taskeditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Heiner Lamprecht
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Heiner Lampbrecht, Axel Pauli
 *
 * \brief Flight Task Editor
 *
 * This class handles creation and modification of flight tasks in a
 * simple editor. The editor is realized as an own modal window.
 *
 */

#ifndef TaskEditor_H
#define TaskEditor_H

#include <QList>
#include <QString>
#include <QTreeWidget>
#include <QLineEdit>
#include <QWidget>
#include <QStringList>
#include <QComboBox>

#include "flighttask.h"
#include "waypoint.h"
#include "listviewfilter.h"
#include "listwidgetparent.h"

class TaskEditor : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskEditor )

public:

  /** Constructor */
  TaskEditor( QWidget* parent, QStringList &taskNamesInUse,
              FlightTask* task=0 );

  /** Destructor */
  virtual ~TaskEditor();

private:
  /**
   * aligns the task list columns to their contents
   */
  void resizeTaskListColumns();

signals:

  /** Signals that a new flight task is ready for take over. */
  void newTask( FlightTask* );

  /** Signals that an edited flight task is ready for take over. */
  void editedTask( FlightTask* );

 private slots:

  /** Handles the addition of a waypoint to the list. */
  void slotAddWaypoint();

  /** Handles the remove of a waypoint from the list. */
  void slotRemoveWaypoint();

  /** Handles moving up of a waypoint in the list. */
  void slotMoveWaypointUp();

  /** Handles moving down of a waypoint in the list. */
  void slotMoveWaypointDown();

  /** Creates the list in reverse order. */
  void slotInvertWaypoints();

  /** Toggle between WP or AF list on user request */
  void slotToggleList( int );

  /** Handles button press ok. */
  void slotAccept();

  /** Handles button press cancel. */
  void slotReject();

 private:

  /** describes the current edit state of object */
  enum EditState
  {
    create, edit
  };

  /** */
  void __showWPList();

  /** */
  void __showAFList();

  /** */
  void __showTask();

  /** creates a deep copy of the waypoint list */
  QList<wayPoint*> *copyWpList();

  /** list containing defined tasks */
  QTreeWidget* taskList;

  /** list with all defined task names */
  QStringList& taskNamesInUse;

  /** number of lists, at the moment waypoint, airfield and outlanding list */
#define NUM_LISTS 3

  /** selection lists with waypoints */
  ListWidgetParent* waypointList[NUM_LISTS];

  /** name of current task */
  QLineEdit* taskName;

  /** name of current edited task */
  QString editedTaskName;

  /** */
  ListViewFilter* filter[NUM_LISTS];

  /** Task point list of flight task */
  QList<TaskPoint *> tpList;

  /** Flight task to be edited */
  FlightTask* planTask;

  /** Flag for indication of edit state */
  enum EditState editState;

  /** Flag for indication of last selected item */
  int lastSelectedItem;

  /** which list to show */
  QComboBox* listSelectCB;

  /** The text for the combo box */
  QString listSelectText[NUM_LISTS];
};

#endif // TaskEditor_H
