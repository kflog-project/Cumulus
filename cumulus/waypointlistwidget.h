/***********************************************************************
**
**   waypointlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WaypointListWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This widget displays a list of waypoints and provides some management
 *        methods.
 *
 * This widget displays a list of waypoints and provides some management methods.
 *
 * \date 2002-2011
 */

#ifndef WAYPOINT_LIST_WIDGET_H
#define WAYPOINT_LIST_WIDGET_H

#include "listwidgetparent.h"
#include "waypoint.h"

class WaypointListWidget : public ListWidgetParent
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( WaypointListWidget )

public:

  WaypointListWidget( QWidget *parent=0, bool showMovePage=true );

  virtual ~WaypointListWidget();

  /**
   * @return A pointer to the currently high lighted waypoint.
   */
  Waypoint *getCurrentWaypoint();

  /**
   * @return A list containing all currently selected waypoints.
   */
  QList<Waypoint *> getSelectedWaypoints();

  /**
   * Removes all currently selected waypoints.
   */
  void deleteSelectedWaypoints();

  /**
   * @param wp Updates the currently high lighted waypoint after editing.
   */
  void updateCurrentWaypoint(Waypoint &wp);

  /**
   * Removes the currently highlighted waypoint.
   */
  void deleteCurrentWaypoint();

  /**
   * @param wp Adds a waypoint to the list.
   */
  void addWaypoint(Waypoint &wp);

  /**
   * Clears and fills the waypoint item list with the current waypoints.
   */
  void fillItemList();

private:

  /**
   * \class _WaypointItem
   *
   * \author André Somers, Axel Pauli
   *
   * \brief A user waypoint item element used by the \ref WaypointListWidget.
   *
   * \date 2002-2010
   */

  class _WaypointItem : public QTreeWidgetItem
    {
      public:
          _WaypointItem(Waypoint&);
          Waypoint &wp;
    };
};

#endif
