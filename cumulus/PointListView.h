/***********************************************************************
**
**   PointListView.h
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
 * \class PointListView
 *
 * \author Axel Pauli
 *
 * \brief This widget provides a list of single points and a means to select one.
 *
 * \date 2014
 *
 */
#ifndef PointListView_h
#define PointListView_h

#include <QPushButton>
#include <QBoxLayout>

#include "listwidgetparent.h"
#include "waypoint.h"
#include "mapcontents.h"

class PointListView : public QWidget
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( PointListView )

public:

  PointListView( ListWidgetParent* lwParent, QWidget *parent=0 );

  virtual ~PointListView();

  ListWidgetParent* listWidget()
    {
      return m_listw;
    };

  Waypoint* getCurrentEntry()
    {
      return m_listw->getCurrentWaypoint();
    };

  /**
   * \return The top level item count of the tree list.
   */
  int topLevelItemCount()
  {
    return m_listw->topLevelItemCount();
  };

protected:

  void showEvent( QShowEvent *event );

public slots:
  /**
   * This signal is called to indicate that a selection has been made.
   */
  void slot_Select();
  /**
   * This slot is called if the info button has been clicked, or the user pressed 'i'
   */
  void slot_Info();
  /**
   * Called when the list view should be closed without selection
   */
  void slot_Close ();

  /**
   * Called to set a point as home site
   */
  void slot_Home();

  void slot_Selected();

  /**
   * Called to reload the airfield item list
   */
  void slot_reloadList()
  {
    m_listw->refillItemList();
  };

signals:
  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(Waypoint*, bool);

  /**
   * This signal is send if the selection is done, and the screen can be closed.
   */
  void done();

  /**
   * Emitted if the user clicks the Info button.
   */
  void info(Waypoint*);

  /**
   * Emitted if a new home position is selected
   */
  void newHomePosition(const QPoint&);

  /**
   * Emitted to move the map to the new home position
   */
  void gotoHomePosition();

private:

  ListWidgetParent* m_listw;
  QBoxLayout*       m_buttonrow;
  QPushButton*      m_cmdSelect;
  QPushButton*      m_cmdHome;

  /** that shall store a home position change */
  bool m_homeChanged;
};

#endif
