/***********************************************************************
 **
 **   airfield.cpp
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

#include <QtCore>

#include "airfield.h"
#include "reachablelist.h"

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const Runway& rw,
                    const float elevation,
                    const float frequency,
                    const QString country,
                    const QString comment,
                    bool winch,
                    bool towing,
                    bool landable ) :
  SinglePoint(name, shortName, typeId, wgsPos, pos, elevation, country, comment),
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
  if ( rwData.direction/256 <= 36 )
    {
      rwShift = (rwData.direction/256 >= 18 ? (rwData.direction/256)-18 : rwData.direction/256);
    }
}

Airfield::~Airfield()
{
}

QString Airfield::getInfoString() const
{
  QString text, elev;
  QString path = "cumulus/";

  elev = Altitude::getText(elevation, true, 0).replace(QRegExp("\\s"),"&nbsp;");

  text = "<HTML><TABLE BORDER=0><TR><TD>"
         "<IMG SRC=" + path + "/" + glConfig->getPixmapName(typeID) + "></TD>"
         "<TD>" + name;

  if (!icao.isEmpty())
    {
      text += " (" + icao + ")";
    }

  text += "<FONT SIZE=-1><BR><BR>" + elev;

  if (frequency > 0)
    {
      text += "&nbsp;/&nbsp;" + frequencyAsString() + "&nbsp;Mhz.";
    }

  text += "&nbsp;&nbsp;</FONT></TD></TR></TABLE></HTML>";

  return text;
}

bool Airfield::drawMapElement( QPainter* targetP )
{
  if ( ! isVisible() )
    {
      curPos = QPoint(-5000, -5000);
      return false;
    }

  //qDebug("Airfield::drawMapElement(): scale: %d %d",scale, _globalMapMatrix->getScaleRatio()  );
  QColor col = ReachableList::getReachColor( wgsPosition );

  curPos = glMapMatrix->map(position);

  // draw also the small dot's in reachability color
  targetP->setPen(QPen(col, 2));

  // Size of circle for reachability.
  int iconSize = 32;

  if( glConfig->useSmallIcons() )
    {
      iconSize = 16;
    }

  if (col == Qt::green)
    {
      targetP->drawPixmap( curPos.x() - iconSize/2, curPos.y() - iconSize/2,
                           glConfig->getGreenCircle(iconSize) );
    }
  else if (col == Qt::magenta)
    {
      targetP->drawPixmap( curPos.x() - iconSize/2, curPos.y() - iconSize/2,
                           glConfig->getMagentaCircle(iconSize) );
    }

  if( glConfig->isRotatable( typeID ) )
    {
      QPixmap image( glConfig->getPixmapRotatable(typeID, winch) );

      // All icons are squares. So we can only take the height as size parameter.
      int size = image.height();

      targetP->drawPixmap( curPos.x() - size/2,
                           curPos.y() - size/2,
                           image, rwShift*iconSize, 0, size, size );
    }
  else
    {
      QPixmap image( glConfig->getPixmap(typeID) );

      int xOffset = image.width() / 2;
      int yOffset = image.height() / 2;

      if( typeID == BaseMapElement::Outlanding )
       {
         // The lower end of the beacon shall directly point to the point at the map.
         yOffset = image.height();
       }

      targetP->drawPixmap(curPos.x() - xOffset, curPos.y() - yOffset, image  );
    }

  return true;
}
