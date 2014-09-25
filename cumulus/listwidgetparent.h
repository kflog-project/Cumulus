/***********************************************************************
**
**   listwidgetparent.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class ListWidgetParent
 *
 * \author Josua Dietze, Axel Pauli
 *
 * \brief Base class for airfield, waypoint and outlanding widget.
 *
 * This widget provides a new widget base class to remove double code in
 * the airfield and waypoint list view and the task editor.
 * Contains standard airfield list and attached filters (filter button row on
 * demand).
 *
 * Subclassed by \ref AirfieldListWidget and \ref WaypointListWidget.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef LISTWIDGET_PARENT_H
#define LISTWIDGET_PARENT_H

#include <QWidget>
#include <QTreeWidget>
#include <QItemDelegate>
#include <QVBoxLayout>

#include "waypoint.h"
#include "listviewfilter.h"
#include "rowdelegate.h"

class ListWidgetParent : public QWidget
{
  Q_OBJECT

  private:
    /**
     * That macro forbids the copy constructor and the assignment operator.
     */
    Q_DISABLE_COPY( ListWidgetParent )

  public:

    ListWidgetParent( QWidget *parent = 0, bool showMovePage=true );

    virtual ~ListWidgetParent();

    /**
     * sets the list row height from configuration
     */
    void configRowHeight();

    /**
     * @returns a pointer to the currently high lighted waypoint.
     * The user must implement this method in his subclass.
     */
    virtual Waypoint* getCurrentWaypoint() = 0;

    /**
     * Retrieves the locations from the map contents and fills
     * the list. It sets the icon size of the list depending of the used font.
     */
    virtual void fillItemList();

    /**
     * Clears and refills the item list, if items are loaded. Called
     * if the map projection has been changed to ensure an update of the
     * projected coordinates.
     */
    void refillItemList();

    /**
     * @returns a pointer to the list widget
     */
    QTreeWidget* listWidget()
    {
      return list;
    };

    /**
     * \return The top level item count of the tree list.
     */
    int topLevelItemCount()
    {
      return list->topLevelItemCount();
    };

    /**
     * aligns the columns to their contents
     */
    virtual void resizeListColumns()
    {
      list->resizeColumnToContents(0);
      list->resizeColumnToContents(1);
      list->resizeColumnToContents(2);
      list->resizeColumnToContents(3);
    };

  public slots:

    /**
     * Called from parent when closing
     */
    void slot_Done();

  signals:

    /**
     * This signal is emitted if the list selection has changed.
     */
    void wpSelectionChanged();

  protected:

    void showEvent( QShowEvent *event );

    QTreeWidget*    list;
    ListViewFilter* filter;

    /** Up and down buttons for page moving */
    QPushButton* up;
    QPushButton* down;

    /** Flag to indicate that a first load of the list items has been done. */
    bool firstLoadDone;

  private:

    RowDelegate* rowDelegate;

  private slots:

    /**
     * Called from tree widget when an entry is tapped on.
     */
    void slot_listItemClicked(QTreeWidgetItem*, int);

    /**
     * Move page up.
     */
    void slot_PageUp();

    /**
     * Move page down.
     */
    void slot_PageDown();
};

#endif
