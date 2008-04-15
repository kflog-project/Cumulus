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

#ifndef ISOLIST_H
#define ISOLIST_H

#include <QRegion>
#include <QList>

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
    return height < other.height;
  };

  /** @AP: This method is only useable in qSort, if the members to be
   *  sorted are values and not pointers.
   */
  static bool lessThan(const IsoListEntry &i1, const IsoListEntry &i2)
    {
      return i1.height < i2.height;
    };
};

struct CompareIso
{
  // The operator sorts in reverse order
  bool operator()(const IsoListEntry *iso1, const IsoListEntry *iso2) const
  {
    return (iso1->height > iso2->height);
  };
};

class IsoList : public QList<IsoListEntry*>
{
 public:

  IsoList() {};

  virtual ~IsoList()
  {
    clear();
  };

  void sort()
  {
    // @AP: using std::sort because qSort can not handle pointer
    // elements
    std::sort( begin(), end(), CompareIso() );
    // qSort(begin(), end(), IsoListEntry::lessThan);
  };

};

#endif
