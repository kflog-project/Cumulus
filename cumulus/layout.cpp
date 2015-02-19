/***********************************************************************
**
**   layout.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "layout.h"
#include "mainwindow.h"

#ifdef ANDROID
#include "jnisupport.h"
#endif

void Layout::adaptFont( QFont& fontRef,
                        const int pxHeight,
                        const int startPointSize,
                        const int minPointSize )
{
  int start = startPointSize;

  // Set font size to start value
  if( fontRef.pointSize() != -1 )
    {
      fontRef.setPointSize( start );
    }
  else
    {
      fontRef.setPixelSize( start );
    }

  // Adapt the font point size to the given pixel height.
  while( start > minPointSize )
    {
      QFontMetrics qfm(fontRef);

      if( (qfm.height() ) > pxHeight )
        {
          // Determine, what kind of font is used
          if( fontRef.pointSize() != -1 )
            {
              fontRef.setPointSize( --start );
            }
          else
            {
              fontRef.setPixelSize( --start );
            }

          continue;
        }
      else
        {
          break;
        }
     }

  if( fontRef.pointSize() != -1 )
    {
      fontRef.setPointSize( font2defaultSize( fontRef.pointSize() ) );
    }

#if 0
  qDebug() << "Layout::adaptFont: pxHeight=" << pxHeight
           << "startPointSize=" << startPointSize
           << "minPointSize" << minPointSize
           << "CalcPointSize=" << fontRef.pointSize();
#endif
}

int Layout::maxTextWidth( const QStringList& list, const QFont& font )
{
  QFontMetrics qfm(font);

  int width = 0;

  for( int i=0; i < list.size(); i++ )
    {
      int w = qfm.boundingRect(list.at(i)).width();

      if( w > width )
        {
          width = w;
        }
    }

  return width;
}

float Layout::getScaledDensity()
{
#ifdef ANDROID

  // We will cache the result because the density is a static value.
  static float density = -1.0;

  if( density == -1.0 )
    {
      QHash<QString, float> dmh = jniGetDisplayMetrics();

      density = dmh.value( "scaledDensity", 1.0 );

      if( density != 1.0 )
	{
	  float xdpi = dmh.value( "xdpi", 160.0 );

	  density = xdpi / 160.0;

	  if( density < 1.0 )
	    {
	      density = 1.0;
	    }
	}
    }

  return density;

#else

  // That is the default for none Android.
  return 1.0;

#endif
}

int Layout::getIntScaledDensity()
{
#ifdef ANDROID

  // We will cache the result because the density is a static value.
  static float density = -1.0;

  if( density == -1.0 )
    {
      QHash<QString, float> dmh = jniGetDisplayMetrics();

      density = dmh.value( "scaledDensity", 1.0 );
    }

  return density;

#else

  // That is the default for none Android.
  return 1.0;

#endif
}

int Layout::mouseSnapRadius()
{
#ifndef ANDROID

  // return the fixed value
  return SnapRadius;

#else

  // calculate mouse snap radius according to the scaled density.
  int sr = SnapRadius;

  // calculate radius in dependency of the scale factor
  QHash<QString, float> dmh = jniGetDisplayMetrics();

  float sd = dmh.value( "scaledDensity", 1.0 );

  if( sd > 1.0 )
    {
      sr = (int) rint( sr * sd );
    }

  return sr;

#endif
}

void Layout::fitGuiFont( QFont& font )
{
#ifndef ANDROID

  // return the fixed value
  Layout::adaptFont( font, GuiFontHeight );

#else

  // calculate GUI font height according to the screen size
  QHash<QString, float> dmh = jniGetDisplayMetrics();

  // Get screen height from Android
  float hp = dmh.value( "heightPixels", 0.0 );

  // We want to display 14 lines in the height of the display
  int gfh = (int) rint( hp / 14.);

  Layout::adaptFont( font, gfh );

  // Check maximum of point size and lower it, if necessary.
  if( font.pointSize() > 14 )
    {
      font.setPointSize( 14 );
    }

#endif

    qDebug() << "Layout::fitGuiFont=" << font.toString()
             << "FontHeight=" << QFontMetrics(font).height();
}

void Layout::fitGuiMenuFont( QFont& font )
{
#ifndef ANDROID

  // return the fixed value
  Layout::adaptFont( font, GuiMenuFontHeight );

#else

  // calculate GUI menu font height according to the screen size
  QHash<QString, float> dmh = jniGetDisplayMetrics();

  // Get screen height from Android
  float hp = dmh.value( "heightPixels", 0.0 );

  // We want to display 11 lines in the height of the display. That should
  // be a good value for a menu font
  int gmfh = (int) rint( hp / 11.);

  Layout::adaptFont( font, gmfh );

  // Check maximum of point size and lower it, if necessary.
  if( font.pointSize() > 18 )
    {
      font.setPointSize( 18 );
    }

#endif

  qDebug() << "Layout::fitGuiMenuFont=" << font.toString()
           << "FontHeight=" << QFontMetrics(font).height();
}

void Layout::fitDialogFont( QFont& font )
{
#ifndef ANDROID

  // return the fixed value
  Layout::adaptFont( font, DialogMinFontSize );

#else

  // calculate GUI font height according to the screen size
  QHash<QString, float> dmh = jniGetDisplayMetrics();

  // Get screen height from Android
  float hp = dmh.value( "heightPixels", 0.0 );

  // We want to display 12 lines in the height of the display
  int dfh = (int) rint( hp / 12.);

  Layout::adaptFont( font, dfh );

  // Check maximum of point size and lower it, if necessary.
  if( font.pointSize() > 16 )
    {
      font.setPointSize( 16 );
    }

#endif

  qDebug() << "Layout::fitDialogFont=" << font.toString()
           << "FontHeight=" << QFontMetrics(font).height();
}

void Layout::fitStatusbarFont( QFont& font )
{
#ifndef ANDROID

  // return the fixed value
  Layout::adaptFont( font, StatusbarFontHeight );

#else

  // calculate GUI font height according to the screen size
  QHash<QString, float> dmh = jniGetDisplayMetrics();

  // Get screen height from Android
  float hp = dmh.value( "heightPixels", 0.0 );

  // // We want to display 17 lines in the height of the display
  int sbfh = (int) rint( hp / 17.);

  Layout::adaptFont( font, sbfh );

  // Check maximum of point size and lower it, if necessary.
  if( font.pointSize() > 12 )
    {
      font.setPointSize( 12 );
    }

#endif

    qDebug() << "Layout::fitStatusbarFont=" << font.toString()
             << "FontHeight=" << QFontMetrics(font).height();
}

int Layout::getMapZoomButtonSize()
{
  QFont font = MainWindow::mainWindow()->font();
  font.setPointSize( 22 );

  int size = QFontMetrics(font).height();

  if( size < 60 )
    {
      size = 60;
    }

  return size;
}

int Layout::getButtonSize()
{
  QFont font = MainWindow::mainWindow()->font();
  font.setPointSize( 18 );

  int size = QFontMetrics(font).height();

#ifndef ANDROID
#ifdef MAEMO

  // Maemo needs some tweaking
  if( size < 60 )
    {
      size = 60;
    }

#else

    // X11 needs some tweaking
  if( size < 40 )
    {
      size = 40;
    }

#endif
#endif

  return size;
}

int Layout::getButtonSize( const int points )
{
#ifndef ANDROID
  Q_UNUSED(points)
  return IconSize;

#else

  QFont font = MainWindow::mainWindow()->font();
  font.setPointSize( points );

  return QFontMetrics(font).height();

#endif
}

int Layout::font2defaultSize( const int size )
{
  // Font: "Roboto" "Normal" "6 7 8 9 10 11 12 14 16 18 20 22 24 26 28 36 48 72"
  int sizeArray[17] = { 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 };

  for( int i = 16; i >= 0; i-- )
    {
      if( sizeArray[i] <= size )
        {
          return sizeArray[i];
        }
    }

  return sizeArray[0];
}

/**
 * Prints the array of font point size together with its related pixel size.
 */
