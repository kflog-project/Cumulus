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
 **                   2008-2025 by Axel Pauli
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
#include "time_cu.h"

Airspace::Airspace() :
  LineElement(),
  m_lLimitType(BaseMapElement::NotSet),
  m_uLimitType(BaseMapElement::NotSet),
  m_lastVConflict(none),
  m_airRegion(0),
  m_icaoClass( AS_Unkown ),
  m_activity( 0 ),
  m_byNotam( false )
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
                    const QList<Frequency> frequencyList,
                    const int icaoClass,
                    QString country,
                    quint8 activity,
                    bool byNotam ) :
  LineElement(name, oType, pP, false, 0, country),
  m_lLimitType(lType),
  m_uLimitType(uType),
  m_frequencyList(frequencyList),
  m_lastVConflict(none),
  m_airRegion(0),
  m_icaoClass(icaoClass),
  m_activity(activity),
  m_byNotam(byNotam)
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
  }

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
  }

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
                               m_frequencyList,
                               m_icaoClass,
                               getCountry(),
                               m_activity,
                               isByNotam() );

  as->setFlarmAlertZone( m_flarmAlertZone );
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
      m_airRegion->m_airspace = 0;
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

  if( ! GeneralConfig::instance()->getItemDrawingEnabled(typeID) ||
      ! glConfig->isBorder(typeID) || ! isVisible())
    {
      return;
    }

  if( m_flarmAlertZone.isValid() )
    {
      // A Flarm Alert zone has to be drawn.
      drawFlarmAlertZone( targetP, opacity );
      return;
    }

  QPolygon mP = glMapMatrix->map(projPolygon);

  if( mP.size() < 3 )
    {
      return;
    }

  QPainterPath pp;

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

