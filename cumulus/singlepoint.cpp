/***********************************************************************
 **
 **   singlepoint.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "singlepoint.h"


SinglePoint::SinglePoint(const QString& n, const QString& gps,
                         BaseMapElement::objectType t,
                         const WGSPoint& wgsP, const QPoint& pos,
                         unsigned int elev, unsigned int secID)
  : BaseMapElement(n, t, secID), wgsPosition(wgsP),
    position(pos), gpsName(gps), curPos(pos),
    elevation(elev)
{
}


SinglePoint::~SinglePoint()
{
}


bool SinglePoint::__isVisible() const
{
  return glMapMatrix->isVisible(position);
}


void SinglePoint::drawMapElement(QPainter* targetP)
{
  if(!__isVisible()) {
    curPos = QPoint(-50, -50);
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


QString SinglePoint::getWPName() const
{
  return gpsName;
}


QPoint SinglePoint::getPosition() const
{
  return position;
}


WGSPoint SinglePoint::getWGSPosition() const
{
  return wgsPosition;
}


QPoint SinglePoint::getMapPosition() const
{
  return curPos;
}


QString SinglePoint::getInfoString() const
{
  return QString();
}


unsigned int SinglePoint::getElevation() const
{
  return elevation;
}
