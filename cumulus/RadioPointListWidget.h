/***********************************************************************
**
**   RadioPointListWidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014-2021 by Axel Pauli
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
 * \date 2014-2021
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

  /**
   * Class constructor.
   *
   * \param itemList Identifiers for lists to be loaded and managed.
   *
   * \param parent Pointer to parent widget.
   *
   * \param showMovePage Shows the move button, if set to true.
   */
  RadioPointListWidget( QVector<enum MapContents::ListID> &itemList,
			QWidget *parent=0,
                        bool showMovePage=true );

  virtual ~RadioPointListWidget();

  /**
   * @returns a pointer to the currently selected entry.
   */
  Waypoint *getCurrentWaypoint();

  /**
   * Clears and fills the RadioPoint item list.
   */
  void fillItemList();

  /**
   * Reset list filter.
   */
  void resetListFilter()
  {
    filter->reset();
  }

  /**
   * This is called in the parent, if the search button is pressed;
   */
  virtual void searchButtonPressed();

public slots:

  /**
   * This slot is called, to pass the search result.
   */
  void slot_SearchResult( const SinglePoint* singlePoint );

private:

  /** Identifiers for list access. */
  QVector<enum MapContents::ListID> m_itemList;

  /** Waypoint temporary storage. */
  Waypoint m_wp;

  class RadioPointItem : public QTreeWidgetItem
    {
      public:

      RadioPointItem(RadioPoint *);
      RadioPoint* m_radioPoint;
    };
};

#endif
