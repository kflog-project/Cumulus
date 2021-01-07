/***********************************************************************
**
**   listwidgetparent.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class ListWidgetParent
 *
 * \author Josua Dietze, Axel Pauli
 *
 * \brief Base class to display a point list together with different command
 *        buttons.
 *
 * This widget provides a new widget base class to remove double code in
 * the point list views and the task editor.
 * Contains standard point list and attached filters (filter button row on
 * demand).
 *
 * Subclassed by \ref AirfieldListWidget, \ref WaypointListWidget
 *               \ref SinglePointListWidget
 *
 * \date 2002-2021
 *
 * \version 1.1
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

class QCheckBox;

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
     * Set visibility of saerch button.
     */
    void setVisibilityOfSearchButton( bool value )
    {
      searchButton->setVisible( value );
    }

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

    /*
     * This method is called, if the search button is pressed. Can be overwritten
     * in a subclass.
     */
    virtual void searchButtonPressed()
    {
    }

  public slots:

    /**
     * Called from parent when closing
     */
    void slot_Done();

  private slots:

    /**
     * Called, if the search button is clicked.
     */
    void slot_searchButtonClicked();

  signals:

    /**
     * This signal is emitted if the list selection has changed.
     */
    void wpSelectionChanged();

    /*
     * This signal is emitted, if the search button is clicked.
     */
    void searchButtonClicked();

  protected:

    void showEvent( QShowEvent *event );

    QTreeWidget*    list;
    ListViewFilter* filter;
    QPushButton* searchButton;

    /** Up and down buttons for page moving */
    QPushButton* up;
    QPushButton* down;

    /** A checkbox to toggle scroller against a big scrollbar. */
    QCheckBox* m_enableScroller;

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
     * Called is the checkbox is toggled.
     */
    void slot_scrollerBoxToggled( int state );

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
