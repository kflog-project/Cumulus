/***********************************************************************
**
**   limitedlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andr√© Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef LIMITEDLIST_H
#define LIMITEDLIST_H

#include <QList>

/**
  * @author AndrÈ Somers, Axel Pauli
  * @short Template for a list which limits the number of items it contains.
  *
  * 2008-02-22 AP: This class was modified from a pointer based list
  * to a value based list during Qt2 -> Qt4 portage.
  *
  * The LimitedList template class implements a QList based value list
  * with a limited number of items. If more items are added, the least
  * important item will be deleted automatically. The item to delete
  * it determined by the getLeastImportantItemIndex() member funtion,
  * which may be overridden by child classes if needed. The default
  * implementation removes the oldest item from the list.
  */

template <class type>
class LimitedList : public QList <type>
{

public:

  /**
   * Constructor
   * @param limit the maximum number of elements in this list. Defaults to 10
   */
  LimitedList(int limit=10);
    
  /**
   * Destructor
   */
  virtual ~LimitedList();

  /**
   * add should be used to add items to the list. It automatically checks if
   * it needs to delete an item afterwards.
   */
  void add( const type &d );

  /**
   * Sets the new limit for the number of items in the list. If the new list
   * is shorter then the last, the excess items are deleted.
   * 
   * @param limit the new limit
   */
  void setLimit( const int limit );

 protected:

  /**
   * getLeastImportantItemIndex is called to identify the item that
   * should be removed if the list is too full. It may be overridden
   * if you need another criteria than simply deleting the oldest item
   * in the list.
   */
  virtual int getLeastImportantItemIndex() const;


 private:

  int _limit;
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
 *
 */

template <class type>
LimitedList<type>::LimitedList( int limit )
{
  _limit = limit;
}


template <class type>
LimitedList<type>::~LimitedList()
{
  // remove all elements in the list
  QList<type>::clear();
}


template <class type>
void LimitedList<type>::add( const type &elem )
{
  QList<type>::prepend( elem );

  int length = QList<type>::count();

  if( length > (int) _limit )
    {
      int idx = getLeastImportantItemIndex();

      if( idx < length )
        {
          QList<type>::removeAt( idx );
        }
  }
}


template <class type>
int LimitedList<type>::getLeastImportantItemIndex() const
{
  return QList<type>::count() - 1;
}


template <class type>
void LimitedList<type>::setLimit( const int limit )
{
  if( _limit <= limit )
    {
      return; // no adaption necessary
    }

  // set new limit
  _limit = limit;

  if( limit == 0 )
    {
      QList<type>::clear();
      return;
    }

  // make sure the list is not longer than the new limit
  while( QList<type>::count() > _limit)
    {
      int idx = getLeastImportantItemIndex();
      QList<type>::removeAt( idx );
    }
}

#endif
