/***********************************************************************
 **
 **   lineelement.cpp
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
      typeID == BaseMapElement::Forest) {
    closed = true;
  }
}

LineElement::~LineElement()
{
}

void LineElement::drawMapElement(QPainter* targetP)
{
  // If the element-type should not be drawn in the actual scale, or if the
  // element is not visible, return.

  if( ! glConfig->isBorder(typeID) || ! isVisible()) {
    return;
  }

  QPen drawP(glConfig->getDrawPen(typeID));

  QPolygon mP( glMapMatrix->map(projPolygon) );

  if(typeID == BaseMapElement::City) {
    //
    // We do not draw the outline of the city directly, because otherwise
    // we will get into trouble with cities lying at the edge of a
    // map-section. So we use a thicker draw a line into the mask-painter.
    //
    QBrush drawB( glConfig->getDrawBrush(typeID) );
    targetP->setPen(QPen(Qt::black, 1, Qt::SolidLine));
    targetP->setBrush(drawB);
    targetP->drawPolygon(mP);
    return;
  }

  targetP->setPen(drawP);

  if(closed) {
    //
    // Lakes do not have a brush, because they are devided into normal
    // sections and we do not want to see section-borders in a lake ...
    //
    if(typeID == BaseMapElement::Lake)
      targetP->setBrush(QBrush(drawP.color(), Qt::SolidPattern));
    else
      targetP->setBrush(glConfig->getDrawBrush(typeID));
    targetP->drawPolygon(mP);
  } else {
    targetP->drawPolyline(mP);
    if(typeID == Highway && drawP.width() > 4) {
      // draw the white line in the middle
      targetP->setPen(QPen(Qt::white, 1));
      targetP->drawPolyline(mP);
    }
  }
}
