/***********************************************************************
**
**   listviewfilter.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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

  private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( ListViewFilterItem )

public:

  ListViewFilterItem( QTreeWidget *tw,
                      ListViewFilterItem* parent=static_cast<ListViewFilterItem *>(0) );

  virtual ~ListViewFilterItem();

  /**
   * Tries to divide the list belonging to this filter into smaller
   * lists, and creates the appropriate ListViewFilterItem instances.
   * these instances are initialized and added to @ref subfilters for
   * future reference. @ref _split is set. */
  void divide( int partcount, QList<ListViewFilterItem *> &subFilters );

  /** Returns the number of items in the list tree assigned to this filter item.*/
  int itemCount()
    {
      if( beginIdx < 0 )
        {
          return 0;
        }

      return (endIdx - beginIdx);
    };

  /** Returns the first text element at the item position */
  QString itemTextAt( const int pos );

  /** Make all items of the filter visible. */
  void showFilterItems();

  /** Reference to ListViewFilterItem one level higher than this instance. */
  ListViewFilterItem *_parent;

  /** Pointer to tree widget with all list elements. */
  QTreeWidget *_tw;

  /** Holds the first letter(s) for the filter. */
  QString from;

  /** Holds the last letter(s) for the filter. */
  QString to;

  /** Holds the text of the assigned button. */
  QString buttonText;

  /** Begin index of this filter item in the tree widget. */
  int beginIdx;

  /** End index of this filter item in the tree widget. */
  int endIdx;

  /** set of filters that further subdivides the result of this filter. */
  QList<ListViewFilterItem *> subfilters;

private:

  int diffLevel(const QString&, const QString&);
};

/**
 * \author André Somers
 *
 * \brief Creates a filter bar for a QTreeWidget
 *
 * Creates a filter bar for a QTreeWidget in order to quickly filter the list view.
 *
 */
class ListViewFilter : public QWidget
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( ListViewFilter )

private:

  /** Defines the maximum number of available filter buttons. */
  static const int buttonCount;

public:

  // defines the strings to be used in widget tree for browsing
  static const QString NextPage;
  static const QString PreviousPage;

  /**
   * Constructor.
   * @arg tw Pointer to the listview this filter works on.
   */
  ListViewFilter( QTreeWidget* tw, QWidget* parent=static_cast<QWidget *>(0) );

  virtual ~ListViewFilter();

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
   * Removes all filter items and resets all to default.
   */
  void clear();

  /**
   * Shows items of active filter in list, optionally divided in pages
   */
  void showPage(bool up);

private:

  /**
   * Activates the indicated filter. The list this filter holds is subdivided
   * if needed and the button row is adjusted to match.
   */
  void activateFilter( ListViewFilterItem* filter, int shrink=0 );

  // pointer to display table view
  QTreeWidget*     _tw;
  QTreeWidgetItem* prev;
  QTreeWidgetItem* next;

  /** list of filter buttons */
  QList<QPushButton *> _buttonList;

  /** list of filter lists accessed by _filterIndex */
  QList< QList<ListViewFilterItem *> > _filterList;

  /** current index to _filterList */
  int _filterIndex;

  /** active filter */
  ListViewFilterItem* _activeFilter;

  /** root filter of the tree */
  ListViewFilterItem* _rootFilter;

  int showIndex;

private slots:

  /**
   * Called if one of the buttons is clicked. The argument indicates which button.
   */
  void slot_CmdPush( int );
};

#endif
