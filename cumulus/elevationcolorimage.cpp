/***********************************************************************
**
**   elevationcolorimage.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009-2015 Axel Pauli, kflog.cumulus@gmail.com
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtGui>

#include "altitude.h"
#include "elevationcolorimage.h"
#include "layout.h"
#include "mapdefaults.h"

ElevationColorImage::ElevationColorImage( QColor colors[], QWidget *parent ) :
  QWidget(parent),
  m_terrainColors(colors),
  m_pixelSize( 10 * Layout::getIntScaledDensity() ),
  m_scale( Layout::getIntScaledDensity() )
{
  if( parent != 0 )
    {
      resize( parent->size() );
    }
}

ElevationColorImage::~ElevationColorImage()
{
}

QSize ElevationColorImage::sizeHint() const
{
  return QSize(160, 480);
}

QSize ElevationColorImage::minimumSizeHint() const
{
  // Minimum height of one color bar should be 8 pixels plus
  // 10 pixels as reserve.

  QFont myFont = font();

  myFont.setPixelSize( m_pixelSize );

  QFontMetrics fm( myFont );

  int tw = fm.width( "10000ft" );

  // Possible height in widget
  int ph = height() - (20 * m_scale);

  // Desired height in widget
  int dh = (51 * 8) * m_scale;

  if( dh > ph )
    {
      dh = ph;
    }

  return QSize( tw + ((70 + 10) * m_scale), dh );
}

void ElevationColorImage::paintEvent( QPaintEvent* /* event */ )
{
  QPainter painter(this);

  QString unit = ( Altitude::getUnit() == Altitude::meters ) ? "m" : "ft";

  // Calculate height per color bar in pixels. For additional space we take
  // one element more.
  int barHeight = height() / 52;

  if( barHeight > (10 * m_scale) )
    {
      // limit height to 10 pixels, if higher
      barHeight = 10 * m_scale;
    }

  // set horizontal start point
  int x = 10 * m_scale;

  // center vertical start point
  int y = ( height() - barHeight * 51 ) / 2;

  // move null point of painter to new position
  painter.translate( QPoint(x, y) );

  // set font size used for text painting
  QFont myFont = painter.font();
  myFont.setPixelSize( m_pixelSize );

  painter.setFont( myFont );
  QPen pen;
  pen.setWidth(3 * m_scale);
  painter.setPen(pen);

  // Draw vertical color bars
  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      painter.fillRect( 0, i * barHeight, 50 * m_scale, barHeight, m_terrainColors[SIZEOF_TERRAIN_COLORS-1-i] );
    }

  // Draw vertical scale line
  painter.drawLine( QLine( 60 * m_scale, 0, 60 * m_scale, barHeight * 51) );

  // Draw bottom line
  y = barHeight * 51;
  painter.drawLine( QLine( 55 * m_scale, y, 65 * m_scale, y ) );

  // Draw Arrows at top
  painter.drawLine( QLine( 60 * m_scale, 0, 55 * m_scale, 15 * m_scale ) );
  painter.drawLine( QLine( 60 * m_scale, 0, 65 * m_scale, 15 * m_scale ) );

  // Draw right scale bars for zero
  y = barHeight * 50 - barHeight / 2;
  painter.drawLine( QLine( 60 * m_scale, y, 65 * m_scale, y ) );

  // Draw scale text for zero bar
  QString text("0");
  y = barHeight * 50;
  painter.drawText( 70 * m_scale, y, text );

  int idx[11]    = {45, 42, 37, 32, 28, 24, 20, 16, 12, 8, 4};
  int height[11] = {100, 250, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};

  for( int i = 0; i < 11; i++ )
    {
      // Draw right scale bars for ...m
      y = barHeight * idx[i] - barHeight / 2;
      painter.drawLine( QLine( 60 * m_scale, y, 65 * m_scale, y ) );

      // Draw scale text for ...m bar
      int altitude = ( Altitude::getUnit() == Altitude::meters ) ? height[i] : (int) (height[i] * 3.2808);
      text = QString::number(altitude) + unit;
      y = barHeight * idx[i];
      painter.drawText( 70 * m_scale, y, text );
    }
}
