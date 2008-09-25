/***********************************************************************
 **
 **   singlepoint.cpp
 **
 **   This file is part of Cumulus.
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

#include "singlepoint.h"

SinglePoint::SinglePoint( const QString& n,
                          const QString& shortName,
                          const BaseMapElement::objectType t,
                          const WGSPoint& wgsP,
                          const QPoint& pos,
                          const unsigned int elev,
                          const unsigned short secID)
  : BaseMapElement(n, t, secID), wgsPosition(wgsP),
    position(pos), shortName(shortName), curPos(pos),
    elevation(elev)
{
}

SinglePoint::~SinglePoint()
{
}

void SinglePoint::drawMapElement(QPainter* targetP)
{
  if(! isVisible() ) {
    curPos = QPoint(-5000, -5000);
    return;
  }

  extern MapMatrix * _globalMapMatrix;
  int scale = _globalMapMatrix->getScaleRatio()/50;
  // qDebug("scale: %d %d",scale,_globalMapMatrix->getScaleRatio()  );
  targetP->setPen(QPen(Qt::black, 2));
  int iconSize = 8;

  if(typeID == BaseMapElement::Village) {
    targetP->setBrush(Qt::NoBrush);
    targetP->drawEllipse(curPos.x() - 5, curPos.y() - 5, 10, 10);
    return;
  }

  curPos = glMapMatrix->map(position);

  if(glMapMatrix->isSwitchScale())
    iconSize = 16;

  if( !glMapMatrix->isSwitchScale2() )
    targetP->drawEllipse(curPos.x(), curPos.y(), scale, scale );
  else
    targetP->drawPixmap(curPos.x() - iconSize, curPos.y() - iconSize, glConfig->getPixmap(typeID));
}
