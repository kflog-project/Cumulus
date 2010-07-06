/***********************************************************************
**
**   flarmdislay.cpp
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

#include <cmath>

#include <QtGui>

#include "flarmdisplay.h"
#include "flarm.h"
#include "calculator.h"
#include "distance.h"
#include "mapconfig.h"

/**
 * Constructor
 */
FlarmDisplay::FlarmDisplay( QWidget *parent ) :
  QWidget( parent ),
  zoomLevel(FlarmDisplay::Low),
  centerX(0),
  centerY(0),
  width(0),
  height(0),
  scale(0.0),
  radius(0)
{
  qDebug( "FlarmDisplay window size is width=%d x height=%d",
          parent->size().width(),
          parent->size().height() );
}

/**
 * Destructor
 */
FlarmDisplay::~FlarmDisplay()
{
  qDebug() << "~FlarmDisplay()";
}

/** Creates the background picture with the radar screen. */
void FlarmDisplay::createBackground()
{
  // define a margin
  const int MARGIN = 15;

  width  = size().width();
  height = size().height();

  qDebug() << "SW=" << width << "SH=" << height;

  if( width > height )
    {
      width = height;
    }
  else if( height > width )
    {
      height = width;
    }

  width  -= ( MARGIN * 2 ); // keep a margin around
  height -= ( MARGIN * 2 );

  centerX  = size().width() / 2;
  centerY  = size().height() / 2;

  background = QPixmap( size() );

  // a very light gray
  background.fill( QColor(240, 240, 240) );

  // calculate the resolution according to the zoom level in meters
  switch( zoomLevel )
    {
      case FlarmDisplay::Low:
        radius = 500;
        break;
      case FlarmDisplay::Middle:
        radius = 1000;
        break;
      case FlarmDisplay::High:
        radius = 5000;
        break;
      default:
        radius = 500;
    }

  QPainter painter( &background );

  // painter.translate( margin, margin );

  QPen pen(Qt::black);
  pen.setWidth(3);
  painter.setPen( pen );
  painter.setBrush(Qt::black);

  // inner black filled circle
  painter.drawEllipse( centerX-3, centerY-3, 6, 6 );

  // scale maximum distance to pixels
  scale = height / radius;

  painter.setBrush(Qt::NoBrush);

  // draw inner radius, half of the outer radius
  int iR = static_cast<int> (rint( radius/2 * scale ));
  painter.drawEllipse( centerX - iR/2, centerY - iR/2, iR, iR );

  // draw outer radius
  int oR = static_cast<int> (rint( radius * scale ));
  painter.drawEllipse( centerX - oR/2, centerY - oR/2, oR, oR );

  QFont f = font();
  f.setPixelSize(18);
  f.setBold( true );
  painter.setFont(f);

  Distance distance( radius );

  // Draw scale unit in the upper left corner
  QString unitText = QString("%1 Km").arg(distance.getKilometers(), 0, 'f', 1);
  painter.drawText( 10, f.pixelSize() + 10, unitText );

  pen.setWidth(0);
  painter.setPen( pen );

  // Draw vertical cross line
  painter.drawLine( centerX, centerY - height/2 -10,
                    centerX, centerY + height/2 + 10 );

  // Draw horizontal cross line
  painter.drawLine( centerX - width/2 - 10, centerY,
                    centerX + width/2 + 10, centerY );
}

/** Switch to a new zoom level. */
void FlarmDisplay::slotSwitchZoom( enum Zoom value )
{
  if( zoomLevel == value )
    {
      return;
    }

  zoomLevel = value;
  createBackground();
  repaint();
}

void FlarmDisplay::showEvent( QShowEvent *event )
{
  qDebug() << "FlarmDisplay::showEvent";

  Q_UNUSED( event )

  if( background.isNull() == true )
    {
      // No base picture available do create one.
      createBackground();
    }
}

void FlarmDisplay::resizeEvent( QResizeEvent *event )
{
  qDebug() << "FlarmDisplay::resizeEvent";

  QWidget::resizeEvent( event );
  createBackground();
}

void FlarmDisplay::paintEvent( QPaintEvent *event )
{
  qDebug() << "FlarmDisplay::paintEvent";

  // Call paint method from QWidget.
  QWidget::paintEvent( event );

  QPainter painter( this );
  QFont f = font();
  f.setPixelSize(12);
  painter.setFont(f);

  // copy background to widget
  painter.drawPixmap( rect(), background );

  // Here starts the Flarm object analysis and drawing
  QHash<QString, Flarm::FlarmAcft> flarmAcfts = Flarm::getPflaaHash();

  if( flarmAcfts.size() == 0 )
    {
      // hash is empty
      return;
    }

  QMutableHashIterator<QString, Flarm::FlarmAcft> it(flarmAcfts);

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      Flarm::FlarmAcft acft = it.value();

      // Make time expire check, check time is in milli seconds.
      if( acft.TimeStamp.elapsed() > 5000 )
        {
          // Object was longer time not updated, so we do remove it from the
          // hash. No other way available as the time expire check.
          it.remove();
          continue;
        }

      int north = acft.RelativeNorth;
      int east  = acft.RelativeEast;

      // Check, if object is inside of the drawing area. Otherwise it is placed
      // at the outer circle.
      if( abs(north) > radius || abs(east) > radius )
        {
          // Objects out of draw range, must be reset to radius.

        }

      // Draw object as triangle
      extern MapConfig* _globalMapConfig;
      QPixmap triangle;

      int relTrack = 0;

      if( acft.Track != INT_MIN )
        {
          // relTrack = acft.Track - calculator->getlastHeading();
        }

      _globalMapConfig->createTriangle( triangle, 30, QColor(Qt::black), relTrack, 1.0);

    }
}
