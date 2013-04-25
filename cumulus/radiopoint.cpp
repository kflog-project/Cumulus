/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& name,
                       const QString& icao,
                       const QString& shortName,
                       BaseMapElement::objectType type,
                       const WGSPoint& wgsP,
                       const QPoint& pos,
                       const float frequency,
                       const QString channel,
                       float elevation,
                       const QString country )
  : SinglePoint( name, shortName, type, wgsP, pos, elevation, country ),
    m_frequency(frequency),
    m_channel(channel),
    m_icao(icao)
{
}

RadioPoint::~RadioPoint()
{
}
