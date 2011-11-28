/***********************************************************************
**
**   waypointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Andre Somers
**                   2007-2011 by Axel Pauli
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

extern MapContents* _globalMapContents;

WaypointListView::WaypointListView( QMainWindow *parent ) :
  QWidget(parent),
  homeChanged( false ),
  priorityOfEditedWp( Waypoint::Top )
{
  setObjectName("WaypointListView");
  par = parent;

  QGridLayout *topLayout = new QGridLayout( this );

  // set the list widget on top
  listw = new WaypointListWidget( this );
  listw->listWidget()->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
  cmdDel->setToolTip( tr( "Delete selected waypoints" ) );
  editRow->addWidget( cmdDel );

  QPushButton *cmdDelAll = new QPushButton( this );
  cmdDelAll->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "clear.png" ) ) );
  cmdDelAll->setIconSize( QSize( IconSize, IconSize ) );
  cmdDelAll->setToolTip( tr( "Delete all waypoints" ) );
  editRow->addWidget( cmdDelAll );

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

  cmdPriority = new QPushButton( tr( "Show All" ), this );
  buttonRow->addWidget( cmdPriority );

  topLayout->addLayout( buttonRow, 1, 0, 1, 2 );

  connect( cmdNew, SIGNAL(clicked()), this, SLOT(slot_newWP()) );
  connect( cmdEdit, SIGNAL(clicked()), this, SLOT(slot_editWP()) );
  connect( cmdDel, SIGNAL(clicked()), this, SLOT(slot_deleteWPs()) );
  connect( cmdDelAll, SIGNAL(clicked()), this, SLOT(slot_deleteAllWPs()) );
  connect( cmdHome, SIGNAL(clicked()), this, SLOT(slot_setHome()) );
  connect( cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()) );
  connect( cmdPriority, SIGNAL(clicked()), this, SLOT(slot_changeDataDisplay()) );
  connect( cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()) );
  connect( cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()) );
  connect( listw, SIGNAL(wpSelectionChanged()), this, SLOT(slot_Selected()) );
  connect( this, SIGNAL(done()), listw, SLOT(slot_Done()) );
}

WaypointListView::~WaypointListView()
{
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
  Waypoint *w = listw->getCurrentWaypoint();

  if ( w )
    {
      emit newWaypoint( w, true );
      slot_Close();
    }
}

/** This slot is called if the info button has been clicked */
void WaypointListView::slot_Info()
{
  Waypoint *w = listw->getCurrentWaypoint();

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

  Waypoint *w = listw->getCurrentWaypoint();

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

  connect( dlg, SIGNAL(wpListChanged(Waypoint &)), this,
           SLOT(slot_wpAdded(Waypoint &)) );

  dlg->setVisible( true );
}

/** Called when the selected waypoint needs to be opened in the editor */
void WaypointListView::slot_editWP()
{
  QList<Waypoint *> wpList = listw->getSelectedWaypoints();

  if( wpList.size() > 1 )
    {
      // Multiple waypoints are selected, edit allows only one selection.
      return;
    }

  Waypoint *wp = getSelectedWaypoint();

  if( wp )
    {
      // Save old priority for later check.
      priorityOfEditedWp = wp->priority;

      WpEditDialog *dlg = new WpEditDialog( this, wp );

      connect( dlg, SIGNAL(wpListChanged(Waypoint &)), this,
               SLOT(slot_wpEdited(Waypoint &)) );

      dlg->setVisible( true );
    }
}

/** Called when the selected waypoints should be deleted from the catalog */
void WaypointListView::slot_deleteWPs()
{
  QList<Waypoint *> wpList = listw->getSelectedWaypoints();

  if( wpList.size() == 0 )
    {
      return;
    }

  int answer= QMessageBox::question(this, tr("Delete"),
                                   tr("Delete selected waypoints?"),
                                   QMessageBox::No, QMessageBox::Yes);

  if( answer == QMessageBox::Yes )
    {
      // The calculator can own a selected waypoint. Important! First
      // announce deletion of waypoint for cancel to have a valid instance.
      const Waypoint* wpc = calculator->getselectedWp();

      if( wpc )
        {
          for( int i = 0; i < wpList.size(); i++ )
            {
              Waypoint* wpl = wpList.at(i);

              if( wpc == wpl )
                {
                  emit deleteWaypoint( wpl );
                }
            }
        }

      // Second delete all selected waypoints
      listw->deleteSelectedWaypoints();

      if( par )
        {
          ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
        }
    }
}

/** Called to remove all waypoints of the catalog. */
void WaypointListView::slot_deleteAllWPs()
{
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  if( wpList.size() == 0 )
    {
      return;
    }

  int answer= QMessageBox::question(this, tr("Delete All"),
                                   tr("Delete all waypoints?"),
                                   QMessageBox::No, QMessageBox::Yes);

  if( answer == QMessageBox::Yes )
    {
      // The calculator can own a waypoint. Important! First
      // announce deletion of waypoint for cancel to have a valid instance.
      const Waypoint* wpc = calculator->getselectedWp();

      if( wpc )
        {
          for( int i = 0; i < wpList.size(); i++ )
            {
              Waypoint& wpl = wpList[i];

              if( *wpc == wpl )
                {
                  emit deleteWaypoint( &wpl );
                }
            }
        }

      // Second delete all waypoints
      listw->deleteAllWaypoints();

      if( par )
        {
          ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
        }
    }
}

/** Called if a waypoint has been edited. */
void WaypointListView::slot_wpEdited(Waypoint& wp)
{
  //  qDebug("WaypointListView::slot_wpEdited");
  listw->updateCurrentWaypoint( wp );

  if( listw->getWaypointPriority() != Waypoint::Top &&
      priorityOfEditedWp != wp.priority )
    {
      // We must update the list view because the waypoint priority has been
      // changed.
      listw->fillItemList();
    }

  if( par )
    {
      ((MainWindow*) par)->viewMap->_theMap->scheduleRedraw( Map::waypoints );
    }
}

/** Called if a waypoint has been added. */
void WaypointListView::slot_wpAdded(Waypoint& wp)
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
  Waypoint *_wp = listw->getCurrentWaypoint();

  if( _wp == static_cast<Waypoint *> ( 0 ) )
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
      conf->setHomeCountryCode( _wp->country );
      conf->setHomeCoord( _wp->origP );
      conf->setHomeElevation( Distance(_wp->elevation) );

      emit newHomePosition( _wp->origP );
      homeChanged = true;
    }
}

/**
 * Called to change the displayed data according their priority.
 */
void WaypointListView::slot_changeDataDisplay()
{
  switch( listw->getWaypointPriority() )
    {
      case Waypoint::Low:
        listw->setWaypointPriority( Waypoint::Normal );
        cmdPriority->setText( tr("Show Normal") );
        break;
      case Waypoint::Normal:
        listw->setWaypointPriority( Waypoint::High );
        cmdPriority->setText( tr("Show High") );
        break;
      case Waypoint::High:
        listw->setWaypointPriority( Waypoint::Top );
        cmdPriority->setText( tr("Show All") );
        break;
      case Waypoint::Top:
      default:
        listw->setWaypointPriority( Waypoint::Low );
        cmdPriority->setText( tr("Show Low") );
        break;
    }

  listw->fillItemList();
}
