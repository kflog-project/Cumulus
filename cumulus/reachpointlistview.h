/***********************************************************************
**
**   reachpointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, Eckhard Völlm,
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class ReachpointListView
 *
 * \author André Somers, Eckhard Völlm, Axel Pauli
 *
 * \brief Reachable point list.
 *
 * This widget provides a list of reachable points and a means to select one.
 *
 * \date 2004-2011
 *
 * \$Id$
 */

#ifndef REACH_POINT_LISTVIEW_H
#define REACH_POINT_LISTVIEW_H

#include <QWidget>
#include <QTreeWidget>
#include <QPixmap>
#include <QBoxLayout>
#include <QPushButton>
#include <QItemDelegate>

#include "waypoint.h"
#include "rowdelegate.h"

class MainWindow;

class ReachpointListView : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( ReachpointListView )

public:

  ReachpointListView( MainWindow* parent=0 );
  virtual ~ReachpointListView();

  /** Returns a pointer to the currently highlighted waypoint. */
  Waypoint* getSelectedWaypoint();

  /**
   * Retrieves the waypoints from the map and fills the list.
   */
  void fillRpList();

  /**
   * Clears the widget list.
   */
  void clearList()
  {
    list->clear();
  }

protected:

  /** This slot is called when the widget is displayed. */
  void showEvent(QShowEvent *event);

public slots:

  /**
   * This slot is called to indicate that a selection has been made.
   */
  void slot_Select();

  /**
   * This slot is called if the info button has been clicked.
   */
  void slot_Info();

  /**
   * This slot is called when the listview should be closed without selection.
   */
  void slot_Close ();

  /**
   * This slot is called if the Show Outland button has been clicked.
   */
  void slot_ShowOl ();

  /**
   * This slot is called if the Hide Outland button has been clicked
   */
  void slot_HideOl ();

  /**
   * This slot is called to set a point as home site.
   */
  void slot_Home();

  /**
   * This slot is called when the list of reachable points has been changed.
   */
  void slot_newList ();

private slots:

  /**
   * A selection was made.
   */
  void slot_Selected();

  /**
   * Move page up.
   */
  void slot_PageUp();

  /**
   * Move page down.
   */
  void slot_PageDown();

signals:

  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(Waypoint*, bool);

  /**
   * This signal is emitted if the selection is done and the
   * screen can be closed.
   */
  void done();

  /**
   * This signal is emitted when the user clicks the Info button.
   */
  void info(Waypoint *);

  /**
   * This signal is emitted when a new home position is selected
   */
  void newHomePosition(const QPoint&);

  /**
   * This signal is emitted to move the map to the new home position
   */
  void gotoHomePosition();

private:

  QTreeWidget* list;
  MainWindow * par;

  /** that stores a home position change */
  bool _homeChanged;
  bool _newList;
  bool _outlandShow;

  RowDelegate* rowDelegate;
  QBoxLayout * buttonrow;
  Waypoint     selectedWp;

  QPushButton* cmdShowOl;
  QPushButton* cmdHideOl;
  QPushButton* cmdHome;
  QPushButton* cmdSelect;

  /** Up and down buttons for page moving */
  QPushButton* cmdPageUp;
  QPushButton* cmdPageDown;
};

#endif
