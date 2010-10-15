/***********************************************************************
**
**   waypointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Andre Somers
**                   2007-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "waypointlistview.h"
#include "generalconfig.h"
#include "wpeditdialog.h"
#include "calculator.h"
#include "mainwindow.h"
#include "layout.h"

WaypointListView::WaypointListView( QMainWindow *parent ) :
  QWidget(parent),
  homeChanged( false )
{
  setObjectName("WaypointListView");
  par = parent;

  QGridLayout *topLayout = new QGridLayout( this );

  // set the list widget on top
  listw = new WaypointListWidget( this );
  topLayout->addWidget( listw, 0, 0 );

  // create a vertical command button row and put it at the right widget side
  QVBoxLayout *editRow = new QVBoxLayout;
  editRow->addStretch( 10 );

  QPushButton *cmdNew = new QPushButton( this );
  cmdNew->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  cmdNew->setIconSize( QSize( IconSize, IconSize ) );
  cmdNew->setToolTip( tr( "Add a new waypoint" ) );
  editRow->addWidget( cmdNew );

  QPushButton *cmdEdit = new QPushButton( this );
  cmdEdit->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "edit_new.png" ) ) );
  cmdEdit->setIconSize( QSize( IconSize, IconSize ) );
  cmdEdit->setToolTip( tr( "Edit selected waypoint" ) );
  editRow->addWidget( cmdEdit );

  QPushButton *cmdDel = new QPushButton( this );
  cmdDel->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  cmdDel->setIconSize( QSize( IconSize, IconSize ) );
  cmdDel->setToolTip( tr( "Delete selected waypoint" ) );
  editRow->addWidget( cmdDel );

  editRow->addSpacing( 10 );

  cmdHome = new QPushButton( this );
  cmdHome->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home_new.png" ) ) );
  cmdHome->setIconSize( QSize( IconSize, IconSize ) );
  cmdHome->setToolTip( tr( "Set home site to selected waypoint" ) );
  editRow->addWidget( cmdHome );

  editRow->addStretch( 10 );
  topLayout->addLayout( editRow, 0, 1 );

  // create a horizontal command button row and put it at the bottom of the widget
  QHBoxLayout *buttonRow = new QHBoxLayout;
  QPushButton *cmdClose = new QPushButton( tr( "Close" ), this );
  buttonRow->addWidget( cmdClose );

  QPushButton *cmdInfo = new QPushButton( tr( "Info" ), this );
  buttonRow->addWidget( cmdInfo );

  cmdSelect = new QPushButton( tr( "Select" ), this );
  buttonRow->addWidget( cmdSelect );

  topLayout->addLayout( buttonRow, 1, 0, 1, 2 );

  connect( cmdNew, SIGNAL(clicked()), this, SLOT(slot_newWP()) );
  connect( cmdEdit, SIGNAL(clicked()), this, SLOT(slot_editWP()) );
  connect( cmdDel, SIGNAL(clicked()), this, SLOT(slot_deleteWP()) );
  connect( cmdHome, SIGNAL(clicked()), this, SLOT(slot_setHome()) );
  connect( cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()) );
  connect( cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()) );
  connect( cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()) );
  connect( listw, SIGNAL(wpSelectionChanged()), this, SLOT(slot_Selected()) );
  connect( this, SIGNAL(done()), listw, SLOT(slot_Done()) );

  // activate keyboard shortcut Return as select
  QShortcut* scSelect = new QShortcut( this );
  scSelect->setKey( Qt::Key_Return );
  connect( scSelect, SIGNAL(activated()), this, SLOT( slot_Select() ) );
}


WaypointListView::~WaypointListView()
{
  // qDebug("WaypointListView::~WaypointListView()");
}


void WaypointListView::showEvent(QShowEvent *)
{
  // Show the home button only if we are not to fast in move to avoid
  // wrong usage. The redefinition of the home position can trigger
  // a reload of the airfield list.
  if( calculator->moving() )
    {
      cmdHome->setVisible(false);
    }
  else
    {
      cmdHome->setVisible(true);
    }

  // Reset home changed
  homeChanged = false;
}


/** This signal is called to indicate that a selection has been made. */
void WaypointListView::slot_Select()
{
  wayPoint *w = listw->getSelectedWaypoint();

  if ( w )
    {
      emit newWaypoint( w, true );
      slot_Close();
    }
}


