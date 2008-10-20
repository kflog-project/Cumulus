/***********************************************************************
**
**   waypointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andre Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypointlistview.h"

#include <QMessageBox>
#include <QShortcut>

#include "generalconfig.h"
#include "wpeditdialog.h"
#include "calculator.h"
#include "cumulusapp.h"

WaypointListView::WaypointListView(QMainWindow *parent) : QWidget(parent)
{
  setObjectName("WaypointListView");
  par=parent;

  QBoxLayout *topLayout = new QVBoxLayout( this );
  QBoxLayout *editrow=new QHBoxLayout;
  topLayout->addLayout(editrow);

  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("add.png")) );
  cmdNew->setIconSize(QSize(26,26));
  editrow->addWidget(cmdNew);

  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("edit_new.png")) );
  cmdEdit->setIconSize(QSize(26,26));
  editrow->addWidget(cmdEdit);

  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("delete.png")) );
  cmdDel->setIconSize(QSize(26,26));
  editrow->addWidget(cmdDel);

  editrow->addSpacing(6);
  QPushButton * cmdHome = new QPushButton(this);
  cmdHome->setIcon( QIcon(GeneralConfig::instance()->loadPixmap("home_new.png")) );
  cmdHome->setIconSize(QSize(26,26));
  editrow->addWidget(cmdHome);

  listw = new WaypointListWidget( this );
  topLayout->addWidget(listw, 10);

  QBoxLayout *buttonrow=new QHBoxLayout;
  topLayout->addLayout( buttonrow );

  QPushButton *cmdClose = new QPushButton(tr("Close"), this);
  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(cmdSelect);

  connect(cmdNew, SIGNAL(clicked()), this, SLOT(slot_newWP()));
  connect(cmdEdit, SIGNAL(clicked()), this, SLOT(slot_editWP()));
  connect(cmdDel, SIGNAL(clicked()), this, SLOT(slot_deleteWP()));
  connect(cmdHome, SIGNAL(clicked()), this, SLOT(slot_setHome()));
  connect(cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()));
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));
  connect(listw, SIGNAL(wpSelectionChanged()), this, SLOT(slot_Selected()));
  connect(this, SIGNAL(done()), listw, SLOT(slot_Done()));

  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ));
}


WaypointListView::~WaypointListView()
{
  // qDebug("WaypointListView::~WaypointListView()");
}


void WaypointListView::showEvent(QShowEvent *)
{
  // listw->listWidget()->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void WaypointListView::slot_Select()
{
  wayPoint *w = listw->getSelectedWaypoint();

  if ( w )
    {
      emit newWaypoint( w, true );
      emit done();
    }
}


/** This slot is called if the info button has been clicked */
void WaypointListView::slot_Info()
{
  wayPoint *w = listw->getSelectedWaypoint();

  if (w)
    {
      emit info(w);
    }
}


/** @ee This slot is called if the listview is closed without selecting */
void WaypointListView::slot_Close ()
{
  emit done();
}

void WaypointListView::slot_Selected()
{
  cmdSelect->setEnabled(true);
  wayPoint *w = listw->getSelectedWaypoint();

  if (w)
    {
      if(w->equals(calculator->getselectedWp()))
        {
          cmdSelect->setEnabled(false);
        }
    }
}


/** Called when a new waypoint needs to be made. */
void WaypointListView::slot_newWP()
{
  WpEditDialog *dlg = new WpEditDialog(this, 0);

  dlg->setAttribute(Qt::WA_DeleteOnClose);

  connect(dlg, SIGNAL(wpListChanged(wayPoint *)),
          this, SLOT(slot_wpAdded(wayPoint *)));

  dlg->show();
}


/** Called when the selected waypoint needs must be opened in the editor */
void WaypointListView::slot_editWP()
{
  wayPoint *wp = getSelectedWaypoint();

  if (wp) {
    WpEditDialog *dlg = new WpEditDialog(this, wp);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    connect(dlg, SIGNAL(wpListChanged(wayPoint *)),
            this, SLOT(slot_wpEdited(wayPoint *)));

    dlg->show();
  }
}


/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListView::slot_deleteWP()
{
  wayPoint* wp = listw->getSelectedWaypoint();
  if ( wp == 0 )
    return;

  int answer= QMessageBox::warning(this,tr("Delete Waypoint"),
                                   tr("Delete selected waypoint?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);

  if( answer == QMessageBox::Ok ) {

    listw->deleteSelectedWaypoint();
    emit deleteWaypoint(wp); // cancel the selected waypoint

    if (par)
      {
        ((CumulusApp*) par)->viewMap->_theMap->scheduleRedraw(Map::waypoints);
      }
  }
}


/** Called if a waypoint has been edited. */
void WaypointListView::slot_wpEdited(wayPoint& wp)
{
//  qDebug("WaypointListView::slot_wpEdited");
  listw->updateSelectedWaypoint( wp );

  if (par)
    {
      ((CumulusApp*) par)->viewMap->_theMap->scheduleRedraw(Map::waypoints);
    }
}


/** Called if a waypoint has been added. */
void WaypointListView::slot_wpAdded(wayPoint& wp)
{
  // qDebug("WaypointListView::slot_wpAdded(): name=%s", wp->name.toLatin1().data());

  listw->addWaypoint(wp);

  if (par)
    {
      ((CumulusApp*) par)->viewMap->_theMap->scheduleRedraw(Map::waypoints);
    }
}


void WaypointListView::slot_setHome()
{
  wayPoint *_wp = listw->getSelectedWaypoint();

  if ( _wp == 0 )
    {
      return;
    }

  int answer= QMessageBox::warning(this,tr("Set home site"),
                                   tr("Use waypoint<br>%1<br>as your home site?").arg(_wp->name),
                                   QMessageBox::Ok | QMessageBox::Cancel );
  if( answer == 1 ) { //ok was chosen

    // Save new data as home position
    GeneralConfig *conf = GeneralConfig::instance();
    conf->setHomeWp(_wp);
    conf->save();

    emit newHomePosition( &_wp->origP );
  }
}
