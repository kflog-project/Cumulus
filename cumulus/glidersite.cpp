/***********************************************************************
 **
 **   glidersite.cpp
 **
 **   This file is part of Cumulus..
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

#include "glidersite.h"
#include "reachablelist.h"


GliderSite::GliderSite(const QString& n, const QString& icao, const QString& gps,
                       const WGSPoint& wgsPos, const QPoint& pos, unsigned int elev, const char* f, bool w, runway *rw)
  : RadioPoint(n, icao, gps, BaseMapElement::Glidersite, wgsPos, pos, f, elev),
    winch(w)
{
  rwData = rw;

  if(rwData)
    rwNum = 1;

  if( rwData->length != 0 ) {
    int rw2 = 90; // defaut direction is 90 degrees

    if( rwData->direction <= 360 ) {
      rw2 = rwData->direction >= 180 ? rwData->direction-180 : rwData->direction;
    }

    shift = ((rw2)/10);
  } else
    shift = 9; // draw in 90 degress if no data avail

}


GliderSite::~GliderSite()
{
  delete rwData;
}


QString GliderSite::getFrequency() const
{
  return frequency;
}


runway GliderSite::getRunway(int /*index*/) const
{
  return *rwData;
}


unsigned int GliderSite::getRunwayNumber() const
{
  return rwNum;
}


bool GliderSite::isWinch() const
{
  return winch;
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


void GliderSite::drawMapElement(QPainter* targetP, QPainter* /*maskP*/)
{
  if(!__isVisible()) {
    curPos = QPoint(-50, -50);
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
                        shift*iconSize, 0, iconSize, iconSize);
  }
}


