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
#include "distance.h"
#include "altitude.h"
#include "speed.h"
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
  radius(0),
  selectedObject("")
{
  /* qDebug( "FlarmDisplay window size is width=%d x height=%d",
          parent->size().width(),
          parent->size().height() ); */
}

/**
 * Destructor
 */
FlarmDisplay::~FlarmDisplay()
{
}

/** Creates the background picture with the radar screen. */
void FlarmDisplay::createBackground()
{
  // define a margin
  const int MARGIN = 20;

  width  = size().width();
  height = size().height();

  // qDebug() << "SW=" << width << "SH=" << height;

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

  // qDebug() << "SWM=" << width << "SHM=" << height;

  centerX  = size().width() / 2;
  centerY  = size().height() / 2;

  background = QPixmap( size() );

  // a very light gray
  background.fill( QColor(248, 248, 248) );

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

  QPen pen(Qt::black);
  pen.setWidth(3);
  painter.setPen( pen );
  painter.setBrush(Qt::black);

  // inner black filled circle
  painter.drawEllipse( centerX-3, centerY-3, 6, 6 );

  // scale pixels to maximum distance
  scale = (double) (height/2) / (double) radius;

  painter.setBrush(Qt::NoBrush);

  // draw inner radius, half of the outer radius
  painter.drawEllipse( centerX - width/4, centerY - height/4, width/2, height/2 );

  // draw outer radius
  painter.drawEllipse( centerX - width/2, centerY - height/2, width, height );

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

/** Update display */
void FlarmDisplay::slotUpdateDisplay()
{
  // Generate a paint event for this widget, if it is visible.
  if( isVisible() == true )
    {
      repaint();
    }
}

/** Set object to be selected. It is the hash key. */
void FlarmDisplay::slotSetSelectedObject( QString newObject )
{
  selectedObject = newObject;
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

void FlarmDisplay::mousePressEvent( QMouseEvent *event )
{
  // qDebug() << "FlarmDisplay::mouseEvent: Pos=" << event->pos();

  if (event->button() != Qt::LeftButton)
    {
      return;
    }

  // Try to find an object in the hash dictionary in the near of the mouse
  // pointer.
  QPoint pos = event->pos();

  QMutableHashIterator<QString, QPoint> it( objectHash );

  bool found = false;

  // Radius for Mouse Snapping
  int delta = 25, dX = 0, dY = 0;

  // Manhattan distance to found point.
  int lastDist = 2*delta + 1;

  QString hashKey;

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      QPoint &acftPosition = it.value();

      // calculate Manhattan distance
      dX = abs(acftPosition.x() - pos.x());
      dY = abs(acftPosition.y() - pos.y());

      if( dX < delta && dY < delta && (dX+dY) < lastDist )
        {
          found = true;
          lastDist = dX+dY;
          selectedObject = it.key();

          /* qDebug() << "Object=" << selectedObject
                   << "Delta=" << delta
                   << "dX" << dX
                   << "dY=" << dY
                   << "MD=" << lastDist; */
        }
    }

  event->accept();
}

void FlarmDisplay::paintEvent( QPaintEvent *event )
{
  //qDebug() << "FlarmDisplay::paintEvent";

  // Call paint method from QWidget.
  QWidget::paintEvent( event );

  QPainter painter( this );

  // copy background to widget
  painter.drawPixmap( rect(), background );

  // Here starts the Flarm object analysis and drawing
  QHash<QString, Flarm::FlarmAcft> flarmAcfts = Flarm::getPflaaHash();

  if( flarmAcfts.size() == 0 )
    {
      // hash is empty
      return;
    }

  objectHash.clear();

  QMutableHashIterator<QString, Flarm::FlarmAcft> it(flarmAcfts);

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      Flarm::FlarmAcft& acft = it.value();

      int north = acft.RelativeNorth;
      int east  = acft.RelativeEast;

      double distAcft = 0.0;

      // Check, if object is inside of the drawing area. Otherwise it is placed
      // at the outer circle.
      if( abs(north) > radius || abs(east) > radius ||
          (distAcft = sqrt( north*north + east*east)) > radius )
        {
          if( distAcft == 0.0 )
            {
              // We need the distance to use the cosine in further processing.
              distAcft = sqrt( north*north + east*east);
            }

          // Object is out of draw range and must be placed at outer radius.
          // We do that by calculating the angle with the triangle sentence
          // and by using polar coordinates.
          double alpha = acos( ((double) east) / distAcft );

          int x = static_cast<int> (rint(cos(alpha) * width/2));
          int y = static_cast<int> (rint(sin(alpha) * height/2));

          // correcting of signs, if necessary
          if( (east < 0 && x > 0) || (east > 0 && x < 0) )
            {
              east = -x;
            }
          else
            {
              east = x;
            }

          if( (north < 0 && y > 0) || (north > 0 && y < 0) )
            {
              north = -y;
            }
          else
            {
              north = y;
            }
        }
      else
        {
          // scale distances
          north = static_cast<int> (rint(static_cast<double> (north) * scale));
          east  = static_cast<int> (rint(static_cast<double> (east)  * scale));
        }

      int relTrack = 0;

      if( acft.Track != INT_MIN )
        {
          int myTrack = calculator->getlastHeading();

          if( myTrack > 180 )
            {
              myTrack -= 360;
            }

          int acftTrack = acft.Track;

          if( acftTrack > 180 )
            {
              acftTrack -= 360;
            }

          relTrack = acftTrack - myTrack;

          /* qDebug() << "myTrack" << myTrack
                   << "acftTrack" << acftTrack
                   << "relTrack" << relTrack; */
        }

      // Draw object as circle, triangle or square
      QPixmap object;
      QColor color(Qt::black);

      if( it.key() == selectedObject )
        {
          // If object is selected, we use another color
          color = QColor(Qt::magenta);

          QFont f = font();
          f.setPixelSize(18);
          f.setBold( true );
          painter.setFont(f);

          QPen pen(Qt::magenta);
          pen.setWidth(3);
          painter.setPen( pen );

          // Draw the Flarm aircraft Id of the selected object.
          painter.drawText( 5, size().height() - 5, acft.ID );

          // Draw the distance to the selected object
          if( distAcft == 0.0 )
            {
              // Calculate distance in meters
              distAcft = sqrt( north*north + east*east);
            }

          QString text = Distance::getText( distAcft, true, -1 );

          QRect textRect = painter.fontMetrics().boundingRect( text );

          painter.drawText( size().width() - 5 - textRect.width(),
                            size().height() - 5, text );


          text = "";

          // Draw the relative vertical separation
          if( acft.RelativeVertical > 0 )
            {
              // prefix positive value with a plus sign
              text = "+";
            }

          text += Altitude::getText( acft.RelativeVertical, true, 0 );

          textRect = painter.fontMetrics().boundingRect( text );

          painter.drawText( size().width() - 5 - textRect.width(),
                            5 + f.pixelSize(), text );

          text = "";

          // Draw climb rate, if available
          if( acft.ClimbRate != INT_MIN )
            {
              Speed speed(acft.ClimbRate);

              if( acft.ClimbRate > 0 )
                {
                  // prefix positive value with a plus sign
                  text = "+";
                }

              text += speed.getVerticalText( true, 1 );

              textRect = painter.fontMetrics().boundingRect( text );

              painter.drawText( size().width() - 5 - textRect.width(),
                                10 + 2 * f.pixelSize(), text );
            }
        }

      if( acft.TurnRate != 0 )
        {
          // Object is circling
          MapConfig::createCircle( object, 30, color, 1.0 );
        }
      else if( acft.Track != INT_MIN )
        {
          // Object with track info
          MapConfig::createTriangle( object, 32, color, relTrack, 1.0 );
        }
      else
        {
          // Object without track info
          MapConfig::createSquare( object, 30, color, 1.0 );
        }

      painter.drawPixmap( centerX + east  - object.size().width()/2,
                          centerY - north - object.size().height()/2,
                          object );

      // store the draw coordinates for mouse snapping
      objectHash.insert( it.key(), QPoint(centerX + east, centerY - north) );
    }
}
