/***********************************************************************
 **
 **   singlepoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2015 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtGui>

#include "generalconfig.h"
#include "singlepoint.h"

SinglePoint::SinglePoint() :
  BaseMapElement(),
  elevation(0.0)
{
}

SinglePoint::SinglePoint( const QString& n,
                          const QString& shortName,
                          const BaseMapElement::objectType t,
                          const WGSPoint& wgsP,
                          const QPoint& pos,
                          const float elevation,
                          const QString country,
                          const QString comment,
                          const unsigned short secID ) :
  BaseMapElement(n, t, secID, country),
  wgsPosition(wgsP),
  position(pos),
  shortName(shortName),
  curPos(pos),
  elevation(elevation),
  comment(comment)
{
}

SinglePoint::~SinglePoint()
{
}

bool SinglePoint::drawMapElement( QPainter* targetP )
{
  if( ! isVisible() )
    {
      curPos = QPoint( -5000, -5000 );
      return false;
    }

  curPos = glMapMatrix->map( position );

  targetP->setPen( QPen( Qt::black, 2 ) );

  QPixmap pixmap = glConfig->getPixmap( typeID, false );

  int xoff = pixmap.size().width() / 2;
  int yoff = pixmap.size().height() / 2;

  if( typeID == BaseMapElement::City ||
      typeID == BaseMapElement::Thermal ||
      typeID == BaseMapElement::Turnpoint )
   {
     // The lower end of the flag shall directly point to the point at the map.
     yoff = pixmap.size().height();
   }

  targetP->drawPixmap( curPos.x() - xoff,
                       curPos.y() - yoff,
                       pixmap );

  return true;
}
