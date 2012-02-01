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

#if defined (MAEMO) || defined (ANDROID)
#define IconSize 32
#define ButtonSize 80
#define DialogMinFontSize 20

// Snap radius size on the map
#define SnapRadius 25

// Plus and minus button size at the map
#define MapPMButtonSize 70

#else

#define IconSize 26
#define ButtonSize 40
#define DialogMinFontSize 14
#define SnapRadius 15
#define MapPMButtonSize 50
#endif

#endif /* LAYOUT_H_ */
