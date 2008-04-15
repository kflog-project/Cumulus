/***********************************************************************
 **
 **   isohypse.cpp
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

#include <QRegion>
#include <QString>

#include "isohypse.h"
#include "mapmatrix.h"


Isohypse::Isohypse(QPolygon pG, unsigned int elev, bool isV, unsigned int secID)
    : LineElement("", BaseMapElement::Isohypse, pG, isV, secID),
    elevation(elev)
{}


Isohypse::~Isohypse()
{}


QRegion* Isohypse::drawRegion( QPainter* targetP, const QRect &viewRect,
                               bool really_draw, bool isolines )
{

  if(glMapMatrix->isVisible(bBox, getTypeID()))
    {
      QPolygon tP = glMapMatrix->map(projPolygon);
      QRegion* reg = new QRegion( tP );

      if(really_draw)
        {
          QRegion viewReg( viewRect );
          QRegion drawReg = reg->intersect( viewReg );

          targetP->setClipRegion( drawReg );
          targetP->fillRect( viewRect, targetP->brush() );

          if( isolines )
            {
              targetP->drawPolyline(tP);
            }
        }

      if (glMapMatrix->isInProjCenterArea(bBox))
        {
          return reg;
        }
      else
        {
          delete reg;
        }
    }

  return 0;
}
