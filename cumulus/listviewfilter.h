/***********************************************************************
**
**   listviewfilter.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LISTVIEWFILTER_H
#define LISTVIEWFILTER_H

#include <QWidget>
#include <QList>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>

class ListViewFilterItem;

typedef QList<ListViewFilterItem*> filterSet;
typedef QList<QTreeWidgetItem*> itemList;

class ListViewFilterItem
{
public:

    ListViewFilterItem(ListViewFilterItem * parent=0);
    ~ListViewFilterItem();

    /**
     * Add the list of ListViewItem objects belonging to this
     * filter to the indicated ListView.
     * @arg lv The QListView to add the items to.
     */
    void addToList(QTreeWidget* tw, bool isRecursive=false);
    /**
     * Tries to divide the list belonging to this filter into smaller
     * lists, and creates the appropriate ListViewFilterItem instances.
     * these instances are initialized and added to @ref subfilters for
     * future reference. @ref _split is set. */
    void divide(int);

    //holds the first letter(s) for the filter
    QString from;
    //holds the last letter(s) for the filter
    QString to;
    //set of filters that further subdivides the result of this filter
    filterSet subfilters;
    //list that holds the items that belong to this filter
    itemList items;
    //flag that indicates this filter has subfilters and the split has allready been done
    bool _split;
    //reference to ListViewFilterItem one lever higher than this instance
    ListViewFilterItem * parent;

private:
    int diffLevel(const QString&, const QString&);
};

/**
 * Creates a filterbar for a QTreeViewWidget in order to quickly filter the listview.
 * @author André Somers
 */
class ListViewFilter : public QWidget
{
    Q_OBJECT

private:

    // defines the number of filter buttons
    static const uint buttonCount;

public:

    // defines the strings to be used in widget tree for browsing
    static const QString NextPage;
    static const QString PreviousPage;

    /**
     * Constructor.
     * @arg lv Reference to the listview this filter works on.
     */
    ListViewFilter(QTreeWidget *tw, QWidget *parent=0);
    ~ListViewFilter();

    /**
     * Re-creates the index for the filter
     * @arg forget Don't try to re-insert any removed items, just forget them.
     *          This is needed when items are deleted from the list.
     */
    void reset(bool forget=false);

    /**
     * Re-selects the root of the filter tree
     */
    void off();

    /**
     * Moves all the listview items back into the listview
     */
    void restoreListViewItems();

    void showPage(bool up);

private:
    //pointer to tree view
    QTreeWidget* _tw; // _lv
    QTreeWidgetItem* prev;
    QTreeWidgetItem* next;
    //list of buttons
    QList<QPushButton*> _buttonList;
    //active filter
    ListViewFilterItem* _activeFilter;
    //root of the filter tree
    ListViewFilterItem* _rootFilter;

    int showIndex;
    int recursionLevel;

    /**
     * Activates the inidicated filter. The list this filter holds is subdivided if needed
     * and the buttonrow is adjusted to match.
     */
    void activateFilter(ListViewFilterItem *, int shrink=0);

private slots:
    /**
     * Called if one of the buttons is clicked. The argument indicates which button.
     */
    void cmdPush(int);
};

#endif
