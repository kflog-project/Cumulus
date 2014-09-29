/***********************************************************************
**
**   RadioPointListWidget.h
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
 * \class RadioPointListWidget
 *
 * \author Axel Pauli
 *
 * \brief This widget provides a list of navigation aids and a means to select
 *  one.
 *
 * \date 2014
 *
 */

#ifndef RadioPointListWidget_h
#define RadioPointListWidget_h

#include <QWidget>
#include <QVector>

#include "radiopoint.h"
#include "waypoint.h"
#include "listwidgetparent.h"
#include "mapcontents.h"

class RadioPointListWidget : public ListWidgetParent
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( RadioPointListWidget )

public:

  RadioPointListWidget( QWidget *parent=0, bool showMovePage=true );

  virtual ~RadioPointListWidget();

  /**
   * @returns a pointer to the currently selected entry.
   */
  Waypoint *getCurrentWaypoint();

  /**
   * Clears and fills the RadioPoint item list.
   */
  void fillItemList();

private:

  Waypoint m_wp;

  class _RadioPointItem : public QTreeWidgetItem
    {
      public:

      _RadioPointItem(RadioPoint *);
      RadioPoint* radioPoint;
    };
};

#endif
