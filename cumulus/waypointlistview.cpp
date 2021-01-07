/***********************************************************************
**
**   waypointlistview.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Andre Somers
**                   2007-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "TaskPointSelectionList.h"
#include "waypointlistview.h"
#include "generalconfig.h"
#include "wpeditdialog.h"
#include "calculator.h"
#include "mainwindow.h"
#include "layout.h"

extern MapContents* _globalMapContents;

WaypointListView::WaypointListView( QWidget *parent ) :
  QWidget(parent),
  m_homeChanged( false ),
  m_editedWpIsTarget( false )
{
  setObjectName("WaypointListView");

  QGridLayout *topLayout = new QGridLayout( this );

  // set the list widget on top
  listw = new WaypointListWidget( this );
  listw->listWidget()->setSelectionMode(QAbstractItemView::ExtendedSelection);
  topLayout->addWidget( listw, 0, 0 );

  // create a vertical command button row and put it at the right widget side
  const int margin = 5 * Layout::getIntScaledDensity();
  QVBoxLayout *editRow = new QVBoxLayout;
  editRow->setContentsMargins( margin, margin, margin, margin );
  editRow->setSpacing( 20 * Layout::getIntScaledDensity() );
  editRow->addStretch( 10 );

  const int iconSize = Layout::iconSize( font() );

  QPushButton *cmdNew = new QPushButton( this );
  cmdNew->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "add.png" ) ) );
  cmdNew->setIconSize( QSize( iconSize, iconSize ) );
  cmdNew->setToolTip( tr( "Add a new waypoint" ) );
  editRow->addWidget( cmdNew );

  cmdEdit = new QPushButton( this );
  cmdEdit->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "edit_new.png" ) ) );
  cmdEdit->setIconSize( QSize( iconSize, iconSize ) );
  cmdEdit->setToolTip( tr( "Edit selected waypoint" ) );
  editRow->addWidget( cmdEdit );

  cmdDel = new QPushButton( this );
  cmdDel->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "delete.png" ) ) );
  cmdDel->setIconSize( QSize( iconSize, iconSize ) );
  cmdDel->setToolTip( tr( "Delete selected waypoints" ) );
  editRow->addWidget( cmdDel );

  cmdDelAll = new QPushButton( this );
  cmdDelAll->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "clear.png" ) ) );
  cmdDelAll->setIconSize( QSize( iconSize, iconSize ) );
  cmdDelAll->setToolTip( tr( "Delete all waypoints" ) );
  editRow->addWidget( cmdDelAll );

  cmdHome = new QPushButton( this );
  cmdHome->setIcon( QIcon( GeneralConfig::instance()->loadPixmap( "home_new.png" ) ) );
  cmdHome->setIconSize( QSize( iconSize, iconSize ) );
  cmdHome->setToolTip( tr( "Set home site to selected waypoint" ) );
  editRow->addWidget( cmdHome );

  editRow->addStretch( 10 );
  topLayout->addLayout( editRow, 0, 1 );

  // create a horizontal command button row and put it at the bottom of the widget
  QHBoxLayout *buttonRow = new QHBoxLayout;
  QPushButton *cmdClose = new QPushButton( tr( "Close" ), this );
  buttonRow->addWidget( cmdClose );

  cmdInfo = new QPushButton( tr( "Info" ), this );
  buttonRow->addWidget( cmdInfo );

  cmdSelect = new QPushButton( tr( "Select" ), this );
  buttonRow->addWidget( cmdSelect );

  cmdPriority = new QPushButton( tr( "Show All" ), this );
  buttonRow->addWidget( cmdPriority );

  topLayout->addLayout( buttonRow, 1, 0, 1, 2 );

  connect( listw, SIGNAL(searchButtonClicked()), this, SLOT( slot_Search()) );
  connect( cmdNew, SIGNAL(clicked()), this, SLOT(slot_newWP()) );
  connect( cmdEdit, SIGNAL(clicked()), this, SLOT(slot_editWP()) );
  connect( cmdDel, SIGNAL(clicked()), this, SLOT(slot_deleteWPs()) );
  connect( cmdDelAll, SIGNAL(clicked()), this, SLOT(slot_deleteAllWPs()) );
  connect( cmdHome, SIGNAL(clicked()), this, SLOT(slot_setHome()) );
  connect( cmdSelect, SIGNAL(clicked()), this, SLOT(slot_Select()) );
  connect( cmdPriority, SIGNAL(clicked()), this, SLOT(slot_changeDataDisplay()) );
  connect( cmdInfo, SIGNAL(clicked()), this, SLOT(slot_Info()) );
  connect( cmdClose, SIGNAL(clicked()), this, SLOT(slot_Close()) );
  connect( listw, SIGNAL(wpSelectionChanged()), this, SLOT(slot_selectionChanged()) );
  connect( this, SIGNAL(done()), listw, SLOT(slot_Done()) );
}

WaypointListView::~WaypointListView()
{
}

void WaypointListView::showEvent( QShowEvent* event )
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
  m_homeChanged = false;

  QWidget::showEvent( event );
}

/**
 * This slot is called, if the search button is pressed;
 */
