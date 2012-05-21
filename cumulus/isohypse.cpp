/***********************************************************************
 **
 **   isohypse.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 by Josua Dietze
 **                   2009-2010 Peter Turczak
 **                   2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>

#include <QPainterPath>
#include <QString>
#include <QSize>

#include "isohypse.h"
#include "mapmatrix.h"

Isohypse::Isohypse( QPolygon elevationCoordinates,
                    const short elevation,
                    const uchar  elevationIndex,
                    const ushort secID,
                    const char typeID ) :
    LineElement( "Isoline", BaseMapElement::Isohypse, elevationCoordinates, false, secID ),
    _elevation(elevation),
    _elevationIndex(elevationIndex),
    _typeID(typeID)
{}


Isohypse::~Isohypse()
{}

QPainterPath* Isohypse::drawRegion( QPainter* targetP,
                                       bool isolines )
{
  QPainterPath *ppath = static_cast<QPainterPath *> (0);

  if( !glMapMatrix->isVisible(bBox, getTypeID() ) || projPolygon.size() < 3 )
    {
      return ppath;
    }

  QPolygon mP = glMapMatrix->map(projPolygon);

  if (mP.boundingRect().isNull())
    {
      // ignore null values
      return ppath;
    }

  ppath = new QPainterPath;

  ppath->moveTo( mP.at(0).x(), mP.at(0).y() );

  for( int i = 1; i < mP.size(); i++ )
    {
      ppath->lineTo( mP.at(i).x(), mP.at(i).y() );
    }

  ppath->closeSubpath();

  targetP->save();

  if( isolines )
    {
      QPen pen = targetP->pen();
      pen.setWidth(1);
      pen.setColor(Qt::black);
      pen.setStyle(Qt::DotLine);
      targetP->setPen(pen);
    }

  targetP->drawPath( *ppath );
  targetP->restore();

  return ppath;
}
