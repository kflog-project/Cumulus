/***********************************************************************
**
**   waypointlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "waypointlistwidget.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "wpeditdialog.h"

extern MapContents* _globalMapContents;
extern MapConfig*   _globalMapConfig;

WaypointListWidget::WaypointListWidget( QWidget *parent, bool showMovePage ) :
  ListWidgetParent( parent, showMovePage ),
  priority( Waypoint::Top )
{
  setObjectName("WaypointListWidget");
  list->setObjectName("WpTreeWidget");

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif
}

WaypointListWidget::~WaypointListWidget()
{
}

/** Clears and refills the waypoint item list. */
void WaypointListWidget::fillItemList()
{
  ListWidgetParent::fillItemList();

  list->setUpdatesEnabled(false);
  list->clear();

  configRowHeight();

  QList<Waypoint> &wpList = _globalMapContents->getWaypointList();

  for (int i=0; i < wpList.count(); i++)
    {
      Waypoint& wp = wpList[i];

      if( priority > Waypoint::High )
        {
          // Show all waypoints.
          list->addTopLevelItem( new WaypointItem(wp) );
        }
      else if( priority == wp.priority )
        {
          // Show only desired waypoints.
          list->addTopLevelItem( new WaypointItem(wp) );
        }
    }

  resizeListColumns();

  filter->reset();

  if ( wpList.count() > 0 )
    {
      list->setCurrentItem(list->topLevelItem(0));
    }

  list->setUpdatesEnabled(true);
}

/** Returns a pointer to the currently selected item. */
Waypoint* WaypointListWidget::getCurrentWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();

  if( li == static_cast<QTreeWidgetItem *> ( 0 ) )
    {
      return static_cast<Waypoint *> ( 0 );
    }

  // Now we're left with the real waypoints
  WaypointItem* wpi = dynamic_cast<WaypointItem *> ( li );

  if( !wpi )
    {
      return static_cast<Waypoint *> ( 0 );
    }

  return &wpi->wp;
}

/**
 * @return A list containing all currently selected waypoints.
 */
QList<Waypoint *> WaypointListWidget::getSelectedWaypoints()
{
  QList<Waypoint *> wpList;

  QList<QTreeWidgetItem *> itemList = list->selectedItems();

  if( itemList.size() )
    {
      for( int i = 0; i < itemList.size(); i++ )
        {
          WaypointItem* wpi = dynamic_cast<WaypointItem *> (itemList.at(i));

            if ( ! wpi )
              {
                continue;
              }

            wpList.append( &wpi->wp );
        }
    }

  return wpList;
}

/**
 * Removes all currently selected waypoints.
 */
void WaypointListWidget::deleteSelectedWaypoints()
{
  QList<Waypoint *> wpList;

  QList<QTreeWidgetItem *> itemList = list->selectedItems();

  if( itemList.size() )
    {
      list->setUpdatesEnabled(false);

      QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

      for( int i = 0; i < itemList.size(); i++ )
        {
          WaypointItem* wpi = dynamic_cast<WaypointItem *> (itemList.at(i));

          if( ! wpi )
            {
              continue;
            }

          // Get waypoint reference from item.
          Waypoint& wp = wpi->wp;

          // At first remove waypoint from filter because there is a reference
          // to the global waypoint list.
          filter->removeListItem( itemList.at(i) );

          // At last remove waypoint from global list in MapContents
          wpList.removeOne( wp );
        }

      // save the modified catalog
      _globalMapContents->saveWaypointList();

      filter->reset();
      resizeListColumns();
      list->setUpdatesEnabled(true);
    }
}

/**
 * Removes all waypoints from the list.
 */
void WaypointListWidget::deleteAllWaypoints()
{
  // remove all waypoints in the catalog
  _globalMapContents->getWaypointList().clear();

  // save the modified catalog
  _globalMapContents->saveWaypointList();

  list->clear();
  filter->reset();
  resizeListColumns();
}

// JD: after adding, deleting or name-changing a waypoint the filter
// and the view must always be reset to regain consistency

/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListWidget::deleteCurrentWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();

  if( li == 0 )
    {
      return;
    }

  Waypoint *wp = getCurrentWaypoint();

  if( !wp )
    {
      return;
    }

  // remove waypoint from waypoint list in MapContents
  _globalMapContents->getWaypointList().removeAll( *wp );
  // save the modified catalog
  _globalMapContents->saveWaypointList();

  // update the filter and reset the view
  list->setUpdatesEnabled(false);
  filter->removeListItem(li);
  filter->reset();
  resizeListColumns();
  list->setUpdatesEnabled(true);
}

void WaypointListWidget::deleteWaypoint(Waypoint &wp)
{
  // Sets the waypoint to be deleted as the current item.
  for( int i = 0; i < list->topLevelItemCount(); i++ )
    {
      WaypointItem* wpi = dynamic_cast<WaypointItem *> (list->topLevelItem(i));

      if( ! wpi )
        {
          continue;
        }

      if( wpi->wp == wp )
        {
          // If the waypoints are identical remove the waypoint from the list.
          list->setCurrentItem( wpi );
          deleteCurrentWaypoint();
          return;
        }
    }

  // There is on waypoint in the waypoint list view.
  // Remove waypoint from global waypoint list in MapContents
  _globalMapContents->getWaypointList().removeAll( wp );
  // Save the modified waypoint list as file.
  _globalMapContents->saveWaypointList();
}

/** Called if a waypoint has been edited. */
void WaypointListWidget::updateCurrentWaypoint(Waypoint& wp)
{
  QTreeWidgetItem* li = list->currentItem();

  if( li == 0 )
    {
      return;
    }

  list->setUpdatesEnabled(false);

  li->setText(1, wp.description);
  li->setText(2, wp.country);
  li->setText(3, wp.icao);
  li->setIcon(0, QIcon(_globalMapConfig->getPixmap(wp.type,false,true)));

  // JD: if the WP name was not changed we just update the item; otherwise
  // we need to resort and therefore reset the filter and view

  if (li->text(0) == wp.name)
    {
      li->setText(0, wp.name);
    }
  else
    {
      li->setText(0, wp.name);
      filter->reset();
      resizeListColumns();
    }

  list->setUpdatesEnabled(true);

  // save modified catalog
  _globalMapContents->saveWaypointList();
}

/** Called if a waypoint has been added. */
void WaypointListWidget::addWaypoint( Waypoint& newWp )
{
  // put new waypoint into the global waypoint list
  QList<Waypoint> &wpList = _globalMapContents->getWaypointList();

  // A waypoint name is limited to 8 characters and has only upper cases.
  newWp.name = newWp.name.left(8).toUpper();
  newWp.wpListMember = true;

  wpList.append( newWp );

  // save the modified waypoint catalog
  _globalMapContents->saveWaypointList();

  // retrieve the reference of the appended waypoint from the global list
  Waypoint& wp = wpList.last();

  filter->addListItem( new WaypointItem(wp) );

  // resort WP list and reset filter and view
  list->setUpdatesEnabled(false);

  filter->reset();
  resizeListColumns();

  list->setUpdatesEnabled(true);
}

WaypointListWidget::WaypointItem::WaypointItem( Waypoint& waypoint ) :
  QTreeWidgetItem(),  wp(waypoint)
{
  setText(0, wp.name);
  setText(1, wp.description.left(15));
  setText(2, wp.country);
  setTextAlignment(2, Qt::AlignCenter);
  setText(3, wp.icao);

  QPixmap wpPm = _globalMapConfig->getPixmap(wp.type, false, false);

  QIcon icon( wpPm );
  setIcon( 0, icon );
}
