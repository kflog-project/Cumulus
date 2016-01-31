/***********************************************************************
**
**   ListViewTabs.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014-2016 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "AirfieldListWidget.h"
#include "generalconfig.h"
#include "ListViewTabs.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "RadioPointListWidget.h"
#include "SinglePointListWidget.h"

extern MapContents *_globalMapContents;

ListViewTabs::ListViewTabs( QWidget* parent ) :
  QWidget( parent )
{
  setObjectName("ListViewTabs");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setWindowTitle( tr("Point Lists") );

  // Don't set this flag!
  // setAttribute(Qt::WA_DeleteOnClose);

  if( parent )
    {
      resize( parent->size() );
    }

  m_clearTimer = new QTimer( this );
  m_clearTimer->setSingleShot( true );
  m_clearTimer->setInterval( 750 );
  connect( m_clearTimer, SIGNAL(timeout()), SLOT(slotClearTabs()) );

  QVBoxLayout *layout = new QVBoxLayout( this );

  m_listViewTabs = new QTabWidget( this );
  m_listViewTabs->setObjectName("listViewTabs");
  layout->addWidget( m_listViewTabs );

  QVector<enum MapContents::ListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderfieldList;
  viewAF = new PointListView( new AirfieldListWidget(itemList) );

  itemList.clear();
  itemList << MapContents::OutLandingList;
  viewOL = new PointListView( new AirfieldListWidget(itemList) );

  itemList.clear();
  itemList << MapContents::HotspotList;
  viewHS = new PointListView( new SinglePointListWidget(itemList) );

  itemList.clear();
  itemList << MapContents::RadioList;
  viewNA = new PointListView( new RadioPointListWidget(itemList) );

  viewRP = new ReachpointListView();
  viewTP = new TaskListView();
  viewWP = new WaypointListView();

  m_textAF = tr( "Airfields" );
  m_textHS = tr( "Hotspots" );
  m_textOL = tr( "Fields" );
  m_textNA = tr( "Navaids" );
  m_textRP = tr( "Reachables" );
  m_textTP = tr( "Task" );
  m_textWP = tr( "Waypoints" );

  connect( viewAF, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewHS, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewOL, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewNA, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewRP, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewTP, SIGNAL(done()), SLOT(slotDone()) );
  connect( viewWP, SIGNAL(done()), SLOT(slotDone()) );
}

ListViewTabs::~ListViewTabs()
{
  // qDebug() << "~ListViewTabs()";
}

void ListViewTabs::showEvent( QShowEvent *event )
{
  m_clearTimer->stop();
  m_listViewTabs->clear();

  GeneralConfig* conf = GeneralConfig::instance();

  // Check, if a flight task is activated.
  if( _globalMapContents->getCurrentTask() != static_cast<FlightTask *> (0) )
    {
      if( viewTP->topLevelItemCount() )
	{
	  m_listViewTabs->addTab( viewTP, m_textTP );
	}
    }

  // Check, if the nearest site calculation is activated.
  if( conf->getNearestSiteCalculatorSwitch() )
    {
      // Can be disabled by the user.
      m_listViewTabs->addTab( viewRP, m_textRP );
    }

  // The waypoint view is always added to have the waypoint editor available.
  m_listViewTabs->addTab( viewWP, m_textWP );

  // If the lists are not empty, we should add the list tabulator.
  // Note, that the list view can be empty. It is filled during the show event.
  if( _globalMapContents->getListLength( MapContents::AirfieldList ) > 0 ||
      _globalMapContents->getListLength( MapContents::GliderfieldList ) > 0 )
    {
      m_listViewTabs->addTab( viewAF, m_textAF );
    }

  // If the list is not empty, we should add the list tabulator.
  // Note, that the list view can be empty. It is filled during the show event.
  if( _globalMapContents->getListLength( MapContents::OutLandingList ) > 0 )
    {
      m_listViewTabs->addTab( viewOL, m_textOL );
    }

  // If the list is not empty, we should add the list tabulator.
  // Note, that the list view can be empty. It is filled during the show event.
  if( _globalMapContents->getListLength( MapContents::RadioList ) > 0 )
    {
      m_listViewTabs->addTab( viewNA, m_textNA );
    }

  // If the list is not empty, we should add the list tabulator.
  // Note, that the list view can be empty. It is filled during the show event.
  if( _globalMapContents->getListLength( MapContents::HotspotList ) > 0 )
    {
      m_listViewTabs->addTab( viewHS, m_textHS );
    }

  QWidget::showEvent( event );
}

void ListViewTabs::hideEvent( QHideEvent* event )
{
  m_clearTimer->start();
  emit hidingWidget();
  QWidget::hideEvent( event );
}

void ListViewTabs::slotClearTabs()
{
  // Clear list view tabs, that takes some time.
  m_listViewTabs->clear();
}

void ListViewTabs::setView( const int view )
{
  int idx = -1;

  // Look, if the desired widget is contained as page in the tab widget.
  switch( view )
    {
      case MainWindow::afView:
	idx = m_listViewTabs->indexOf( viewAF );
	break;

      case MainWindow::hsView:
        idx = m_listViewTabs->indexOf( viewHS );
        break;

      case MainWindow::naView:
        idx = m_listViewTabs->indexOf( viewNA );
        break;

      case MainWindow::olView:
        idx = m_listViewTabs->indexOf( viewOL );
        break;

      case MainWindow::rpView:
	idx = m_listViewTabs->indexOf( viewRP );
	break;

      case MainWindow::tpView:
	idx = m_listViewTabs->indexOf( viewTP );
	break;

      case MainWindow::wpView:
	idx = m_listViewTabs->indexOf( viewWP );
	break;

      default:
	break;
    }

  if( idx != -1 )
    {
      m_listViewTabs->setCurrentIndex( idx );
    }
}

void ListViewTabs::slotDone()
{
  // We remove all list and filter content before closing widget to spare memory.
  viewAF->listWidget()->slot_Done();
  viewHS->listWidget()->slot_Done();
  viewOL->listWidget()->slot_Done();
  viewNA->listWidget()->slot_Done();
  viewWP->listWidget()->slot_Done();
  hide();
}
