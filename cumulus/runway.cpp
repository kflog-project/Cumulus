/***********************************************************************
**
**   runway.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2008-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

Runway::Runway( const QString& name,
                const float rwLength,
                const unsigned short head,
                const unsigned short surf,
                const bool open,
                const bool bidirectional,
                const float rwWidth ) :
 m_name(name),
 m_length(rwLength),
 m_heading(head),
 m_surface(surf),
 m_isOpen(open),
 m_isBidirectional(bidirectional),
 m_width(rwWidth)
{
}

void Runway::printData()
{
  qDebug() << "RWY-Name=" << m_name
           << "RWY-Heading=" << (m_heading / 256) << "/"<< (m_heading & 255)
           << "Length=" << m_length
           << "Width=" << m_width
           << "Sfc=" << m_surface
           << "Open=" << m_isOpen
           << "BiDir=" << m_isBidirectional;
}

/**
 * Get translation string for surface type.
 */
QString Runway::item2Text( const int surfaceType, QString defaultValue )
{
  if( surfaceTranslations.isEmpty() )
    {
      loadTranslations();
    }

  return surfaceTranslations.value( surfaceType, defaultValue );
}

/**
 * Get surface type for translation string.
 */
int Runway::text2Item( const QString& text )
{
  if( surfaceTranslations.isEmpty() )
    {
      // Load object - translation data
      loadTranslations();
    }

  return surfaceTranslations.key( text );
}

void Runway::loadTranslations()
{
  // Load translation data
  surfaceTranslations.insert( Runway::Unknown,  QObject::tr( "Unknown" ) );
  surfaceTranslations.insert( Runway::Grass,    QObject::tr( "Grass" ) );
  surfaceTranslations.insert( Runway::Asphalt,  QObject::tr( "Asphalt" ) );
  surfaceTranslations.insert( Runway::Concrete, QObject::tr( "Concrete" ) );
  surfaceTranslations.insert( Runway::Sand,     QObject::tr( "Sand" ) );
  surfaceTranslations.insert( Runway::Water,    QObject::tr( "Water" ) );
  surfaceTranslations.insert( Runway::Gravel,   QObject::tr( "Gravel" ) );
  surfaceTranslations.insert( Runway::Ice,      QObject::tr( "Ice" ) );
  surfaceTranslations.insert( Runway::Snow,     QObject::tr( "Snow" ) );
  surfaceTranslations.insert( Runway::Soil,     QObject::tr( "Soil" ) );

  // load sorted translation strings
  QHashIterator<int, QString> it(surfaceTranslations);

  while( it.hasNext() )
    {
      it.next();
      sortedTranslations.append( it.value() );
    }

  sortedTranslations.sort();
}

/**
 * Get sorted translations
 */
QStringList& Runway::getSortedTranslationList()
{
  if( surfaceTranslations.isEmpty() ) {
    // Load surface - translation data
    loadTranslations();
  }

  // qDebug("Runway::getSortedTranslationList: size: %d", sortedTranslations.size());

  return sortedTranslations;
}
