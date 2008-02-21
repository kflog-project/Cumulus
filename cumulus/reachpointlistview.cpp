/***********************************************************************
**
**   reachpointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard V�llm, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QPushButton>
#include <QFont>
#include <Q3ListViewItem>

#include "reachpointlistview.h"
#include "cumulusapp.h"
#include "waypointlistview.h"
#include "generalconfig.h"
#include "mapcontents.h"
#include "mapconfig.h"
#include "wpeditdialog.h"
#include "waypointcatalog.h"
#include "reachablelist.h"
#include "colorlistviewitem.h"
#include "sonne.h"

extern CuCalc* calculator;

ReachpointListView::ReachpointListView(CumulusApp *parent, const char *name ) : QWidget(parent,name)
{
  // load pixmap of arrows for relative bearing
  _arrows = GeneralConfig::instance()->loadPixmap("arrows-15.png");
  _newList=true; //make sure we fill our list the first time it is shown

  par=parent;
  _outlandShow = true;  // Show outlandigs by default;
  QBoxLayout *topLayout = new QVBoxLayout( this );


  list= new Q3ListView(this, "reachpointlist");
  list->addColumn(tr("Name"));
  list->addColumn(tr("Dist"));
  list->addColumn(tr("Brg"));
  list->addColumn(tr("RB"));
  list->addColumn(tr("Arv"));
  list->addColumn(tr("SS"));

  list->setColumnAlignment( 1, Qt::AlignRight );
  list->setColumnAlignment( 2, Qt::AlignRight );
  //list->setColumnAlignment( 3, Qt::AlignRight );
  list->setColumnAlignment( 4, Qt::AlignRight );
  list->setColumnAlignment( 5, Qt::AlignRight );
    
  list->setAllColumnsShowFocus(true);
  list->setSelectionMode( Q3ListView::Single );
  list->setSorting(4,false);
  fillRpList();

  topLayout->addWidget(list,10);
  QBoxLayout *buttonrow=new QHBoxLayout(topLayout);

  /** @ee add a close button */
  QPushButton *cmdClose = new QPushButton(tr("Close"), this);
  buttonrow->addWidget(cmdClose);

  QPushButton *cmdInfo = new QPushButton(tr("Info"), this);
  buttonrow->addWidget(cmdInfo);

  cmdHideOl = new QPushButton(tr("Hide Outland"), this);
  buttonrow->addWidget(cmdHideOl);

  cmdShowOl = new QPushButton(tr("Show Outland"), this);
  buttonrow->addWidget(cmdShowOl);

  cmdSelect = new QPushButton(tr("Select"), this);
  buttonrow->addWidget(cmdSelect);

  connect(cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()));
  connect(cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()));
  // @ee add a close button
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));
  connect(cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()));
  connect(cmdHideOl, SIGNAL(clicked()), this, SLOT(slot_HideOl()));
  connect(cmdShowOl, SIGNAL(clicked()), this, SLOT(slot_ShowOl()));
  connect(list, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slot_Selected()));
  cmdShowOl->hide();
  cmdHideOl->show();
}


ReachpointListView::~ReachpointListView()
{}


