/***********************************************************************
 **
 **   lineelement.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2009 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QColor>

#include "lineelement.h"

LineElement::LineElement( const QString& name,
                          const BaseMapElement::objectType t,
                          const QPolygon& pP,
                          const bool isV,
                          const unsigned short secID )
  : BaseMapElement(name, t, secID), projPolygon(pP),
    bBox(pP.boundingRect()), valley(isV), closed(false)
{
  if( typeID == BaseMapElement::Lake ||
      typeID == BaseMapElement::City ||
      typeID == BaseMapElement::Forest)
    {
      closed = true;
    }
}

LineElement::~LineElement()
{
}

bool LineElement::drawMapElement(QPainter* targetP)
{
  // If the element-type should not be drawn in the actual scale, or if the
  // element is not visible, return.

  if( ! glConfig->isBorder(typeID) || ! isVisible())
    {
      return false;
    }

  QPolygon mP( glMapMatrix->map(projPolygon) );

  if(typeID == BaseMapElement::City)
    {
      // We do not draw the outline of the city directly, because otherwise
      // we will get into trouble with cities lying at the edge of a
      // map-section.
      // @AP: Not clear what is meant.
      targetP->setPen(glConfig->getDrawPen(typeID));
      targetP->setBrush(glConfig->getDrawBrush(typeID));
      targetP->drawPolygon(mP);
      return true;
    }

  const QPen& drawP(glConfig->getDrawPen(typeID));
  targetP->setPen(drawP);

  if(closed)
    {
      // Lakes do not have a brush, because they are divided into normal
      // sections and we do not want to see section borders in a lake ...
      targetP->setBrush(glConfig->getDrawBrush(typeID));
      targetP->drawPolygon(mP);
      return true;
  }

  targetP->drawPolyline(mP);

  if(typeID == BaseMapElement::Highway && drawP.width() > 4)
    {
      // draw the white line in the middle
      targetP->setPen(QPen(Qt::white, 1));
      targetP->drawPolyline(mP);
    }

  return true;
}
