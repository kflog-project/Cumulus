/***********************************************************************
 **
 **   airfield.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2015 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtCore>

#include "airfield.h"
#include "layout.h"
#include "mapconfig.h"
#include "reachablelist.h"

extern MapConfig* _globalMapConfig;

int Airfield::instances = 0;

QPixmap* Airfield::m_bigAirfields = 0;
QPixmap* Airfield::m_smallAirfields = 0;

Airfield::Airfield() :
  SinglePoint(),
  m_frequency(0.0),
  m_atis(0.0),
  m_winch(false),
  m_towing(false),
  m_rwShift(0),
  m_landable(true)
 {
  instances++;
  createStaticIcons();
 }

Airfield::Airfield( const QString& name,
                    const QString& icao,
                    const QString& shortName,
                    const BaseMapElement::objectType typeId,
                    const WGSPoint& wgsPos,
                    const QPoint& pos,
                    const QList<Runway>& rwList,
                    const float elevation,
                    const float frequency,
                    const QString country,
                    const QString comment,
                    bool winch,
                    bool towing,
                    bool landable,
                    const float atis ) :
  SinglePoint(name, shortName, typeId, wgsPos, pos, elevation, country, comment),
  m_icao(icao),
  m_frequency(frequency),
  m_atis(atis),
  m_rwList(rwList),
  m_winch(winch),
  m_towing(towing),
  m_rwShift(0),
  m_landable(landable)
{
  instances++;
  createStaticIcons();
  calculateRunwayShift();
}

void Airfield::createStaticIcons()
{
  if( m_bigAirfields == 0 )
    {
      qDebug() << "Airfield::createStaticIconsBig()";

      m_bigAirfields = new QPixmap[36];

      for( int i = 0; i < 360/10; i++ )
        {
	  m_bigAirfields[i] = _globalMapConfig->getAirfield( i*10, 32, false );
        }
    }

  if( m_smallAirfields == 0 )
    {
      m_smallAirfields = new QPixmap[36];

      for( int i = 0; i < 360/10; i++ )
        {
	  m_smallAirfields[i] = _globalMapConfig->getAirfield( i*10, 16, true );
        }
    }
}

Airfield::~Airfield()
{
  instances--;
}

QString Airfield::getInfoString() const
{
  QString text, elev;
  QString path = "cumulus/";

  elev = Altitude::getText(elevation, true, 0).replace(QRegExp("\\s"),"&nbsp;");

  text = "<HTML><TABLE BORDER=0><TR><TD>"
         "<IMG SRC=" + path + "/" + glConfig->getPixmapName(typeID) + "></TD>"
         "<TD>" + name;

  if (!m_icao.isEmpty())
    {
      text += " (" + m_icao + ")";
    }

  text += "<FONT SIZE=-1><BR><BR>" + elev;

  if (m_frequency > 0)
    {
      text += "&nbsp;/&nbsp;" + frequencyAsString(m_frequency) + "&nbsp;Mhz.";
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
  int iconSize = 32 * Layout::getIntScaledDensity();

  if( glConfig->useSmallIcons() )
    {
      iconSize /= 2;
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
      QPixmap pm = glConfig->useSmallIcons() ? m_smallAirfields[m_rwShift] : m_bigAirfields[m_rwShift];

      targetP->drawPixmap( curPos.x() - pm.width() / 2,
                           curPos.y() - pm.height() / 2,
                           pm );
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
