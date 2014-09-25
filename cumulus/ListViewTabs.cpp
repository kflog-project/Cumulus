/***********************************************************************
**
**   ListViewTabs.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 Axel Pauli
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

#include "ListViewTabs.h"
#include "mapcontents.h"

ListViewTabs::ListViewTabs( QWidget* parent ) :
  QWidget( parent )
{
  setObjectName("ListViewTabs");
  setWindowFlags( Qt::Tool );
  setWindowModality( Qt::WindowModal );
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle( tr("Point Lists") );

  if( parent )
    {
      resize( parent->size() );
    }

  QVBoxLayout *layout = new QVBoxLayout( this );

  m_listViewTabs = new QTabWidget( this );
  m_listViewTabs->setObjectName("listViewTabs");
  layout->addWidget( m_listViewTabs );

  QVector<enum MapContents::MapContentsListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderfieldList;
  viewAF = new AirfieldListView( itemList, 0 );

  itemList.clear();
  itemList << MapContents::OutLandingList;
  viewOL = new AirfieldListView( itemList, 0 );

  viewRP = new ReachpointListView( 0 );
  viewTP = new TaskListView( 0 );
  viewWP = new WaypointListView( 0 );
}

ListViewTabs::~ListViewTabs()
{
  qDebug() << "~ListViewTabs()";
}

void ListViewTabs::showEvent( QShowEvent *event )
{
  qDebug() << "ListViewTabs::showEvent()";

  m_listViewTabs->clear();

  if( viewTP->topLevelItemCount() )
    {
      m_listViewTabs->addTab( viewTP, tr( "Task" ) );
    }

  m_listViewTabs->addTab( viewWP, tr( "Waypoints" ) );

  m_listViewTabs->addTab( viewRP, tr( "Reachable" ) );

  if( viewAF->topLevelItemCount() )
    {
      m_listViewTabs->addTab( viewAF, tr( "Airfields" ) );
    }

  if( viewOL->topLevelItemCount() )
    {
      m_listViewTabs->addTab( viewOL, tr( "Outlandings" ) );
    }

  QWidget::showEvent( event );
}
