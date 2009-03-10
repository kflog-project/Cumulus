/***********************************************************************
**
**   waypointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, Eckhard Völlm, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef REACH_POINT_LISTVIEW_H
#define REACH_POINT_LISTVIEW_H

#include <QWidget>
#include <QTreeWidget>
#include <QPixmap>
#include <QBoxLayout>
#include <QPushButton>
#include <QItemDelegate>

#include "waypoint.h"
#include "rowdelegate.h"

class MainWindow;

/**
 * This widget provides a list of reachable waypoints and a means to
 * select one.
 *
 * @author Eckhard Völlm
 */

class ReachpointListView : public QWidget
{
    Q_OBJECT

public:

    ReachpointListView(MainWindow *parent=0);
    ~ReachpointListView();

    /** Returns a pointer to the currently highlighted waypoint. */
    wayPoint * getSelectedWaypoint();


public slots: // Public slots

    /**
     * This slot is called to indicate that a selection has been made.
     */
    void slot_Select();

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
     * This slot is called if the Show Outland button has been clicked
     */
    void slot_ShowOl ();

    /**
     * This slot is called if the Hide Outland button has been clicked
     */
    void slot_HideOl ();

    /**
     * Signaled if the list of reachable points has changed
     */
    void slot_newList ();

    /**
     * Retrieves the waypoints from the map contents, and fills the list.
     */
    void fillRpList();

    /**
     * Clears the widget list.
     */
    void clearList()
    {
      list->clear();
    }

signals: // Signals

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
    void info(wayPoint *);


private:

    QTreeWidget* list;
    MainWindow * par;
    RowDelegate* rowDelegate;
    QBoxLayout * buttonrow;
    wayPoint    selectedWp;
    bool       _outlandShow;
    QPushButton *cmdShowOl;
    QPushButton *cmdHideOl;
    QPixmap     _arrows;
    bool        _newList;
    QPushButton *cmdSelect;

private slots:

    void slot_Selected();

protected:

    void showEvent(QShowEvent *);

private: // Private methods

};

#endif
