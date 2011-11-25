/***********************************************************************
**
**   waypointlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

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
}

WaypointListWidget::~WaypointListWidget()
{
}

/** Clears and refills the waypoint item list. */
void WaypointListWidget::fillItemList()
{
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
          list->addTopLevelItem( new _WaypointItem(wp) );
        }
      else if( priority == wp.priority )
        {
          // Show only desired waypoints.
          list->addTopLevelItem( new _WaypointItem(wp) );
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
  _WaypointItem* wpi = dynamic_cast<_WaypointItem *> ( li );

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
          _WaypointItem* wpi = dynamic_cast<_WaypointItem *> (itemList.at(i));

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
          _WaypointItem* wpi = dynamic_cast<_WaypointItem *> (itemList.at(i));

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
void WaypointListWidget::addWaypoint(Waypoint& newWp)
{
  // put new waypoint into the global waypoint list
  QList<Waypoint> &wpList = _globalMapContents->getWaypointList();

  // A waypoint name is limited to 8 characters and has only upper cases.
  newWp.name = newWp.name.left(8).toUpper();

  wpList.append( newWp );

  // save the modified waypoint catalog
  _globalMapContents->saveWaypointList();

  // retrieve the reference of the appended waypoint from the global list
  Waypoint& wp = wpList.last();

  filter->addListItem( new _WaypointItem(wp) );

  // resort WP list and reset filter and view
  list->setUpdatesEnabled(false);

  filter->reset();
  resizeListColumns();

  list->setUpdatesEnabled(true);
  // qDebug("WaypointListWidget::addWaypoint: name=%s", wp.name.toLatin1().data() );
}

WaypointListWidget::_WaypointItem::_WaypointItem( Waypoint& waypoint ) :
  QTreeWidgetItem(),  wp(waypoint)
{
  QPainter pnt;
  QPixmap selectIcon;

  setText(0, wp.name);
  setText(1, wp.description);
  setText(2, wp.country);
  setTextAlignment(2, Qt::AlignCenter);
  setText(3, wp.icao);

  selectIcon = QPixmap( 18, 18 );
  pnt.begin(&selectIcon);
  selectIcon.fill( Qt::white );
  pnt.drawPixmap(1, 1, _globalMapConfig->getPixmap(wp.type, false, true) );
  pnt.end();
  QIcon icon;
  icon.addPixmap( _globalMapConfig->getPixmap(wp.type, false, true) );
  icon.addPixmap( selectIcon, QIcon::Selected );
  setIcon( 0, icon );
}
