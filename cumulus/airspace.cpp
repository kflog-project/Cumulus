/***********************************************************************
 **
 **   airspace.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **   Modified:       2008      by Josua Dietze
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QPainterPath>

#include "airspace.h"
#include "airregion.h"
#include "generalconfig.h"
#include "calculator.h"

Airspace::Airspace(QString name, BaseMapElement::objectType oType, QPolygon pP,
                   int upper, BaseMapElement::elevationType uType,
                   int lower, BaseMapElement::elevationType lType)
  : LineElement(name, oType, pP), lLimitType(lType), uLimitType(uType),
    _airRegion(0)
{
  // All Airspaces are closed regions ...
  closed = true;

  // Normalize values
  double lLim=0.0;
  switch( lLimitType )
  {
  case GND:
  case MSL:
  case STD:
    lLim = Distance::mFromFeet*lower;
    break;
  case FL:
    lLim = Distance::mFromFeet*100.0*lower;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    lLim=0.0;
    break;
  };

  lLimit.setMeters( lLim );
  double uLim=0.0;
  switch( uLimitType )
  {
  case GND:
  case MSL:
  case STD:
    uLim = Distance::mFromFeet*upper;
    break;
  case FL:
    uLim = Distance::mFromFeet*100.0*upper;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    uLim=0.0;
    break;
  };

  uLimit.setMeters( uLim );
  _lastVConflict=none;
}


Airspace::~Airspace()
{
  // @AP: Remove the class pointer in related AirRegion object. We
  // have a cross reference here. First deleted object will reset its
  // pointer in the other object. Check is necessary to avoid usage of
  // null pointer.

  if( _airRegion )
    {
      _airRegion->airspace = 0;
    }
}

/**
 * Tells the caller, if the airspace is drawable or not
 */
bool Airspace::isDrawable() const
{
  return ( GeneralConfig::instance()->getAirspaceDrawingEnabled(typeID) &&
           glConfig->isBorder(typeID) &&
           isVisible() );
};

void Airspace::drawRegion( QPainter* targetP, const QRect &viewRect,
                              qreal opacity )
{
  // qDebug("Airspace::drawRegion(): TypeId=%d, opacity=%f, Name=%s",
  //         typeID, opacity, getInfoString().toLatin1().data() );

  if( !GeneralConfig::instance()->getAirspaceDrawingEnabled(typeID) ||
      ! glConfig->isBorder(typeID) || ! isVisible())
    {
      return;
    }

  // @JD: replaced clipping and filling with polygon drawing,
  //      regions not needed anymore
  QPolygon tP = glMapMatrix->map(projPolygon);

  QPainterPath pp;

  if( tP.size() < 3 )
    {
      return;
    }

  pp.moveTo( tP.at(0).x(), tP.at(0).y() );

  for( int i = 1; i < tP.size(); i++ )
    {
      pp.lineTo( tP.at(i).x(), tP.at(i).y() );
    }

  pp.closeSubpath();

  QBrush drawB( glConfig->getDrawBrush(typeID) );

  if( opacity < 100.0 )
    {
      // use solid filled air regions
      drawB.setStyle( Qt::SolidPattern );
    }

  QPen drawP = glConfig->getDrawPen(typeID);
  drawP.setJoinStyle(Qt::RoundJoin);
  // increase drawPen, it is to small under X11
  //@JD: polygon drawing blew it up further for some reason, thus reduced increasing
  drawP.setWidth(drawP.width() + 1);

  targetP->setPen(drawP);
  targetP->setBrush(drawB);

  targetP->setClipRegion( viewRect );

  if( opacity < 100.0 && opacity > 0.0 )
    {
      // Draw airspace filled with opacity factor
      targetP->setOpacity( opacity/100.0 );
      targetP->drawPolygon(tP);
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }
  else if( opacity == 0.0 )
    {
      // draw only airspace borders without any filling inside
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }

  // Draw the outline of the airspace with the selected brush
  targetP->drawPath(pp);
}

/**
 * Return a pointer to the mapped airspace region data. The caller takes
 * the ownership about the returned object.
 */
QPainterPath* Airspace::createRegion()
{
  QPolygon tP = glMapMatrix->map(projPolygon);

  QPainterPath *path = new QPainterPath;
  path->addPolygon(tP);
  path->closeSubpath();
  return path;
}

/**
 * Returns a text representing the type of the airspace
 */
QString Airspace::getTypeName (objectType type)
{
  switch(type)
  {
    case BaseMapElement::AirA:
      return QObject::tr("AS-A");
    case BaseMapElement::AirB:
      return QObject::tr("AS-B");
    case BaseMapElement::AirC:
      return QObject::tr("AS-C");
    case BaseMapElement::AirD:
      return QObject::tr("AS-D");
    case BaseMapElement::AirE:
      return QObject::tr("AS-E");
    case BaseMapElement::WaveWindow:
      return QObject::tr("Wave Window");
    case BaseMapElement::AirF:
      return QObject::tr("AS-F");
    case BaseMapElement::Restricted:
      return QObject::tr("Restricted");
    case BaseMapElement::Danger:
      return QObject::tr("Danger");
    case BaseMapElement::Prohibited:
      return QObject::tr("Prohibited");
    case BaseMapElement::ControlC:
      return QObject::tr("CTR-C");
    case BaseMapElement::ControlD:
      return QObject::tr("CTR-D");
    case BaseMapElement::LowFlight:
      return QObject::tr("Low Flight");
    case BaseMapElement::Tmz:
      return QObject::tr("TMZ");
    case BaseMapElement::GliderSector:
      return QObject::tr("Glider Sector");
    default:
      return "<B><EM>" + QObject::tr("unknown") + "</EM></B>";
  }
}