/** Retreives the reachpoints from the mapcontents, and fills the list. */
void ReachpointListView::fillRpList()
{
  int safetyAlt = (int)GeneralConfig::instance()->getSafetyAltitude().getMeters();

  if( calculator == 0 )
    return;
  // int n= calculator->getReachList()->getNumberSites();
  extern MapConfig * _globalMapConfig;
  Q3ListViewItem* si = list->selectedItem();
  QString sname;
  QPixmap icon;
  QPixmap iconImage;
  QBitmap iconMask;
  QPainter pnt;
  icon.resize(18,18);

  bool selected = false;
  if( si )
    sname = si->text(0);
  list->clear();
  // Create a pointer to the list of nearest sites
  QList<ReachablePoint*> *pl = calculator->getReachList()->getList();
  int num = 0;
  // Do a loop over all entries in the table of nearest sites
  for (int i = 0; i < pl->count()&&
         num < calculator->getReachList()->getMaxNrOfSites(); i++, num++) {
    ReachablePoint *rp = pl->at(i);

    if( !_outlandShow && (rp->getType() == BaseMapElement::Outlanding) )
      continue;

    // Setup string for bearing
    QString bearing=QString("%1°").arg(rp->getBearing());
    // Calculate relative bearing too, very cool feature
    int relbearing = rp->getBearing() - calculator->getlastHeading();

    while (relbearing < 0)
      relbearing += 360;
    // This string is only used for sorting and is eclipsed by next field
    QString RB = QString("  %1").arg(relbearing);

    // calculate sunset
    QString sr, ss;
    QDate date = QDate::currentDate();
  
    Sonne::sonneAufUnter( sr, ss, date, rp->getWgsPos(), 0 );

    ColorListViewItem * li=new ColorListViewItem(list, rp->getName(),
                                                 rp->getDistance().getText(false,1),
                                                 bearing,
                                                 RB,
                                                 rp->getArrivalAlt().getText(false,0),
                                                 " " + ss );

    // icons now supported
    //icon.fill();
    pnt.begin(&icon);
    if (rp->getReachable()==yes) {
      //draw green circle
      //pnt.drawPixmap(0,0, _globalMapConfig->getPixmap("green_circle.xpm"));
      icon.fill(QColor(0,255,0));
    } else if (rp->getReachable() == belowSavety) {
      //draw magenta circle
      //pnt.drawPixmap(0,0, _globalMapConfig->getPixmap("magenta_circle.xpm"));
      icon.fill(QColor(255,0,255));
    } else {
      //pnt.drawPixmap(0,0, _globalMapConfig->getPixmap("red_circle.xpm"));
      //icon.fill(QColor(255,0,0));
      icon.fill(QColor(255,255,255));
    }

    pnt.drawPixmap(1, 1, _globalMapConfig->getPixmap(rp->getType(),false,true) );
    pnt.end();
    li->setPixmap(0, icon);

    // set Pixmap for rel. Bearing
    int rot=((relbearing+7)/15) % 24;
    QPixmap arrow (16,16);
    bitBlt(&arrow, 0, 0, &_arrows, rot*16, 0, 16, 16);
    li->setPixmap(3, arrow );
    // store name of last selected to avoid jump to first element on each fill
    if( rp->getName() == sname ) {
      selected = true;
      list->setSelected( li, true );
    }
    // print non reachable sites too but in read color
    if( rp->getArrivalAlt().getMeters() > 0 )
      li->setColor(Qt::darkGreen); //li->setRed(false);
    else if  (rp->getArrivalAlt().getMeters() > -safetyAlt)
      li->setColor(Qt::darkMagenta);
    else
      li->setColor(Qt::black); //li->setRed(true);
  }
  // sort list
  list->sort();
  if (!selected) {
    list->setCurrentItem(list->firstChild());
  }

}


void ReachpointListView::showEvent(QShowEvent *)
{
  if (_newList) {
    fillRpList();
    _newList=false;
  }
  list->setFocus();
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_ShowOl()
{
  _outlandShow = true;
  cmdShowOl->hide();
  cmdHideOl->show();
  fillRpList();
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_HideOl()
{
  _outlandShow = false;
  cmdShowOl->show();
  cmdHideOl->hide();
  fillRpList();
}


/** This signal is called to indicate that a selection has been made. */
void ReachpointListView::slot_Select()
{
  emit newWaypoint(getSelectedWaypoint(), true);
  emit done();
}


/** This slot is called if the info button has been clicked */
void ReachpointListView::slot_Info()
{
  wayPoint *airfieldInfo = getSelectedWaypoint();

  if( airfieldInfo ) {
    emit info(airfieldInfo);
  }
}


/** @ee This slot is called if the listview is closed without selecting */
void ReachpointListView::slot_Close ()
{
  emit done();
}

void ReachpointListView::slot_Selected() {
  if(ReachpointListView::getSelectedWaypoint()->equals(calculator->getselectedWp())) {
    cmdSelect->setEnabled(false);
  } else {
    cmdSelect->setEnabled(true);
  }
}


/** Returns a pointer to the currently highlighted reachpoint. */
wayPoint * ReachpointListView::getSelectedWaypoint()
{
  ReachablePoint * rp;
  int i,n;
  n =  calculator->getReachList()->getNumberSites();
  Q3ListViewItem* li = list->selectedItem();
  // @ee may be null
  if (li) {
    for (i=0; i < n; i++) {
      rp=calculator->getReachList()->getSite(i);
      if (rp->getName()==li->text(0)) {
        selectedWp = *(rp->getWaypoint());
        selectedWp.importance = wayPoint::Normal;  // set importance to normal

        // if( rp->isOrignAfl() ) {
        // swap name and description for uniform waypoint handling
        // QString tmp = selectedWp.name;
        // selectedWp.name = selectedWp.description;
        // selectedWp.description = tmp;
        // }

        return &selectedWp;
        break;
      }
    }
  }
  return 0;
}


void ReachpointListView::slot_newList()
{
  if ( this->isVisible()) {
    fillRpList();
    _newList=false;
  } else {
    _newList=true;
  }
}

