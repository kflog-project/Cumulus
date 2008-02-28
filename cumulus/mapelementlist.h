/***********************************************************************
 **
 **   mapelementlist.h
 **
 **   This file is part of Cumulus.
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

#ifndef MAPELEMENTLIST_H
#define MAPELEMENTLIST_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QSet>

#include "basemapelement.h"

/**
 * @author Eggert Ehmke
 */

class MapElementList : public QObject, public QList<BaseMapElement*>
{
  Q_OBJECT

    public:
  /**
   * Constructor
   */
  MapElementList( QObject *parent=0 );

  /**
   * Destructor
   */
  virtual ~MapElementList();

  /**
   * Appends an item into the list. If the list does not
   * allready contain the item, it is added. Otherwise
   * it is disgarded.
   */
  void append (BaseMapElement* elem);

 private:

  QSet<QString> m_set;
  QTimer* m_timer;

  void createSet();

  private slots:

  void destroySet();

};

#endif
