/***********************************************************************
**
**   listwidgetparent.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************/

#include <QtGui>

#include "layout.h"
#include "listwidgetparent.h"
#include "generalconfig.h"

ListWidgetParent::ListWidgetParent( QWidget *parent, bool showMovePage ) :
  QWidget(parent)
{
  setObjectName("ListWidgetParent");

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setContentsMargins( 0, 0, 0, 0  );

  list = new QTreeWidget( this );
  list->setObjectName("WpListWidgetParent");
  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setColumnCount(4);
  list->setAllColumnsShowFocus(true);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setSelectionBehavior(QAbstractItemView::SelectRows);
  list->setFocusPolicy( Qt::StrongFocus );

  QStringList sl;
  sl << tr("Name") << tr("Description") << tr("Country") << tr("ICAO");
  list->setHeaderLabels(sl);
  list->setFocus();

  filter = new ListViewFilter( list, this );
  filter->setObjectName( "ListViewFilter" );
#ifdef ANDROID
  list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  QScroller::grabGesture(list, QScroller::LeftMouseButtonGesture);
#endif

  up = new QPushButton( this );
  up->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png")) );
  up->setIconSize( QSize(IconSize, IconSize) );
  up->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  up->setToolTip( tr("move page up") );

  down = new QPushButton( this );
  down->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png")) );
  down->setIconSize( QSize(IconSize, IconSize) );
  down->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  down->setToolTip( tr("move page down") );

  QVBoxLayout* movePageBox = new QVBoxLayout;
  movePageBox->setSpacing( 0 );
  movePageBox->addWidget( up, 10 );
  movePageBox->addSpacing( 10 );
  movePageBox->addWidget( down, 10 );

  QHBoxLayout *hBox = new QHBoxLayout;

  hBox->addWidget( list );
  hBox->addLayout( movePageBox );

  topLayout->addWidget( filter );
  topLayout->addLayout( hBox);

  if( showMovePage == false )
    {
      up->setVisible( false );
      down->setVisible( false );
    }

  connect( list, SIGNAL( itemClicked(QTreeWidgetItem*,int) ),
           this, SLOT( slot_listItemClicked(QTreeWidgetItem*,int) ) );

  connect( up, SIGNAL(pressed()), this, SLOT(slot_PageUp()) );
  connect( down, SIGNAL(pressed()), this, SLOT(slot_PageDown()) );

  rowDelegate   = 0;
  firstLoadDone = false;
}

ListWidgetParent::~ListWidgetParent()
{
  delete filter;
}

void ListWidgetParent::showEvent( QShowEvent *event )
{
  Q_UNUSED(event)

  // load list items during first show
  if( firstLoadDone == false )
    {
      firstLoadDone = true;
      fillItemList();
    }

  // align columns to contents before showing
  resizeListColumns();
  list->setFocus();
}

/**
 * Clears and refills the item list, if items are loaded. Called
 * if the map projection has been changed to ensure an update of the
 * projected coordinates.
 */
void ListWidgetParent::refillItemList()
{
  if ( list->topLevelItemCount() > 0 )
    {
      fillItemList();
    }
}

void ListWidgetParent::configRowHeight()
{
  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();

  if ( rowDelegate )
    {
      rowDelegate->setVerticalMargin( afMargin );
    }
  else
    {
      rowDelegate = new RowDelegate( list, afMargin );
      list->setItemDelegate( rowDelegate );
    }
}

/** This slot is called from parent when closing. */
void ListWidgetParent::slot_Done()
{
  // Remove all list and filter items.
  filter->clear();
  list->clear();
  firstLoadDone = false;
}

/** This slot sends a signal to indicate that a selection has been made. */
void ListWidgetParent::slot_listItemClicked(QTreeWidgetItem* li, int)
{
  // qDebug("ListWidgetParent::slot_listItemClicked");
  if( li == 0)
    {
      return;
    }

  emit wpSelectionChanged();
}

/**
 * Move page up.
 */
void ListWidgetParent::slot_PageUp()
{
  QTreeWidgetItem *item = list->currentItem();

  if( item )
    {
      QModelIndex index = list->currentIndex();
      QRect rect = list->visualRect( index );

      // Calculate rows per page. Headline must be subtracted.
      int pageRows = ( list->height() / rect.height() ) - 1;

      int itemIdx = list->indexOfTopLevelItem( item );
      int newIdx  = itemIdx - pageRows;

      if( filter->activeFilter() )
        {
          if( newIdx < filter->activeFilter()->beginIdx )
            {
              newIdx = filter->activeFilter()->beginIdx;
            }

          list->setCurrentItem( list->topLevelItem(newIdx) );
          list->scrollToItem( list->topLevelItem(newIdx) );
        }
    }
}

/**
 * Move page down.
 */
void ListWidgetParent::slot_PageDown()
{
  QTreeWidgetItem *item = list->currentItem();

  if( item )
    {
      QModelIndex index = list->currentIndex();
      QRect rect = list->visualRect( index );

      // Calculate rows per page. Headline must be subtracted.
      int pageRows = ( list->height() / rect.height() ) - 1;

      int itemIdx = list->indexOfTopLevelItem( item );
      int newIdx  = itemIdx + pageRows;

      if( filter->activeFilter() )
        {
          if( newIdx >= filter->activeFilter()->endIdx )
            {
              newIdx = filter->activeFilter()->endIdx - 1;
            }

          list->setCurrentItem( list->topLevelItem(newIdx) );
          list->scrollToItem( list->topLevelItem(newIdx) );
        }
    }
}
