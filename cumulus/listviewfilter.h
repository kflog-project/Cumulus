/***********************************************************************
**
**   listviewfilter.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LIST_VIEW_FILTER_H
#define LIST_VIEW_FILTER_H

#include <QWidget>
#include <QList>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>

class ListViewFilterItem : QObject
{
  Q_OBJECT

public:

  ListViewFilterItem( ListViewFilterItem* parent=static_cast<ListViewFilterItem *>(0) );

  virtual ~ListViewFilterItem();

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
  QList<ListViewFilterItem *> subfilters;

  //list that holds the items that belong to this filter
  QList<QTreeWidgetItem *> items;

  //flag that indicates this filter has subfilters and the split has already been done
  bool _split;

  //reference to ListViewFilterItem one lever higher than this instance
  ListViewFilterItem* parent;

private:

  int diffLevel(const QString&, const QString&);
};

/**
 * Creates a filter bar for a QTreeViewWidget in order to quickly filter the listview.
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
  ListViewFilter( QTreeWidget* tw, QWidget* parent=static_cast<QWidget *>(0) );

  ~ListViewFilter();

  void addListItem(QTreeWidgetItem* it);
  void removeListItem(QTreeWidgetItem* it);

  /**
   * Re-creates the index for the filter
   * @arg forget Don't try to re-insert any removed items, just forget them.
   *          This is needed when items are deleted from the list.
   */
  void reset();

  /**
   * Re-selects the root of the filter tree
   */
  void off();

  /**
   * Clears the filter tree and deletes all items (for refilling)
   */
  void clear();

  /**
   * Shows items of active filter in list, optionally divided in pages
   */
  void showPage(bool up);

private:

  //pointer to tree view
  QTreeWidget* _tw; // _lv
  QTreeWidgetItem* prev;
  QTreeWidgetItem* next;
  //list of buttons
  QList<QPushButton *> _buttonList;
  //active filter
  ListViewFilterItem* _activeFilter;
  //root of the filter tree
  ListViewFilterItem* _rootFilter;

  int showIndex;
  int recursionLevel;

  /**
   * Activates the indicated filter. The list this filter holds is subdivided
   * if needed and the button row is adjusted to match.
   */
  void activateFilter( ListViewFilterItem* filter, int shrink=0 );

private slots:
  /**
   * Called if one of the buttons is clicked. The argument indicates which button.
   */
  void slot_CmdPush( int );
};

#endif
