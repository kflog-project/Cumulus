/***********************************************************************
 **
 **   singlepoint.cpp
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

#include <QtGui>
#include "singlepoint.h"

extern MapMatrix* _globalMapMatrix;

SinglePoint::SinglePoint( const QString& n,
                          const QString& shortName,
                          const BaseMapElement::objectType t,
                          const WGSPoint& wgsP,
                          const QPoint& pos,
                          const float elevation,
                          const QString country,
                          const QString comment,
                          const unsigned short secID ) :
  BaseMapElement(n, t, secID),
  wgsPosition(wgsP),
  position(pos),
  shortName(shortName),
  curPos(pos),
  elevation(elevation),
  country(country.toUpper()),
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

  int scale = _globalMapMatrix->getScaleRatio() / 50;
  // qDebug("scale: %d %d",scale,_globalMapMatrix->getScaleRatio()  );
  targetP->setPen( QPen( Qt::black, 2 ) );

  if( typeID == BaseMapElement::Village )
    {
      targetP->setBrush( Qt::NoBrush );
      targetP->drawEllipse( curPos.x() - 5, curPos.y() - 5, 10, 10 );
      return true;
    }

  if( !glMapMatrix->isSwitchScale2() )
    {
      targetP->drawEllipse(curPos.x() - scale/2, curPos.y() - scale/2, scale, scale );
    }
  else
    {
       QPixmap pixmap = glConfig->getPixmap(typeID);

       targetP->drawPixmap( curPos.x() - pixmap.size().width() / 2,
                            curPos.y() - pixmap.size().height() / 2,
                            pixmap );
    }

  return true;
}
