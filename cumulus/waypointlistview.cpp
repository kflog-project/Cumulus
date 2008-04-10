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

#include <QPushButton>
#include <QMessageBox>

#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "wpeditdialog.h"
#include "cumulusapp.h"

extern MapContents * _globalMapContents;
extern MapConfig * _globalMapConfig;

WaypointListView::WaypointListView(CumulusApp *parent) : QWidget(parent)
{
  setObjectName("WaypointListView");
  par=parent;
  
  QBoxLayout *topLayout = new QVBoxLayout( this );
  QBoxLayout *editrow=new QHBoxLayout(topLayout);

  editrow->addStretch(10);

  QPushButton * cmdNew = new QPushButton(this);
  cmdNew->setPixmap(GeneralConfig::instance()->loadPixmap("new.png"));
  cmdNew->setFlat(true);
  editrow->addWidget(cmdNew);

  QPushButton * cmdEdit = new QPushButton(this);
  cmdEdit->setPixmap(GeneralConfig::instance()->loadPixmap("edit.png"));
  cmdEdit->setFlat(true);
  editrow->addWidget(cmdEdit);

  QPushButton * cmdDel = new QPushButton(this);
  cmdDel->setPixmap(GeneralConfig::instance()->loadPixmap("trash.png"));
  cmdDel->setFlat(true);
  editrow->addWidget(cmdDel);

  editrow->addSpacing(6);
  QPushButton * cmdHome = new QPushButton(this);
  cmdHome->setPixmap(GeneralConfig::instance()->loadPixmap("home.png"));
  cmdHome->setFlat(true);
  editrow->addWidget(cmdHome);

  list= new Q3ListView(this, "waypointlist");
  list->addColumn(tr("Name"));
  list->addColumn(tr("Description"));
  list->addColumn(tr("ICAO"));

  list->setAllColumnsShowFocus(true);

  filter=new ListViewFilter(list, this, "listfilter");
  topLayout->addWidget(filter);

  topLayout->addWidget(list,10);
  QBoxLayout *buttonrow=new QHBoxLayout(topLayout);

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
  connect(list, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slot_Selected()));
}


WaypointListView::~WaypointListView()
{
  // qDebug("WaypointListView::~WaypointListView()");
}


/** Retreives the waypoints from the mapcontents, and fills the list. */
void WaypointListView::fillWpList(QList<wayPoint*> *wpList, Q3ListView *list, ListViewFilter *filter)
{
  if( list == 0 ) {
    list = WaypointListView::list;
  }

  if( filter == 0 ) {
    filter = WaypointListView::filter;
  }

  list->clear();

  int n = 0;

  if( wpList ) {
    n = wpList->count();
    //qDebug("WaypointListView::fillWpList() %d", n);
    
    for (int i=0; i < n; i++) {
      wayPoint * wp=(wayPoint*)wpList->at(i);
      new _WaypointItem(list, wp);
    }
  }

  if ( n>0 ) {
    list->setCurrentItem(list->firstChild());
  }


  if(filter == WaypointListView::filter) {
    filter->reset();
  } else {
    filter->reset(true);
  }
}


