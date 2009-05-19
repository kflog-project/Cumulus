/***********************************************************************
**
**   waypointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2002 by André Somers
**                  2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINT_LISTVIEW_H
#define WAYPOINT_LISTVIEW_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>

#include "waypointlistwidget.h"
#include "waypoint.h"

class WaypointCatalog;

/**
 * This widget provides a list of waypoints and a means to select one.
 * @author André Somers
 */
class WaypointListView : public QWidget
{
    Q_OBJECT

public:
    WaypointListView(QMainWindow *parent=0);

    ~WaypointListView();

    WaypointListWidget* listWidget() {
      return listw;
    };

    wayPoint* getSelectedWaypoint() {
      return listw->getSelectedWaypoint();
    };

public slots: // Public slots
    /**
     * This slot is called to indicate that a selection has been made.
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
     * Called when the selected waypoint should be deleted from the
     * catalog
     */
    void slot_deleteWP();

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
    void slot_wpEdited(wayPoint& wp);

    /**
     * Called if a waypoint has been added.
     */
    void slot_wpAdded(wayPoint& wp);

    /**
     * Called to set a waypoint as homesite
     */
    void slot_setHome();

signals: // Signals
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint*, bool);

    /**
     * This signal is emitted if a waypoint is deleted.
     */
    void deleteWaypoint(wayPoint*);

    /**
     * This signal is send if the selection is done, and
     * the screen can be closed.
     */
    void done();

    /**
     * Emitted if the user clicks the Info button.
     */
    void info(wayPoint*);

    /**
     * Emitted if a new home position is selected
     */
    void newHomePosition(const QPoint&);

private:
    WaypointListWidget* listw;
    QMainWindow* par;
    QPushButton* cmdSelect;

private slots:

    void slot_Selected();

protected:

    void showEvent(QShowEvent *);
};

#endif
