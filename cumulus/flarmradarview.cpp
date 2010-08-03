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
#include "flarmradarview.h"
#include "generalconfig.h"

/**
 * Constructor
 */
FlarmRadarView::FlarmRadarView( QWidget *parent ) :
  QWidget( parent ),
  display(0)
{
  setAttribute( Qt::WA_DeleteOnClose );
  setContentsMargins(-4,-8,-4,-8);

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(5);

  display = new FlarmDisplay( this );
  topLayout->addWidget( display, 2 );

  connect( Flarm::instance(), SIGNAL(newFlarmPflaaData()),
           display, SLOT(slot_UpdateDisplay()) );

  buttonBox = new QGroupBox( this );
  buttonBox->setContentsMargins(2,2,2,2);

  int size = 40;

  QPushButton *zoomButton  = new QPushButton;
  zoomButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("zoom32.png")));
  zoomButton->setIconSize(QSize(32, 32));
  zoomButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  zoomButton->setMinimumSize(size, size);
  zoomButton->setMaximumSize(size, size);

  QPushButton *listButton  = new QPushButton;
  listButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("list32.png")));
  listButton->setIconSize(QSize(32, 32));
  listButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  listButton->setMinimumSize(size, size);
  listButton->setMaximumSize(size, size);

  display->setUpdateInterval( 2 );
  updateButton = new QPushButton( "2s" );
  updateButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  updateButton->setMinimumSize(size, size);
  updateButton->setMaximumSize(size, size);

  QPushButton *closeButton = new QPushButton;
  closeButton->setIcon(QIcon(GeneralConfig::instance()->loadPixmap("cancel.png")));
  closeButton->setIconSize(QSize(32, 32));
  closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::QSizePolicy::Preferred);
  closeButton->setMinimumSize(size, size);
  closeButton->setMaximumSize(size, size);

  connect( zoomButton, SIGNAL(clicked() ), this, SLOT(slotZoom()) );
  connect( listButton, SIGNAL(clicked() ), this, SLOT(slotOpenListView()) );
  connect( updateButton, SIGNAL(clicked() ), this, SLOT(slotUpdateInterval()) );
  connect( closeButton, SIGNAL(clicked() ), this, SLOT(slotClose()) );

  // vertical box with operator buttons
  QVBoxLayout *vbox = new QVBoxLayout;

  vbox->setSpacing(0);
  vbox->addWidget( zoomButton );
  vbox->addSpacing(32);
  vbox->addWidget( listButton );
  vbox->addSpacing(32);
  vbox->addWidget( updateButton );
  vbox->addStretch(2);
  vbox->addWidget( closeButton );
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );
}

/**
 * Destructor
 */
FlarmRadarView::~FlarmRadarView()
{
}

/** Called if zoom level shall be changed. */
void FlarmRadarView::slotZoom()
{
  enum FlarmDisplay::Zoom zoom = display->getZoomLevel();

  if( zoom == FlarmDisplay::Low )
    {
      display->slot_SwitchZoom( FlarmDisplay::Middle );
    }
  else if( zoom == FlarmDisplay::Middle )
    {
      display->slot_SwitchZoom( FlarmDisplay::High );
    }
  else
    {
      display->slot_SwitchZoom( FlarmDisplay::Low );
    }
}

/** Called if list view button was pressed. */
void FlarmRadarView::slotOpenListView()
{
  emit openListView();
}

/** Called if close button was pressed. */
void FlarmRadarView::slotClose()
{
  // Ask FlarmWidget to close the widget.
  emit closeRadarView();
}

/** Called if update interval button was pressed. */
void FlarmRadarView::slotUpdateInterval()
{
  QString text = updateButton->text();

  QString newText = "2s";
  int newValue = 2;

  if( text == "1s" )
    {
      newText = "2s";
      newValue = 2;
    }
  else if( text == "2s" )
    {
      newText = "3s";
      newValue = 3;
    }
  else if( text == "3s" )
    {
      newText = "1s";
      newValue = 1;
    }

  updateButton->setText( newText );
  display->setUpdateInterval( newValue );
}
