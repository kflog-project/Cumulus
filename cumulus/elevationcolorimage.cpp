/***********************************************************************
**
**   elevationcolorimage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QString>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QPoint>

#include "elevationcolorimage.h"
#include "mapdefaults.h"
#include "altitude.h"

/**
 * This class shows the used elevation colors of the map in a vertical bar.
 * The right side of the bar is labeled with elevation numbers according
 * to the current altitude unit (meters or feed).
 */

/** A reference to the terrain color array has to be passed. The colors
 *  from the array are taken for the elevation color bars. Update first
 *  colors in the array before a new paintEvent is fired.
 */
ElevationColorImage::ElevationColorImage( QColor colors[], QWidget *parent ) :
  QWidget(parent)
{
  terrainColors = colors;
}

ElevationColorImage::~ElevationColorImage()
{
}

QSize ElevationColorImage::sizeHint() const
{
  return QSize(140, 480);
}

QSize ElevationColorImage::minimumSizeHint() const
{
  // Minimum height of one color bar should be 8 pixels plus
  // 10 pixels as reserve.
  return QSize(140, 51*8 + 10);
}

void ElevationColorImage::paintEvent( QPaintEvent * /* event */ )
{
  QPainter painter(this);
  int x, y;
  int altitude;
  QString text;
  QString unit = ( Altitude::getUnit() == Altitude::meters ) ? "m" : "ft";

  // calculate height per color bar in pixels
  int barHeight = height() / 51;

  if( barHeight > 10 )
    {
      // limit height to 10 pixels, if higher
      barHeight = 10;
    }

  // set horizontal start point
  x = 10;

  // center vertical start point
  y = ( height() - barHeight*51 ) / 2;

  // move null point of painter to new position
  painter.translate( QPoint(x, y) );

  // set font size used for text painting
  QFont newFont = painter.font();
  newFont.setPixelSize( 11 );
  painter.setFont( newFont) ;

  QPen pen;
  pen.setWidth(3);
  painter.setPen(pen);

  // Draw vertical color bars
  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      painter.fillRect( 0, i*barHeight, 50, barHeight, terrainColors[SIZEOF_TERRAIN_COLORS-1-i] );
    }

  // Draw vertical scale line
  painter.drawLine( QLine( 60, 0, 60, barHeight*51) );

  // Draw bottom line
  y = barHeight*51;
  painter.drawLine( QLine( 55, y, 65, y ) );

  // Draw Arrows at top
  painter.drawLine( QLine( 60, 0, 55, 15 ) );
  painter.drawLine( QLine( 60, 0, 65, 15 ) );

  // Draw right scale bars for zero
  y = barHeight*50 - barHeight/2;
  painter.drawLine( QLine( 60, y, 65, y ) );

  // Draw scale text for zero bar
  text = "0";
  y = barHeight*50;
  painter.drawText( 70, y, text );

  int idx[11]    = {45, 42, 37, 32, 28, 24, 20, 16, 12, 8, 4};
  int height[11] = {100, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};

  for( int i = 0; i < 11; i++ )
    {
      // Draw right scale bars for ...m
      y = barHeight * idx[i]- barHeight/2;
      painter.drawLine( QLine( 60, y, 65, y ) );

      // Draw scale text for ...m bar
      altitude = ( Altitude::getUnit() == Altitude::meters ) ? height[i] : (int) (height[i] * 3.2808);
      text = QString::number(altitude) + unit;
      y = barHeight * idx[i];
      painter.drawText( 70, y, text );
    }
}
