/***********************************************************************
**
**   airfieldlistwidget.cpp
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

#include "airfieldlistwidget.h"

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

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

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  this->m_itemList = itemList;

  // For outlandings we do display the comment instead of ICAO in the list view
  if( itemList.at(0) == MapContents::OutLandingList )
    {
      QTreeWidgetItem *headerItem = list->headerItem();
      headerItem->setText( 3, tr("Comment") );
    }
}

AirfieldListWidget::~AirfieldListWidget()
{
}

/** Clears and refills the airfield item list. */
void AirfieldListWidget::fillItemList()
{
  // call base class
  ListWidgetParent::fillItemList();

  list->setUpdatesEnabled(false);
  list->clear();

  configRowHeight();

  for ( int item = 0; item < m_itemList.size(); item++ )
    {
      int nr = _globalMapContents->getListLength(m_itemList.at(item));

      // qDebug("fillItemList N: %d, items %d", item, nr );

      for (int i=0; i<nr; i++ )
        {
          Airfield* site = static_cast<Airfield *> (_globalMapContents->getElement( m_itemList.at(item), i ));
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

  m_wp.name = site->getWPName();
  m_wp.origP = site->getWGSPosition();
  m_wp.elevation = site->getElevation();
  m_wp.projP = site->getPosition();
  m_wp.description = site->getName();
  m_wp.type = site->getTypeID();
  m_wp.elevation = site->getElevation();
  m_wp.icao = site->getICAO();
  m_wp.frequency = site->getFrequency();
  m_wp.runway = site->getRunway().direction;
  m_wp.length = site->getRunway().length;
  m_wp.surface = site->getRunway().surface;
  m_wp.comment = site->getComment();
  m_wp.isLandable = true;
  m_wp.country = site->getCountry();

  return &m_wp;
}

AirfieldListWidget::_AirfieldItem::_AirfieldItem(Airfield* site) :
  QTreeWidgetItem(), airfield(site)
{
  QString name = site->getWPName();
  // Limitation for name is set in Welt2000 to 8 characters
  setText(0, name);
  setText(1, site->getName());
  setText(2, site->getCountry());
  setTextAlignment(2, Qt::AlignCenter);

  if( site->getTypeID() != BaseMapElement::Outlanding )
    {
      setText(3, site->getICAO());
    }
  else
    {
      setText(3, site->getComment());
    }

  // set landing site type icon
  QPixmap afPm = _globalMapConfig->getPixmap(site->getTypeID(), false, false);

  setIcon( 0, QIcon( afPm) );
}
