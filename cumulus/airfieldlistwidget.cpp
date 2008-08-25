/***********************************************************************
**
**   airfieldlistwidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "airfieldlistwidget.h"

#include <QRegExp>
#include <QVBoxLayout>

#include "generalconfig.h"
#include "mapconfig.h"
#include "cucalc.h"
#include "airport.h"

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

AirfieldListWidget::AirfieldListWidget(QWidget *parent ) : WPListWidgetClass(parent)
{
  setObjectName("AirfieldListWidget");
  list->setObjectName("AFTreeWidget");

  wp = new wayPoint();

  itemList[0] = MapContents::AirportList;
  itemList[1] = MapContents::GliderSiteList;
  itemList[2] = MapContents::AddSitesList;
}


AirfieldListWidget::~AirfieldListWidget()
{
  // JD: Never forget to take ALL items out of the WP list !
  // Items are deleted in filter destructor
  while ( list->topLevelItemCount() > 0 )
    list->takeTopLevelItem(0);

  delete wp;
}

/** Retrieves the airfields from the mapcontents, and fills the list. */
void AirfieldListWidget::fillWpList()
{
  if( listFilled ) {
    return;
  }

  int Nr = 0;
  list->setUpdatesEnabled(false);
//  list->clear();
  configRowHeight();

  for( int item = 0; item<3; item++) {
    int nr = _globalMapContents->getListLength(itemList[item]);
    if( nr > Nr ) {
      Nr = nr;
    }
//    qDebug("fillWpList N: %d, items %d", item, nr );
    for(int i=0; i<nr; i++ ) {
      Airport* site = (Airport*)_globalMapContents->getElement( itemList[item], i );
      new _AirfieldItem(list, site);
    }
  }

  list->setSortingEnabled(true);
  list->sortByColumn(0,Qt::AscendingOrder);
  list->setSortingEnabled(false);

  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->resizeColumnToContents(2);

  if (Nr>0) {
    // @AP: set only to true if something was read
    listFilled = true;
  }

  filter->reset();
}


/** Returns a pointer to the currently highlighted airfield. */
wayPoint* AirfieldListWidget::getSelectedWaypoint()
{
  QTreeWidgetItem* li = list->currentItem();
  if ( li == 0)
    return 0;
  
  // Special rows selected?
  QString test = li->text(1);

  if (test == "Next Page" || test == "Previous Page")
    return 0;

  // Now we're left with the real waypoints/airports
  _AirfieldItem* apli = static_cast<_AirfieldItem*>(li);

  // @ee may be null if the cast failed.
  if (apli == 0)
    return 0;

  Airport* site = apli->airport;

  wp->name = site->getWPName();
  wp->origP = site->getWGSPosition();
  wp->elevation = site->getElevation();
  wp->projP = site->getPosition();
  wp->description = site->getName();
  wp->type = site->getTypeID();
  wp->elevation = site->getElevation();
  wp->icao = site->getICAO();
  wp->frequency = site->getFrequency().toDouble();
  wp->runway = site->getRunway(0).direction;
  wp->length = site->getRunway(0).length;
  wp->surface = site->getRunway(0).surface;
  wp->comment = "";
  wp->isLandable = true;
  return wp;
}

AirfieldListWidget::_AirfieldItem::_AirfieldItem(QTreeWidget* tw, Airport* site, int type):
  QTreeWidgetItem(tw, type), airport(site)
{
  QString name = site->getWPName();
  QRegExp blank("[ ]");
  name.replace(blank, QString::null);
  name = name.left(8);
  setText(0, name);
  setText(1, site->getName());
  setText(2, site->getICAO());
  // create landing site type icon
  setIcon( 0, _globalMapConfig->getListIcon(site->getTypeID()) );
}
