/***********************************************************************
 **
 **   isolist.h
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

#ifndef ISO_LIST_H
#define ISO_LIST_H

#include <QRegion>
#include <QList>

/**
 * @short Entry in the isolist
 *
 * This class contains a @ref QRegion and a height. A list of entries
 * like this is created when the map is drawn and is used to detect the
 * elevation at a given position, for instance under the mouse cursor.
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
  IsoListEntry( QRegion* region, const int height=0 );

  /**
   * Copy constructor is needed to make a deep copy of the QRegion pointer.
   */
  IsoListEntry( const IsoListEntry& x );

  /**
   * Assignment operator is needed to make a deep copy of the QRegion pointer.
   */
  IsoListEntry& operator=(const IsoListEntry& x);

  /**
   * Destructor
   */
  virtual ~IsoListEntry();

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

  bool operator()(const IsoListEntry &iso1, const IsoListEntry &iso2) const
  {
    return (iso1.height < iso2.height);
  };

  bool operator()(const IsoListEntry *iso1, const IsoListEntry *iso2) const
  {
    return (iso1->height < iso2->height);
  };

  QRegion* region;
  int height;

  /**
  * Compares two items, in this case, IsoListEntries.
  *
  * The items are compared by height only. The result is a reverse sorted
  * list, highest entry at lowest position.
  */
  bool operator < (const IsoListEntry& other) const
  {
    return height > other.height;
  };
};

class IsoList : public QList<IsoListEntry>
{
 public:

  IsoList() {};

  virtual ~IsoList() {};

  void sort()
  {
    qSort( begin(), end() );
  };

};

#endif
