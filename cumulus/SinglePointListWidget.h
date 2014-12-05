/***********************************************************************
**
**   SinglePointListWidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SinglePointListWidget
 *
 * \author Axel Pauli
 *
 * \brief This widget provides a list of single points and a means to select
 *  one.
 *
 * \date 2014
 *
 */

#ifndef SinglePointListWidget_h
#define SinglePointListWidget_h

#include <QWidget>
#include <QVector>

#include "singlepoint.h"
#include "waypoint.h"
#include "listwidgetparent.h"
#include "mapcontents.h"

class SinglePointListWidget : public ListWidgetParent
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( SinglePointListWidget )

public:

  /**
   * Class constructor.
   *
   * \param itemList Identifiers for lists to be loaded and managed.
   *
   * \param parent Pointer to parent widget.
   *
   * \param showMovePage Shows the move button, if set to true.
   */
  SinglePointListWidget( QVector<enum MapContents::ListID> &itemList,
			 QWidget *parent=0,
			 bool showMovePage=true );

  virtual ~SinglePointListWidget();

  /**
   * @returns a pointer to the currently selected entry.
   */
  Waypoint *getCurrentWaypoint();

  /**
   * Clears and fills the SinglePoint item list.
   */
  void fillItemList();

private:

  /** Identifiers for list access. */
  QVector<enum MapContents::ListID> m_itemList;

  /** Waypoint temporary storage. */
  Waypoint m_wp;

  class SinglePointItem : public QTreeWidgetItem
    {
      public:

      SinglePointItem( SinglePoint *sp );

      SinglePoint* m_singlePoint;
    };
};

#endif
