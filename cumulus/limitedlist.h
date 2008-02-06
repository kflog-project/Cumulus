/***********************************************************************
**
**   limitedlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LIMITEDLIST_H
#define LIMITEDLIST_H

#include <Q3PtrList>

/**
  *@author André Somers
  * @short Template for a list which limits the number of items it contains.
  *
  * The LimitedList template class implements a QList-based list with a
  * limited number of items. If more items are added, the least important
  * item will be deleted automatically. The item to delete it determined by
  * the getLeastImportantItem() memberfuntion, wich may be overridden by
  * child classes if needed. The default implementation removes the oldest
  * item from the list.
  */
template <class type>
class LimitedList : public Q3PtrList <type>
{

public:
    /**
     * Constructor
     * @param limit the maximum number of elements in this list. Defaults to
     *              10
     */
    LimitedList(uint limit=10);
    
    /**
     * Destructor
     */
    virtual ~LimitedList();

    /**
      * add should be used to add items to the list. It automatically checks if
      * it needs to delete an item afterwards.
      */
    void add ( const type *d );

    /**
     * Sets the new limit for the number of items in the list. If the new list
     * is shorter then the last, the excess items are deleted.
     * 
     * @param limit the new limit
     */
    void setLimit(uint limit);

protected:
    /**
     * getLeastImportantItem is called to identify the item that should be
     * removed if the list is too full. It may be overridden if you need
     * another criteria than simply deleting the oldest item in the list.
     */
    virtual uint getLeastImportantItem();


private:
    uint _limit;
};


/**
  * IMPLEMENTATION
  * ====================================================================
  *
  * The implementation is stored in the header file because that's the
  * only way it will compile without linkererrors. This has to do with
  * the fact that it's a template. See:
  *  http://www.ecs.fullerton.edu/~sowell/cs331/TemplateClasses.html
  * for information on the how and why.
  */



template <class type>
LimitedList<type>::LimitedList(uint limit)
{
    _limit=limit;
    Q3PtrList<type>::setAutoDelete(true);
}


template <class type>
LimitedList<type>::~LimitedList()
{}


template <class type>
void LimitedList<type>::add
    ( const type *d )
{
    Q3PtrList<type>::prepend(d);
    if (Q3PtrList<type>::count()>
            _limit) {
        uint o = getLeastImportantItem();
        if (o<Q3PtrList<type>::count())
            Q3PtrList<type>::remove(o);
    }
}


template <class type>
uint LimitedList<type>::getLeastImportantItem()
{
    return Q3PtrList<type>::count()-1;
}


template <class type>
void LimitedList<type>::setLimit(uint limit)
{
    //set new limit
    _limit=limit;

    //make sure the list is not longer than the new limit
    while (Q3PtrList<type>::count()>
            _limit) {
        uint o=getLeastImportantItem();
        if (o>Q3PtrList<type>::count())
            break;  //make sure we don't end up in an endless loop
        Q3PtrList<type>::remove(o);
    }
}

#endif

