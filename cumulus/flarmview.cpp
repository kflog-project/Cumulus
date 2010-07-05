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

#include "flarmview.h"

/**
 * Constructor
 */
FlarmView::FlarmView( QWidget *parent ) :
  QWidget( parent ),
  zoomLevel(FlarmView::Low)
{
  qDebug( "FlarmView window size is width=%d x height=%d",
          parent->size().width(),
          parent->size().height() );

  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing(0);

  screen = new QWidget( this );
  screen->setContentsMargins(-9,-9,-9,-9);
  screen->setFixedWidth( parent->size().width() );

  screen->setAutoFillBackground(true);
  screen->setBackgroundRole(QPalette::Window);
  screen->setPalette( QPalette(QColor(Qt::lightGray)) );

  topLayout->addWidget( screen );

  QGroupBox* buttonBox = new QGroupBox( this );

  QPushButton *zoomButton = new QPushButton( tr("Zoom") );
  QPushButton *listButton = new QPushButton( tr("List") );
  QPushButton *closeButton = new QPushButton( tr("Close") );

  QVBoxLayout *vbox = new QVBoxLayout;
  vbox->addWidget( zoomButton );
  vbox->addWidget( listButton );
  vbox->addWidget( closeButton );
  // vbox->addStretch(1);
  buttonBox->setLayout( vbox );

  topLayout->addWidget( buttonBox );
}

/**
 * Destructor
 */
FlarmView::~FlarmView()
{
}

/** Creates the base picture with the radar screen. */
void FlarmView::createBasePicture()
{
  basePicture = QPixmap( screen->size() );
  basePicture.fill( Qt::lightGray );

  double r1; // inner radius in meters
  double r2; // outer radius in meters

  // calculate the resolution according to the zoom level
  switch( zoomLevel )
    {
      case FlarmView::Low:
        r1 = 250;
        r2 = 500;
        break;
      case FlarmView::Middle:
        r1 = 500;
        r2 = 1000;
        break;
      case FlarmView::High:
        r1 = 2500;
        r2 = 5000;
        break;
      default:
        r1 = 250;
        r2 = 500;
    }

  // define a margin
  const int margin = 5;

  QPainter painter( &basePicture );

  painter.translate( margin, margin );

  QPen pen(Qt::black);
  pen.setWidth(3);
  painter.setPen( pen );
  painter.setBrush(Qt::black);

  // inner black filled circle
  painter.drawEllipse( 0, 0, 5, 5 );

  // scale distances to pixels
  double scale = screen->size().height() - (margin*2) / r2;

  painter.setBrush(Qt::NoBrush);

  // inner radius
  int iR = static_cast<int> (rint( r1*scale ));
  painter.drawEllipse( 0, 0, iR, iR );

  // outer radius
  int oR = static_cast<int> (rint( r2*scale ));
  painter.drawEllipse( 0, 0, oR, oR );
}
