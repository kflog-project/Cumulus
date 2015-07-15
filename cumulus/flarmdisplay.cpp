/***********************************************************************
**
**   flarmdisplay.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2015 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include <QtGui>

#include "altitude.h"
#include "calculator.h"
#include "distance.h"
#include "flarmaliaslist.h"
#include "flarmdisplay.h"
#include "flarm.h"
#include "layout.h"
#include "mapconfig.h"
#include "speed.h"

// Initialize static variables
enum FlarmDisplay::Zoom FlarmDisplay::zoomLevel = FlarmDisplay::Low;

QString FlarmDisplay::selectedObject = "";

FlarmDisplay::FlarmDisplay( QWidget *parent ) :
  QWidget( parent ),
  centerX(0),
  centerY(0),
  width(0),
  height(0),
  scale(0.0),
  radius(0),
  updateInterval(2)
{
}

FlarmDisplay::~FlarmDisplay()
{
}

/** Creates the background picture with the radar screen. */
void FlarmDisplay::createBackground()
{
  // Get the scaled density
  const int SD = Layout::getIntScaledDensity();

  // Define a margin
  const int MARGIN = 20 * SD;

  width  = size().width();
  height = size().height();

  if( width > height )
    {
      width = height;
    }
  else if( height > width )
    {
      height = width;
    }

  // keep a margin around
  width  -= ( MARGIN * 2 );
  height -= ( MARGIN * 2 );

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
        radius = 6000;
        break;
      default:
        radius = 500;
        break;
    }

  QPainter painter( &background );

  QPen pen(Qt::black);
  pen.setWidth(3 * SD);
  painter.setPen( pen );
  painter.setBrush(Qt::black);

  // inner black filled circle
  painter.drawEllipse( centerX - (3 * SD), centerY - (3 * SD), 6 * SD, 6 *SD );

  // scale pixels to maximum distance
  scale = (double) (height/2) / (double) radius;

  painter.setBrush(Qt::NoBrush);

  // draw inner radius, half of the outer radius
  painter.drawEllipse( centerX - width/4, centerY - height/4, width/2, height/2 );

  // draw outer radius
  painter.drawEllipse( centerX - width/2, centerY - height/2, width, height );

  QFont f = painter.font();

  f.setPointSize( FlarmDisplayTextPointSize );
  f.setBold( true );

  painter.setFont(f);

  Distance distance( radius );

  // Draw scale unit in the upper left corner
  QString unitText = QString("%1 Km").arg(distance.getKilometers(), 0, 'f', 1);
  QFontMetrics fm = QFontMetrics( font() );

  painter.drawText( 5 * SD, fm.boundingRect(unitText).height() + (5 * SD), unitText );
  pen.setWidth(1 * SD);
  painter.setPen( pen );

  // Draw vertical cross line
  painter.drawLine( centerX, centerY - height/2 - (10 * SD),
                    centerX, centerY + height/2 + (10 * SD) );

  // Draw horizontal cross line
  painter.drawLine( centerX - width/2 - (10 * SD), centerY,
                    centerX + width/2 + (10 * SD), centerY );

  // Draw the selected Flarm object identifier, if an selection is active.
  if( selectedObject.isEmpty() == false )
    {
      // If object is selected, we use another color
      QPen pen(Qt::magenta);
      pen.setWidth(3 * SD);
      painter.setPen( pen );

      const QHash<QString, QString> &aliasHash = FlarmAliasList::getAliasHash();

      // Try to map the Flarm Id to an alias name
      QString actfId = aliasHash.value(selectedObject, selectedObject );

      // Draw the Flarm Id of the selected object.
      painter.drawText( 5 * SD, size().height() - (5 * SD), actfId );
    }
}

/** Switch to a new zoom level. */
void FlarmDisplay::slot_SwitchZoom( enum Zoom value )
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
void FlarmDisplay::slot_UpdateDisplay()
{
  static int counter = 0;

  // Generate a paint event for this widget, if it is visible.
  if( isVisible() == true && (counter % updateInterval) == 0 )
    {
      repaint();
    }

  counter++;
}

/** Reset display to background. */
void FlarmDisplay::slot_ResetDisplay()
{
  repaint();
}

/** Set object to be selected. It is the hash key. */
void FlarmDisplay::slot_SetSelectedObject( QString newObject )
{
  selectedObject = newObject;
  update();
}

void FlarmDisplay::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  createBackground();
}

void FlarmDisplay::resizeEvent( QResizeEvent *event )
{
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
  int delta = Layout::mouseSnapRadius();
  int dX = 0, dY = 0;

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

  if( found == true )
    {
      // Report new selection to FlarmListView
      emit newObjectSelection( selectedObject );
      createBackground();
      update();
    }

  event->accept();
}

