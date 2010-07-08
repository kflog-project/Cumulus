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

#include "flarm.h"
#include "flarmdisplay.h"
#include "flarmview.h"
#include "gpsnmea.h"

/**
 * Constructor
 */
FlarmView::FlarmView( QWidget *parent ) :
  QWidget( parent )
{
  setContentsMargins(-4,-8,-4,-8);

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  display = new FlarmDisplay( this );
  topLayout->addWidget( display, 2 );

  connect( Flarm::instance(), SIGNAL(newFlarmPflaaData()),
           display, SLOT(slotUpdateDisplay()) );

  QGroupBox* buttonBox = new QGroupBox( this );

  QPushButton *zoomButton  = new QPushButton( tr("Zoom") );
  QPushButton *listButton  = new QPushButton( tr("List") );
  QPushButton *closeButton = new QPushButton( tr("Close") );

  connect( zoomButton, SIGNAL(clicked() ), this, SLOT(slotZoom()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slotClosed()) );

  // vertical box with operator buttons
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

void FlarmView::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  // Start $PFLAA data collecting
  Flarm::setCollectPflaa( true );
}


/** Called to report widget closing. */
void FlarmView::slotClosed()
{
  // hide widget
  setVisible( false );

  // Stop $PFLAA collecting
  Flarm::setCollectPflaa( false );

  // Informs others about widget closing.
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
