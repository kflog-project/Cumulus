/***********************************************************************
 **
 **   mapelementlist.h
 **
 **   This file is part of Cumulus.
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

#ifndef MAP_ELEMENT_LIST_H
#define MAP_ELEMENT_LIST_H

#include <QObject>
#include <QList>
#include <QTimer>
#include <QSet>

#include "airfield.h"

/**
 * @author Eggert Ehmke
 *
 * This class is an extension of an Airfield QList. It checks in the
 * append method, if the item to be appended is already known with its name.
 * If that is true, no item will be appended. Furthermore a timer is fired.
 * The timer expires after 10s the last append has been done. The expire method
 * will clear the check set which was built up during append to avoid multiple
 * entries. The assumption is that after timeout no items will be more added
 * to the list.
 *
 */

class MapElementList : public QObject, public QList<Airfield>
{
  Q_OBJECT

    public:
  /**
   * Constructor
   */
  MapElementList( QObject *parent=0, const char* name="" );

  /**
   * Destructor
   */
  virtual ~MapElementList();

  /**
   * Appends an item at the end of the list. If the list does not
   * already contain the item, it is added. Otherwise
   * it is discarded.
   */
  void append (Airfield& elem);

 private:

  QSet<QString> itemSet;
  QTimer* timer;

  void createSet();

  private slots:

  void destroySet();

};

#endif
