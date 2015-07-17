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
 **                   2008-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QPainterPath>

#include "airspace.h"
#include "airregion.h"
#include "calculator.h"
#include "generalconfig.h"
#include "mapconfig.h"

Airspace::Airspace() :
  m_lLimitType(BaseMapElement::NotSet),
  m_uLimitType(BaseMapElement::NotSet),
  m_lastVConflict(none),
  m_airRegion(0),
  m_id(-1)
{
  // All Airspaces are closed regions ...
  closed = true;
}

Airspace::Airspace( QString name,
                    BaseMapElement::objectType oType,
                    QPolygon pP,
                    const float upper,
                    const BaseMapElement::elevationType uType,
                    const float lower,
                    const BaseMapElement::elevationType lType,
                    const int identifier,
                    QString country,
		    QString flarmKey ) :
  LineElement(name, oType, pP, false, 0, country),
  m_lLimitType(lType),
  m_uLimitType(uType),
  m_lastVConflict(none),
  m_airRegion(0),
  m_id(identifier),
  m_flarmId(flarmKey)
{
  // All Airspaces are closed regions ...
  closed = true;

  // Normalize values
  double lLim=0.0;

  switch( m_lLimitType )
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

  m_lLimit.setMeters( lLim );
  double uLim=0.0;

  switch( m_uLimitType )
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

  m_uLimit.setMeters( uLim );
  m_lastVConflict=none;
}

Airspace* Airspace::createAirspaceObject()
{
  // We need that method because the default constructor cannot setup a
  // complete airspace. The default constructor is only used as a collection
  // container during parsing of airspace source file.
  Airspace* as = new Airspace( getName(),
                               getTypeID(),
                               getProjectedPolygon(),
                               m_uLimit.getFeet(),
                               m_uLimitType,
                               m_lLimit.getFeet(),
                               m_lLimitType,
                               m_id,
                               getCountry(),
			       m_flarmId );

  return as;
}

Airspace::~Airspace()
{
  // @AP: Remove the class pointer in related AirRegion object. We
  // have a cross reference here. First deleted object will reset its
  // pointer in the other object. Check is necessary to avoid usage of
  // null pointer.

  if( m_airRegion )
    {
      m_airRegion->airspace = 0;
    }
}

/**
 * Tells the caller, if the airspace is drawable or not
 */
bool Airspace::isDrawable() const
{
  return ( GeneralConfig::instance()->getItemDrawingEnabled(typeID) &&
           glConfig->isBorder(typeID) &&
           isVisible() );
};

