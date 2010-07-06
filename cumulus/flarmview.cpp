/***********************************************************************
**
**   flarmview.cpp
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

#include "flarmdisplay.h"
#include "flarmview.h"

/**
 * Constructor
 */
FlarmView::FlarmView( QWidget *parent ) :
  QWidget( parent )
{
  qDebug( "FlarmView window size is width=%d x height=%d",
          parent->size().width(),
          parent->size().height() );

  setContentsMargins(-4,-8,-4,-8);

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  display = new FlarmDisplay( this );
  topLayout->addWidget( display, 2 );

  QGroupBox* buttonBox = new QGroupBox( this );

  QPushButton *zoomButton  = new QPushButton( tr("Zoom") );
  QPushButton *listButton  = new QPushButton( tr("List") );
  QPushButton *closeButton = new QPushButton( tr("Close") );

  connect( zoomButton, SIGNAL(clicked() ), this, SLOT(slotZoom()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slotClosed()) );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget( zoomButton );
  vbox->addWidget( listButton );
  vbox->addWidget( closeButton );
  vbox->addStretch(1);
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );
}

/**
 * Destructor
 */
FlarmView::~FlarmView()
{
}

/** Called to report widget closing. */
void FlarmView::slotClosed()
{
  setVisible( false );
  display->slotResetBackground();
  emit closed();
}

/** Called if zoom level shall be changed. */
void FlarmView::slotZoom()
{
  enum FlarmDisplay::Zoom zoom = display->getZoomLevel();

  if( zoom == FlarmDisplay::Low )
    {
      display->slotSwitchZoom( FlarmDisplay::Middle );
    }
  else if( zoom == FlarmDisplay::Middle )
    {
      display->slotSwitchZoom( FlarmDisplay::High );
    }
  else
    {
      display->slotSwitchZoom( FlarmDisplay::Low );
    }
}
