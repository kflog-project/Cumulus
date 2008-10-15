/***********************************************************************
 **
 **   isolist.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "isolist.h"

/**
 * Constructor.
 * @param region Region in coordinate system of the map object, not in KFLog system
 * @param height the elevation of the isoline in meters
 */
IsoListEntry::IsoListEntry( QRegion* region, const int height )
{
  this->region = region;
  this->height = height;
};

/**
 * Copy constructor is needed to make a deep copy of the QRegion pointer.
 */
IsoListEntry::IsoListEntry( const IsoListEntry& x )
{
  height = x.height;
  region = x.region;

  // Make a deep copy of the QRegion object, if it exists.
  if( x.region )
    {
      region = new QRegion( *x.region );
    }
}

/**
 * Assignment operator is needed to make a deep copy of the QRegion pointer.
 */
IsoListEntry& IsoListEntry::operator=(const IsoListEntry& x)
{
  if( this == &x )
    {
      // Same object, nothing to do.
      return *this;
    }

  // overtake height
  height = x.height;

  // The QRegion object of the left side must be deleted, if it exists.
  if( region )
    {
      delete region;
      region = static_cast<QRegion *> (0);
    }

  // Make a deep copy of the QRegion object from the right side, if it exists.
  if( x.region )
    {
      region = new QRegion( *x.region );
    }

  return *this;
}

/**
 * Destructor
 */
IsoListEntry::~IsoListEntry()
{
  // QRegion must be deleted, if it was allocated.
  if( region )
    {
      delete region;
    }
}
