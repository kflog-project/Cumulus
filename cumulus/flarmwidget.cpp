/***********************************************************************
**
**   flarmwidget.h
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

#include <QtGui>

#include "flarm.h"
#include "flarmwidget.h"
#include "flarmlistview.h"
#include "flarmradarview.h"

/**
 * Constructor
 */
FlarmWidget::FlarmWidget( QWidget *parent ) :
  QWidget( parent ),
  radarView(0),
  listView(0)
{
}

FlarmWidget::~FlarmWidget()
{
}

void FlarmWidget::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // Start $PFLAA data collecting
  Flarm::setCollectPflaa( true );

  // Dynamically created
  if( radarView == static_cast<FlarmRadarView *> (0) )
    {
      radarView = new FlarmRadarView( this );
      radarView->resize( size() );
      radarView->setVisible( true );

      connect( radarView, SIGNAL(openListView() ), this, SLOT(slotOpenListView()) );
      connect( radarView, SIGNAL(closeRadarView() ), this, SLOT(slotCloseRadarView()) );
    }

  // Dynamically created
  if( listView == static_cast<FlarmListView *> (0) )
    {
      listView = new FlarmListView( this );
      listView->resize( size() );
      listView->setVisible( false );

      connect( listView, SIGNAL(closeListView() ), this, SLOT(slotCloseListView()) );
    }
}

/** Called if list view shall be opened with all Flarm objects. */
void FlarmWidget::slotOpenListView()
{
  radarView->setVisible( false );
  listView->setVisible( true );
}

/** Called if list view shall be closed with all Flarm objects. */
void FlarmWidget::slotCloseListView()
{
  radarView->setVisible( true );
  listView->setVisible( false );
}

/** Called if radar view shall be closed. */
void FlarmWidget::slotCloseRadarView()
{
  // Stop $PFLAA collecting
  Flarm::setCollectPflaa( false );

  // Destroy the radar view widget
  radarView->close();
  radarView = static_cast<FlarmRadarView *> (0);

  // Destroy the list view widget
  listView->close();
  listView = static_cast<FlarmListView *> (0);

  // hide this widget
  setVisible( false );

  // Informs MapView about closing.
  emit closed();
}
