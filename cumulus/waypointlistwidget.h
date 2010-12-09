/***********************************************************************
**
**   waypointlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2010 by Axel Pauli
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
 * \brief This widget provides a list of waypoints and a means to select one.
 *
 * \date 2002-2010
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
  wayPoint *getSelectedWaypoint();

  /**
   * @param wp Updates the currently high lighted waypoint after editing.
   */
  void updateSelectedWaypoint(wayPoint &wp);

  /**
   * Removes the currently highlighted waypoint.
   */
  void deleteSelectedWaypoint();

  /**
   * @param wp Adds a waypoint to the list.
   */
  void addWaypoint(wayPoint &wp);

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
          _WaypointItem(wayPoint&);
          wayPoint &wp;
    };
};

#endif
