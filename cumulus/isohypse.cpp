/***********************************************************************
 **
 **   isohypse.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 Axel Pauli, Josua Dietze
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <math.h>

#include <QRegion>
#include <QString>
#include <QSize>

#include "isohypse.h"
#include "generalconfig.h"
#include "mapmatrix.h"

extern MapMatrix* _globalMapMatrix;
extern MapConfig* _globalMapConfig;

Isohypse::Isohypse( QPolygon pG, uint elevation, bool isValley,
                    uint secID, const ushort typeID )
    : LineElement( "", BaseMapElement::Isohypse, pG, isValley, secID ),
    _elevation(elevation),
    _typeID(typeID)
{}


Isohypse::~Isohypse()
{}

QRegion* Isohypse::drawRegion( QPainter* targetP, const QRect &viewRect,
                               bool really_draw, bool isolines )
{
  if( !glMapMatrix->isVisible(bBox, getTypeID()) )
    {
     return static_cast<QRegion *> (0);
    }

  QPolygon tP = glMapMatrix->map(projPolygon);

  if(really_draw)
    {
      if( tP.boundingRect().isNull() )
        {
          // ignore null values and return also no region
          return static_cast<QRegion *> (0);
        }

      // @AP: try from me to improve isoline drawing. My approach
      // is to skip small drawing areas on certain scales.

      int scale = (int)rint(_globalMapMatrix->getScale(MapMatrix::CurrentScale));
      //qDebug("Scale=%d", scale);

      int skipW = 0;
      int skipH = 0;

      if (scale>1500)
        {
          skipW = skipH = 20;
        }
      else if (scale>1000)
        {
          skipW = skipH = 20;
        }
      else if (scale>725)
        {
          skipW = skipH = 15;
        }
      else if (scale>500)
        {
          skipW = skipH = 12;
        }
      else if (scale>250)
        {
          skipW = skipH = 8;
        }
      else if (scale>150)
        {
          skipW = skipH = 6;
        }
      else if (scale>100)
        {
          skipW = skipH = 4;
        }

      if( GeneralConfig::instance()->getMapLoadIsoLines() == false )
        {
          // draw all ground data in this case, otherwise on
          // higher map scales are blue areas to see.
          skipW = skipH = 0;
        }

      if( tP.boundingRect().width() > skipW &&
          tP.boundingRect().height() > skipH )
        {
          /* qDebug( "DrawReg: x=%d, y=%d, w=%d, h=%d",
                  drawReg.boundingRect().x(),
                  drawReg.boundingRect().y(),
                  drawReg.boundingRect().width(),
                  drawReg.boundingRect().height() ); */

          targetP->setClipRegion( viewRect );

          targetP->drawPolygon(tP);

          if( isolines )
            {
              targetP->drawPolyline(tP);
            }
        }
    }

  // The region is returned for the elevation finding in every
  // case, also when drawing was skipped.
  if( glMapMatrix->isInProjCenterArea(bBox) )
    {
      return new QRegion( tP );
    }

  return static_cast<QRegion *> (0);
}