/** This slot is called if the info button has been clicked */
void WaypointListView::slot_Info()
{
  wayPoint *w = listw->getSelectedWaypoint();

  if( w )
    {
      emit info( w );
    }
}

/** @ee This slot is called if the listview is closed without selecting */
void WaypointListView::slot_Close()
{
  // That will switch back to the map view. This must be done first to ensure
  // that the home position change does work.
  emit done();

  // Check, if we have not a valid GPS fix. In this case we do move the map
  // to the new home position.
  if( homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      homeChanged = false;
    }
}

void WaypointListView::slot_Selected()
{
  cmdSelect->setEnabled( true );

  wayPoint *w = listw->getSelectedWaypoint();

  if( w )
    {
      if( w->equals( calculator->getselectedWp() ) )
        {
          cmdSelect->setEnabled( false );
        }
    }
}

/** Called when a new waypoint needs to be made. */
void WaypointListView::slot_newWP()
{
  WpEditDialog *dlg = new WpEditDialog( this, 0 );

  connect( dlg, SIGNAL(wpListChanged(wayPoint &)), this,
           SLOT(slot_wpAdded(wayPoint &)) );

  dlg->show();
}

/** Called when the selected waypoint needs to be opened in the editor */
void WaypointListView::slot_editWP()
{
  wayPoint *wp = getSelectedWaypoint();

  if( wp )
    {
      WpEditDialog *dlg = new WpEditDialog( this, wp );

      connect( dlg, SIGNAL(wpListChanged(wayPoint &)), this,
               SLOT(slot_wpEdited(wayPoint &)) );

      dlg->show();
    }
}

/** Called when the selected waypoint should be deleted from the catalog */
void WaypointListView::slot_deleteWP()
{
  wayPoint* wp = listw->getSelectedWaypoint();

  if ( wp == static_cast<wayPoint *>(0) )
    {
      return;
    }

  int answer= QMessageBox::question(this, tr("Delete"),
                                   tr("Delete selected waypoint?"),
                                   QMessageBox::No, QMessageBox::Yes);

  if( answer == QMessageBox::Yes )
    {
      // @AP: Important! First announce deletion of waypoint for cancel
      //      to have a valid instance.
      emit deleteWaypoint( wp );

      // Second delete selected waypoint
      listw->deleteSelectedWaypoint();

      if( par )
        {
          ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
        }
    }
}

/** Called if a waypoint has been edited. */
void WaypointListView::slot_wpEdited(wayPoint& wp)
{
  //  qDebug("WaypointListView::slot_wpEdited");
  listw->updateSelectedWaypoint( wp );

  if( par )
    {
      ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
    }
}

/** Called if a waypoint has been added. */
void WaypointListView::slot_wpAdded(wayPoint& wp)
{
  // qDebug("WaypointListView::slot_wpAdded(): name=%s", wp->name.toLatin1().data());
  listw->addWaypoint( wp );

  if( par )
    {
      ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
    }
}

/** Called to set a new home position */
void WaypointListView::slot_setHome()
{
  wayPoint *_wp = listw->getSelectedWaypoint();

  if( _wp == static_cast<wayPoint *> ( 0 ) )
    {
      return;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getHomeLat() == _wp->origP.lat() && conf->getHomeLon()
      == _wp->origP.lon() )
    {
      // no new coordinates, ignore request
      return;
    }

  int answer= QMessageBox::question(this,
                                   tr("Set home site"),
                                   tr("Use point<br><b>%1</b><br>as your home site?").arg(_wp->name) +
                                   tr("<br>Change can take<br>a few seconds."),
                                   QMessageBox::No, QMessageBox::Yes );

  if( answer == QMessageBox::Yes )
    {
      // save new home position and elevation
      conf->setHomeCoord( _wp->origP );
      conf->setHomeElevation( Distance(_wp->elevation) );

      emit newHomePosition( _wp->origP );
      homeChanged = true;
    }
}
