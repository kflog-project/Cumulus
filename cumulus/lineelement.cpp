/***********************************************************************
 **
 **   lineelement.cpp
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

#include <QColor>

#include "lineelement.h"
#include "generalconfig.h"

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
  // Reset screen bounding box
  sbBox = QRect();

  // If the element-type should not be drawn in the actual scale, or if the
  // element is not visible, return.
  if( ! glConfig->isBorder(typeID) || ! isVisible())
    {
      return false;
    }

  // Do check load and drawing options
  GeneralConfig *conf = GeneralConfig::instance();

  switch( typeID )
    {
    case BaseMapElement::Forest:

      if( conf->getMapLoadForests() == false )
        {
          return false;
        }

      break;

    case BaseMapElement::Highway:

      if( conf->getMapLoadHighways() == false )
        {
          return false;
        }

      break;

    case BaseMapElement::Railway:

      if( conf->getMapLoadRailways() == false )
        {
          return false;
        }

      break;

    case BaseMapElement::Road:

      if( conf->getMapLoadRoads() == false )
        {
          return false;
        }

      break;

    case BaseMapElement::River:

      if( conf->getMapLoadWaterways() == false )
        {
          return false;
        }

      break;

    case BaseMapElement::City:

      if( conf->getMapLoadCities() == false )
        {
          return false;
        }

      break;

    default:

      break;
    }

  QPolygon mP( glMapMatrix->map(projPolygon) );

  // Save screen bounding box
  sbBox = mP.boundingRect();

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
