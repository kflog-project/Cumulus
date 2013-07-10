/***********************************************************************
**
**   runway.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2008-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

Runway::Runway( const float rwLength,
                const unsigned short head,
                const unsigned short surf,
                const bool open,
                const bool bidirectional,
                const float rwWidth ) :
 length(rwLength),
 heading(head),
 surface(surf),
 isOpen(open),
 isBidirectional(bidirectional),
 width(rwWidth)
{
}

void Runway::printData()
{
  qDebug() << "RWY-Heading=" << (heading / 256) << "/"<< (heading & 255)
           << "Length=" << length
           << "Width=" << width
           << "Sfc=" << surface
           << "Open=" << isOpen
           << "BiDir=" << isBidirectional;
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
