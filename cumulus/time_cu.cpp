/***********************************************************************
**
**   time_cu.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
****************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * This class handles the time unit to be used for display.
 */

#include "time_cu.h"

// Initializer for static member variable, UTC is the default time unit.
Time::timeUnit Time::_timeUnit = utc;
