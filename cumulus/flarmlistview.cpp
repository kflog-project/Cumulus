/***********************************************************************
**
**   flarmlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   V1.2
**
***********************************************************************/

#include <cmath>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef QTSCROLLER
#include <QtScroller>
#endif

#include "flarmlistview.h"
#include "flarmaliaslist.h"
#include "flarm.h"
#include "FlarmNet.h"
#include "generalconfig.h"
#include "distance.h"
#include "altitude.h"
#include "speed.h"
#include "calculator.h"
#include "distance.h"
#include "mapconfig.h"
#include "MainWindow.h"

QString FlarmListView::selectedListObject  = "";
QString FlarmListView::selectedFlarmObject = "";

/**
 * Constructor
 */
FlarmListView::FlarmListView( QWidget *parent ) :
  QWidget( parent ),
  list(0),
  rowDelegate(0)
{
  setAttribute( Qt::WA_DeleteOnClose );
  setWindowFlags( Qt::Tool );
  resize( MainWindow::mainWindow()->size() );

  QBoxLayout *topLayout = new QVBoxLayout( this );

  list = new QTreeWidget( this );
  list->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
  list->setHorizontalScrollMode( QAbstractItemView::ScrollPerPixel );

#ifdef QSCROLLER
  QScroller::grabGesture( list->viewport(), QScroller::LeftMouseButtonGesture );
#endif

#ifdef QTSCROLLER
  QtScroller::grabGesture( list->viewport(), QtScroller::LeftMouseButtonGesture );
#endif

  list->setRootIsDecorated(false);
  list->setItemsExpandable(false);
  list->setUniformRowHeights(true);
  list->setAlternatingRowColors(true);
  list->setSortingEnabled(false);
  list->setSelectionMode(QAbstractItemView::SingleSelection);
  list->setSelectionBehavior(QAbstractItemView::SelectRows);
  list->setColumnCount(11);
  list->hideColumn(0);
  list->setFocusPolicy(Qt::StrongFocus);

  QStringList sl;

  sl << tr("Hash")
     << tr("ID")
     << tr("H-Dist")
     << tr("V-Dist")
     << tr("Trk")
     << tr("Vg")
     << tr("CR")
     << tr("CS")
     << tr("MHz")
     << tr("Type")
     << "";

  list->setHeaderLabels(sl);

  QTreeWidgetItem* headerItem = list->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );
  headerItem->setTextAlignment( 5, Qt::AlignCenter );
  headerItem->setTextAlignment( 6, Qt::AlignCenter );
  headerItem->setTextAlignment( 7, Qt::AlignCenter );
  headerItem->setTextAlignment( 8, Qt::AlignCenter );
  headerItem->setTextAlignment( 9, Qt::AlignCenter );

  QFontMetrics qfm( font() );
  list->setColumnWidth( 4, qfm.height() );

  connect( list, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT(slot_ListItemClicked( QTreeWidgetItem*, int )) );

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
}

void FlarmListView::showEvent( QShowEvent *event )
{
  configRowHeight();

  // Set the list object to be selected to the current selected Flarm object.
  // List selection can be changed by user interaction.
  selectedListObject = selectedFlarmObject;
  fillItemList( selectedFlarmObject );
  list->setFocus();
  QWidget::showEvent( event );
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
void FlarmListView::fillItemList( QString& object2Select )
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

  int iconSize = QFontMetrics(font()).height() - 4;
  list->setIconSize( QSize(iconSize, iconSize) );

  QMutableHashIterator<QString, Flarm::FlarmAcft> it(flarmAcfts);

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

           climb += speed.getVerticalText( false, 1 );
         }

       // Try tp map the Flarm Id to an alias name
       const QHash<QString, QPair<QString, bool> >& aliasHash =
           FlarmAliasList::getAliasHash();

       // Try to map the Flarm Id to an alias name
       QString actfId = acft.ID.toUpper();
       bool look4Reg = true;

       if( aliasHash.contains( actfId ) )
         {
           actfId = aliasHash.value( actfId ).first;
           look4Reg = false;
         }

       // Try to load Flarmnet data
       GeneralConfig *conf = GeneralConfig::instance();
       QStringList fnd;

       if( conf->useFlarmNet() == true )
         {
           bool ok;
           uint fid = acft.ID.toUInt( &ok, 16);
           ok = FlarmNet::getData( fid, fnd );

           if( ok == true && look4Reg == true && fnd.at(0).size() > 0 )
             {
               // Kennzeichen instead of hex id
               actfId = fnd.at(0);
             }
         }

      // Add hash key as invisible column
      sl << it.key()
         << actfId
         << Distance::getText( distAcft, true, -1 )
         << vertical
         << "";

      if( acft.GroundSpeed != INT_MIN )
        {
          sl << Speed( acft.GroundSpeed ).getHorizontalText( false, 0 );
        }

      sl << climb;

      if( fnd.size() == 4 )
        {
          // display other Flarm data e.g. Type, WKZ, Frequenz
          ( fnd.at(2).size() > 0 ) ? sl << fnd.at(2) : sl << " ";
          ( fnd.at(3).size() > 0 ) ? sl << fnd.at(3) : sl << " ";
          ( fnd.at(1).size() > 0 ) ? sl << fnd.at(1) : sl << " ";
        }

      QTreeWidgetItem* item = new QTreeWidgetItem( sl );
      item->setTextAlignment( 1, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( 2, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 3, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 4, Qt::AlignCenter );
      item->setTextAlignment( 5, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 6, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( 7, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( 8, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( 9, Qt::AlignLeft|Qt::AlignVCenter );

      QPixmap pixmap;

      if( north == 0 && east == 0 )
        {
          // Special case Flarm object is above or below us. We draw a circle.
          MapConfig::createCircle( pixmap,
                                   iconSize,
                                   QColor(Qt::black),
                                   1.0 );
        }
      else
        {
          int alpha = static_cast<int> (rint(atan2( ((double) north), (double) east ) * 180. / M_PI));

          // correct angle because the different coordinate systems.
          int heading2Object = (360 - calculator->getLastHeading()) + (90 - alpha);

          // qDebug() << "ID=" << it.key() << "Alpha" << alpha << "H2O=" << heading2Object;
          MapConfig::createTriangle( pixmap,
                                     iconSize,
                                     QColor(Qt::black),
                                     heading2Object,
                                     1.0,
                                     QColor(Qt::cyan) );
        }

      QIcon qi;
      qi.addPixmap( pixmap );
      item->setIcon( 4, qi );

      list->addTopLevelItem( item );

      if( object2Select == it.key() )
        {
          // This item is the current selected one.
          list->setCurrentItem( item );
        }
    }

  list->sortByColumn ( 2, Qt::AscendingOrder );
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
      if( i != 4 )
        {
          list->resizeColumnToContents( i );
        }
    }
}

/**
 * This slot is called if the user clicks in a new row of the list. The new
 * list selection must be saved otherwise it will get lost during the next
 * update cycle.
 */
void FlarmListView::slot_ListItemClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( column )

  if( item )
    {
      selectedListObject = item->text( 0 );
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
      // Save the hash key of the selected object
      selectedFlarmObject = item->text( 0 );
      selectedListObject  = selectedFlarmObject;

      // Emit new object selection to FlarmDisplay.
      emit newObjectSelection( selectedFlarmObject );

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

      selectedFlarmObject = "";
      selectedListObject  = "";

      emit newObjectSelection( selectedFlarmObject );
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
      fillItemList( selectedListObject );
    }
}
