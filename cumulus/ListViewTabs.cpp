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
  setWindowTitle( tr("Lists Overview") );

  if( parent )
    {
      resize( parent->size() );
    }

  QHBoxLayout *hbox = new QHBoxLayout( this );

  m_listViewTabs = new QTabWidget( this );
  m_listViewTabs->setObjectName("listViewTabs");
  hbox->addWidget( m_listViewTabs );

  QVector<enum MapContents::MapContentsListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderfieldList;
  viewAF = new AirfieldListView( itemList, this );

  itemList.clear();
  itemList << MapContents::OutLandingList;
  viewOL = new AirfieldListView( itemList, this );

  viewRP = new ReachpointListView( this );
  viewTP = new TaskListView( this );
  viewWP = new WaypointListView( this );
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

  m_listViewTabs->addTab( viewAF, tr( "Airfields" ) );
  m_listViewTabs->addTab( viewRP, tr( "Reachable" ) );

  if( viewOL->topLevelItemCount() )
    {
      m_listViewTabs->addTab( viewOL, tr( "Outlandings" ) );
    }

  QWidget::showEvent( event );
}