void Layout::fontPoints2Pixel( QFont font )
{
  // Font: "Roboto" "Normal" "6 7 8 9 10 11 12 14 16 18 20 22 24 26 28 36 48 72"
  int sizeArray[18] = { 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72 };

  for( int i = 0; i <  18; i++ )
    {
      font.setPointSize( sizeArray[i] );
      qDebug() << "Font Points =" << font.pointSize() << "- Pixels =" << QFontMetrics(font).height();
    }
}


QString Layout::getCbSbStyle()
{
  // See: http://qt-project.org/forums/viewthread/30208 and
  //      http://qt-project.org/doc/qt-4.8/stylesheet-examples.html#customizing-qscrollbar

  const QString ss =
    /* Sets up scrollbar size, border, color */
    "QScrollBar:vertical {"
	"border: 1px solid grey;"
	"width: 1em;"
        /* margin sets up how far the handle can travel. Here over the whole line.*/
	"margin: 0px 0px 0px 0px;"
    "}"

    /* Sets up the color and height of the handle */
    "QScrollBar::handle:vertical {"
        "border-radius: 5px;"
	"background: lightgray;"
	"min-height: 2em;"
	"border: 1px solid grey;"
    "}"

    /* This removes the bottom button by setting the height to 0px */
    "QScrollBar::add-line:vertical {"
	"height: 0px;"
	"subcontrol-position: bottom;"
	"subcontrol-origin: margin;"
    "}"

    /* This removes the top button by setting the height to 0px */
    "QScrollBar::sub-line:vertical {"
	"height: 0px;"
	"subcontrol-position: top;"
	"subcontrol-origin: margin;"
    "}"

    /* Definition not needed due to no buttons are defined.
    "QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {"
	"border: 2px solid grey;"
	"width: 5px;"
	"height: 5px;"
	"background: white;"
    "}"
    */

    /* Need this to get rid of cross hatching on scrollbar background.
       Not necessary for Cumulus. */
    /*
    "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
	"background: none;"
    "}" */
     ;

  return ss;
}

