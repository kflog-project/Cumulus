/***********************************************************************
**
**   ListViewTabs.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2014 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class ListViewTabs
 *
 * \author Axel Pauli
 *
 * \brief This class manages different display lists in a QTabWidget.
 *
 * \date 2014
 *
 * \version $Id$
 */

#ifndef ListViewTabs_h
#define ListViewTabs_h

#include <QString>
#include <QTabWidget>
#include <QString>
#include <QWidget>

#include "airfieldlistview.h"
#include "reachpointlistview.h"
#include "tasklistview.h"
#include "waypointlistview.h"

#include "generalconfig.h"

class ListViewTabs : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( ListViewTabs )

 public:

  ListViewTabs( QWidget* parent = 0 );

  virtual ~ListViewTabs();

  /**
   * Lists managed by the tab widget.
   */
  AirfieldListView*   viewAF;
  AirfieldListView*   viewOL;
  ReachpointListView* viewRP;
  TaskListView*       viewTP;
  WaypointListView*   viewWP;

 protected:

  void showEvent( QShowEvent *event );

 private:

  QTabWidget* m_listViewTabs;

};

#endif

