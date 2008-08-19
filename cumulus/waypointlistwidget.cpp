/***********************************************************************
**
**   waypointlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andre Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/


#include "waypointlistwidget.h"

#include <QPushButton>
#include <QMessageBox>

#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "wpeditdialog.h"

extern MapContents* _globalMapContents;
extern MapConfig* _globalMapConfig;


WaypointListWidget::WaypointListWidget(QWidget *parent) : WPListWidgetClass(parent)
{
  setObjectName("WaypointListWidget");
  list->setObjectName("WPTreeWidget");
}


WaypointListWidget::~WaypointListWidget()
{
  // JD: Never forget to take ALL items out of the list !
  // Items are deleted in filter destructor
  while ( list->topLevelItemCount() > 0 )
    list->takeTopLevelItem(0);
}


/** Retrieves waypoints or airfields from the mapcontents, and fills the list. */
void WaypointListWidget::fillWpList()
{
  QList<wayPoint*> *wpList = _globalMapContents->getWaypointList();

  list->setUpdatesEnabled(false);
//  list->clear();
  configRowHeight();

  int n = 0;

  if( wpList ) {
    n = wpList->count();
    //qDebug("WaypointListWidget::fillWpList() %d", n);
    
    for (int i=0; i < n; i++) {
      wayPoint * wp=(wayPoint*)wpList->at(i);
      new _WaypointItem(list, wp);
    }
  }

  list->setSortingEnabled(true);
  list->sortByColumn(0,Qt::AscendingOrder);
  list->setSortingEnabled(false);

  if ( n>0 ) {
    list->setCurrentItem(list->topLevelItem(0));
//from airfieldlistview:
    // @AP: set only to true if something was read
//    if(list == WaypointListWidget::list) {
//      listFilled = true;
//    }
  }


  if(filter == WaypointListWidget::filter) {
    filter->reset();
  } else {
    filter->reset(true);
  }
  list->setUpdatesEnabled(true);
}


/** Returns a pointer to the currently highlighted waypoint. */
wayPoint* WaypointListWidget::getSelectedWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();
  if ( li == 0)
    return 0;
  
  // Special rows selected?
  QString test = li->text(1);

  if (test == "Next Page" || test == "Previous Page")
    return 0;

  // Now we're left with the real waypoints/airports
  _WaypointItem* wpi = static_cast<_WaypointItem*>(li);

  if (!wpi)
    return 0;

  return wpi->wp;
}


/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListWidget::deleteSelectedWaypoint()
{
  QTreeWidgetItem * li = list->currentItem();
  if ( li== 0)
    return;

  wayPoint* w = getSelectedWaypoint();
  filter->restoreListViewItems();

  // remove from list

  // remove from waypointlist in MapContents
  _globalMapContents->getWaypointList()->removeAll( w );
  // save the modified catalog
  _globalMapContents->saveWaypointList();
  delete list->takeTopLevelItem( list->currentIndex().row() );

  filter->reset(true);
}


/** Called if a waypoint has been edited. */
void WaypointListWidget::updateSelectedWaypoint(wayPoint* wp)
{
  QTreeWidgetItem * li = list->currentItem();
  if ( li== 0 )
    return;

  if( wp == 0 ) {
    qDebug("WaypointListWidget::updateSelectedWaypoint: empty waypoint given");
    return;
  }

  li->setText(0, wp->name);
  li->setText(1, wp->description);
  li->setText(2, wp->icao);
  li->setIcon(0, QIcon(_globalMapConfig->getPixmap(wp->type,false,true)));
  list->sortByColumn(0);
  filter->reset();

  // save modified catalog
  _globalMapContents->saveWaypointList();
}


/** Called if a waypoint has been added. */
void WaypointListWidget::addWaypoint(wayPoint * wp)
{
  if( wp == 0 ) {
    qDebug("WaypointListWidget::updateSelectedWaypoint: empty waypoint given");
    return;
  }

  wayPoint* newWp = new wayPoint(*wp);
  new _WaypointItem(list, newWp);
//  list->sortByColumn(0);
  filter->reset();

  // qDebug("WaypointListWidget::addWaypoint: name=%s", wp->name.toLatin1().data() );

  _globalMapContents->getWaypointList()->append(newWp);
  // save the modified catalog
  _globalMapContents->saveWaypointList();
}


WaypointListWidget::_WaypointItem::_WaypointItem(QTreeWidget* tw, wayPoint* waypoint):
  QTreeWidgetItem(tw),  wp(waypoint)
{
  if (!wp)
    return;

  QPainter pnt;
  QPixmap selectIcon;

  QString name = wp->name;
  QRegExp blank("[ ]");
  //name.replace(blank, QString::null);
  name = name.left(10);

  setText(0, name);
  setText(1, wp->description);
  setText(2, wp->icao);

  selectIcon = QPixmap(18,18);
  pnt.begin(&selectIcon);
  selectIcon.fill( Qt::white );
  pnt.drawPixmap(1, 1, _globalMapConfig->getPixmap(wp->type,false,true) );
  pnt.end();
  QIcon icon;
  icon.addPixmap( _globalMapConfig->getPixmap(wp->type,false,true) );
  icon.addPixmap( selectIcon, QIcon::Selected );
  setIcon( 0, icon );
//  setIcon(0, QIcon(_globalMapConfig->getPixmap(wp->type,false,true)));
}
