/***********************************************************************
 **
 **   airport.cpp
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

#include <QObject>

#include "airport.h"
#include "reachablelist.h"

QHash<int, QString> Airport::surfaceTranslations;
QStringList Airport::sortedTranslations;

Airport::Airport(const QString& n, const QString& i,
                 const QString& abbr, BaseMapElement::objectType t,
                 const WGSPoint& wgsPos, const QPoint& pos,
                 unsigned int e, const QString& f,
                 bool v, runway *rw )
  : RadioPoint(n, i, abbr, t, wgsPos, pos, f, e), vdf(v)
{
  rwData = rw;

  if(rwData)
    rwNum = 1;

  int rw2 = 90; // defaut direction is 90 degrees

  if( rwData->direction <= 360 ) {
    rw2 = rwData->direction >= 180 ? rwData->direction-180 : rwData->direction;
  }

  shift = ((rw2)/10);
}


Airport::~Airport()
{
  delete rwData;
}


QString Airport::getFrequency() const
{
  return frequency;
}


runway Airport::getRunway(int /*index*/) const
{
  return *rwData;
}


unsigned int Airport::getRunwayNumber() const
{
  return rwNum;
}


QString Airport::getInfoString() const
{
  QString text, elev;
  QString path = "cumulus/";

  elev = Altitude::getText(elevation,true,0).replace(QRegExp("\\s"),"&nbsp;");
  text = "<QT><TABLE BORDER=0><TR><TD>"
    "<IMG SRC=" + path + glConfig->getPixmapName(typeID) + "></TD>"
    "<TD>" + name;
  if (!icao.isEmpty())
    text += " (" + icao + ")";
  text += "<FONT SIZE=-1><BR><BR>" + elev;
  if (!frequency.isEmpty())
    text += "&nbsp;/&nbsp;" + frequency + "&nbsp;Mhz.";
  text += "&nbsp;&nbsp;</FONT></TD></TR></TABLE></QT>";

  return text;
}


/**
 * Get translation string for surface type.
 */

QString Airport::item2Text( const int surfaceType, QString defaultValue )
{
  if( surfaceTranslations.isEmpty() )
    {
      loadTranslations();
    }

  return surfaceTranslations.value( surfaceType, defaultValue );
}

/**
 * Get surface typee for translation string.
 */
const int Airport::text2Item( const QString& text )
{
  if( surfaceTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTranslations();
    }

  return surfaceTranslations.key( text );
}

void  Airport::loadTranslations()
{
  // Load translation data
  surfaceTranslations.insert( Airport::NotSet, QObject::tr( "Unknown" ) );
  surfaceTranslations.insert( Airport::Grass, QObject::tr( "Grass" ) );
  surfaceTranslations.insert( Airport::Asphalt, QObject::tr( "Asphalt" ) );
  surfaceTranslations.insert( Airport::Concrete, QObject::tr( "Concrete" ) );

  // load sorted translation strings
  for( int i=0; i < surfaceTranslations.size(); i++ )
    {
      sortedTranslations.append( surfaceTranslations.value(i) );
    }

  sortedTranslations.sort();  
}

void Airport::drawMapElement(QPainter* targetP, QPainter*/* maskP*/)
{
  if(!__isVisible()) {
    curPos = QPoint(-50, -50);
    return;
  }

  extern MapMatrix * _globalMapMatrix;
  int scale = _globalMapMatrix->getScaleRatio()/50;

  // qDebug("scale: %d %d",scale,_globalMapMatrix->getScaleRatio()  );
  QColor col = ReachableList::getReachColor( wgsPosition );

  // ReachableList::reachable reachable=ReachableList::getReachable(name);
  targetP->setPen(QPen(col, 2));  // draw also the small dot's in reachability color
  int iconSize = 16;

  curPos = glMapMatrix->map(position);

  if(glMapMatrix->isSwitchScale())
    iconSize = 32;

  if( !glMapMatrix->isSwitchScale2() )
    targetP->drawEllipse(curPos.x(), curPos.y(), scale,scale );
  else {
    if (col == Qt::green) {
      targetP->drawPixmap(curPos.x() - 9, curPos.y() -9,
                          glConfig->getPixmap("green_circle.xpm"));
    } else if (col == Qt::magenta) {
      targetP->drawPixmap(curPos.x() - 9, curPos.y() -9,
                          glConfig->getPixmap("magenta_circle.xpm"));
    }
    if( glConfig->isRotatable( typeID ) ) {
      QPixmap image( glConfig->getPixmapRotatable(typeID, false) );
      targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2, image,
                          shift*iconSize, 0, iconSize, iconSize);
    } else {
      QPixmap image( glConfig->getPixmap(typeID) );
      targetP->drawPixmap(curPos.x() - iconSize/2, curPos.y() - iconSize/2, image  );
    }
  }

}


