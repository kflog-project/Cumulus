/***********************************************************************
**
**   airfieldlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "airfieldlistview.h"

#include <QMessageBox>
#include <QRegExp>
#include <QFont>

#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "cucalc.h"
#include "airport.h"

extern MapContents *_globalMapContents;
extern MapConfig   *_globalMapConfig;

AirfieldListView::AirfieldListView(QMainWindow *parent, const char *name ) : QWidget(parent,name)
{
  listFilled = false;
  par=parent;
  QBoxLayout *topLayout = new QVBoxLayout( this );

  list= new Q3ListView(this, "airfieldlist");

//   list->addColumn(tr("Name"),75);
//   list->addColumn(tr("Description"),100);
//   list->addColumn(tr("ICAO"),40);

  list->addColumn(tr("Name"));
  list->addColumn(tr("Description"));
  list->addColumn(tr("ICAO"));

  list->setAllColumnsShowFocus(true);
  list->setSorting(0,true);
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

  connect(cmdSelect, SIGNAL(clicked()),
          this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()),
          this, SLOT(slot_Info()));
  connect(cmdClose, SIGNAL(clicked()),
          this, SLOT(slot_Close()));
  connect(list, SIGNAL(selectionChanged(Q3ListViewItem*)),
          this, SLOT(slot_Selected()));

  itemList[0] = MapContents::AirportList;
  itemList[1] = MapContents::GliderList;
  itemList[2] = MapContents::AddSitesList;
  wp = new wayPoint();
}


AirfieldListView::~AirfieldListView()
{
  delete wp;
}


/** Retreives the airfields from the mapcontents, and fills the list. */
void AirfieldListView::fillWpList(Q3ListView *list, ListViewFilter *filter)
{
  if( listFilled && list == 0) {
    return;
  }

  if(list == 0) {
    list = AirfieldListView::list;
  }

  if(filter == 0) {
    filter = AirfieldListView::filter;
  }

  int Nr = 0;
  list->clear();

  for( int item = 0; item<3; item++) {
    // qDebug("fillWpList N: %d", item );
    int nr = _globalMapContents->getListLength(itemList[item]);
    if( nr > Nr ) {
      Nr = nr;
    }
    for(int i=0; i<nr; i++ ) {
      Airport* site = (Airport*)_globalMapContents->getElement( itemList[item], i );
      new _AirfieldItem(list, site);
    }
  }

  list->sort();

  if (Nr>0) {
    list->setCurrentItem(list->firstChild());
    // @AP: set only to true, if something was read
    if(list == AirfieldListView::list) {
      listFilled = true;
    }
  }

  if(filter == AirfieldListView::filter) {
    filter->reset();
  } else {
    filter->reset(true);
  }
}


void AirfieldListView::showEvent(QShowEvent *)
{
  list->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void AirfieldListView::slot_Select()
{
  // qDebug("AirfieldListView::slot_Select");
  emit newWaypoint(getSelectedAirfield(), true);
  emit done();
  filter->off();
}


/** This slot is called if the info button has been clicked */
void AirfieldListView::slot_Info()
{
  // qDebug("AirfieldListView::slot_Info");

  wayPoint *airfieldInfo = getSelectedAirfield();

  if( airfieldInfo ) {
    emit info(airfieldInfo);
  }
}


/** @ee This slot is called if the listview is closed without selecting */
void AirfieldListView::slot_Close ()
{
  // qDebug("AirfieldListView::slot_Close");
  filter->off();
  emit done();
}

void AirfieldListView::slot_Selected() {
  if(AirfieldListView::getSelectedAirfield()->equals(calculator->getselectedWp())) {
    cmdSelect->setEnabled(false);
  } else {
    cmdSelect->setEnabled(true);
  }
}

/** Returns a pointer to the currently highlighted airfield. */
wayPoint * AirfieldListView::getSelectedAirfield(Q3ListView *list)
{
  if(list == 0) {
    list = AirfieldListView::list;
  }

  Q3ListViewItem* li = list->selectedItem();
  _AirfieldItem* apli = static_cast<_AirfieldItem*>(li);

  // @ee may be null, which is the case if there is no selection or the cast failed.
  if(apli == 0) {
    return 0;
  }

  Airport* site = apli->airport;

  wp->name = site->getWPName();
  wp->origP = site->getWGSPosition();
  wp->elevation = site->getElevation();
  wp->projP = site->getPosition();
  wp->description = site->getName();
  wp->type = site->getTypeID();
  wp->elevation = site->getElevation();
  wp->icao = site->getICAO();
  wp->frequency = site->getFrequency().toDouble();
  wp->runway = site->getRunway(0).direction;
  wp->length = site->getRunway(0).length;
  wp->surface = site->getRunway(0).surface;
  wp->comment = "";
  wp->isLandable = true;
  return wp;
}


void AirfieldListView::slot_setHome()
{
  wayPoint * _wp = getSelectedAirfield();

  if (_wp==0) {
    return;
  }

  int answer= QMessageBox::warning(this,tr("Set homesite?"),
                                   tr("Do you want to use airfield\n%1\nas your homesite?").arg(_wp->name),
                                   QMessageBox::Ok | QMessageBox::Default,
                                   QMessageBox::Cancel | QMessageBox::Escape );
  if( answer == QMessageBox::Ok ) {

    // Save new data as home position
    GeneralConfig *conf = GeneralConfig::instance();

    conf->setHomeLat(_wp->origP.lat());
    conf->setHomeLon(_wp->origP.lon());
    conf->save();

    QPoint newPos( _wp->origP.lat(), _wp->origP.lon() );
    emit newHomePosition( &newPos );
  }
}


AirfieldListView::_AirfieldItem::_AirfieldItem(Q3ListView* lv, Airport* site):
  Q3ListViewItem(lv),
  airport(site)
{
  QString name = site->getWPName();
  QRegExp blank("[ ]");
  name.replace(blank, QString::null);
  name = name.left(8);
  setText(0, name);
  setText(1, site->getName());
  setText(2, site->getICAO());
  setPixmap(0, _globalMapConfig->getPixmap(site->getTypeID(),false,true));
}

