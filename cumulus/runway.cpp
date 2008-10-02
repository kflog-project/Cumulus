/***********************************************************************
**
**   runway.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QObject>

#include "runway.h"

// declare static objects used for translations
QHash<int, QString> Runway::surfaceTranslations;
QStringList Runway::sortedTranslations;

Runway::Runway( const unsigned short len,
                const unsigned short dir,
                const unsigned short surf,
                const bool open )
 : length(len),
   direction(dir),
   surface(surf),
   isOpen(open)
{
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
const int Runway::text2Item( const QString& text )
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
