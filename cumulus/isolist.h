/***********************************************************************
 **
 **   isolist.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2007 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef ISOLIST_H
#define ISOLIST_H

#include <QRegion>
#include <Q3PtrList>
#include <Q3PtrCollection>

/**
 * @short Entry in the isolist
 *
 * This class contains a @ref QRegion and a height. A list of entries
 * like this is created when the map is drawn, and is used to detect the
 * elevation at a given position, for instance under the mousecursor.
 *
 */

class IsoListEntry
{
 public:

  /**
   * Constructor.
   * @param region Region in coordinate system of the map-object, not in KFLog system
   * @param height the elevation of the isoline in meters
   */
  IsoListEntry( QRegion* region=0, int height=0 )
  {
    this->region = region;
    this->height = height;
  };

  /**
   * Destructor
   */
  virtual ~IsoListEntry()
  {
    if( region ) delete region;
  };

  bool operator == (const IsoListEntry& x)
  {
    return x.height==height;
  }

  bool operator >= (const IsoListEntry& x)
  {
    return x.height>=height;
  }

  bool operator <= (const IsoListEntry& x)
  {
    return x.height<=height;
  }

  bool operator < (const IsoListEntry& x)
  {
    return x.height<height;
  }

  bool operator > (const IsoListEntry& x)
  {
    return x.height>height;
  }

  QRegion* region;
  int height;
};

class IsoList : public Q3PtrList<IsoListEntry>
{
 public:

  IsoList() {};

  virtual ~IsoList()
  {
    clear();
  };

 protected:

  /**
   * Compares two items, in this case, BaseMapElements.
   * The items are compared by name only.
   */
  virtual int compareItems (Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);

};

#endif
