/***********************************************************************
**
**   waypointlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
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
  ListWidgetParent( parent, showMovePage )
{
  setObjectName("WaypointListWidget");
  list->setObjectName("WpTreeWidget");
}

WaypointListWidget::~WaypointListWidget()
{
}

void WaypointListWidget::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // load list items during first show
  if( firstLoadDone == false )
    {
      firstLoadDone = true;
      fillItemList();
    }
}

/** Clears and refills the waypoint item list. */
void WaypointListWidget::fillItemList()
{
  // qDebug() << "WaypointListWidget::fillItemList()";

  list->setUpdatesEnabled(false);
  list->clear();

  configRowHeight();

  QList<wayPoint> &wpList = _globalMapContents->getWaypointList();

  for (int i=0; i < wpList.count(); i++)
    {
      wayPoint& wp = wpList[i];
      list->addTopLevelItem( new _WaypointItem(wp) );
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
wayPoint* WaypointListWidget::getSelectedWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();

  if ( li == static_cast<QTreeWidgetItem *>(0) )
    {
      return static_cast<wayPoint *>(0);
    }

  // Now we're left with the real waypoints
  _WaypointItem* wpi = static_cast<_WaypointItem *> (li);

  if ( !wpi )
    {
      return static_cast<wayPoint *>(0);
    }

  return &wpi->wp;
}

// JD: after adding, deleting or name-changing a waypoint the filter
// and the view must always be reset to regain consistency

/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListWidget::deleteSelectedWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();

  if ( li== 0 )
    {
      return;
    }

  wayPoint *wp = getSelectedWaypoint();

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
void WaypointListWidget::updateSelectedWaypoint(wayPoint& wp)
{
  QTreeWidgetItem* li = list->currentItem();

  if ( li == 0 )
    {
      return;
    }

  list->setUpdatesEnabled(false);

  li->setText(1, wp.description);
  li->setText(2, wp.icao);
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
void WaypointListWidget::addWaypoint(wayPoint& newWp)
{
  // put new waypoint into the global waypoint list
  QList<wayPoint> &wpList = _globalMapContents->getWaypointList();
  wpList.append( newWp );

  // save the modified waypoint catalog
  _globalMapContents->saveWaypointList();

  // retrieve the reference of the appended waypoint from the global list
  wayPoint& wp = wpList.last();

  filter->addListItem( new _WaypointItem(wp) );

  // resort WP list and reset filter and view
  list->setUpdatesEnabled(false);

  filter->reset();
  resizeListColumns();

  list->setUpdatesEnabled(true);
  // qDebug("WaypointListWidget::addWaypoint: name=%s", wp.name.toLatin1().data() );
}

WaypointListWidget::_WaypointItem::_WaypointItem( wayPoint& waypoint ) :
  QTreeWidgetItem(),  wp(waypoint)
{
  QPainter pnt;
  QPixmap selectIcon;

  //QString name = wp.name;
  //QRegExp blank("[ ]");
  //name.replace(blank, QString::null);
  //name = name.left(10);

  setText(0, wp.name);
  setText(1, wp.description);
  setText(2, wp.icao);

  selectIcon = QPixmap( 18, 18 );
  pnt.begin(&selectIcon);
  selectIcon.fill( Qt::white );
  pnt.drawPixmap(1, 1, _globalMapConfig->getPixmap(wp.type,false,true) );
  pnt.end();
  QIcon icon;
  icon.addPixmap( _globalMapConfig->getPixmap(wp.type,false,true) );
  icon.addPixmap( selectIcon, QIcon::Selected );
  setIcon( 0, icon );
}
