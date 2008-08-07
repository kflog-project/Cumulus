/***********************************************************************
**
**   wplistwidgetclass.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 by Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WPLISTWIDGETCLASS_H
#define WPLISTWIDGETCLASS_H

#include <QWidget>
#include <QTreeWidget>
#include <QItemDelegate>

#include "waypoint.h"
#include "listviewfilter.h"
#include "rowdelegate.h"

//class WaypointCatalog;

/**
 * This widget provides a list of waypoints and a means to select one.
 *
 * @author Josua Dietze
 */
class WPListWidgetClass : public QWidget
{
    Q_OBJECT
public:
    WPListWidgetClass(QWidget *parent=0);

    ~WPListWidgetClass();

    /**
     * sets the list row height from configuration
     */
    void configRowHeight();

    /**
     * @returns a pointer to the currently highlighted waypoint.
     */
    virtual wayPoint* getSelectedWaypoint() {return 0;};

    /**
     * retrieves the waypoints or airfields from the map contents and fills
     * the list.
     */
    virtual void fillWpList() { return; };

    /**
     * @returns a pointer to the "list" widget
     */
    QTreeWidget* listWidget() { return list; };

public slots: // Public slots

    /**
     * Called from parent when closing
     */
    void slot_Done();

    /**
     * Called from parent when "Select" button was pressed
     */
    void slot_Select();


signals: // Signals

    /**
     * This signal is emitted if the list selection has changed.
     */
    void wpSelectionChanged();

protected:
    void showEvent(QShowEvent *);
    QTreeWidget* list;
    ListViewFilter * filter;
    bool listFilled;

private:
    RowDelegate* rowDelegate;

private slots:

    /**
     * Called from tree widget when selection changed.
     */
    void slot_SelectionChanged();

};

#endif
