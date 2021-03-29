/***********************************************************************
**
**   flarmwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2011 Axel Pauli
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
#include "flarmaliaslist.h"
/**
 * Constructor
 */
FlarmWidget::FlarmWidget( QWidget *parent ) :
  QWidget( parent ),
  radarView(0),
  listView(0),
  aliasList(0)
{
}

FlarmWidget::~FlarmWidget()
{
}

void FlarmWidget::showEvent( QShowEvent *event )
{
  // Start $PFLAA data collecting
  Flarm::setCollectPflaa( true );

  QFont fnt = font();
  fnt.setBold(true);

  // Dynamically created
  if( radarView == static_cast<FlarmRadarView *> (0) )
    {
      radarView = new FlarmRadarView( this );
      radarView->resize( size() );
      radarView->setVisible( true );

      connect( radarView, SIGNAL(openListView() ), this, SLOT(slotOpenListView()) );
      connect( radarView, SIGNAL(openAliasList() ), this, SLOT(slotOpenAliasList()) );
      connect( radarView, SIGNAL(closeRadarView() ), this, SLOT(slotCloseRadarView()) );
    }

  // Dynamically created
  if( listView == static_cast<FlarmListView *> (0) )
    {
      listView = new FlarmListView( this );
      listView->resize( size() );
      listView->setVisible( false );
      listView->setFont( fnt );

      connect( listView, SIGNAL(closeListView()), this, SLOT(slotCloseListView()) );

      FlarmDisplay* display = radarView->getDisplay();

      connect( listView, SIGNAL(newObjectSelection(QString)),
               display, SLOT(slot_SetSelectedObject(QString)) );

      connect( listView, SIGNAL(newObjectSelection(QString)),
               radarView, SLOT(slotShowAddButton(QString)) );

      connect( display, SIGNAL(newObjectSelection(QString)),
               listView, SLOT(slot_SetSelectedObject(QString)) );

      connect( display, SIGNAL(newObjectSelection(QString)),
               radarView, SLOT(slotShowAddButton(QString)) );
    }

  QWidget::showEvent( event );
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

/** Called if alias list shall be opened with all Flarm objects. */
void FlarmWidget::slotOpenAliasList()
{
  if( aliasList )
    {
      // Prevents multiple instances of alias list, if system works slow.
      return;
    }

  aliasList = new FlarmAliasList( this );
  // aliasList->resize( size() );

  FlarmDisplay* display = radarView->getDisplay();

  connect( aliasList, SIGNAL(newObjectSelection(QString)),
           display, SLOT(slot_SetSelectedObject(QString)) );

  connect( aliasList, SIGNAL(newObjectSelection(QString)),
           listView, SLOT(slot_SetSelectedObject(QString)) );

  connect( aliasList, SIGNAL(newObjectSelection(QString)),
           radarView, SLOT(slotShowAddButton(QString)) );

  connect( aliasList, SIGNAL(closed() ), this, SLOT(slotAliasListClosed()) );

  radarView->setVisible( false );
  aliasList->setVisible( true );
}

/** Called if alias list was closed with all Flarm objects. */
void FlarmWidget::slotAliasListClosed()
{
  radarView->setVisible( true );
  aliasList = static_cast<FlarmAliasList *>(0);
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
