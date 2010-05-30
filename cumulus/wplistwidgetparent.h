/***********************************************************************
**
**   wplistwidgetparent.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WP_LISTWIDGET_PARENT_H
#define WP_LISTWIDGET_PARENT_H

#include <QWidget>
#include <QTreeWidget>
#include <QItemDelegate>

#include "waypoint.h"
#include "listviewfilter.h"
#include "rowdelegate.h"

/**
 * \author Josua Dietze
 *
 * \brief Base class for airfield, waypoint, outlanding widget.
 *
 * This widget provides a new widget base class to remove double code in
 * airfield list view, waypoint list view and task editor.
 * Contains standard airfield list and attached filters (filter button row on
 * demand).
 *
 * Subclassed by airfieldlistwidget and waypointlistwidget.
 *
 */

class WpListWidgetParent : public QWidget
{
  Q_OBJECT

  private:
    /**
     * That macro forbids the copy constructor and the assignment operator.
     */
    Q_DISABLE_COPY( WpListWidgetParent )

  public:

    WpListWidgetParent( QWidget *parent = 0 );

    virtual ~WpListWidgetParent();

    /**
     * sets the list row height from configuration
     */
    void configRowHeight();

    /**
     * @returns a pointer to the currently high lighted waypoint.
     * The user must implement this method in his subclass.
     */
    virtual wayPoint* getSelectedWaypoint() = 0;

    /**
     * Retrieves the locations from the map contents and fills
     * the list. The user must implement this method in his subclass.
     */
    virtual void fillWpList() = 0;

    /**
     * @returns a pointer to the "list" widget
     */
    QTreeWidget* listWidget()
    {
      return list;
    };

    /**
     * aligns the columns to their contents
     */
    virtual void resizeListColumns()
    {
      list->resizeColumnToContents(0);
      list->resizeColumnToContents(1);
      list->resizeColumnToContents(2);
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

    void showEvent(QShowEvent *);

    QTreeWidget*    list;
    ListViewFilter* filter;

    /** Flag to indicate that a first load of the list items has been done. */
    bool firstLoadDone;

  private:

    RowDelegate* rowDelegate;

  private slots:

    /**
     * Called from tree widget when an entry is tapped on.
     */
    void slot_listItemClicked(QTreeWidgetItem*, int);
};

#endif
