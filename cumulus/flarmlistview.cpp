/***********************************************************************
**
**   flarmlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "flarmlistview.h"
#include "flarm.h"
#include "generalconfig.h"
#include "distance.h"
#include "altitude.h"
#include "speed.h"
#include "calculator.h"
#include "distance.h"
#include "mapconfig.h"

/**
 * Constructor
 */
FlarmListView::FlarmListView( QWidget *parent ) :
  QWidget( parent ),
  list(0),
  rowDelegate(0)
{
  setAttribute( Qt::WA_DeleteOnClose );

  QBoxLayout *topLayout = new QVBoxLayout( this );

  list = new QTreeWidget( this );

  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setSelectionBehavior(QAbstractItemView::SelectRows);
  list->setColumnCount(8);
  list->hideColumn(0);
  list->setFocusPolicy(Qt::StrongFocus);

  QStringList sl;

  sl << tr("Hash")
     << tr("ID")
     << tr("Distance")
     << tr("Vertical")
     << tr("DR")
     << tr("Speed")
     << tr("Climb")
     << " ";

  list->setHeaderLabels(sl);

  topLayout->addWidget( list, 10 );
  QBoxLayout *buttonrow = new QHBoxLayout;
  topLayout->addLayout( buttonrow );

  QPushButton *cmdClose = new QPushButton( tr( "Close" ), this );
  buttonrow->addWidget( cmdClose );

  QPushButton *cmdSelect = new QPushButton( tr( "Select" ), this );
  buttonrow->addWidget( cmdSelect );

  QPushButton *cmdUnselect = new QPushButton( tr( "Unselect" ), this );
  buttonrow->addWidget( cmdUnselect );

  connect( cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()) );
  connect( cmdUnselect, SIGNAL(clicked()), this, SLOT(slot_Unselect()) );
  connect( cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()) );

  connect( Flarm::instance(), SIGNAL(newFlarmPflaaData()),
           this, SLOT(slot_Update()) );
}

/**
 * Destructor
 */
FlarmListView::~FlarmListView()
{
  qDebug() << "FlarmListView::~FlarmListView()";
}

void FlarmListView::showEvent( QShowEvent *event )
{
  qDebug() << "FlarmListView::showEvent";

  Q_UNUSED( event )

  configRowHeight();
  fillItemList();
  list->setFocus();
}

void FlarmListView::configRowHeight()
{
  // set new row height from configuration
  int afMargin = GeneralConfig::instance()->getListDisplayAFMargin();

  if( rowDelegate )
    {
      rowDelegate->setVerticalMargin( afMargin );
    }
  else
    {
      rowDelegate = new RowDelegate( list, afMargin );
      list->setItemDelegate( rowDelegate );
    }
}

/**
 * Fills the item list with their data.
 */
void FlarmListView::fillItemList()
{
  list->clear();

  // Here starts the Flarm object analysis and drawing
  QHash<QString, Flarm::FlarmAcft> flarmAcfts = Flarm::getPflaaHash();

  if( flarmAcfts.size() == 0 )
    {
      // hash is empty
      resizeListColumns();
      return;
    }

  QMutableHashIterator<QString, Flarm::FlarmAcft> it(flarmAcfts);

  QString selection = "";

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      Flarm::FlarmAcft& acft = it.value();

      QStringList sl;

      int north = acft.RelativeNorth;
      int east  = acft.RelativeEast;

      double distAcft = sqrt( north*north + east*east);

      QString vertical = "";

      // Calculate the relative vertical separation
      if( acft.RelativeVertical > 0 )
        {
          // prefix positive value with a plus sign
          vertical = "+";
        }

      vertical += Altitude::getText( acft.RelativeVertical, true, 0 );

      QString climb = "";

      // Calculate climb rate, if available
       if( acft.ClimbRate != INT_MIN )
         {
           Speed speed(acft.ClimbRate);

           if( acft.ClimbRate > 0 )
             {
               // prefix positive value with a plus sign
               climb = "+";
             }

           climb += speed.getVerticalText( true, 1 );
         }

      // Add hash key as invisible column
      sl << it.key()
         << acft.ID
         << Distance::getText( distAcft, true, -1 )
         << vertical
         << ""
         << Speed( acft.GroundSpeed ).getHorizontalText( false )
         << climb;

      QTreeWidgetItem* item = new QTreeWidgetItem( sl );
      item->setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 4, Qt::AlignCenter );
      item->setTextAlignment( 5, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 6, Qt::AlignRight|Qt::AlignVCenter );

      double alpha = atan2( ((double) north), (double) east ) * 180. / M_PI;

      // correct angle because the different coordinate systems.
      alpha = 90 - alpha;

      // qDebug() << "ID=" << it.key() << "Alpha" << alpha;

      QPixmap pixmap;

      MapConfig::createTriangle( pixmap, this->font().pointSize() + 4,
                                 QColor(Qt::black), alpha, 1.0 );
      QIcon qi;
      qi.addPixmap( pixmap );
      item->setIcon( 4, qi );

      list->addTopLevelItem( item );
    }

  list->sortByColumn ( 2, Qt::AscendingOrder );

  list->setCurrentItem(list->topLevelItem(0));
  resizeListColumns();
}

/**
 * aligns the columns to their contents
 */
void FlarmListView::resizeListColumns()
{
  int count = list->columnCount();

  for( int i = 0; i < count; i++ )
    {
      list->resizeColumnToContents( i );
    }
}

/**
 * This slot is called to indicate that a selection has been made.
 */
void FlarmListView::slot_Select()
{
  QTreeWidgetItem* item = list->currentItem();

  if( item )
    {
      emit newObjectSelection( item->text( 0 ) );

      // Request closing widget.
      emit closeListView();
    }
}

/**
 * This slot is called to make a deselection of the selected row.
 */
void FlarmListView::slot_Unselect()
{
  if( list->topLevelItemCount() )
    {
      list->clearSelection();
      emit newObjectSelection( "" );
    }
}

/**
 * This slot is called when the list view should be closed without selection.
 */
void FlarmListView::slot_Close()
{
  // Requests FlarmWidget to close the widget.
  emit closeListView();
}

/**
 * Called if new Flarm data are available.
 */
void FlarmListView::slot_Update()
{
  static int interval = 0;

  if( isVisible() == false )
    {
      // widget is not visible, do nothing in this case.
      return;
    }

  if( ++interval % 3 == 0 )
    {
      // Update all 3s the list.
      fillItemList();

      qDebug() << "FlarmListView::slot_update()";
    }
}
