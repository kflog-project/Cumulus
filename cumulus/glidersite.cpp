/***********************************************************************
 **
 **   glidersite.cpp
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

#include "glidersite.h"
#include "reachablelist.h"

GliderSite::GliderSite( const QString& name,
                        const QString& icao,
                        const QString& shortName,
                        const WGSPoint& wgsPos,
                        const QPoint& pos,
                        const Runway& rw,
                        const unsigned int elevation,
                        const QString& frequency,
                        bool winch,
                        bool towing )
  : SinglePoint(name, shortName, BaseMapElement::Glidersite, wgsPos, pos, elevation),
    icao(icao),
    frequency(frequency),
    rwData(rw),
    winch(winch),
    towing(towing)
{
  // calculate the default runway shift in 1/10 degrees.
  rwShift = 90/10; // default direction is 90 degrees

  // calculate the real runway shift in 1/10 degrees.
  if( rwData.direction <= 360 ) {
    rwShift = (rwData.direction >= 180 ? rwData.direction-180 : rwData.direction) / 10;
  }
}

GliderSite::~GliderSite()
{
}

QString GliderSite::getInfoString() const
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

void GliderSite::drawMapElement(QPainter* targetP)
{
  if( ! isVisible()) {
    curPos = QPoint(-5000, -5000);
    return;
  }

  extern MapMatrix * _globalMapMatrix;
  int scale = _globalMapMatrix->getScaleRatio()/50;
  QColor col = ReachableList::getReachColor( wgsPosition );
  // ReachableList::reachable reachable=ReachableList::getReachable(name);
  targetP->setPen(QPen(col, 2));  // draw small dots also colored

  curPos = glMapMatrix->map(position);

  int iconSize = 16;
  if(glMapMatrix->isSwitchScale())
    iconSize = 32;

  if( !glMapMatrix->isSwitchScale2() )
    targetP->drawEllipse(curPos.x(), curPos.y(), scale,scale );
  else {
    if (col == Qt::green) {
      //draw red circle
      targetP->drawPixmap(curPos.x() - 9, curPos.y() -9, glConfig->getPixmap("green_circle.xpm"));
    } else if (col == Qt::magenta) {
      //draw magenta circle
      targetP->drawPixmap(curPos.x() - 9, curPos.y() -9, glConfig->getPixmap("magenta_circle.xpm"));
    }

    QPixmap image( glConfig->getPixmapRotatable(typeID, winch) );
    targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2, image,
                        rwShift*iconSize, 0, iconSize, iconSize);
  }
}