void FlarmDisplay::paintEvent( QPaintEvent *event )
{
  // Call paint method from QWidget.
  QWidget::paintEvent( event );

  QPainter painter( this );

  // copy background to widget
  painter.drawPixmap( rect(), background );

  // Here starts the Flarm object analysis and drawing
  QHash<QString, Flarm::FlarmAcft> flarmAcfts = Flarm::getPflaaHash();

  if( flarmAcfts.size() == 0 )
    {
      // qDebug() << "FlarmDisplay::paintEvent: empty hash";
      // hash is empty
      return;
    }

  QFont font = this->font();
  font.setPointSize( FlarmDisplayIconPointSize );

  // Calculate an icon size from a font height.
  int is = QFontMetrics(font).height();

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
      double distAcftShort;
      double alpha;
      bool doScale = true;

      // Check, if object is inside of the drawing area. Otherwise it is placed
      // at the outer circle.
      if( abs(north) > radius || abs(east) > radius ||
          (distAcft = sqrt( north*north + east*east)) > radius )
        {
          doScale = false;

          if( distAcft == 0.0 )
            {
              // We need the distance to use the cosine in further processing.
              distAcft = sqrt( north*north + east*east);
            }

          // Object is out of draw range and must be placed at outer radius.
          // We do that by calculating the angle with the triangle sentence
          // and by using polar coordinates.
          alpha = atan2( ((double) north), (double) east );

          east  = static_cast<int> (rint(cos(alpha) * width/2));
          north = static_cast<int> (rint(sin(alpha) * height/2));

          distAcftShort = sqrt( north*north + east*east);
        }
      else
        {
          distAcft = sqrt( north*north + east*east);
          distAcftShort = distAcft;

          alpha = atan2( ((double) north), (double) east );
        }

      double heading2Object = ((double) (360. - calculator->getlastHeading()) * M_PI / 180.) + (M_PI_2 - alpha);

      /*
      qDebug() << "North=" << north
               << "East=" << east
               << "Heading=" << calculator->getlastHeading()
               << "H2O=" << heading2Object * 180. / M_PI
               << "Alpha=" << alpha * 180. / M_PI;
      */

      double x = cos(heading2Object) * distAcftShort;
      double y = sin(heading2Object) * distAcftShort;

      // scale distances, if not reduced to outer circle
      if( doScale )
        {
          north = static_cast<int> (rint(x * scale));
          east  = static_cast<int> (rint(y * scale));
        }
      else
        {
          north = static_cast<int> (rint(x));
          east  = static_cast<int> (rint(y));
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
      QColor color;

      if( acft.Alarm == Flarm::Important )
        {
          color = QColor(255, 128, 0);
        }
      else if( acft.Alarm == Flarm::Urgent )
        {
          color = Qt::red;
        }

      if( it.key() == selectedObject )
        {
          // If a Flarm object is selected, we use another color
          color = Qt::magenta;
        }

      QPen pen(Qt::black);
      pen.setWidth(3);
      painter.setPen( pen );

      if( acft.TurnRate != INT_MIN )
        {
          // Object is circling, not yet supported by FLARM atm.
          if( color.isValid() == false )
            {
              color = getLiftColor( acft.ClimbRate );
            }

          MapConfig::createCircle( object, is, color,
                                   1.0, Qt::transparent, pen );
        }
      else if( acft.Track != INT_MIN )
        {
          // Object with track info
          if( color.isValid() == false )
             {
               color = getLiftColor( acft.ClimbRate );
             }

          MapConfig::createTriangle( object, is+4, color, relTrack,
                                     1.0, Qt::transparent, pen );
        }
      else
        {
          // Object without track info
          if( color.isValid() == false )
             {
               color = Qt::black;
             }

          MapConfig::createSquare( object, is, color, 1.0, pen );
        }

      if( it.key() == selectedObject )
        {
          // If a Flarm object is selected, we draw some additional information
          QFont f = painter.font();

          f.setPointSize( FlarmDisplayTextPointSize );
          f.setBold( true );

          painter.setFont(f);

          QPen pen(color);
          pen.setWidth(3);
          painter.setPen( pen );

          // Draw the distance to the selected object
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
                            5 + painter.fontMetrics().height(), text );

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
                                5 + 2 * painter.fontMetrics().height(), text );
            }
        }

      painter.drawPixmap( centerX + east  - object.size().width()/2,
                          centerY - north - object.size().height()/2,
                          object );

      // store the draw coordinates for mouse snapping
      objectHash.insert( it.key(), QPoint(centerX + east, centerY - north) );
    }
}

/** Returns a color related to the current lift. */
QColor FlarmDisplay::getLiftColor( double lift )
{
  if( lift == INT_MIN )
    {
      // lift is undefined
      return Qt::black;
    }

  short sl = static_cast<short>(lift * 10.0);

  if( sl < -40 )
    {
      return QColor("#1874cd");
    }

  if( sl < -30 )
    {
      return QColor("#1e90ff");
    }

  if( sl < -20 )
    {
      return QColor("#00bfff");
    }

  if( sl < -10 )
    {
      return QColor("#87cefa");
    }

  if( sl < -5 )
    {
      return QColor("#00e5ff");
    }

  if( sl < 0 )
    {
      return QColor("#60f5ff");
    }

  if( sl < 5 )
    {
      return QColor("#90EE90");
    }

  if( sl < 10 )
    {
      return QColor("#FFFF00");
    }

  if( sl < 20 )
    {
      return QColor("#FFD700");
    }

  if( sl < 30 )
    {
      return QColor("#FFA500");
    }

  if( sl < 40 )
    {
      return QColor("#FF8C00");
    }

  if( sl < 50 )
    {
      return QColor("#FF7F50");
    }

  // Default is black
  return Qt::black;
}
