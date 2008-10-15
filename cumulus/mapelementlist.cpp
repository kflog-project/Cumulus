/***********************************************************************
**
**   mapelementlist.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "mapelementlist.h"

#define TIMEOUT 10000 // timeout in milli seconds

MapElementList::MapElementList( QObject *parent, const char* name )
 : QObject(parent)
{
  setObjectName( name );
  timer = new QTimer( this );
  timer->setSingleShot( true );
  connect(timer, SIGNAL(timeout()), SLOT(destroySet()));
}

MapElementList::~MapElementList()
{
}

// Filter out double elements
void MapElementList::append(Airport& elem)
{
  QString item = elem.getName();

  if( !itemSet.contains( item ) )
    {
      QList<Airport>::append( elem );
      itemSet.insert( item );
    }

  // restart destroy set timer
  timer->start(TIMEOUT);
}

void MapElementList::createSet()
{
  itemSet.clear();

  int cnt = count();

  for (int i=0; i<cnt; i++)
    {
      itemSet.insert( at(i).getName() );
    }
}

void MapElementList::destroySet()
{
  itemSet.clear();
  qDebug( "Delete double check set for list %s, probably no longer needed.",
          objectName().toLatin1().data() );
}