void WaypointListView::slot_Search()
{
  if( listw->listWidget()->topLevelItemCount() == 0 )
    {
      // list is empty, return
      return;
    }

  TaskPointSelectionList* wpsl = new TaskPointSelectionList( this, tr("Waypoints") );
  wpsl->setAttribute(Qt::WA_DeleteOnClose);
  wpsl->fillSelectionListWithWaypoints();
  wpsl->resize( MainWindow::mainWindow()->size() );

  connect( wpsl, SIGNAL(takeThisPoint(const SinglePoint*)),
           SLOT(slot_SearchResult( const SinglePoint*)) );

  wpsl->show();
}

/**
 * This slot is called, to pass the search result.
 */
void WaypointListView::slot_SearchResult( const SinglePoint* sp )
{
  // Reset list filter, to get visible the whole list.
  listw->resetListFilter();

  QTreeWidget* lw = listw->listWidget();

  for( int i = 0; i < lw->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem* twi = lw->topLevelItem( i );
      const QString& name = sp->getName();

      if( twi->text(0) == name )
        {
          lw->setCurrentItem( twi );
          lw->scrollToItem( twi, QAbstractItemView::PositionAtTop );
          break;
        }
    }
}

void WaypointListView::slot_Select()
{
  // Select button is pressed. Emits the current selected waypoint as signal
  // and closes the widget.
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
  if( m_homeChanged == true && GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      emit gotoHomePosition();
      m_homeChanged = false;
    }
}

void WaypointListView::slot_selectionChanged()
{
  if( calculator->moving() )
    {
      cmdHome->setVisible(false);
      m_homeChanged = false;
    }
  else
    {
      cmdHome->setVisible(true);
    }

  QList<QTreeWidgetItem *> itemList = listw->listWidget()->selectedItems();

  if( itemList.isEmpty() )
    {
      cmdSelect->setEnabled( false );
      cmdHome->setEnabled( false );
      cmdInfo->setEnabled( false );
      cmdEdit->setEnabled( false );
      cmdDel->setEnabled( false );
      cmdDelAll->setEnabled( false );
      return;
    }

  if( itemList.size() > 1 )
    {
      cmdSelect->setEnabled( false );
      cmdHome->setEnabled( false );
      cmdInfo->setEnabled( false );
      cmdEdit->setEnabled( false );
      cmdDel->setEnabled( true );
      cmdDelAll->setEnabled( true );
      return;
    }

  cmdSelect->setEnabled( true );
  cmdHome->setEnabled( true );
  cmdInfo->setEnabled( true );
  cmdEdit->setEnabled( true );
  cmdDel->setEnabled( true );
  cmdDelAll->setEnabled( true );

  Waypoint *w = listw->getCurrentWaypoint();

  if( w )
    {
      if( w->equals( calculator->getTargetWp() ) )
        {
          cmdSelect->setEnabled( false );
          return;
        }

      GeneralConfig *conf = GeneralConfig::instance();

      if( conf->getHomeLat() == w->wgsPoint.lat() &&
          conf->getHomeLon() == w->wgsPoint.lon() )
        {
          // Selected item is the home position. Disable button press.
          cmdHome->setEnabled( false );
          return;
        }
    }

  cmdSelect->setEnabled( true );
}

/** Called when a new waypoint needs to be made. */
void WaypointListView::slot_newWP()
{
  WpEditDialog *dlg = new WpEditDialog( this, 0 );

  connect( dlg, SIGNAL(wpEdited(Waypoint &)), this,
           SLOT(slot_addWp(Waypoint &)) );

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

  Waypoint *wp = getCurrentEntry();

  if( wp )
    {
      // Check, if waypoint is set in calculator as target. Then we must
      // update the selection after the editing is finished.
      const Waypoint* calcWp = calculator->getTargetWp();

      if( calcWp != 0 && *wp == *calcWp )
        {
          m_editedWpIsTarget = true;
        }
      else
        {
          m_editedWpIsTarget = false;
        }

      WpEditDialog *dlg = new WpEditDialog( this, wp );

      connect( dlg, SIGNAL(wpEdited(Waypoint &)),
               this, SLOT(slot_wpEdited(Waypoint &)) );

      dlg->setVisible( true );
    }
}

