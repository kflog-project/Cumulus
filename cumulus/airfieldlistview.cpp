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

#include "generalconfig.h"
#include "cucalc.h"
#include "airport.h"

AirfieldListView::AirfieldListView(QMainWindow *parent ) : QWidget(parent)
{
  setObjectName("AirfieldListView");

  par=parent;
  QBoxLayout *topLayout = new QVBoxLayout( this );

  listw = new AirfieldListWidget( this );

  topLayout->addWidget(listw,10);

  QBoxLayout *buttonrow=new QHBoxLayout;
  topLayout->addLayout(buttonrow);

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
  connect(listw, SIGNAL(wpSelectionChanged()),
          this, SLOT(slot_Selected()));
  connect(this, SIGNAL(done()),
          listw, SLOT(slot_Done()));
  connect(cmdSelect, SIGNAL(clicked()),
          listw, SLOT(slot_Select()));

  wp = new wayPoint();
}


AirfieldListView::~AirfieldListView()
{
  delete wp;
}

void AirfieldListView::showEvent(QShowEvent *)
{
  listw->setFocus();
}



/** This signal is called to indicate that a selection has been made. */
void AirfieldListView::slot_Select()
{
  wayPoint *w = listw->getSelectedWaypoint();
  if ( w ) {
    emit newWaypoint( w, true );
    emit done();
  }
}


/** This slot is called if the info button has been clicked */
void AirfieldListView::slot_Info()
{
  // qDebug("AirfieldListView::slot_Info");

  wayPoint *w = listw->getSelectedWaypoint();

  if ( w )
    emit info( w );
}


/** @ee This slot is called if the listview is closed without selecting */
void AirfieldListView::slot_Close ()
{
  // qDebug("AirfieldListView::slot_Close");
  emit done();
}

void AirfieldListView::slot_Selected() {
  cmdSelect->setEnabled(true);
  wayPoint *w = listw->getSelectedWaypoint();
  if (w)
    if(w->equals(calculator->getselectedWp()))
      cmdSelect->setEnabled(false);
}


void AirfieldListView::slot_setHome()
{
  wayPoint* _wp = listw->getSelectedWaypoint();

  if ( _wp == 0 ) {
    return;
  }

  int answer= QMessageBox::warning(this,
                                   tr("Set Homesite"),
                                   tr("Use airfield\n%1\nas your homesite?").arg(_wp->name),
                                   QMessageBox::Ok | QMessageBox::Cancel );
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
