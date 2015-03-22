/***********************************************************************
**
**   listviewfilter.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef LIST_VIEW_FILTER_H
#define LIST_VIEW_FILTER_H

#include <QWidget>
#include <QList>
#include <QTreeWidget>
#include <QPushButton>
#include <QString>

/**
 * \class ListViewFilterItem
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Filter item of a QTreeWidget list.
 *
 * \see ListViewFilter
 *
 * Creates a filter item as subset of a bigger list. This class is used by the
 * \ref ListViewFilter class.
 *
 * \date 2004-2015
 */

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
   * These instances are initialized and added to @ref subfilters for
   * future reference.
   */
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

  /** Set of filters that further subdivides the result of this filter. */
  QList<ListViewFilterItem *> subfilters;

private:

  int diffLevel(const QString&, const QString&);
};

/**
 * \class ListViewFilter
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Creates a filter bar for a QTreeWidget
 *
 * \see ListViewFilterItem
 *
 * Creates a filter bar for a QTreeWidget in order to quickly filter the list view.
 *
 * \date 2004-2015
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

  /**
   * \param tw A pointer to the list view this filter works on.
   *
   * \param parent A pointer to the parent widget.
   */
  ListViewFilter( QTreeWidget* tw, QWidget* parent=static_cast<QWidget *>(0) );

  virtual ~ListViewFilter();

  /** Adds a item to the list. */
  void addListItem(QTreeWidgetItem* it);

  /** Removes an item from the list. */
  void removeListItem(QTreeWidgetItem* it);

  /**
   * Resets all filters to the root filter.
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

  /** Returns the active filter */
  ListViewFilterItem* activeFilter() const
    {
      return _activeFilter;
    };

  /** Returns the root filter. */
  ListViewFilterItem* rootFilter() const
    {
      return _rootFilter;
    };

private:

  /**
   * Activates the indicated filter. The list this filter holds is subdivided
   * if needed and the button row is adjusted to match.
   */
  void activateFilter( ListViewFilterItem* filter, int shrink=0 );

  /**
   * \return The current state of the flag.
   */
  bool isTopButtonContained() const
  {
    return m_isTopButtonContained;
  };

  /**
   * Sets the top button contained flag.
   *
   * \param flag New value to be set.
   */
  void setTopButtonContained( bool flag )
  {
    m_isTopButtonContained = flag;
  };

  /** Pointer to display table view */
  QTreeWidget* _tw;

  /** List of filter buttons. */
  QList<QPushButton *> _buttonList;

  /** List of filter lists accessed by _filterIndex. */
  QList< QList<ListViewFilterItem *> > _filterList;

  /** current index to _filterList */
  int _filterIndex;

  /** active filter */
  ListViewFilterItem* _activeFilter;

  /** root filter of the tree */
  ListViewFilterItem* _rootFilter;

  /** Set to true, if top button is contained in the button list. */
  bool m_isTopButtonContained;

private slots:

  /**
   * Called if one of the buttons is clicked. The argument indicates which button.
   */
  void slot_CmdPush( int );
};

#endif
