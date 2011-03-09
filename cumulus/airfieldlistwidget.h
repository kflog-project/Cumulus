/***********************************************************************
**
**   airfieldlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class AirfieldListWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This widget provides a list of airfields and a means to select one.
 *
 * \date 2002-2010
 *
 */

#ifndef AIRFIELD_LIST_WIDGET_H
#define AIRFIELD_LIST_WIDGET_H

#include <QWidget>
#include <QVector>

#include "waypoint.h"
#include "listwidgetparent.h"
#include "mapcontents.h"

class AirfieldListWidget : public ListWidgetParent
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( AirfieldListWidget )

public:

    AirfieldListWidget( QVector<enum MapContents::MapContentsListID> &itemList,
                        QWidget *parent=0, bool showMovePage=true );

    virtual ~AirfieldListWidget();

    /**
     * @returns a pointer to the currently high lighted waypoint.
     */
    Waypoint *getCurrentWaypoint();

    /**
     * Clears and fills the airfield item list.
     */
    void fillItemList();

private:

    Waypoint wp;
    QVector<enum MapContents::MapContentsListID> itemList;

class _AirfieldItem : public QTreeWidgetItem
  {
    public:

      _AirfieldItem(Airfield*);
      Airfield* airfield;
  };
};

#endif