/**
 * Called if a waypoint shall be deleted.
 */
void WaypointListView::slot_deleteWp(Waypoint& wp)
{
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  if( wpList.size() == 0 )
    {
      return;
    }

  // The calculator can own a selected waypoint. Important! First
  // announce deletion of waypoint for cancel to have a valid instance.
  const Waypoint* wpc = calculator->getTargetWp();

  if( wpc && *wpc == wp )
    {
      emit deleteWaypoint( &wp );
    }

  // Second delete the waypoint
  listw->deleteWaypoint( wp );

  MainWindow::mainWindow()->viewMap->getMap()->scheduleRedraw( Map::waypoints );
}

/** Called when the selected waypoints should be deleted from the catalog */
void WaypointListView::slot_deleteWPs()
{
  QList<Waypoint *> wpList = listw->getSelectedWaypoints();

  if( wpList.size() == 0 )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete?" ),
                  tr( "Delete selected waypoints?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // The calculator can own a selected waypoint. Important! First
      // announce deletion of waypoint for cancel to have a valid instance.
      const Waypoint* wpc = calculator->getTargetWp();

      if( wpc )
        {
          for( int i = 0; i < wpList.size(); i++ )
            {
              Waypoint* wpl = wpList.at(i);

              if( *wpc == *wpl )
                {
                  emit deleteWaypoint( wpl );
                  break;
                }
            }
        }

      // Second delete all selected waypoints
      listw->deleteSelectedWaypoints();

      MainWindow::mainWindow()->viewMap->getMap()->scheduleRedraw( Map::waypoints );
   }
}

/**
 * Called to remove all listed waypoints in the listview.
 * Note! The listview can show waypoints separated by priority.
 */
void WaypointListView::slot_deleteAllWPs()
{
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  if( wpList.size() == 0 )
    {
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Delete All?" ),
                  tr( "Delete all listed waypoints?" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // The calculator can own a waypoint. Important! First
      // announce deletion of waypoint for cancel to have a valid instance.
      const Waypoint* wpc = calculator->getTargetWp();

      if( wpc )
        {
          for( int i = 0; i < wpList.size(); i++ )
            {
              Waypoint& wpl = wpList[i];

              if( *wpc == wpl )
                {
                  emit deleteWaypoint( &wpl );
                  break;
                }
            }
        }

      // First select all items in the list
      listw->listWidget()->selectAll();

      // Second delete all selected waypoints
      listw->deleteSelectedWaypoints();

      MainWindow::mainWindow()->viewMap->getMap()->scheduleRedraw( Map::waypoints );
    }
}

/** Called if a waypoint has been edited. */
void WaypointListView::slot_wpEdited( Waypoint& wp )
{
  if( m_editedWpIsTarget == true )
    {
      // Update the target waypoint in the calculator.
      emit newWaypoint( &wp, true );
    }

  listw->updateCurrentWaypoint( wp );
  listw->fillItemList();

  MainWindow::mainWindow()->viewMap->getMap()->scheduleRedraw( Map::waypoints );
}

/** Called if a waypoint should be added. */
void WaypointListView::slot_addWp( Waypoint& wp )
{
  listw->addWaypoint( wp );

  MainWindow::mainWindow()->viewMap->getMap()->scheduleRedraw( Map::waypoints );
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

  if( conf->getHomeLat() == _wp->wgsPoint.lat() &&
      conf->getHomeLon() == _wp->wgsPoint.lon() )
    {
      // no new coordinates, ignore request
      return;
    }

  QMessageBox mb( QMessageBox::Question,
                  tr( "Set home site"),
                  tr("Use point<br><b>%1</b><br>as your home site?").arg(_wp->name) +
                  tr("<br>Change can take<br>a few seconds."),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  if( mb.exec() == QMessageBox::Yes )
    {
      // save new home position and elevation
      conf->setHomeCountryCode( _wp->country );
      conf->setHomeName( _wp->name );
      conf->setHomeCoord( _wp->wgsPoint );
      conf->setHomeElevation( Distance(_wp->elevation) );

      emit newHomePosition( _wp->wgsPoint );
      m_homeChanged = true;
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
