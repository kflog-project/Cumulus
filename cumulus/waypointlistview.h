/***********************************************************************
**
**   waypointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2002      by André Somers
**                  2007-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WaypointListView
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This widget provides a list of waypoints and a means to select one.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef WAYPOINT_LISTVIEW_H
#define WAYPOINT_LISTVIEW_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>

#include "waypointlistwidget.h"
#include "waypoint.h"

class WaypointListView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointListView )

public:

  WaypointListView(QMainWindow *parent=0);

  virtual ~WaypointListView();

  WaypointListWidget* listWidget()
  {
    return listw;
  };

  Waypoint* getSelectedWaypoint()
  {
    return listw->getCurrentWaypoint();
  };

protected:

  virtual void showEvent(QShowEvent* event);

public slots:

  /**
    * Called if a waypoint has been added.
    */
   void slot_wpAdded(Waypoint& wp);

private slots:

  /**
   * This slot is called if the select button is pressed.
   */
  void slot_Select();

  /**
   * This slot is called if the info button has been
   * clicked, or the user pressed 'i'
   */
  void slot_Info();

  /**
   * Called when the listview should be closed without selection
   */
  void slot_Close ();

  /**
   * Called when the selected waypoints should be deleted from the
   * catalog
   */
  void slot_deleteWPs();

  /**
   * Called to remove all waypoints of the catalog.
   */
  void slot_deleteAllWPs();

  /**
   * Called when the selected waypoint needs must be opened in
   * the editor
   */
  void slot_editWP();

  /**
   * Called when a new waypoint needs to be made.
   */
  void slot_newWP();

  /**
   * Called if a waypoint has been edited.
   */
  void slot_wpEdited(Waypoint& wp);

  /**
   * Called to set a waypoint as homesite
   */
  void slot_setHome();

  /**
   * Called to reload the waypoint item list
   */
  void slot_reloadList()
  {
    listw->refillItemList();
  };

  /**
   * Called to change the displayed data according their priority.
   */
  void slot_changeDataDisplay();

  /**
   * Called, if the selection in the list is changed.
   */
  void slot_selectionChanged();

signals: // Signals
  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(Waypoint*, bool);

  /**
   * This signal is emitted if a waypoint is deleted.
   */
  void deleteWaypoint(Waypoint*);

  /**
   * This signal is send if the selection is done, and
   * the screen can be closed.
   */
  void done();

  /**
   * Emitted if the user clicks the Info button.
   */
  void info(Waypoint*);

  /**
   * Emitted if a new home position is selected.
   */
  void newHomePosition(const QPoint&);

  /**
   * Emitted to move the map to the new home position.
   */
  void gotoHomePosition();

private:

  WaypointListWidget* listw;
  QMainWindow*        par;
  QPushButton*        cmdSelect;
  QPushButton*        cmdHome;
  QPushButton*        cmdPriority;
  QPushButton*        cmdInfo;
  QPushButton*        cmdEdit;
  QPushButton*        cmdDel;
  QPushButton*        cmdDelAll;

  /** that shall store a home position change */
  bool homeChanged;

  /** Old priority of edited waypoint. */
  Waypoint::Priority priorityOfEditedWp;
};

#endif
