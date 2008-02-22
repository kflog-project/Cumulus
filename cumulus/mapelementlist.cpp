/***********************************************************************
**
**   mapelementlist.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "generalconfig.h"
#include "mapelementlist.h"


MapElementList::MapElementList( QObject * parent, const char * name )
 : QObject(parent, name)
{
  m_timer = new QTimer();
  m_timer->setSingleShot ( true );
  connect(m_timer, SIGNAL(timeout()), SLOT(destroySet()));
}


MapElementList::~MapElementList()
{
  delete m_timer;
  m_set.clear();
}


// Filter out double elements
void MapElementList::append(BaseMapElement* elem)
{

  if( !m_set.contains(elem->getName()) )
    {
      QList<BaseMapElement*>::append(elem);
      m_set.insert( elem->getName() );
    }
  else
    {
      delete elem;
    }

  m_timer->start(10000);
}


void MapElementList::createSet()
{
  m_set.clear();

  int cnt = count();

  for (int i=0; i<cnt; i++)
    {
      m_set.insert( at(i)->getName() );
    }
}


void MapElementList::destroySet()
{
  m_set.clear();
  qDebug( "Deleted set for list %s, probably no longer needed.", name() );
}

