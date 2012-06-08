/***********************************************************************
**
**   layout.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010-2012 by Axel Pauli
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
 * \date 2010-2012
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
#define ButtonSize 80

// Snap radius size on the map
#define SnapRadius 25

// Plus and minus button size at the map
#define MapPMButtonSize 70

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
#define MapLabelFontPointSize 20

// Map city label font point size
#define MapCityLabelFontPointSize 11

// Map bearing indicator font point size
#define MapBearingIndicatorFontPointSize 20

// Map Flarm painter font height in pixels
#define MapFlarmPainterFontHeight 24

// Flarm display painter font height in pixels
#define FlarmDisplayPainterFontPixelSize 24

// WhatsThat font point size
#define WhatsThatFontPointSize 20

#elif defined ANDROID

#define IconSize 32
#define ButtonSize 80

// Snap radius size on the map
#define SnapRadius 25

// Plus and minus button size at the map
#define MapPMButtonSize 70

// Dialog font size in pixels
#define DialogMinFontSize 22

// Statusbar font height in pixels
#define StatusbarFontHeight 20

// GUI font height in pixels
#define GuiFontHeight 24

// GUI menu font height in pixels
#define GuiMenuFontHeight 32

// Map scale bar font height in pixels
#define MapScalebarFontHeight 20

// Map label font point size
#define MapLabelFontPointSize 10

// Map city label font point size
#define MapCityLabelFontPointSize 8

// Map bearing indicator font point size
#define MapBearingIndicatorFontPointSize 12

// Map Flarm painter font height in pixels
#define MapFlarmPainterFontHeight 24

// Flarm display painter font height in pixels
#define FlarmDisplayPainterFontPixelSize 24

// WhatsThat font point size
#define WhatsThatFontPointSize 16

#else

#define IconSize 26
#define ButtonSize 42
#define SnapRadius 15
#define MapPMButtonSize 50

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

// Map Flarm painter font height in pixels
#define MapFlarmPainterFontHeight 24

// Flarm display painter font height in pixels
#define FlarmDisplayPainterFontPixelSize 24

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
   * \param startPointSize start optimization with this point size
   * \param minPointSize   minimum font point size
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

};

#endif /* LAYOUT_H_ */
