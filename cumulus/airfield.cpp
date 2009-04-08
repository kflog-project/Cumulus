/***********************************************************************
 **
 **   airfield.cpp
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

#include "airfield.h"
#include "reachablelist.h"
#include "map.h"

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const Runway& rw,
                    const unsigned int elevation,
                    const QString& frequency,
                    bool winch,
                    bool towing,
                    bool landable )
    : SinglePoint(name, shortName, typeId, wgsPos, pos, elevation),
    icao(icao),
    frequency(frequency),
    rwData(rw),
    winch(winch),
    towing(towing),
    landable(landable)
{
  // calculate the default runway shift in 1/10 degrees.
  rwShift = 90/10; // default direction is 90 degrees

  // calculate the real runway shift in 1/10 degrees.
  if ( rwData.direction <= 360 )
    {
      rwShift = (rwData.direction >= 180 ? rwData.direction-180 : rwData.direction) / 10;
    }
}

Airfield::~Airfield()
{
}

QString Airfield::getInfoString() const
  {
    QString text, elev;
    QString path = "cumulus/";

    elev = Altitude::getText(elevation,true,0).replace(QRegExp("\\s"),"&nbsp;");

    text = "<HTML><TABLE BORDER=0><TR><TD>"
           "<IMG SRC=" + path + glConfig->getPixmapName(typeID) + "></TD>"
           "<TD>" + name;
    if (!icao.isEmpty())
      text += " (" + icao + ")";

    text += "<FONT SIZE=-1><BR><BR>" + elev;

    if (!frequency.isEmpty())
      text += "&nbsp;/&nbsp;" + frequency + "&nbsp;Mhz.";

    text += "&nbsp;&nbsp;</FONT></TD></TR></TABLE></HTML>";

    return text;
  }

void Airfield::drawMapElement( QPainter* targetP,
                               const bool drawLabel,
                               const bool drawLabelInfo )
{
  if ( ! isVisible() )
    {
      curPos = QPoint(-5000, -5000);
      return;
    }

  extern MapConfig* _globalMapConfig;
  extern MapMatrix* _globalMapMatrix;

  int scale = _globalMapMatrix->getScaleRatio()/50;

  //qDebug("Airfield::drawMapElement(): scale: %d %d",scale, _globalMapMatrix->getScaleRatio()  );
  QColor col = ReachableList::getReachColor( wgsPosition );

  // draw also the small dot's in reach ability color
  targetP->setPen(QPen(col, 2));
  int iconSize = 32;

  curPos = glMapMatrix->map(position);

  if ( _globalMapConfig->useSmallIcons() )
    {
      iconSize = 16;
    }

  if ( !glMapMatrix->isSwitchScale2() )
    {
      targetP->drawEllipse(curPos.x(), curPos.y(), scale, scale );
    }
  else
    {
      if (col == Qt::green)
        {
          targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2,
                              glConfig->getGreenCircle(iconSize));
        }
      else if (col == Qt::magenta)
        {
          targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2,
                              glConfig->getMagentaCircle(iconSize));
        }

      if ( glConfig->isRotatable( typeID ) )
        {
          QPixmap image( glConfig->getPixmapRotatable(typeID, winch) );
          targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2, image,
                              rwShift*iconSize, 0, iconSize, iconSize);
        }
      else
        {
          QPixmap image( glConfig->getPixmap(typeID) );
          targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2, image  );
        }
    }

  // Draw airfield labels on demand
  if( drawLabel == true )
    {
      Map::getInstance()->drawLabel( targetP,
                                     iconSize / 2 + 3,
                                     shortName,
                                     curPos,
                                     wgsPosition,
                                     landable,
                                     drawLabelInfo );
     }
}
