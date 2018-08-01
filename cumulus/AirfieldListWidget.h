/***********************************************************************
**
**   AirfieldListWidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class AirfieldListWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This widget provides a list of airfields and a means to select one.
 *
 * \date 2002-2018
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

    AirfieldListWidget( QVector<enum MapContents::ListID> &itemList,
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

    /** Identifiers for list access. */
    QVector<enum MapContents::ListID> m_itemList;

    /** Waypoint temporary storage. */
    Waypoint m_wp;

class AirfieldItem : public QTreeWidgetItem
  {
    public:

      AirfieldItem(Airfield* item);
      Airfield* airfield;
  };
};

#endif
