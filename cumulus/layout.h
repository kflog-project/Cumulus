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
 * This class defines common GUI parameters.
 *
 * \date 2010-2012
 *
 * \version $Id$
 *
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <QtGui>

#if defined (MAEMO) || defined (ANDROID)
#define IconSize 32
#define ButtonSize 80
#define DialogMinFontSize 12

// Snap radius size on the map
#define SnapRadius 25

// Plus and minus button size at the map
#define MapPMButtonSize 70

// Statusbar font height in pixels
#define StatusbarFontHeight 16

#else

#define IconSize 26
#define ButtonSize 40
#define DialogMinFontSize 14
#define SnapRadius 15
#define MapPMButtonSize 50

// Statusbar font height in pixels
#define StatusbarFontHeight 16

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

};

#endif /* LAYOUT_H_ */
