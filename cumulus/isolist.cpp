/***********************************************************************
 **
 **   isolist.cpp
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

#include "isolist.h"

/**
 * Compares two items, in this case, IsoListEntries.
 *
 * The items are compared by height only. The result is a reverse sorted
 * list, highest entry at lowest position.
 */
int IsoList::compareItems( Q3PtrCollection::Item item1, Q3PtrCollection::Item item2 )
{
  if( ((IsoListEntry*)item1)->height == ((IsoListEntry*)item2)->height )
    {
      return 0;
    }

  if( ((IsoListEntry*)item1)->height < ((IsoListEntry*)item2)->height )
    {
      return 1;
    }

  return -1;
}
