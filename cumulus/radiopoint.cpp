/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& n, const QString& i,
                       const QString& g, BaseMapElement::objectType t,
                       const WGSPoint& wgsP, const QPoint& pos,
                       const float freq, float elev,
                       const QString country )
  : SinglePoint( n, g, t, wgsP, pos, elev, country ),
    frequency(freq), icao(i)
{
}

RadioPoint::~RadioPoint()
{
}