void WaypointListView::showEvent(QShowEvent *)
{
  list->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void WaypointListView::slot_Select()
{
  emit newWaypoint(getSelectedWaypoint(), true);
  emit done();

  filter->off();
}


/** This slot is called if the info button has been clicked */
void WaypointListView::slot_Info()
{
  if (getSelectedWaypoint()) {
    emit info(getSelectedWaypoint());
  }
}


/** @ee This slot is called if the listview is closed without selecting */
void WaypointListView::slot_Close ()
{
  filter->off();
  emit done();
}

void WaypointListView::slot_Selected() {
  if(WaypointListView::getSelectedWaypoint()->equals(calculator->getselectedWp())) {
    cmdSelect->setEnabled(false);
  } else {
    cmdSelect->setEnabled(true);
  }
}

/** Returns a pointer to the currently highlighted waypoint. */
wayPoint * WaypointListView::getSelectedWaypoint(Q3ListView *list)
{
  if( list == 0 ) {
    list = WaypointListView::list;
  }

  Q3ListViewItem* li = list->selectedItem();
  _WaypointItem* wpi = static_cast<_WaypointItem*>(li);

  if (!wpi) {
    return 0;
  }

  return wpi->wp;
}


/** Called when a new waypoint needs to be made. */
void WaypointListView::slot_newWP()
{
  WPEditDialog *dlg=new WPEditDialog(this, 0);
  dlg->setAttribute(Qt::WA_DeleteOnClose);

  connect(dlg, SIGNAL(wpListChanged(wayPoint *)),
          this, SLOT(slot_wpAdded(wayPoint *)));

  dlg->show();
}


/** Called when the selected waypoint needs must be opened in the editor */
void WaypointListView::slot_editWP()
{
  wayPoint *wp=getSelectedWaypoint();
  if (wp)
    {
      WPEditDialog *dlg=new WPEditDialog(this, getSelectedWaypoint());
      dlg->setAttribute(Qt::WA_DeleteOnClose);

      connect(dlg, SIGNAL(wpListChanged(wayPoint *)),
              this, SLOT(slot_wpEdited(wayPoint *)));

      dlg->show();
    }
}


/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListView::slot_deleteWP()
{
  if ( getSelectedWaypoint() == 0 ) {
    return;
  }

  int answer= QMessageBox::warning(this,tr("Delete?"),
                                   tr("Delete highlighted\nwaypoint?"),
                                   QMessageBox::Ok,
                                   QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default);

  if( answer == QMessageBox::Ok ) {
    //first, make sure our waypointlist filter is won't interfere
    filter->restoreListViewItems();

    // next, obtain a reference to the waypoint
    wayPoint *wp=getSelectedWaypoint();

    emit deleteWaypoint(wp); // cancel the selected waypoint

    // remove from waypointlist in MapContents
    _globalMapContents->getWaypointList()->remove( wp );
    // save the modified catalog
    _globalMapContents->saveWaypointList();

    // remove from listView
    delete list->selectedItem();
    if (par)    
      par->viewMap->_theMap->sceduleRedraw(Map::waypoints);
    filter->reset(true);
  }
}


/** Called if a waypoint has been edited. */
void WaypointListView::slot_wpEdited(wayPoint * wp)
{
  if ( wp ) {
    Q3ListViewItem * li;
    li=list->selectedItem();
    if( li ) {
      li->setText(0, wp->name);
      li->setText(1, wp->description);
      li->setText(2, wp->icao);
      li->setPixmap(0, _globalMapConfig->getPixmap(wp->type,false,true));
      list->sort();
      filter->reset();

      // save modified catalog
      _globalMapContents->saveWaypointList();

      if (par) par->viewMap->_theMap->sceduleRedraw(Map::waypoints);
    } else
      qDebug("WaypointListView::slot_wpEdited() has empty list");
  }
}


/** Called if a waypoint has been added. */
void WaypointListView::slot_wpAdded(wayPoint * wp)
{
  qDebug("WaypointListView::slot_wpAdded");
  if ( wp ) {
    wayPoint *newWp = new wayPoint(*wp);
    new _WaypointItem(list, newWp);
    list->sort();
    filter->reset();

    // qDebug("WaypointListView::slot_wpAdded(): name=%s",wp->name.latin1());

    _globalMapContents->getWaypointList()->append(newWp);
    // save the modified catalog
    _globalMapContents->saveWaypointList();

    if (par) par->viewMap->_theMap->sceduleRedraw(Map::waypoints);
  }
}


void WaypointListView::slot_setHome()
{
  wayPoint *_wp = getSelectedWaypoint();

  if ( _wp==0 ) {
    return;
  }

  int answer= QMessageBox::warning(this,tr("Set homesite?"),
                                   tr("Do you want to use waypoint\n%1\nas your homesite?").arg(_wp->name),
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::Cancel | QMessageBox::Escape );
  if( answer == 1 ) { //ok was chosen

    // Save new data as home position
    GeneralConfig *conf = GeneralConfig::instance();
    wayPoint *w = new wayPoint(*_wp);
    conf->setHomeWp(w);
    conf->save();

    QPoint newPos( _wp->origP.lat(), _wp->origP.lon() );
    emit newHomePosition( &newPos );
  }
}


WaypointListView::_WaypointItem::_WaypointItem(Q3ListView* lv, wayPoint* waypoint):
  Q3ListViewItem(lv),
  wp(waypoint)
{
  if (!wp) {
    return;
  }

  QString name = wp->name;
  QRegExp blank("[ ]");
  //name.replace(blank, QString::null);
  name = name.left(10);

  setText(0, name);
  setText(1, wp->description);
  setText(2, wp->icao);
  setPixmap(0, _globalMapConfig->getPixmap(wp->type,false,true));
}


