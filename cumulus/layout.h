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
#define MinFontSize 20
#else
#define IconSize 26
#define ButtonSize 40
#define MinFontSize 14
#endif

#endif /* LAYOUT_H_ */
