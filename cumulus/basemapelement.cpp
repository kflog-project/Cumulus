/***********************************************************************
**
**   basemapelement.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2008 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QObject>

#include "basemapelement.h"

MapMatrix* BaseMapElement::glMapMatrix = 0;
MapConfig* BaseMapElement::glConfig = 0;

QHash<int, QString> BaseMapElement::objectTranslations;
QStringList         BaseMapElement::sortedTranslations;

BaseMapElement::BaseMapElement(const QString& n, objectType tID, unsigned int secID)
  : name(n), typeID(tID), MapSegment(secID)
{
}


BaseMapElement::~BaseMapElement()
{
}


bool BaseMapElement::__isVisible() const
{
  return true;
}


void BaseMapElement::initMapElement(MapMatrix* matrix, MapConfig* config)
{
  glMapMatrix = matrix;
  glConfig = config;

  qDebug("BaseMapElement::initMapElement ()");
}

/**
 * Get translation string for BaseMapelement object type.
 */

QString BaseMapElement::item2Text( const int objectType, QString defaultValue )
{
  if( objectTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTranslations();
    }

  return objectTranslations.value( objectType, defaultValue );
}

/**
 * Get BaseMapelement objectType for translation string.
 */
const int BaseMapElement::text2Item( const QString& text )
{
  if( objectTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTranslations();
    }

  return objectTranslations.key( text );
}

/**
 * Load object - translation data
 */
void BaseMapElement::loadTranslations()
{
  objectTranslations.insert( BaseMapElement::AerialRailway, QObject::tr( "Aerial railway" ) );
  objectTranslations.insert( BaseMapElement::Airfield, QObject::tr( "Airfield" ) );
  objectTranslations.insert( BaseMapElement::Airport, QObject::tr( "Airport" ) );
  objectTranslations.insert( BaseMapElement::AmbHeliport, QObject::tr( "Ambul. Airport" ) );
  objectTranslations.insert( BaseMapElement::Balloon, QObject::tr( "Balloon" ) );
  objectTranslations.insert( BaseMapElement::City, QObject::tr( "City" ) );
  objectTranslations.insert( BaseMapElement::CivHeliport, QObject::tr( "Civil Heliport" ) );
  objectTranslations.insert( BaseMapElement::IntAirport, QObject::tr( "Int. Airport" ) );
  objectTranslations.insert( BaseMapElement::MilAirport, QObject::tr( "Mil. Airport" ) );
  objectTranslations.insert( BaseMapElement::CivMilAirport, QObject::tr( "Civil/Mil. Airport" ) );
  objectTranslations.insert( BaseMapElement::ClosedAirfield, QObject::tr( "Closed Airfield" ) );
  objectTranslations.insert( BaseMapElement::Glidersite, QObject::tr( "Glider site" ) );
  objectTranslations.insert( BaseMapElement::HangGlider, QObject::tr( "Hang glider" ) );
  objectTranslations.insert( BaseMapElement::Landmark, QObject::tr( "Landmark" ) );
  objectTranslations.insert( BaseMapElement::MilHeliport, QObject::tr( "Mil. Heliport" ) );
  objectTranslations.insert( BaseMapElement::UltraLight, QObject::tr( "Ultralight" ) );
  objectTranslations.insert( BaseMapElement::Parachute, QObject::tr( "Parachute" ) );
  objectTranslations.insert( BaseMapElement::Outlanding, QObject::tr( "Outlanding" ) );
  objectTranslations.insert( BaseMapElement::Obstacle, QObject::tr( "Obstacle" ) );
  objectTranslations.insert( BaseMapElement::ObstacleGroup, QObject::tr( "Obstacle group" ) );
  objectTranslations.insert( BaseMapElement::LightObstacleGroup, QObject::tr( "Obstacle group lighted" ) );
  objectTranslations.insert( BaseMapElement::LightObstacle, QObject::tr( "Obstacle lighted" ) );
  objectTranslations.insert( BaseMapElement::Railway, QObject::tr( "Railway" ) );
  objectTranslations.insert( BaseMapElement::Road, QObject::tr( "Road" ) );
  objectTranslations.insert( BaseMapElement::Village, QObject::tr( "Village" ) );
  objectTranslations.insert( BaseMapElement::Turnpoint, QObject::tr( "Turnpoint" ) );
  objectTranslations.insert( BaseMapElement::Thermal, QObject::tr( "Thermal" ) );
  objectTranslations.insert( BaseMapElement::CompPoint, QObject::tr( "Comp. Reporting Point" ) );
  objectTranslations.insert( BaseMapElement::Vor, QObject::tr( "VOR" ) );
  objectTranslations.insert( BaseMapElement::VorDme, QObject::tr( "VOR/DME" ) );
  objectTranslations.insert( BaseMapElement::VorTac, QObject::tr( "VORTAC" ) );
  objectTranslations.insert( BaseMapElement::Ndb, QObject::tr( "NDB" ) );
  objectTranslations.insert( BaseMapElement::EmptyPoint, QObject::tr( "Empty Point" ) );

  // load sorted translation strings
  for( int i=0; i < objectTranslations.size(); i++ )
  {
    sortedTranslations.append( objectTranslations.value(i) );
  }

  sortedTranslations.sort();
}

void BaseMapElement::printMapElement(QPainter* , bool )
{
  qWarning("BaseMapElement::printMapElement");
}


void BaseMapElement::drawMapElement(QPainter* , QPainter* )
{
  qWarning("BaseMapElement::drawMapElement");
}

/**
  * Get sorted translations
  */
QStringList& BaseMapElement::getSortedTranslationList() {
  if( objectTranslations.isEmpty() ) {
    // Load object - translation data
    loadTranslations();
  }
  qDebug ("BaseMapElement::getSortedTranslationList: size: %d", objectTranslations.size());

  return sortedTranslations;
}


/** Read property of int MapSegment. */
const unsigned int& BaseMapElement::getMapSegment()
{
  return MapSegment;
}


/** Write property of int MapSegment. */
void BaseMapElement::setMapSegment( const unsigned int& _newVal)
{
  MapSegment = _newVal;
}

bool BaseMapElement::operator < (const BaseMapElement& other) const
{
  return getName() < other.getName();
}