QString Airspace::getInfoString() const
{
  QString text, tempL, tempU;

  QString type;

  switch(lLimitType) {
  case MSL:
    tempL.sprintf("%s MSL", lLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    if(lLimit.getMeters())
      tempL.sprintf("%s GND", lLimit.getText(true,0).toLatin1().data());
    else
      tempL = "GND";
    break;
  case FL:
    tempL.sprintf("FL %d (%s)", (int) rint(lLimit.getFeet()/100.), lLimit.getText(true,0).toLatin1().data());
    break;
  case STD:
    tempL.sprintf("%s STD", lLimit.getText(true,0).toLatin1().data());
    break;
  case UNLTD:
    tempL = QObject::tr("Unlimited");
    break;
  default:
    break;
  }

  switch(uLimitType) {
  case MSL:
    if(uLimit.getMeters() >= 99999)
      tempU = QObject::tr("Unlimited");
    else
      tempU.sprintf("%s MSL", uLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    tempU.sprintf("%s GND", uLimit.getText(true,0).toLatin1().data());
    break;
  case FL:
    tempU.sprintf("FL %d (%s)", (int) rint(uLimit.getFeet()/100.), uLimit.getText(true,0).toLatin1().data());
    break;
  case STD:
    tempU.sprintf("%s STD", uLimit.getText(true,0).toLatin1().data());
    break;
  case UNLTD:
    tempU = QObject::tr("Unlimited");
    break;
  default:
    break;
  }

  text = getTypeName(typeID);

  text += " " + name + "<BR>" +
    "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}


/**
 * Returns true if the given altitude conflicts with the airspace
 * properties. Only the altitude is considered not the current
 * position.
 */
//bool Airspace::conflicts (const Altitude& alt) const
Airspace::ConflictType Airspace::conflicts (const AltitudeCollection& alt,
                                            const AirspaceWarningDistance& dist) const
{
  Altitude lowerAlt(0);
  Altitude upperAlt(0);

  //set which altitude to use from our range of available altitudes,
  //and apply uncertainty margins

  //#warning FIXME: we should take our GPS error into account
  switch (lLimitType)
    {
      case NotSet:
        break;
      case MSL:
        lowerAlt = alt.gpsAltitude;
        break;
      case GND:
        lowerAlt = alt.gndAltitude + alt.gndAltitudeError; // we need to use a conservative estimate
        if (lLimit == 0)
          lowerAlt.setMeters(1); // we're always above ground
        break;
      case FL:
      case STD:
        lowerAlt = alt.stdAltitude; // flight levels are always at pressure altitude!
        break;
      case UNLTD:
        _lastVConflict = none;
        return none;
    }

  switch (uLimitType)
    {
      case NotSet:
        upperAlt.setMeters(100000);
        break;
      case MSL:
        upperAlt = alt.gpsAltitude;
        break;
      case GND:
        upperAlt = alt.gndAltitude - alt.gndAltitudeError; //we need to use a conservative estimate
        break;
      case FL:
      case STD:
        upperAlt = alt.stdAltitude;
        break;
      case UNLTD:
        upperAlt = uLimit - 1; //we are always below the upper border of an Unlimited airspace
        break;
    }

  //check to see if we're inside the airspace
  if ((lowerAlt.getMeters() >= lLimit.getMeters()) &&
      (upperAlt.getMeters() <= uLimit.getMeters())) {
    _lastVConflict=inside;
    // qDebug("vertical conflict: %d, airspace: %s", _lastVConflict, getName().latin1());
    return inside;
  }

  // @AP: very near and near will not work, if you use the defined
  // operators. Changed it to the getMeters method, that will work
  // fine.

  //not inside. Check to see if we're very near to the airspace
  if ((lowerAlt.getMeters() >= (lLimit.getMeters() - dist.verBelowVeryClose.getMeters())) &&
      (upperAlt.getMeters() <= (uLimit.getMeters() + dist.verAboveVeryClose.getMeters()))) {
    _lastVConflict=veryNear;
    return veryNear;
  }

  //not very near. Just near then?
  if ((lowerAlt.getMeters() >= (lLimit.getMeters() - dist.verBelowClose.getMeters())) &&
      (upperAlt.getMeters() <= (uLimit.getMeters() + dist.verAboveClose.getMeters()))) {
    _lastVConflict=near;
    return near;
  }

  //nope, we're not even near.
  _lastVConflict=none;
  return none;
}

bool Airspace::operator < (const Airspace& other) const
{
  int a1C = getUpperL(), a2C = other.getUpperL();

  if (a1C > a2C) {
    return false;
  } else if (a1C < a2C) {
    return true;
  } else { //equal
    int a1F = getLowerL(), a2F = other.getLowerL();
    return (a1F < a2F);
  }
}
