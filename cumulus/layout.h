/***********************************************************************
**
**   layout.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief GUI layout definitions
 *
 * This class defines common GUI layout parameters.
 *
 * \date 2010-2013
 *
 * \version $Id$
 *
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <QtGui>

#if defined MAEMO

// @AP: It seems that under MAEMO point size and pixel size are to be identically.
//      That is a big difference to Android!

#define IconSize 32

// Snap radius size on the map
#define SnapRadius 25

// Dialog font size in pixels
#define DialogMinFontSize 22

// Statusbar font height in pixels
#define StatusbarFontHeight 25

// GUI font height in pixels
#define GuiFontHeight 27

// GUI menu font height in pixels
#define GuiMenuFontHeight 44

// Map scale bar font height in pixels
#define MapScalebarFontHeight 22

// Map label font point size
#define MapLabelFontPointSize 18

// Map city label font point size
#define MapCityLabelFontPointSize 11

// Map bearing indicator font point size
#define MapBearingIndicatorFontPointSize 20

// Map Flarm label font point size
#define MapFlarmLabelFontPointSize 16

// Flarm display text point size
#define FlarmDisplayTextPointSize 20

// Flarm display icon point size
#define FlarmDisplayIconPointSize 24

// WhatsThat font point sizeFlarmDisplayPainterFontPixelSize
#define WhatsThatFontPointSize 16

#elif defined ANDROID

#define IconSize 32

// Snap radius size on the map
#define SnapRadius 25

// Dialog font size in pixels
#define DialogMinFontSize 22

// Statusbar font height in pixels
#define StatusbarFontHeight 20

// GUI font height in pixels
#define GuiFontHeight 16

// GUI menu font height in pixels
#define GuiMenuFontHeight 22

// Map scale bar font height in pixels
#define MapScalebarFontHeight 20

// Map label font point size
#define MapLabelFontPointSize 7

// Map city label font point size
#define MapCityLabelFontPointSize 7

// Map bearing indicator font point size
#define MapBearingIndicatorFontPointSize 12

// Map Flarm label font point size
#define MapFlarmLabelFontPointSize 12

// Flarm display text point size
#define FlarmDisplayTextPointSize 12

// Flarm display icon point size
#define FlarmDisplayIconPointSize 16

// WhatsThat font point sizeFlarmDisplayPainterFontPixelSize
#define WhatsThatFontPointSize 16

#else

#define IconSize 26
#define SnapRadius 15

// Dialog font size in pixels
#define DialogMinFontSize 22

// Statusbar font height in pixels
#define StatusbarFontHeight 20

// GUI font height in pixels
#define GuiFontHeight 24

// GUI menu font height in pixels
#define GuiMenuFontHeight 32

// map scale bar font height in pixels
#define MapScalebarFontHeight 20

// Map label font point size
#define MapLabelFontPointSize 10

// Map city label font point size
#define MapCityLabelFontPointSize 7

// Map bearing indicator font point size
#define MapBearingIndicatorFontPointSize 12

// Map Flarm label font point size
#define MapFlarmLabelFontPointSize 16

// Flarm display text point size
#define FlarmDisplayTextPointSize 16

// Flarm display icon point size
#define FlarmDisplayIconPointSize 24

// WhatsThat font point size
#define WhatsThatFontPointSize 16

#endif

class Layout
{
  public:

  /**
   * Adapt the font point size to the given height in pixels.
   *
   * \param fontRef        Reference to a font object
   * \param pxHeight       maximum font height in pixels
   * \param startPointSize start optimization with this point/pixel size
   * \param minPointSize   minimum font point/pixel size
   */
  static void adaptFont( QFont& fontRef,
                         const int pxHeight,
                         const int startPointSize=30,
                         const int minPointSize=6 );

  /**
   * Calculates the maximum text width according to the passed font.
   *
   * \param list The strings to be evaluated
   * \param font The font to be used
   *
   * \return The maximum text width
   */
  static int maxTextWidth( const QStringList& list, const QFont& font );

  /**
   * Calculates a icon size according to the passed font.
   *
   * @param font Font to be used for icon size calculation
   *
   * @return Icon size related to passed font.
   */
  static int iconSize( const QFont& font )
  {
    QFontMetrics qfm( font );

    return( qfm.height() - 4 );
  }

  /**
   * Calculates a mouse snap radius adapted to the screen resolution under
   * Android.
   *
   * @return mouse snap radius
   */
  static int mouseSnapRadius();

  static void fitGuiFont( QFont& font );

  static void fitGuiMenuFont( QFont& font );

  static void fitDialogFont( QFont& font );

  static void fitStatusbarFont( QFont& font );

  /**
   * Gets the zoom button size in pixels drawn at the map.
   *
   * \return The size of the zoom buttons in pixels.
   */
  static int getMapZoomButtonSize();

  static int getButtonSize();

#ifdef ANDROID

  /**
   * Gets the scaled density of the screen resolution by Android.
   *
   * @return The scaled density of the screen resolution.
   */
  static float getScaledDensity();

#endif

};

#endif /* LAYOUT_H_ */
