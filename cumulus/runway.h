/***********************************************************************
**
**   runway.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RUNWAY_H
#define RUNWAY_H

/**
 * This class is used for defining a runway. It is used for all "small"
 * airports, not for the international airports, they use the struct
 * "intrunway", because they need two points per runway.
 *
 * @author Heiner Lamprecht
 *
 */

class Runway
{

public:

    Runway( unsigned short len, unsigned short dir, unsigned short surf, bool open )
    {
        length = len;
        direction = dir;
        surface = surf;
        isOpen  = open;
    };

    /**
     * The length of the runway, given in meters.
     */
    unsigned short length;

    /**
     * The direction of the runway, given in steps of 10 degree.
     */
    unsigned short direction;

    /**
     * The surface of the runway.
     *
     * @see Airport#SurfaceType
     */
    unsigned short surface;

    /**
     * Flag to indicate if the runway is open or closed.
     */
    bool isOpen;
};

#endif
