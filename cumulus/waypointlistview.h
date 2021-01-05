/***********************************************************************
**
**   waypointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2002      by André Somers
**                  2007-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class WaypointListView
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This widget provides a list of waypoints and a means to select one.
 *
 * \date 2002-2021
 *
 * \version 1.1
 */

#ifndef WAYPOINT_LISTVIEW_H
#define WAYPOINT_LISTVIEW_H

#include <QWidget>
#include <QPoint>
#include <QPushButton>

#include "singlepoint.h"
#include "waypointlistwidget.h"
#include "waypoint.h"

class WaypointListView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointListView )

public:

  WaypointListView(QWidget *parent=0);

  virtual ~WaypointListView();

  WaypointListWidget* listWidget()
  {
    return listw;
  };

  /**
   * \return The top level item count of the tree list.
   */
  int topLevelItemCount()
  {
    return listw->topLevelItemCount();
  };

  Waypoint* getCurrentEntry()
  {
    return listw->getCurrentWaypoint();
  };

protected:

  virtual void showEvent(QShowEvent* event);

public slots:

  /**
    * Called if a waypoint should be added.
    */
  void slot_addWp(Waypoint& wp);

   /**
     * Called if a waypoint should be deleted.
     */
  void slot_deleteWp(Waypoint& wp);

private slots:

  /**
   * This slot is called, if the select button is pressed.
   */
  void slot_Select();

  /**
   * This slot is called, if the search button is pressed;
   */
  void slot_Search();

  /**
   * This slot is called, to pass the search result.
   */
  void slot_SearchResult( const SinglePoint* singlePoint );

  /**
   * This slot is called if the info button has been
   * clicked, or the user pressed 'i'
   */
  void slot_Info();

  /**
   * Called when the listview should be closed without selection
   */
  void slot_Close();

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

signals:

  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(Waypoint* wp, bool);

  /**
   * This signal is emitted if a waypoint is deleted.
   */
  void deleteWaypoint( Waypoint* wp );

  /**
   * This signal is send if the selection is done, and
   * the screen can be closed.
   */
  void done();

  /**
   * Emitted if the user clicks the Info button.
   */
  void info( Waypoint* wp );

  /**
   * Emitted if a new home position is selected.
   */
  void newHomePosition( const QPoint& newHome );

  /**
   * Emitted to move the map to the new home position.
   */
  void gotoHomePosition();

private:

  WaypointListWidget* listw;
  QPushButton*        cmdSearch;
  QPushButton*        cmdSelect;
  QPushButton*        cmdHome;
  QPushButton*        cmdPriority;
  QPushButton*        cmdInfo;
  QPushButton*        cmdEdit;
  QPushButton*        cmdDel;
  QPushButton*        cmdDelAll;

  /** that stores a home position change */
  bool m_homeChanged;

  /** Remember flag, if edited waypoint is the selected target. */
  bool m_editedWpIsTarget;
};

#endif
