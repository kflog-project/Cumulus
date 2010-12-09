/***********************************************************************
**
**   reachpointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, Eckhard Völlm,
**                   2008-2009 by Axel Pauli
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
 * \date 2004-2010
 *
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
  wayPoint* getSelectedWaypoint();

  /**
   * Retrieves the waypoints from the map contents and fills the list.
   */
  void fillRpList();

  /**
   * Clears the widget list.
   */
  void clearList()
  {
    list->clear();
  }

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

signals:

  /**
   * This signal is emitted if a new waypoint is selected.
   */
  void newWaypoint(wayPoint*, bool);

  /**
   * This signal is emitted if the selection is done and the
   * screen can be closed.
   */
  void done();

  /**
   * This signal is emitted when the user clicks the Info button.
   */
  void info(wayPoint *);

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
  RowDelegate* rowDelegate;
  QBoxLayout * buttonrow;
  wayPoint     selectedWp;
  bool         _outlandShow;
  QPixmap      _arrows;
  bool         _newList;

  QPushButton* cmdShowOl;
  QPushButton* cmdHideOl;
  QPushButton* cmdHome;
  QPushButton* cmdSelect;

private slots:

  void slot_Selected();

protected:

  /** This slot is called when the widget is displayed. */
  void showEvent(QShowEvent *);

private:

  /** that stores a home position change */
  bool homeChanged;
};

#endif
