/***********************************************************************
**
**   airfieldlistwidget.cpp
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

#include "airfieldlistwidget.h"

#include <QtGui>

#include "generalconfig.h"
#include "mapconfig.h"
#include "calculator.h"
#include "airfield.h"

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

AirfieldListWidget::AirfieldListWidget( QVector<enum MapContents::MapContentsListID> &itemList,
                                        QWidget *parent,
                                        bool showMovePage ) :
                                        ListWidgetParent( parent, showMovePage )
{
  setObjectName("AirfieldListWidget");
  list->setObjectName("AfTreeWidget");

  this->itemList = itemList;

  // For outlandings we do display the comment instead of ICAO in the list view
  if( itemList.at(0) == MapContents::OutLandingList )
    {
      QTreeWidgetItem *headerItem = list->headerItem();
      headerItem->setText( 2, tr("Comment") );
    }
}

AirfieldListWidget::~AirfieldListWidget()
{
}

/** Clears and refills the airfield item list. */
void AirfieldListWidget::fillItemList()
{
  list->setUpdatesEnabled(false);
  list->clear();

  configRowHeight();

  for ( int item = 0; item < itemList.size(); item++ )
    {
      int nr = _globalMapContents->getListLength(itemList.at(item));

      // qDebug("fillItemList N: %d, items %d", item, nr );

      for (int i=0; i<nr; i++ )
        {
          Airfield* site = static_cast<Airfield *> (_globalMapContents->getElement( itemList.at(item), i ));
          filter->addListItem( new _AirfieldItem(site) );
        }
    }

  // sorting is done in filter->reset()
  resizeListColumns();

  filter->reset();
  list->setUpdatesEnabled(true);
}

/** Returns a pointer to the currently highlighted airfield. */
Waypoint* AirfieldListWidget::getCurrentWaypoint()
{
  // qDebug("AirfieldListWidget::getCurrentWaypoint()");
  QTreeWidgetItem* li = list->currentItem();

  if ( li == 0 )
    {
      return 0;
    }

  // Now we're left with the real airfields
  _AirfieldItem* afItem = static_cast<_AirfieldItem *> (li);

  // @ee may be null if the cast failed.
  if ( afItem == static_cast<_AirfieldItem *> (0) )
    {
      return static_cast<Waypoint *> (0);
    }

  Airfield* site = afItem->airfield;

  wp.name = site->getWPName();
  wp.origP = site->getWGSPosition();
  wp.elevation = site->getElevation();
  wp.projP = site->getPosition();
  wp.description = site->getName();
  wp.type = site->getTypeID();
  wp.elevation = site->getElevation();
  wp.icao = site->getICAO();
  wp.frequency = site->getFrequency().toDouble();
  wp.runway = site->getRunway().direction;
  wp.length = site->getRunway().length;
  wp.surface = site->getRunway().surface;
  wp.comment = site->getComment();
  wp.isLandable = true;

  return &wp;
}

AirfieldListWidget::_AirfieldItem::_AirfieldItem(Airfield* site) :
  QTreeWidgetItem(), airfield(site)
{
  QString name = site->getWPName();
  // QRegExp blank("[ ]");
  // name.replace(blank, QString::null);
  // Limitation is set in Welt2000 to 9 characters
  // name = name.left(8);
  setText(0, name);
  setText(1, site->getName());

  if( site->getTypeID() != BaseMapElement::Outlanding )
    {
      setText(2, site->getICAO());
    }
  else
    {
      setText(2, site->getComment());
    }

  // create landing site type icon
  setIcon( 0, _globalMapConfig->getListIcon(site->getTypeID()) );
}
