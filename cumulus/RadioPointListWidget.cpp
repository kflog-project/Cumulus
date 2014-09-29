/***********************************************************************
**
**   RadioPointListWidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "RadioPointListWidget.h"

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

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

RadioPointListWidget::RadioPointListWidget( QWidget *parent,
                                           bool showMovePage ) :
  ListWidgetParent( parent, showMovePage )
{
  setObjectName("RadioPointListWidget");
  list->setObjectName("RpTreeWidget");

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif
}

RadioPointListWidget::~RadioPointListWidget()
{
}

/** Clears and refills the airfield item list. */
void RadioPointListWidget::fillItemList()
{
  // call base class
  ListWidgetParent::fillItemList();

  list->setUpdatesEnabled(false);
  list->clear();

  configRowHeight();

  int rpListSize = _globalMapContents->getListLength( MapContents::RadioList );

  for( int item = 0; item < rpListSize; item++ )
    {
      RadioPoint* site = _globalMapContents->getElement( MapContents::RadioList, item );
      filter->addListItem( new _RadioPointItem(site) );
    }

  // sorting is done in filter->reset()
  resizeListColumns();

  filter->reset();
  list->setUpdatesEnabled(true);
}

/** Returns a pointer to the currently highlighted airfield. */
Waypoint* RadioPointListWidget::getCurrentWaypoint()
{
  // qDebug("RadioPointListWidget::getCurrentWaypoint()");
  QTreeWidgetItem* li = list->currentItem();

  if( li == 0 )
    {
      return 0;
    }

  _RadioPointItem* rpItem = dynamic_cast<_RadioPointItem *> (li);

  // May be null if the cast failed.
  if ( rpItem == static_cast<_RadioPointItem *> (0) )
    {
      return static_cast<Waypoint *> (0);
    }

  RadioPoint* site = rpItem->radioPoint;

  m_wp.name = site->getWPName();
  m_wp.wgsPoint = site->getWGSPosition();
  m_wp.elevation = site->getElevation();
  m_wp.projPoint = site->getPosition();
  m_wp.description = site->getName();
  m_wp.type = site->getTypeID();
  m_wp.elevation = site->getElevation();
  m_wp.icao = site->getICAO();
  m_wp.frequency = site->getFrequency();
  m_wp.comment = site->getAdditionalText();
  m_wp.country = site->getCountry();

  return &m_wp;
}

RadioPointListWidget::_RadioPointItem::_RadioPointItem(RadioPoint* site) :
  QTreeWidgetItem(), radioPoint(site)
{
  QString name = site->getWPName();
  // Limitation for name is set in Welt2000 to 8 characters
  setText(0, name);
  setText(1, site->getName());
  setText(2, site->getCountry());
  setTextAlignment(2, Qt::AlignCenter);
  setText(3, site->getComment());

  // set landing site type icon
  QPixmap pm = _globalMapConfig->getPixmap(site->getTypeID(), false, false);

  setIcon( 0, QIcon( pm) );
}
