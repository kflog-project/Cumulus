/***********************************************************************
**
**   AirfieldListWidget.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include "AirfieldListWidget.h"

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "airfield.h"
#include "calculator.h"
#include "generalconfig.h"
#include "mainwindow.h"
#include "mapconfig.h"
#include "TaskPointSelectionList.h"

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

AirfieldListWidget::AirfieldListWidget( QVector<enum MapContents::ListID> &itemList,
                                        QWidget *parent,
                                        bool showMovePage ) :
  ListWidgetParent( parent, showMovePage ),
  m_itemList(itemList)
{
  setObjectName("AirfieldListWidget");
  list->setObjectName("AfTreeWidget");

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
      int nr = _globalMapContents->getListLength( m_itemList.at(item) );

      for ( int i = 0; i < nr; i++ )
        {
          Airfield* site = dynamic_cast<Airfield *> (_globalMapContents->getElement( m_itemList.at(item), i ));

          if (site == 0)
            {
              continue;
            }

          filter->addListItem (new AirfieldItem (site));
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
  QTreeWidgetItem* li = list->currentItem();

  if ( li == 0 )
    {
      return 0;
    }

  AirfieldItem* afItem = dynamic_cast<AirfieldItem *> (li);

  // May be null if the cast failed.
  if ( afItem == static_cast<AirfieldItem *> (0) )
    {
      return static_cast<Waypoint *> (0);
    }

  Airfield* site = afItem->airfield;

  m_wp.name = site->getWPName();
  m_wp.wgsPoint = site->getWGSPosition();
  m_wp.elevation = site->getElevation();
  m_wp.projPoint = site->getPosition();
  m_wp.description = site->getName();
  m_wp.type = site->getTypeID();
  m_wp.elevation = site->getElevation();
  m_wp.icao = site->getICAO();
  m_wp.comment = site->getComment();
  m_wp.country = site->getCountry();
  m_wp.rwyList = site->getRunwayList();
  m_wp.frequencyList = site->getFrequencyList();

  return &m_wp;
}

AirfieldListWidget::AirfieldItem::AirfieldItem(Airfield* site) :
  QTreeWidgetItem(), airfield(site)
{
  QString name = site->getWPName();
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

  // set type icon
  QPixmap afPm = _globalMapConfig->getPixmap(site->getTypeID(), false);

  setIcon( 0, QIcon( afPm) );
}

/**
 * This method is called, if the search button is pressed;
 */
void AirfieldListWidget::searchButtonPressed()
{
  if( listWidget()->topLevelItemCount() == 0 )
    {
      // list is empty, return
      return;
    }

  TaskPointSelectionList* sl = new TaskPointSelectionList( this, tr("Airfields") );
  sl->setAttribute(Qt::WA_DeleteOnClose);
  sl->fillSelectionListWithAirfields();
  sl->resize( MainWindow::mainWindow()->size() );

  connect( sl, SIGNAL(takeThisPoint(const SinglePoint*)),
           this, SLOT(slot_SearchResult( const SinglePoint*)) );

  sl->show();
}

/**
 * This slot is called, to pass the search result.
 */
void AirfieldListWidget::slot_SearchResult( const SinglePoint* sp )
{
  // Make the whole list visible to ensure visibility of selection.
  resetListFilter();

  // qDebug() << "WPName:" << sp->getWPName();
  // qDebug() << "Name:" << sp->getName();

  QTreeWidget* lw = listWidget();

  for( int i = 0; i < lw->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* twi = lw->topLevelItem( i );
      const QString name = sp->getWPName();

      if( twi->text(0) == name )
        {
          lw->setCurrentItem( twi );
          lw->scrollToItem( twi, QAbstractItemView::PositionAtTop );
          break;
        }
    }
}