void Airspace::drawFlarmAlertZone( QPainter* targetP, qreal opacity )
{
  if( ! GeneralConfig::instance()->getItemDrawingEnabled(typeID) ||
      ! glConfig->isBorder(typeID) || ! isVisible() )
    {
      return;
    }

  if( m_flarmAlertZone.isValid() == false || m_flarmAlertZone.isActive() == false )
    {
      return;
    }

  // Map radius to map scale
  int scaledRadius = (int) rint( double(m_flarmAlertZone.Radius) / glMapMatrix->getScale() );

  // project center point to map
  QPoint pcp = glMapMatrix->wgsToMap( m_flarmAlertZone.Latitude, m_flarmAlertZone.Longitude );

  // map projected center point to map display
  QPoint mcp = glMapMatrix->map( pcp );

  QBrush drawB( glConfig->getDrawBrush(typeID) );

  if( opacity <= 100.0 )
    {
      // use solid filled air regions
      drawB.setStyle( Qt::SolidPattern );
    }

  QPen drawP = glConfig->getDrawPen(typeID);
  drawP.setJoinStyle(Qt::RoundJoin);

  int lw = GeneralConfig::instance()->getAirspaceLineWidth();

  if( lw > 1 && glConfig->useSmallIcons() )
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
      targetP->drawEllipse( mcp.x() - scaledRadius, mcp.y() - scaledRadius,
                            scaledRadius * 2, scaledRadius * 2 );

      // Reset opacity, that a solid line is drawn as next
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }
  else if( opacity == 0.0 )
    {
      // Draw only the alert zone borders without any filling inside
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }

  // Draw the outline of the alert zone with the selected brush
  targetP->drawEllipse( mcp.x() - scaledRadius, mcp.y() - scaledRadius,
                        scaledRadius * 2, scaledRadius * 2 );
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
    case BaseMapElement::AirFlarm:
      return QObject::tr("Flarm");
    case BaseMapElement::AirFir:
      return QObject::tr("FIR");
    case BaseMapElement::AirG:
      return QObject::tr("AS-G");
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
    case BaseMapElement::Sua:
      return QObject::tr("SUA");
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

  switch( m_lLimitType )
  {
    case MSL:
      tempL = QString( "%1 ft (%2 m) MSL" ).arg( m_lLimit.getFeet(), 0, 'f', 0 )
                                           .arg( m_lLimit.getMeters(), 0, 'f', 0 );
      break;
    case GND:
      if( m_lLimit.getMeters() )
        tempL = QString( "%1 ft (%2 m) AGL" ).arg( m_lLimit.getFeet(), 0, 'f', 0 )
                                             .arg( m_lLimit.getMeters(), 0, 'f', 0 );
      else
        tempL = "GND";
      break;
    case FL:
      tempL = QString( "FL %1 (%2)" ).arg( (int) rint( m_lLimit.getFeet() / 100.) )
                                     .arg( m_lLimit.getText( true, 0 ) );
      break;
    case STD:
      tempL = QString( "%1 STD" ).arg( m_lLimit.getText(true,0) );
      break;
    case UNLTD:
      tempL = QObject::tr("Unlimited");
      break;
    default:
      break;
  }

  switch( m_uLimitType )
  {
    case MSL:
      if(m_uLimit.getMeters() >= 99999)
        tempU = QObject::tr("Unlimited");
      else
        tempU = QString( "%1 ft (%2 m) MSL" ).arg( m_uLimit.getFeet(), 0, 'f', 0 )
                                             .arg( m_uLimit.getMeters(), 0, 'f', 0 );
      break;
    case GND:
      tempU = QString( "%1 ft (%2 m) AGL" ).arg( m_uLimit.getFeet(), 0, 'f', 0 )
                                           .arg( m_uLimit.getMeters(), 0, 'f', 0 );

      break;
    case FL:
      tempU = QString( "FL %1 (%2)" ).arg( (int) rint(m_uLimit.getFeet() / 100.) )
                                     .arg( m_uLimit.getText(true,0) );
      break;
    case STD:
      tempU = QString( "%1 STD" ).arg( m_uLimit.getText(true,0) );
      break;
    case UNLTD:
      tempU = QObject::tr("Unlimited");
      break;
    default:
      break;
  }

  text = getTypeName(typeID);

  if( m_flarmAlertZone.isValid() )
    {
      text += " " + FlarmBase::translateAlarmType( m_flarmAlertZone.ZoneType );
    }
  else
    {
      if( name.startsWith( text ) == true )
        {
          // An airspace name can start with the type name. In this case
          // suppress the type name at the beginning.
          text = name;
        }
      else
        {
          text += " " + name;
        }

      if( getIcaoClass() <=6 )
        {
          QString tmp = QString( QObject::tr("AS-") );
          tmp.append( QChar(getIcaoClass() + 0x41 ) );
          tmp.append( ": " );
          text.insert( 0, tmp );
        }

      // Handle airspace frequency. Often it is already contained in the
      // airspace name.
      if( m_frequencyList.size() > 0 )
        {
          Frequency fq;
          bool found = Frequency::getMainFrequency( m_frequencyList, fq );

          if( found == true )
            {
              QString fqstr = fq.frequencyAsString( false );

              text += "<BR>";

              // Look, if frequency has a call name defined
              if( fq.getCallSign().isEmpty() == false )
                {
                  text += fq.getCallSign() + " ";
                }

              text += fqstr;
            }
        }
    }

  text += QString("<BR>") + "<FONT SIZE=-1>" + tempL + " / " + tempU;

  if( m_flarmAlertZone.isValid() )
    {
      text += ", ";

      if( m_flarmAlertZone.ActivityLimit == 0 )
        {
          text += QObject::tr("always active");
        }
      else
        {
          QDateTime dt;
          dt.setMSecsSinceEpoch( qint64(m_flarmAlertZone.ActivityLimit) * 1000 );

          QString dtString;

          if( Time::getTimeUnit() == Time::local )
            {
              dtString = dt.toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
            }
          else
            {
              dtString = dt.toTimeSpec(Qt::UTC).toString("yyyy-MM-dd hh:mm:ss") + " UTC";
            }

          text += QObject::tr("active until ") + dtString;
        }
    }

  text += "</FONT>";

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
           << "AsType=" << getTypeName(getTypeID())
           << "IcaoClass=" << getIcaoClass()
           << "Country=" << getCountry()
           << "ULimit=" << m_uLimit.getMeters()
           << "LLimit=" << m_lLimit.getMeters();
}
