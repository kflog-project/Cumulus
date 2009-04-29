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
#include "calculator.h"
#include "airfield.h"

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

AirfieldListWidget::AirfieldListWidget(QWidget *parent ) : WpListWidgetParent(parent)
{
  setObjectName("AirfieldListWidget");
  list->setObjectName("AFTreeWidget");

  wp = new wayPoint();

  itemList[0] = MapContents::AirfieldList;
  itemList[1] = MapContents::GliderSiteList;
}


AirfieldListWidget::~AirfieldListWidget()
{
  // JD: Never forget to take ALL items out of the WP list !
  // Items are deleted in filter destructor
  while ( list->topLevelItemCount() > 0 )
    list->takeTopLevelItem(0);

  delete wp;
}

/** Clears and refills the airfield item list, if the list is not empty. */

void AirfieldListWidget::refillWpList()
{
//  qDebug("AirfieldListWidget::refillWpList()");
  if( ! listFilled ) {
    // list is empty, ignore request
    return;
  }

  // Remove all content from list widget; deleting is done in filter
  while ( list->topLevelItemCount() > 0)
    list->takeTopLevelItem(0);

  filter->clear();

  listFilled = false;

  // reload list
  fillWpList();
}


/** Retrieves the airfields from the map contents, and fills the list. */
void AirfieldListWidget::fillWpList()
{
//  qDebug("AirfieldListWidget::fillWpList()");
  if( listFilled ) {
    return;
  }

  list->setUpdatesEnabled(false);
  configRowHeight();

  for( int item = 0; item < 3; item++) {
    int nr = _globalMapContents->getListLength(itemList[item]);

//    qDebug("fillWpList N: %d, items %d", item, nr );
    for(int i=0; i<nr; i++ ) {
      Airfield* site = static_cast<Airfield *> (_globalMapContents->getElement( itemList[item], i ));
      filter->addListItem( new _AirfieldItem(site) );
    }
  }
  // sorting is done in filter->reset()

  filter->reset();
  resizeListColumns();
  list->setUpdatesEnabled(true);

  listFilled = true;
}


/** Returns a pointer to the currently highlighted airfield. */
wayPoint* AirfieldListWidget::getSelectedWaypoint()
{
//  qDebug("AirfieldListWidget::getSelectedWaypoint()");
  QTreeWidgetItem* li = list->currentItem();
  if ( li == 0)
    return 0;

  // Special rows selected?
  QString test = li->text(1);

  if ( test == ListViewFilter::NextPage || test == ListViewFilter::PreviousPage )
    return 0;

  // Now we're left with the real waypoints/airports
  _AirfieldItem* apli = static_cast<_AirfieldItem*>(li);

  // @ee may be null if the cast failed.
  if (apli == 0)
    return 0;

  Airfield* site = apli->airport;

  wp->name = site->getWPName();
  wp->origP = site->getWGSPosition();
  wp->elevation = site->getElevation();
  wp->projP = site->getPosition();
  wp->description = site->getName();
  wp->type = site->getTypeID();
  wp->elevation = site->getElevation();
  wp->icao = site->getICAO();
  wp->frequency = site->getFrequency().toDouble();
  wp->runway = site->getRunway().direction;
  wp->length = site->getRunway().length;
  wp->surface = site->getRunway().surface;
  wp->comment = site->getComment();
  wp->isLandable = true;
  return wp;
}

AirfieldListWidget::_AirfieldItem::_AirfieldItem(Airfield* site):
  QTreeWidgetItem(), airport(site)
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
