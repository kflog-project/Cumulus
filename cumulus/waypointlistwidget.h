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
 * \author André Somers, Axel Pauli
 *
 * \brief This widget provides a list of waypoints and a means to select one.
 *
 */

#ifndef WAYPOINT_LIST_WIDGET_H
#define WAYPOINT_LIST_WIDGET_H

#include "wplistwidgetparent.h"
#include "waypoint.h"

class WaypointListWidget : public WpListWidgetParent
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
   * @returns a pointer to the currently highlighted waypoint.
   */
  wayPoint *getSelectedWaypoint();

  /**
   * @updates the currently highlighted waypoint after editing.
   */
  void updateSelectedWaypoint(wayPoint &);

  /**
   * @removes the currently highlighted waypoint.
   */
  void deleteSelectedWaypoint();

  /**
   * @adds a waypoint.
   */
  void addWaypoint(wayPoint &);

  /**
   * Clears and fills the waypoint item list with the current waypoints.
   */
  void fillItemList();

protected:

  void showEvent( QShowEvent *event );

private:

  class _WaypointItem : public QTreeWidgetItem
    {
      public:
          _WaypointItem(wayPoint&);
          wayPoint &wp;
    };
};

#endif