void Airspace::drawRegion( QPainter* targetP, qreal opacity )
{
  // qDebug("Airspace::drawRegion(): TypeId=%d, opacity=%f, Name=%s",
  //         typeID, opacity, getInfoString().toLatin1().data() );

  if( !GeneralConfig::instance()->getItemDrawingEnabled(typeID) ||
      ! glConfig->isBorder(typeID) || ! isVisible())
    {
      return;
    }

  QPolygon mP = glMapMatrix->map(projPolygon);

  QPainterPath pp;

  if( mP.size() < 3 )
    {
      return;
    }

  pp.moveTo( mP.at(0) );

  for( int i = 1; i < mP.size(); i++ )
    {
      pp.lineTo( mP.at(i) );
    }

  pp.closeSubpath();

  QBrush drawB( glConfig->getDrawBrush(typeID) );

  if( opacity <= 100.0 )
    {
      // use solid filled air regions
      drawB.setStyle( Qt::SolidPattern );
    }

  QPen drawP = glConfig->getDrawPen(typeID);
  drawP.setJoinStyle(Qt::RoundJoin);

  int lw = GeneralConfig::instance()->getAirspaceLineWidth();

  extern MapConfig* _globalMapConfig;

  if( lw > 1 && _globalMapConfig->useSmallIcons() )
    {
      lw = (lw + 1) / 2;
    }

  drawP.setWidth(lw);

  targetP->setPen(drawP);
  targetP->setBrush(drawB);

  if( opacity <= 100.0 && opacity > 0.0 )
    {
      // Draw airspace filled with opacity factor
      targetP->setOpacity( opacity/100.0 );
      targetP->drawPolygon(mP);

      // Reset opacity, that a solid line is drawn as next
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
  QPolygon mP = glMapMatrix->map(projPolygon);

  QPainterPath *path = new QPainterPath;
  path->addPolygon(mP);
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
    case BaseMapElement::AirFir:
      return QObject::tr("FIR");
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
    case BaseMapElement::Ctr:
      return QObject::tr("CTR");
    case BaseMapElement::LowFlight:
      return QObject::tr("Low Flight");
    case BaseMapElement::Rmz:
      return QObject::tr("RMZ");
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

  switch(m_lLimitType) {
  case MSL:
    tempL.sprintf("%s MSL", m_lLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    if(m_lLimit.getMeters())
      tempL.sprintf("%s GND", m_lLimit.getText(true,0).toLatin1().data());
    else
      tempL = "GND";
    break;
  case FL:
    tempL.sprintf("FL %d (%s)", (int) rint(m_lLimit.getFeet()/100.), m_lLimit.getText(true,0).toLatin1().data());
    break;
  case STD:
    tempL.sprintf("%s STD", m_lLimit.getText(true,0).toLatin1().data());
    break;
  case UNLTD:
    tempL = QObject::tr("Unlimited");
    break;
  default:
    break;
  }

  switch(m_uLimitType) {
  case MSL:
    if(m_uLimit.getMeters() >= 99999)
      tempU = QObject::tr("Unlimited");
    else
      tempU.sprintf("%s MSL", m_uLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    tempU.sprintf("%s GND", m_uLimit.getText(true,0).toLatin1().data());
    break;
  case FL:
    tempU.sprintf("FL %d (%s)", (int) rint(m_uLimit.getFeet()/100.), m_uLimit.getText(true,0).toLatin1().data());
    break;
  case STD:
    tempU.sprintf("%s STD", m_uLimit.getText(true,0).toLatin1().data());
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
Airspace::ConflictType Airspace::conflicts( const AltitudeCollection& alt,
                                            const AirspaceWarningDistance& dist ) const
{
  Altitude lowerAlt(0);
  Altitude upperAlt(0);

  //set which altitude to use from our range of available altitudes,
  //and apply uncertainty margins

  switch (m_lLimitType)
    {
      case NotSet:
        break;
      case MSL:
        lowerAlt = alt.gpsAltitude;
        break;
      case GND:
        lowerAlt = alt.gndAltitude + alt.gndAltitudeError; // we need to use a conservative estimate
        if (m_lLimit == 0)
          lowerAlt.setMeters(1); // we're always above ground
        break;
      case FL:
      case STD:
        lowerAlt = alt.stdAltitude; // flight levels are always at pressure altitude!
        break;
      case UNLTD:
        m_lastVConflict = none;
        return none;
    }

  switch (m_uLimitType)
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
        upperAlt = m_uLimit - 1; //we are always below the upper border of an Unlimited airspace
        break;
    }

  //check to see if we're inside the airspace
  if ((lowerAlt.getMeters() >= m_lLimit.getMeters()) &&
      (upperAlt.getMeters() <= m_uLimit.getMeters()))
    {
      m_lastVConflict = inside;
      // qDebug("vertical conflict: %d, airspace: %s", _lastVConflict, getName().latin1());
      return inside;
    }

  // @AP: very near and near will not work, if you use the defined
  // operators. Changed it to the getMeters method, that will work
  // fine.

  //not inside. Check to see if we're very near to the airspace
  if ((lowerAlt.getMeters() >= (m_lLimit.getMeters() - dist.verBelowVeryClose.getMeters())) &&
      (upperAlt.getMeters() <= (m_uLimit.getMeters() + dist.verAboveVeryClose.getMeters())))
    {
      m_lastVConflict = veryNear;
      return veryNear;
    }

  //not very near. Just near then?
  if ((lowerAlt.getMeters() >= (m_lLimit.getMeters() - dist.verBelowClose.getMeters())) &&
      (upperAlt.getMeters() <= (m_uLimit.getMeters() + dist.verAboveClose.getMeters())))
    {
      m_lastVConflict = near;
      return near;
    }

  //nope, we're not even near.
  m_lastVConflict=none;
  return none;
}

bool Airspace::operator < (const Airspace& other) const
{
  int a1C = getUpperL(), a2C = other.getUpperL();

  if (a1C > a2C)
    {
      return false;
    }
  else if (a1C < a2C)
    {
      return true;
    }
  else
    { //equal
      int a1F = getLowerL(), a2F = other.getLowerL();
      return (a1F < a2F);
    }
}

void Airspace::debug()
{
  qDebug() << "AsName=" << getName()
           << "ASId=" << getId()
           << "AsType=" << getTypeName(getTypeID())
           << "Country=" << getCountry()
           << "ULimit=" << m_uLimit.getMeters()
           << "LLimit=" << m_lLimit.getMeters();
}
