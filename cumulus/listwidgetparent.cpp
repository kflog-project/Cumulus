/***********************************************************************
**
**   listwidgetparent.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008      by Josua Dietze
**                   2009-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
************************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "layout.h"
#include "listwidgetparent.h"
#include "generalconfig.h"
#include"TaskPointSelectionList.h"

ListWidgetParent::ListWidgetParent( QWidget *parent, bool showMovePage ) :
  QWidget(parent),
  m_enableScroller(0)
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

  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  // const int iconSize = Layout::iconSize( font() );

  searchButton = new QPushButton( this );
  searchButton->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "fernglas.png" ) ) );
  searchButton->setIconSize( QSize(Layout::getButtonSize(24), Layout::getButtonSize(24)) );
  searchButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Fixed );
  searchButton->setToolTip( tr( "Search and select a waypoint" ) );

  up = new QPushButton( this );
  up->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "up.png", true )));
  up->setIconSize( QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)) );
  up->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  up->setToolTip( tr("move page up") );

#if defined(QSCROLLER) || defined(QTSCROLLER)

  m_enableScroller = new QCheckBox("][");
  m_enableScroller->setCheckState( Qt::Checked );
  m_enableScroller->setMinimumHeight( Layout::getButtonSize(12) );

  connect( m_enableScroller, SIGNAL(stateChanged(int)),
	   this, SLOT(slot_scrollerBoxToggled(int)) );

#endif

  down = new QPushButton( this );
  down->setIcon( QIcon(GeneralConfig::instance()->loadPixmap( "down.png", true )));
  down->setIconSize( QSize(Layout::getButtonSize(12), Layout::getButtonSize(12)) );
  down->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred );
  down->setToolTip( tr("move page down") );

  QVBoxLayout* movePageBox = new QVBoxLayout;
  movePageBox->setSpacing( 0 );
  movePageBox->addWidget( up, 10 );

#if defined(QSCROLLER) || defined(QTSCROLLER)

  movePageBox->addSpacing( 15 * Layout::getIntScaledDensity() );
  movePageBox->addWidget( m_enableScroller, 0, Qt::AlignCenter );
  movePageBox->addSpacing( 15 * Layout::getIntScaledDensity() );

#endif

  movePageBox->addWidget( down, 10 );

  QHBoxLayout *hBox = new QHBoxLayout;
  hBox->addWidget( list );
  hBox->addLayout( movePageBox );

  QHBoxLayout *filterBox = new QHBoxLayout;
  filterBox->setSpacing( 0 );
  filterBox->addWidget( filter, 10 );
  filterBox->addSpacing( 10 * Layout::getIntScaledDensity() );
  filterBox->addWidget( searchButton );

  topLayout->addLayout( filterBox );
  topLayout->addLayout( hBox);

  if( showMovePage == false )
    {
      up->setVisible( false );
      down->setVisible( false );
    }

  connect( list, SIGNAL( itemClicked(QTreeWidgetItem*,int) ),
           this, SLOT( slot_listItemClicked(QTreeWidgetItem*,int) ) );

  connect( searchButton, SIGNAL(clicked()), this, SLOT( slot_searchButtonClicked()) );

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

void ListWidgetParent::fillItemList()
{
  // calculates the needed icon size
  QFontMetrics qfm( font() );
  int iconSize = qfm.height() - 8;

  // Sets the icon size of a list entry
  list->setIconSize( QSize(iconSize, iconSize) );
}

/**
 * Clears and refills the item list, if the widget is visible.
 */
void ListWidgetParent::refillItemList()
{
  slot_Done();

  if ( isVisible() )
    {
      // The list is filled only, if the widget is visible.
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
  if( up->isDown() )
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

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slot_PageUp()));
    }
}

/**
 * Move page down.
 */
void ListWidgetParent::slot_PageDown()
{
  if( down->isDown() )
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

      // Start repetition timer, to check, if button is longer pressed.
      QTimer::singleShot(300, this, SLOT(slot_PageDown()));
    }
}

void ListWidgetParent::slot_scrollerBoxToggled( int state )
{
  if( m_enableScroller == 0 )
    {
      return;
    }

  if( state == Qt::Checked )
    {

#ifdef QSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
      list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
      QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

#ifdef ANDROID
       // Reset scrollbar style sheet to default.
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( "" );
#endif

    }
  else if( state == Qt::Unchecked)
    {

#ifdef QSCROLLER
      QScroller::ungrabGesture( list->viewport() );
 #endif

#ifdef QTSCROLLER
       QtScroller::ungrabGesture( list->viewport() );
#endif

#ifdef ANDROID
       // Make the vertical scrollbar bigger for Android
       QScrollBar* lvsb = list->verticalScrollBar();
       lvsb->setStyleSheet( Layout::getCbSbStyle() );
#endif

    }
}

/**
 * Called, if the search button is clicked.
 */
void ListWidgetParent::slot_searchButtonClicked()
{
  // Check, if entries are contained in the list.
  if( list->topLevelItemCount() == 0 )
    {
      return;
    }

  // inform subscribers about the search button press.
  emit searchButtonClicked();

  // inform subclasses about the search button press.
  searchButtonPressed();
}
