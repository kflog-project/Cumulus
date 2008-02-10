/***************************************************************************
 cumulusapp.cpp  -  main application class
                          -------------------
 begin                : Sun Jul 21 2002
 copyright            : (C) 2002 by Andre Somers
 ported to Qt4.3/X11  : (C) 2008 by Axel pauli
 email                : andre@kflog.org, axel@kflog.org

  This file is distributed under the terms of the General Public
  Licence. See the file COPYING for more information.

  $Id$

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <QtGlobal>
#include <QDesktopWidget>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <Q3PtrList>
#include <QMessageBox>
#include <QSound>

#include "generalconfig.h"
#include "cumulusapp.h"
#include "airport.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "cucalc.h"
#include "igclogger.h"
#include "windanalyser.h"
#include "configdialog.h"
#include "wpeditdialog.h"
#include "preflightdialog.h"
#include "wgspoint.h"
#include "waypoint.h"
#include "hwinfo.h"
#include "gliderlist.h"
#include "gpsstatusdialog.h"
#include "target.h"

/**
 * Global available instance of this class
 */
CumulusApp *_globalCumulusApp = (CumulusApp *) 0;

/**
 * Global available instance of logger class
 */
IgcLogger *logger = (IgcLogger *) 0;

/**
 * Used for transforming the map items.
 */
MapMatrix *_globalMapMatrix = (MapMatrix *) 0;

/**
 * Contains all mapelements and takes control over drawing or printing
 * the elements.
 */
MapContents *_globalMapContents = (MapContents *) 0;

/**
 * Contains all configuration-info for drawing and printing the elements.
 */
MapConfig *_globalMapConfig = (MapConfig *) 0;

/**
 * Contains all map view infos
 */
MapView *_globalMapView = (MapView *) 0;


// A signal SIGCONT has been catched. It is send out
// when the cumulus process was stopped and then reactivated.
// We have to renew the connection to our GPS Receiver.
static void resumeGpsConnection( int sig )
{
  if ( sig == SIGCONT )
    {
      gps->forceReset();
      // @ee reinstall signal handler
      signal ( SIGCONT, resumeGpsConnection );
    }
}

/** As the name tells ...
  *
  */
void CumulusApp::playSound( const QString *name )
{
  GeneralConfig * conf = GeneralConfig::instance();

  if ( ! conf->getAlarmSoundOn() )
    {
      return ;
    }

  if ( name )
    {
      QSound sound( *name );
      sound.play();
    }
  else if( QSound::isAvailable() )
    {
      // Default sound is played, if QSound is available
      QString file = GeneralConfig::instance()->getInstallRoot() + "/sounds/Alarm.wav";
      qDebug( "SoundFile=%s", file.toLatin1().data() );
      QSound sound( file );
      sound.play();
    }
  else
    {
      // Last default -> do beep only
      QApplication::beep ();
    }
}


void CumulusApp::slotNotification( const QString& msg, const bool sound )
{
  if ( msg.isEmpty() )
    {
      setCaption ( "Cumulus: " + calculator->gliderType() );
      return ;
    }

  if ( sound )
    {
      playSound();
    }

  setCaption ( msg + " " );
  viewMap->message( msg );
}


CumulusApp::CumulusApp( QMainWindow *parent, Qt::WindowFlags flags ) : QMainWindow( parent, flags )
{
  _globalCumulusApp = this;
  menuBarVisible = false;
  listViewTabs = 0;

  // get last saved window geometrie from generalconfig and set it again
  resize( GeneralConfig::instance()->getWindowSize() );

  qWarning( "Cumulus, Release: %s, Build date:  %s based on Qt/X11 Version %s",
            CU_VERSION,  __DATE__, QT_VERSION_STR );

  qDebug( "Desktop size is %dx%d, width=%d, height=%d",
          QApplication::desktop()->screenGeometry().width(),
          QApplication::desktop()->screenGeometry().height(),
          QApplication::desktop()->screenGeometry().width(),
          QApplication::desktop()->screenGeometry().height() );

  qDebug( "Main window size is %dx%d, width=%d, height=%d",
          size().width(),
          size().height(),
          size().width(),
          size().height() );

  // @AP: Display some environment variables, to get more clearness
  // about their settings during process startup
  char *pwd = getenv( "PWD" );
  char *user = getenv( "USER" );
  char *home = getenv( "HOME" );
  char *lang = getenv( "LANG" );
  char *ldpath = getenv( "LD_LIBRARY_PATH" );
  char *qtdir = getenv( "QTDIR" );
  char *qwsdisplay = getenv( "DISPLAY" );

  qDebug( "PWD=%s", pwd ? pwd : "NULL" );
  qDebug( "USER=%s", user ? user : "NULL" );
  qDebug( "HOME=%s", home ? home : "NULL" );
  qDebug( "LANG=%s", lang ? lang : "NULL" );
  qDebug( "LD_LIBRARY_PATH=%s", ldpath ? ldpath : "NULL" );
  qDebug( "QTDIR=%s", qtdir ? qtdir : "NULL" );
  qDebug( "QDir::homeDirPath()=%s", QDir::homeDirPath().toLatin1().data() );
  qDebug( "DISPLAY=%s", qwsdisplay ? qwsdisplay : "NULL" );

  if( ! QSound::isAvailable() )
    {
      qWarning( "QSound is not available on this device!" );
    }

  // Check, if in users home a cumulus application directory exists,
  // otherwise create it.
  QDir cuApps( QDir::homeDirPath() + "/cumulus" );

  if ( ! cuApps.exists() )
    {
      cuApps.mkdir( QDir::homeDirPath() + "/cumulus" );
    }

  // @AS: Grab the keyboard to get all hardware keys (f1..f12) too
  setFocusPolicy( Qt::StrongFocus );
  setFocus();
  //grabKeyboard(); // @AP: make problems on Qt4.3/X11

  this->installEventFilter( this );

  setIcon( GeneralConfig::instance()->loadPixmap( "cumulus.png" ) );
  setCaption( "Cumulus" );

  // use showMaximized() only for PDA
  show();

#warning switch screensaver off, if necessary

  ws = new WaitScreen( this, "Waitscreen" );

  ws->slot_SetText1( tr( "Creating map elements..." ) );
  _globalMapMatrix = new MapMatrix( this );

  _globalMapContents = new MapContents( this, ws );

  _globalMapConfig = new MapConfig( this );

  BaseMapElement::initMapElement( _globalMapMatrix, _globalMapConfig );

  calculator = new CuCalc( this );

  connect( _globalMapMatrix, SIGNAL( displayMatrixValues( int, bool ) ),
           _globalMapConfig, SLOT( slotSetMatrixValues( int, bool ) ) );

  connect( _globalMapMatrix, SIGNAL( homePositionChanged() ),
           _globalMapContents, SLOT( slotReloadWelt2000Data() ) );

  connect( _globalMapConfig, SIGNAL( configChanged() ),
           _globalMapMatrix, SLOT( slotInitMatrix() ) );

  _globalMapConfig->slotReadConfig();

  _altimeterModeDlg = new AltimeterModeDialog( this );
  _varioModeDlg = new VarioModeDialog( this );

  ws->slot_SetText1( tr( "Creating views..." ) );

  view = mapView;

  qDebug( "Main window size is %dx%d, width=%d, height=%d",
          size().width(),
          size().height(),
          size().width(),
          size().height() );

  // This is the main widget of cumulus
  viewMap = new MapView( this, "MapView" );
  _globalMapView = viewMap;

  setCentralWidget( viewMap );

  listViewTabs = new QTabWidget( this, "ListViewTabs", Qt::WStyle_StaysOnTop );
  listViewTabs->resize( this->size() );

  QFont fnt( "Helvetica", 12, QFont::Bold );

  viewWP = new WaypointListView( 0, "WaypointListView" );
  viewAF = new AirfieldListView( 0, "AirfieldListView" );
  viewRP = new ReachpointListView( 0, "ReachpointListView" );
  viewTP = new TaskListView( 0, "TaskListView" );
  viewWP->setFont( fnt );
  viewAF->setFont( fnt );
  viewRP->setFont( fnt );
  viewTP->setFont( fnt );

  _taskListVisible = false;
  _reachpointListVisible = false;
  listViewTabs->addTab( viewWP, tr( "Waypoints" ) );
  //listViewTabs->addTab( viewRP, tr( "Reachable" ) ); --> added in slot_readconfig
  listViewTabs->addTab( viewAF, tr( "Airfields" ) );

  // waypoint info widget
  viewInfo = new WPInfoWidget( this, "WPInfoview" );

  viewWP->fillWpList( _globalMapContents->getWaypointList() );

  //create global objects
  gps = new GPSNMEA( this );
  gps->blockSignals( true );
  logger = IgcLogger::instance();
  _preFlightDialog = NULL;
  _gliderFlightDlg = NULL;

  initActions();
  initMenuBar();

  ws->slot_SetText1( tr( "Setting up connections..." ) );
  // set dynamical Info
  viewMap->getAltitudeWidget()->setPreText( _altimeterModeDlg->Pretext() );

  // create connections between the components
  connect( _globalMapMatrix, SIGNAL( matrixChanged() ),
           viewMap->_theMap, SLOT( slotRedraw() ) );
  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           _globalMapContents, SLOT( slotReloadMapData() ) );

  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewMap->_theMap, SLOT( slotDraw() ) );

  connect( viewMap->_theMap, SIGNAL( isRedrawing( bool ) ),
           this, SLOT( slotMapDrawEvent( bool ) ) );

  connect( _altimeterModeDlg, SIGNAL( newAltimeterMode() ),
           this, SLOT( slotNewAltimeterMode() ) );
  connect( _altimeterModeDlg, SIGNAL( settingsChanged() ),
           calculator, SLOT( slot_settingschanged() ) );

  connect( _varioModeDlg, SIGNAL( newVarioTime( int ) ),
           calculator->getVario(), SLOT( slotNewVarioTime( int ) ) );
  connect( _varioModeDlg, SIGNAL( newTEKMode( bool ) ),
           calculator->getVario(), SLOT( slotNewTEKMode( bool ) ) );
  connect( _varioModeDlg, SIGNAL( newTEKAdjust( int ) ),
           calculator->getVario(), SLOT( slotNewTEKAdjust( int ) ) );

  connect( calculator, SIGNAL( newAirspeed( const Speed& ) ),
           calculator->getVario(), SLOT( slotNewAirspeed( const Speed& ) ) );

  connect( gps, SIGNAL( statusChange( GPSNMEA::connectedStatus ) ),
           viewMap, SLOT( slotGPSStatus( GPSNMEA::connectedStatus ) ) );
  connect( gps, SIGNAL( newSatConstellation() ),
           viewMap, SLOT( slotSatConstellation() ) );
  connect( gps, SIGNAL( statusChange( GPSNMEA::connectedStatus ) ),
           this, SLOT( slotGpsStatus( GPSNMEA::connectedStatus ) ) );
  connect( gps, SIGNAL( newSatConstellation() ),
           logger, SLOT( slotConstellation() ) );
  connect( gps, SIGNAL( newSatConstellation() ),
           calculator->getWindAnalyser(), SLOT( slot_newConstellation() ) );
  connect( gps, SIGNAL( newSpeed() ),
           calculator, SLOT( slot_Speed() ) );
  connect( gps, SIGNAL( newPosition() ),
           calculator, SLOT( slot_Position() ) );
  connect( gps, SIGNAL( newAltitude() ),
           calculator, SLOT( slot_Altitude() ) );
  connect( gps, SIGNAL( newHeading() ),
           calculator, SLOT( slot_Heading() ) );
  connect( gps, SIGNAL( newFix() ),
           calculator, SLOT( slot_newFix() ) );
  connect( gps, SIGNAL( statusChange( GPSNMEA::connectedStatus ) ),
           calculator, SLOT( slot_GpsStatus( GPSNMEA::connectedStatus ) ) );

  connect( viewWP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewWP, SIGNAL( deleteWaypoint( wayPoint* ) ),
           calculator, SLOT( slot_WaypointDelete( wayPoint* ) ) );
  connect( viewWP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewWP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewWP, SIGNAL( newHomePosition( const QPoint* ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint* ) ) );
  connect( viewWP, SIGNAL( newHomePosition( const QPoint* ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint* ) ) );

  connect( viewAF, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewAF, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewAF, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewAF, SIGNAL( newHomePosition( const QPoint* ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint* ) ) );

  connect( viewRP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewRP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewRP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );

  connect( viewTP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewTP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewTP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );

  connect( viewMap->_theMap, SIGNAL( waypointSelected( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewMap->_theMap, SIGNAL( airspaceWarning( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );
  connect( viewMap, SIGNAL( toggleLDCalculation( const bool ) ),
           calculator, SLOT( slot_toggleLDCalculation(const bool) ) );

  connect( viewInfo, SIGNAL( waypointAdded( wayPoint* ) ),
           viewWP, SLOT( slot_wpAdded( wayPoint* ) ) );
  connect( viewInfo, SIGNAL( waypointSelected( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewInfo, SIGNAL( newHomePosition( const QPoint* ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint* ) ) );

  connect( listViewTabs, SIGNAL( currentChanged( QWidget* ) ),
           this, SLOT( slot_tabChanged( QWidget* ) ) );

  connect( calculator, SIGNAL( newWaypoint( const wayPoint* ) ),
           this, SLOT( slotWaypointChanged( const wayPoint* ) ) );
  connect( calculator, SIGNAL( newWaypoint( const wayPoint* ) ),
           viewMap, SLOT( slot_Waypoint( const wayPoint* ) ) );
  connect( calculator, SIGNAL( newBearing( int ) ),
           viewMap, SLOT( slot_Bearing( int ) ) );
  connect( calculator, SIGNAL( newRelBearing( int ) ),
           viewMap, SLOT( slot_RelBearing( int ) ) );
  connect( calculator, SIGNAL( newDistance( const Distance& ) ),
           viewMap, SLOT( slot_Distance( const Distance& ) ) );
  connect( calculator, SIGNAL( newETA( const QTime& ) ),
           viewMap, SLOT( slot_ETA( const QTime& ) ) );
  connect( viewMap, SIGNAL( toggleETACalculation( const bool ) ),
           calculator, SLOT( slot_toggleETACalculation(const bool) ) );
  connect( calculator, SIGNAL( newHeading( int ) ),
           viewMap, SLOT( slot_Heading( int ) ) );
  connect( calculator, SIGNAL( newSpeed( const Speed& ) ),
           viewMap, SLOT( slot_Speed( const Speed& ) ) );
  connect( calculator, SIGNAL( newAltitude( const Altitude& ) ),
           viewMap, SLOT( slot_Altitude( const Altitude& ) ) );
  connect( calculator, SIGNAL( newPosition( const QPoint&, const int ) ),
           viewMap, SLOT( slot_Position( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( newPosition( const QPoint&, const int ) ),
           viewMap->_theMap, SLOT( slot_position( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( switchManualInFlight() ),
           viewMap->_theMap, SLOT( slot_switchManualInFlight() ) );
  connect( calculator, SIGNAL( glidePath( const Altitude& ) ),
           viewMap, SLOT( slot_GlidePath( const Altitude& ) ) );
  connect( calculator, SIGNAL( bestSpeed( const Speed& ) ),
           viewMap, SLOT( slot_bestSpeed( const Speed& ) ) );
  connect( calculator, SIGNAL( newMc( const Speed& ) ),
           viewMap, SLOT( slot_Mc( const Speed& ) ) );
  connect( calculator, SIGNAL( newVario( const Speed& ) ),
           viewMap, SLOT( slot_vario( const Speed& ) ) );
  connect( viewMap, SIGNAL( toggleVarioCalculation( const bool ) ),
           calculator, SLOT( slot_toggleVarioCalculation(const bool) ) );
  connect( calculator, SIGNAL( newWind( Vector& ) ),
           viewMap, SLOT( slot_wind( Vector& ) ) );
  connect( calculator, SIGNAL( newLD( const double&, const double&) ),
           viewMap, SLOT( slot_LD( const double&, const double&) ) );
  connect( calculator, SIGNAL( flightModeChanged( CuCalc::flightmode ) ),
           viewMap, SLOT( setFlightStatus() ) );
  connect( calculator, SIGNAL( flightModeChanged( CuCalc::flightmode ) ),
           logger, SLOT( slotFlightMode( CuCalc::flightmode ) ) );
  connect( calculator, SIGNAL( taskpointSectorTouched() ),
           logger, SLOT( slotTaskSectorTouched() ) );

  connect( calculator, SIGNAL( taskInfo( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );

  connect( ( QObject* ) calculator->getReachList(), SIGNAL( newReachList() ),
           this, SLOT( slot_newReachList() ) );

  connect( logger, SIGNAL( logging( bool ) ),
           viewMap, SLOT( setFlightStatus() ) );
  connect( logger, SIGNAL( logging( bool ) ),
           this, SLOT( slot_Logging( bool ) ) );
  connect( logger, SIGNAL( madeEntry() ),
           viewMap, SLOT( slot_LogEntry() ) );

  ws->slot_SetText1( tr( "Setting up accelerators..." ) );

  //create the keyboard accelerators
  accAfView    = new Q3Accel( this );
  accInfoView  = new Q3Accel( this );
  accManualNav = new Q3Accel( this );
  accGpsNav    = new Q3Accel( this );
  accRpView    = new Q3Accel( this );
  accTpView    = new Q3Accel( this );
  accWpView    = new Q3Accel( this );
  accMenuBar   = new Q3Accel( this );

  // most accelerators are now QActions these could also go as
  // QAction, even when not in a menu

  // WaypointListView accelerators Key_F30=0x104d
  accAfView->connectItem( accAfView->insertItem( Qt::Key_Space ),
                          viewAF, SLOT( slot_Select() ) );

  accRpView->connectItem( accRpView->insertItem( Qt::Key_Space ),
                          viewRP, SLOT( slot_Select() ) );

  accTpView->connectItem( accTpView->insertItem( Qt::Key_Space ),
                          viewTP, SLOT( slot_Select() ) );

  accWpView->connectItem( accWpView->insertItem( Qt::Key_Space ),
                          viewWP, SLOT( slot_Select() ) );

  // InfoView accelerators
  accInfoView->connectItem( accInfoView->insertItem( Qt::Key_Space ),
                            viewInfo, SLOT( slot_selectWaypoint() ) );

  accInfoView->connectItem( accInfoView->insertItem( Qt::Key_K ),
                            viewInfo, SLOT( slot_KeepOpen() ) );

  // Manual navigation accelerators. These are only available if there is no GPS connection
  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_Up ),
                             calculator, SLOT( slot_changePositionN() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_Right ),
                             calculator, SLOT( slot_changePositionE() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_Down ),
                             calculator, SLOT( slot_changePositionS() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_Left ),
                             calculator, SLOT( slot_changePositionW() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_H ),
                             calculator, SLOT( slot_changePositionHome() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_C ),
                             calculator, SLOT( slot_changePositionWp() ) );

  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_F9 ),
                             this, SLOT( slotSwitchToWPListView() ) );
  // Consider qwertz keyboards y <-> z are interchanged
  accManualNav->connectItem( accManualNav->insertItem( Qt::Key_Y ),
                             viewMap->_theMap , SLOT( slotZoomIn() ) );

  // GPS navigation accelerators. These are only available if there is a GPS connection
  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_Up ),
                          calculator, SLOT( slot_McUp() ) );

  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_Down ),
                          calculator, SLOT( slot_McDown() ) );

  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_H ),
                          this, SLOT( slotNavigateHome() ) );

  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_F9 ),
                          this, SLOT( slotSwitchToWPListView() ) );
  // Zoom in map
  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_Right ),
                          viewMap->_theMap , SLOT( slotZoomIn() ) );
  // Consider qwertz keyboards y <-> z are interchanged
  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_Y ),
                          viewMap->_theMap , SLOT( slotZoomIn() ) );
  // Zoom out map
  accGpsNav->connectItem( accGpsNav->insertItem( Qt::Key_Left ),
                          viewMap->_theMap , SLOT( slotZoomOut() ) );

  // setting of menu bar accelerators.
  accMenuBar->connectItem( accMenuBar->insertItem( Qt::Key_Space ),
                           this, SLOT( slotToggleMenu() ) ); // IPAQ rocker button

  accMenuBar->connectItem( accMenuBar->insertItem( Qt::Key_M ),
                           this, SLOT( slotToggleMenu() ) );

  accMenuBar->connectItem( accMenuBar->insertItem( Qt::Key_F12 ),
                           this, SLOT( slotToggleMenu() ) ); // F12 menu button Opie

  calculator->setPosition( _globalMapMatrix->getMapCenter( false ) );

  slot_readconfig();
  setView( mapView );

  // set the default glider to be the last one selected.
  calculator->setGlider( GliderList::getStoredSelection() );
  QString gt = calculator->gliderType();

  if ( !gt.isEmpty() ) setCaption ( "Cumulus - " + gt );

  viewMap->_theMap->setDrawing( true );

  // This actions initiates the map loading procedures
  viewMap->_theMap->slotDraw();

  ws->hide();
  ws->setScreenUsage( false );
  calculator->newSites();  // New sites have been loaded in map draw
  // this call is responsible for setting correct AGL/STD for manual mode,
  // must be called after viewMap->_theMap->draw(), there the AGL info is loaded
  // I do not connect since it is never emitted, only called once here
  calculator->slot_changePosition(MapMatrix::NotSet);
  gps->blockSignals( false );

  show();
  qDebug( "End startup cumulusapp" );
}


CumulusApp::~CumulusApp()
{
  // qDebug ("CumulusApp::~CumulusApp()");

#warning switch screensaver on, if necessary

  // @AS: Release the keyboard
  // releaseKeyboard();

#warning Question: Should we save the main window size on exit?
  // @AP: we do that later
  // save main window size
  // GeneralConfig::instance()->setWindowSize( size() );
  // GeneralConfig::instance()->save();

  delete logger;
}


void CumulusApp::initMenuBar()
{
  QFont cf = this->font();
  QFont font( "Helvetica", 12 );
  this->setFont( font );

  fileMenu = new Q3PopupMenu( this );
  fileMenu->setFont( font );
  actionFileQuit->addTo( fileMenu );

  viewMenu = new Q3PopupMenu( this );
  viewMenu->setFont( font );
  actionViewTaskpoints->addTo( viewMenu );
  actionViewTaskpoints->setEnabled( false );
  actionViewWaypoints->addTo( viewMenu );
  actionViewReachpoints->addTo( viewMenu );
  actionViewAirfields->addTo( viewMenu );
  actionViewInfo->addTo( viewMenu );
  actionViewInfo->setEnabled( false );

  viewMenu->insertSeparator();

  actionToggleStatusbar->addTo( viewMenu );
  // actionToggleMenu->addTo( viewMenu );
  actionToggleWpLabels->addTo( viewMenu );
  actionToggleWpLabelsEI->addTo( viewMenu );

  viewMenu->insertSeparator();

  actionViewGPSStatus->addTo( viewMenu );

  mapMenu = new Q3PopupMenu( this );
  mapMenu->setFont( font );
  actionToggleLogging->addTo( mapMenu );
  actionRememberWaypoint->addTo( mapMenu );
  actionSelectTask->addTo( mapMenu );
  actionToggleManualInFlight->addTo( mapMenu );
  mapMenu->insertSeparator();
  actionEnsureVisible->addTo( mapMenu );
  actionZoomInZ->addTo( mapMenu );
  actionZoomOutZ->addTo( mapMenu );

  setupMenu = new Q3PopupMenu( this );
  setupMenu->setFont( font );
  actionSetupConfig->addTo( setupMenu );
  actionPreFlight->addTo( setupMenu );
  actionSetupInFlight->addTo( setupMenu );

  helpMenu = new Q3PopupMenu( this );
  helpMenu->setFont( font );
  //  actionHelp->addTo(helpMenu);
  //  actionWhatsThis->addTo( helpMenu );
  actionHelpAboutApp->addTo( helpMenu );

  menuBar()->setFont( font );

  menuBar()->insertItem( tr( "&File" ),  fileMenu );
  menuBar()->insertItem( tr( "&View" ),  viewMenu );
  menuBar()->insertItem( tr( "&Map" ),   mapMenu );
  menuBar()->insertItem( tr( "&Setup" ), setupMenu );
  menuBar()->insertSeparator();
  menuBar()->insertItem( tr( "&Help" ),  helpMenu );
  menuBar()->hide();

  this->setFont( cf );
}


/** initializes all QActions of the application */
void CumulusApp::initActions()
{

  actionFileQuit = new Q3Action( tr( "Exit" ), tr( "E&xit" ),
                                 0, this );

  connect( actionFileQuit, SIGNAL( activated() ),
           this, SLOT( slotFileQuit() ) );

  actionViewWaypoints = new Q3Action ( tr( "Waypoints" ), tr( "&Waypoints" ),
                                       Qt::Key_W, this );

  actionViewAirfields = new Q3Action ( tr( "Airfields" ), tr( "&Airfields" ),
                                       Qt::Key_O, this );

  actionViewReachpoints = new Q3Action ( tr( "Reachable" ), tr( "&Reachable" ),
                                         Qt::Key_E, this );

  actionViewTaskpoints = new Q3Action ( tr( "Task" ), tr( "&Task" ),
                                        Qt::Key_T, this );

  connect( actionViewAirfields, SIGNAL( activated() ),
           this, SLOT( slotSwitchToAFListView() ) );
  connect( actionViewWaypoints, SIGNAL( activated() ),
           this, SLOT( slotSwitchToWPListView() ) );
  connect( actionViewReachpoints, SIGNAL( activated() ),
           this, SLOT( slotSwitchToReachListView() ) );
  connect( actionViewTaskpoints, SIGNAL( activated() ),
           this, SLOT( slotSwitchToTaskListView() ) );

  actionViewInfo = new Q3Action( tr( "Info" ), tr( "&Info" ),
                                 Qt::Key_I, this );
  connect( actionViewInfo, SIGNAL( activated() ),
           this, SLOT( slotSwitchToInfoView() ) );

  actionToggleStatusbar = new Q3Action( tr( "Statusbar" ), tr( "&Statusbar" ),
                                        0, this, 0, true );
  connect( actionToggleStatusbar, SIGNAL( toggled( bool ) ),
           this, SLOT( slotViewStatusBar( bool ) ) );

  actionToggleStatusbar->setOn( true );

  actionViewGPSStatus = new Q3Action( tr( "GPS Status" ), tr( "GPS Status" ),
                                      Qt::Key_G, this );
  connect( actionViewGPSStatus, SIGNAL( activated() ),
           this, SLOT( slotGpsStatusDialog() ) );

  actionZoomInZ = new Q3Action ( tr( "Zoom in" ), tr( "Zoom in" ),
                                 Qt::Key_Z, this );
  connect ( actionZoomInZ, SIGNAL( activated() ),
            viewMap->_theMap , SLOT( slotZoomIn() ) );
  actionZoomOutZ = new Q3Action ( tr( "Zoom out" ), tr( "Zoom out" ),
                                  Qt::Key_X, this );
  connect ( actionZoomOutZ, SIGNAL( activated() ),
            viewMap->_theMap , SLOT( slotZoomOut() ) );

  actionToggleWpLabels = new Q3Action ( tr( "Waypoint labels" ), tr( "Waypoint labels" ),
                                        Qt::Key_A, this, 0, true );
  actionToggleWpLabels->setOn( _globalMapConfig->getShowWpLabels() );
  connect( actionToggleWpLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleWpLabels( bool ) ) );

  actionToggleWpLabelsEI = new Q3Action ( tr( "Waypoint extra info" ),
                                          tr( "Waypoint extra info" ),
                                          Qt::Key_S, this, 0, true );
  actionToggleWpLabelsEI->setOn( _globalMapConfig->getShowWpLabelsExtraInfo() );
  connect( actionToggleWpLabelsEI, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleWpLabelsExtraInfo( bool ) ) );

  actionToggleLogging = new Q3Action( tr( "IGC Logging" ), tr( "Logging" ),
                                      Qt::Key_L, this, 0, true );
  connect ( actionToggleLogging, SIGNAL( activated() ),
            logger, SLOT( slotToggleLogging() ) );

  actionEnsureVisible = new Q3Action ( tr( "Ensure waypoint visible" ),
                                       tr( "Ensure waypoint visible" ),
                                       Qt::Key_V, this );
  connect ( actionEnsureVisible, SIGNAL( activated() ),
            this, SLOT( slotEnsureVisible() ) );

  actionSelectTask = new Q3Action( tr( "Select task" ), tr( "Select task" ),
                                   Qt::Key_T + Qt::SHIFT, this );
  connect ( actionSelectTask, SIGNAL( activated() ),
            this, SLOT( slotPreFlightTask() ) );

  actionToggleManualInFlight = new Q3Action( tr( "Manual" ), tr( "Manual" ),
                               Qt::Key_M + Qt::SHIFT, this, 0, true );
  actionToggleManualInFlight->setEnabled(false);
  connect( actionToggleManualInFlight, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleManualInFlight( bool ) ) );

  actionPreFlight = new Q3Action( tr( "Pre Flight" ), tr( "Pre Flight" ),
                                  Qt::Key_P, this );
  connect ( actionPreFlight, SIGNAL( activated() ),
            this, SLOT( slotPreFlightGlider() ) );

  actionRememberWaypoint = new Q3Action( tr( "Remember waypoint" ),
                                         tr( "Remember waypoint" ),
                                         Qt::Key_R, this );
  connect( actionRememberWaypoint, SIGNAL( activated() ),
           this, SLOT( slotRememberWaypoint() ) );

  actionSetupConfig = new Q3Action( tr( "General Setup" ), tr ( "General &Setup" ),
                                    Qt::Key_S + Qt::SHIFT, this );
  connect ( actionSetupConfig, SIGNAL( activated() ),
            this, SLOT( slotConfig() ) );
  actionSetupInFlight = new Q3Action( tr( "In Flight" ), tr ( "In &Flight" ),
                                      Qt::Key_F, this );
  connect ( actionSetupInFlight, SIGNAL( activated() ),
            this, SLOT( slotGliderFlight() ) );

  //  actionHelp = new Q3Action (tr("Help"), tr("&Help"), Qt::Key_H, this);
  //  connect(actionHelp, SIGNAL(activated()),this,SLOT(slotHelp()));
  // actionWhatsThis = new Q3Action( tr( "What's this ?" ), tr( "What's this ?" ), 0, this );
  // connect ( actionWhatsThis, SIGNAL( activated() ), this, SLOT( whatsThis() ) );

  actionHelpAboutApp = new Q3Action( tr( "About Cumulus" ), tr( "About Cumulus" ),
                                     Qt::Key_V, this );
  connect( actionHelpAboutApp, SIGNAL( activated() ),
           this, SLOT( slotVersion() ) );
}

/**
 * Toggle on/off all actions, which have key accelerators defined.
 */

void  CumulusApp::toggelActions( const bool toggle )
{


  actionViewWaypoints->setEnabled( toggle );
  actionViewAirfields->setEnabled( toggle );

  if( toggle )
    {
      GeneralConfig * conf = GeneralConfig::instance();
      actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );
    }
  else
    {
      actionViewReachpoints->setEnabled( toggle );
    }

  actionViewTaskpoints->setEnabled( toggle );
  actionViewInfo->setEnabled( toggle );
  actionViewGPSStatus->setEnabled( toggle );
  actionZoomInZ->setEnabled( toggle );
  actionZoomOutZ->setEnabled( toggle );
  actionToggleWpLabels->setEnabled( toggle );
  actionToggleWpLabelsEI->setEnabled( toggle );
  actionEnsureVisible->setEnabled( toggle );
  actionSelectTask->setEnabled( toggle );
  actionPreFlight->setEnabled( toggle );
  actionRememberWaypoint->setEnabled( toggle );
  actionSetupConfig->setEnabled( toggle );
  actionSetupInFlight->setEnabled( toggle );
  actionHelpAboutApp->setEnabled( toggle );
  actionToggleLogging->setEnabled( toggle );
  // do not toggle actionToggleManualInFlight, status may not be changed
}


void CumulusApp::slotFileQuit ()
{
  close();
}


/**
 * Make sure the user really wants to quit
 */
void CumulusApp::closeEvent ( QCloseEvent* evt )
{
  // @AP: All close events will be ignored, if we are not in the map
  // view to avoid any possibility of confusion with the two close
  // buttons.
  if( view != mapView )
    {
      evt->ignore();
      return;
    }

  playSound();

  QMessageBox mb( tr( "Are you sure?" ),
                  tr( "<qt><b>Cumulus will be terminated.<br>Are you sure?</b></qt>" ),
                  QMessageBox::Warning,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No,
                  QMessageBox::NoButton );

  QFont fnt( "Helvetica", 12, QFont::Bold );
  mb.setFont( fnt );

  switch ( mb.exec() )
    {
    case QMessageBox::Yes:
      // save and exit
      evt->accept();
      break;
    case QMessageBox::No:
      // exit without saving
      evt->ignore();
      break;
    case QMessageBox::Cancel:
      // don't save and don't exit
      evt->ignore();
      break;
    }
}


void CumulusApp::slotToggleMenu()
{
  if ( !menuBar()->isVisible() )
    {
      menuBarVisible = true;
      menuBar()->show();
    }
  else
    {
      menuBarVisible = false;
      menuBar()->hide();
    }
}


void CumulusApp::slotToggleWpLabels( bool toggle )
{
  _globalMapConfig->setShowWpLabels( toggle );
  viewMap->_theMap->quickDraw();
}


void CumulusApp::slotToggleWpLabelsExtraInfo( bool toggle )
{
  _globalMapConfig->setShowWpLabelsExtraInfo( toggle );
  viewMap->_theMap->quickDraw();
}


void CumulusApp::slotViewStatusBar( bool toggle )
{
  if ( toggle )
    viewMap->statusBar() ->show();
  else
    viewMap->statusBar() ->hide();
}


/** Called if the logging is actually toggled */
void CumulusApp::slot_Logging ( bool logging )
{
  actionToggleLogging->blockSignals ( true );
  actionToggleLogging->setOn ( logging );
  actionToggleLogging->blockSignals ( false );
}



/** Read property of enum view. */
const CumulusApp::appView CumulusApp::getView()
{
  return view;
}


/** Called if the user clicks a tab with a list-type view */
void CumulusApp::slot_tabChanged( QWidget * w )
{
  //switch to the correct view
  if ( w == viewWP )
    {
      setView( wpView );
    }
  else if ( w == viewTP )
    {
      setView( tpView );
    }
  else if ( w == viewRP )
    {
      setView( rpView );
    }
  else if ( w == viewAF )
    {
      setView( afView );
    }
}


/** Write property of internal view. */
void CumulusApp::setView( const appView& newVal, const wayPoint* wp )
{
  // qDebug("CumulusApp::setView called with argument %d", newVal);

  switch ( newVal )
    {

    case mapView:

      // @AP: set focus to cumulus widget, otherwise F-Key events will
      // not routed to it
      setFocus();

      // @AP: We display the menu bar only in the map view widget,
      // if it is visible. In all other views we hide it to save
      // space for the other widgets.
      if( menuBarVisible )
        {
          menuBar()->show();
        }
      else
        {
          menuBar()->hide();
        }

      fileMenu->setEnabled( true );
      mapMenu->setEnabled( true );
      viewMenu->setEnabled( true );
      setupMenu->setEnabled( true );
      helpMenu->setEnabled( true );

      listViewTabs->hide();
      viewInfo->hide();
      viewMap->show();

      accAfView->setEnabled( false );
      accRpView->setEnabled( false );
      accTpView->setEnabled( false );
      accWpView->setEnabled( false );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( !gps->getConnected() || calculator->isManualInFlight());
      accGpsNav->setEnabled( gps->getConnected() && !calculator->isManualInFlight() );
      accMenuBar->setEnabled( true );

      // Switch on all action accelerators in this view
      toggelActions( true );
      viewMap->statusBar()->clear(); // remove temporary statusbar messages

      break;

    case wpView:
      qDebug("wpView");
      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      listViewTabs->showPage( viewWP );
      listViewTabs->show();

      accAfView->setEnabled( false );
      accWpView->setEnabled( true );
      accRpView->setEnabled( false );
      accTpView->setEnabled( false );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    case rpView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      listViewTabs->showPage( viewRP );
      listViewTabs->show();

      accAfView->setEnabled( false );
      accRpView->setEnabled( true );
      accWpView->setEnabled( false );
      accTpView->setEnabled( false );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    case afView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      viewAF->fillWpList();
      listViewTabs->showPage( viewAF );
      listViewTabs->show();

      accAfView->setEnabled( true );
      accRpView->setEnabled( false );
      accWpView->setEnabled( false );
      accTpView->setEnabled( false );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    case tpView:

      // only allow switching to this view if there is anything to see
      if ( _globalMapContents->getCurrentTask() == 0 )
        {
          return;
        }

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      viewAF->fillWpList();
      listViewTabs->showPage( viewTP );
      listViewTabs->show();

      accAfView->setEnabled( false );
      accRpView->setEnabled( false );
      accWpView->setEnabled( false );
      accTpView->setEnabled( true );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    case infoView:

      qDebug("infoView");

      if ( ! wp )
        {
          return;
        }

      menuBar()->hide();
      viewMap->hide();
      listViewTabs->hide();
      viewInfo->showWP( view, wp );

      accAfView->setEnabled( false );
      accRpView->setEnabled( false );
      accTpView->setEnabled( false );
      accWpView->setEnabled( false );
      accInfoView->setEnabled( true );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    case tpSwitchView:

      menuBar()->hide();
      viewMap->hide();
      listViewTabs->hide();

      accAfView->setEnabled( false );
      accRpView->setEnabled( false );
      accTpView->setEnabled( false );
      accWpView->setEnabled( false );
      accInfoView->setEnabled( false );
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( false );
      accMenuBar->setEnabled( false );
      toggelActions( false );

      break;

    default:
      // @AP: Should normally not happen but Vorsicht ist die Mutter
      // der Porzellankiste ;-)
      qWarning( "CumulusApp::setView(): unknown view %d to be set", newVal );
      return;
    }

  // save new view value
  view = newVal;
}


/** Switches to mapview. */
void CumulusApp::slotSwitchToMapView()
{
  setView( mapView );
}


/** Switches to the WaypointList View */
void CumulusApp::slotSwitchToWPListView()
{
  setView( wpView );
}


/** Switches to the WaypointList View if there is
 * no task, and to the task list if there is .
 */
void CumulusApp::slotSwitchToWPListViewExt()
{
  if ( _globalMapContents->getCurrentTask() )
    {
      setView( tpView );
    }
  else
    {
      setView( wpView );
    }
}


/** Switches to the AirfieldList View */
void CumulusApp::slotSwitchToAFListView()
{
  setView( afView );
}


/** Switches to the ReachablePointList View */
void CumulusApp::slotSwitchToReachListView()
{
  setView( rpView );
}


/** Switches to the WaypointList View */
void CumulusApp::slotSwitchToTaskListView()
{
  setView( tpView );
}


/** This slot is called to switch to the info view. */
void CumulusApp::slotSwitchToInfoView()
{
  qDebug("CumulusApp::slotSwitchToInfoView()");
  if ( view == wpView )
    {
      setView( infoView, viewWP->getSelectedWaypoint() );
    }
  if ( view == rpView )
    {
      setView( infoView, viewRP->getSelectedWaypoint() );
    }
  if ( view == afView )
    {
      setView( infoView, viewAF->getSelectedAirfield() );
    }
  if ( view == tpView )
    {
      setView( infoView, viewTP->getSelectedWaypoint() );
    }
  else
    {
      setView( infoView, calculator->getselectedWp() );
    }
}


/** @ee This slot is called to switch to the info view with selected waypoint. */
void CumulusApp::slotSwitchToInfoView( wayPoint* wp )
{
  if( wp )
    {
      setView( infoView, wp );
    }
}


/** Opens the configdialog. */
void CumulusApp::slotConfig()
{
  ConfigDialog * dlg = new ConfigDialog ( this );
  connect( dlg, SIGNAL( settingsChanged() ),
           this, SLOT( slot_readconfig() ) );
  connect( dlg,  SIGNAL( welt2000ConfigChanged() ),
           _globalMapContents, SLOT( slotReloadWelt2000Data() ) );

  dlg->show();
}


/** Opens the inflight glider settings dialog. */
void CumulusApp::slotGliderFlight()
{
  if ( _gliderFlightDlg == NULL )
    _gliderFlightDlg = new GliderFlightDialog ( this );
  connect( _gliderFlightDlg, SIGNAL( settingsChanged() ),
           calculator, SLOT( slot_settingschanged() ) );
  _gliderFlightDlg->load();
  _gliderFlightDlg->show();
}


void CumulusApp::slotAltimeterMode()
{
  _altimeterModeDlg->work();
  // qDebug("CumulusApp::slotAltimeterMode()");
}


void CumulusApp::slotNewAltimeterMode()
{
  viewMap->getAltitudeWidget() ->setPreText( _altimeterModeDlg->Pretext() );
  // qDebug("CumulusApp::slotNewAltimeterMode()");
}


void CumulusApp::slotVarioMode()
{
  _varioModeDlg->load();
  // qDebug("slotVarioMode");
}


/** Shows version and copyright information */
void CumulusApp::slotVersion()
{
  QMessageBox::about ( this,
                       "Cumulus",
                       QString(tr(
                                 "<qt>"
                                 "<table cellspacing=0 cellpadding=0 border=0>"
                                 "<tr>"
                                 "<th>Cumulus X11 version %1</th>"
                                 "</tr>"
                                 "<tr>"
                                 "<td><font size=-1>(compiled at %2 with QT %3)</font></td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>&copy; 2002-2008 by</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>Andr&eacute; Somers, Eggert Ehmke</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>Axel Pauli, Eckhard V&ouml;llm</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>Hendrik M&uuml;ller, Florian Ehinger</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>Michael Enke, Heiner Lamprecht</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>"
                                 "<a href=\"http://www.kflog.org/cumulus\">http://www.kflog.org/cumulus</a><br/>"
                                 "</td>"
                                 "</tr>"
                                 "<tr>"
                                 "<td>Published under the GPL</td>"
                                 "</tr>"
                                 "</table>"
                                 "</qt>" ).arg( QString(CU_VERSION) ).arg( QString( __DATE__ )).arg( QString(QT_VERSION_STR) ) ) );
}

void CumulusApp::slotWaypointChanged( const wayPoint *newWp )
{
  // qDebug("CumulusApp::slotWaypointChanged() is called" );

  if( newWp != 0  )
    {
      actionViewInfo->setEnabled( true );
      actionViewInfo->setAccel( Qt::Key_I ); // re-enable acceleraror key
    }
  else
    {
      actionViewInfo->setEnabled( false );
      actionViewInfo->setAccel( 0 ); // disenable acceleraror key
    }
}


void CumulusApp::slotRememberWaypoint()
{
  static uint count = 1;
  QString name;

  name = tr( "WP%1 %2" ).arg( count ).arg( QTime::currentTime().toString().left( 5 ) );

  // @AP: let us check, if user waypoint is already known from its
  // position. In this case we will reject the insertion to avoid senseless
  // duplicates.

  QPoint pos = calculator->getlastPosition();
  Q3PtrList<wayPoint>* wpList = _globalMapContents->getWaypointList();

  for ( uint i = 0; i < wpList->count(); i++ )
    {
      wayPoint *wpElem = wpList->at( i );

      if ( wpElem->origP == pos )
        {
          return ; // we have it already
        }
    }

  count++;
  wayPoint* wp = new wayPoint;
  wp->name = name;
  wp->origP = calculator->getlastPosition();
  wp->projP = _globalMapMatrix->wgsToMap( wp->origP );
  wp->description = tr( "created by user" );
  wp->importance = wayPoint::High; // high to make sure it is visible
  wp->frequency = 0.0;
  wp->runway = 0;
  wp->length = 0;
  AltitudeCollection alt = calculator->getAltitudeCollection();
  wp->elevation = int ( ( alt.gpsAltitude - alt.gndAltitude ).getMeters() );
  wp->type = BaseMapElement::Turnpoint;
  wp->isLandable = false;

  viewWP->slot_wpAdded( wp );
}


/** This slot is called if the configuration has changed and at the
    start of the program to read the initial configuration. */
void CumulusApp::slot_readconfig()
{
  // @AP: WARNING POPUP crash test statement, please let it commented out here
  // viewMap->theMap->checkAirspace(calculator->getlastPosition());

  GeneralConfig * conf = GeneralConfig::instance();

  // configure units
  Distance::setUnit( Distance::distanceUnit( conf->getUnitDist() ) );
  Speed::setHorizontalUnit( Speed::speedUnit( conf->getUnitSpeed() ) );
  Speed::setVerticalUnit( Speed::speedUnit( conf->getUnitVario() ) );
  Altitude::setUnit( Altitude::altitude( conf->getUnitAlt() ) );
  WGSPoint::setFormat( WGSPoint::Format( conf->getUnitPos() ) );

  // other config changes
  viewMap->slot_settingschange();
  calculator->slot_settingschanged();
  _globalMapConfig->slotReadConfig();
  viewTP->slot_updateTask();

  // configure reconnect of GPS receiver in case of process stop
  QString device = conf->getGpsDevice();

  if( device.startsWith("/dev/" ) )
    {
      // @ee install signal handler
      signal ( SIGCONT, resumeGpsConnection );
    }

  GPSNMEA::DeliveredAltitude altRef = ( GPSNMEA::DeliveredAltitude ) conf->getGpsAltitude();
  gps->setDeliveredAltitude( altRef );
  gps->setDeliveredUserAltitude( conf->getGpsUserAltitudeCorrection() );
  gps->slot_reset();

  if(1)
    {
      // 1 will be replaced in future with a construct like
      // conf->getNearestSiteCalculatorSwitch() != lastConf->getNearestSiteCalculatorSwitch()
      actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );
      if(conf->getNearestSiteCalculatorSwitch())
        {
          if(!_reachpointListVisible)
            {
              listViewTabs->insertTab( viewRP, tr( "Reachable" ), _taskListVisible ? 2 : 1 );
              calculator->newSites();
              _reachpointListVisible = true;
            }
        }
      else
        {
          if(_reachpointListVisible)
            {
              // changes in listViewTabs trigger slot_tabChanged (if viewRP was last active),
              // this slot calls setView and tries to set the view to viewRP
              // but since this doesn't exist (removePage), sets the view to the next one
              // which is viewAF; that's the reason we have to a) call setView(mapView);
              // or b) disconnect before removePage and connect again behind
              disconnect( listViewTabs, SIGNAL( currentChanged( QWidget* ) ),
                          this, SLOT( slot_tabChanged( QWidget* ) ) );
              listViewTabs->removePage( viewRP );
              connect( listViewTabs, SIGNAL( currentChanged( QWidget* ) ),
                       this, SLOT( slot_tabChanged( QWidget* ) ) );
              calculator->clearReachable();
              viewRP->fillRpList();   // this clears the listView
              viewMap->_theMap->quickDraw();
              _reachpointListVisible = false;
            }
        }
    }
}


/** Called if the status of the GPS changes, and controls the availability of manual navigation. */
void CumulusApp::slotGpsStatus( GPSNMEA::connectedStatus status )
{
  switch ( status )
    {
    case GPSNMEA::validFix:
      viewMap->message( tr( "GPS fix established" ) );
      break;
    case GPSNMEA::noFix:
      viewMap->message(tr( "GPS connection - no fix" ) );
      break;
    default:
      viewMap->message( tr( "GPS lost" ) );
    }

  playSound();

  if ( ( status < GPSNMEA::validFix || calculator->isManualInFlight()) && ( view == mapView ) )
    {  // no GPS data
      accManualNav->setEnabled( true );
      accGpsNav->setEnabled( false );
    }
  else
    {  // GPS data valid
      accManualNav->setEnabled( false );
      accGpsNav->setEnabled( true );
    }
}


/** This slot is called if the user presses C in manual navigation mode. It centers
  * the map on the current waypoint. */
void CumulusApp::slotCenterToWaypoint()
{

  if ( calculator->getselectedWp() )
    {
      _globalMapMatrix->centerToLatLon( calculator->getselectedWp() ->origP );
      viewMap->_theMap->sceduleRedraw();

    }
}


/** Called if the user pressed V in mapview.
  * Adjusts the zoomfactor so that the currently selected waypoint
  * is displayed as good as possible. */
void CumulusApp::slotEnsureVisible()
{
  if ( calculator->getselectedWp() )
    {
      double newScale = _globalMapMatrix->ensureVisible( calculator->getselectedWp() ->origP );
      if ( newScale > 0 )
        {
          viewMap->_theMap->slotSetScale( newScale );
        }
      else
        {
          viewMap->message( tr( "Waypoint out of map range." ) );
        }
    }
}


/** Opens the preflight dialog and brings the selected tabulator in foreground */
void CumulusApp::slotPreFlightGlider()
{
  slotPreFlight("gliderselection");
}


/** Opens the preflight dialog and brings the selected tabulator in foreground */
void CumulusApp::slotPreFlightTask()
{
  slotPreFlight("taskselection");
}


/** Opens the preflight dialog and brings the selected tabulator in foreground */
void CumulusApp::slotPreFlight(const char *tabName)
{
  _preFlightDialog = new PreFlightDialog( this, tabName );

  connect( _preFlightDialog, SIGNAL( settingsChanged() ),
           this, SLOT( slotPreFlightDataChanged() ) );

  connect( _preFlightDialog, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );

  _preFlightDialog->show();
}


void CumulusApp::slotPreFlightDataChanged()
{
  // qDebug("CumulusApp::slotPreFlightDataChanged()");
  setCaption ( "Cumulus - " + calculator->gliderType() );

  if ( _globalMapContents->getCurrentTask() == 0 )
    {
      if ( _taskListVisible )
        {
          // see comment for removePage( viewRP )
          disconnect( listViewTabs, SIGNAL( currentChanged( QWidget* ) ),
                      this, SLOT( slot_tabChanged( QWidget* ) ) );
          listViewTabs->removePage( viewTP );
          connect( listViewTabs, SIGNAL( currentChanged( QWidget* ) ),
                   this, SLOT( slot_tabChanged( QWidget* ) ) );
          actionViewTaskpoints->setEnabled( false );
          _taskListVisible = false;
        }
    }
  else
    {
      if ( !_taskListVisible )
        {
          listViewTabs->insertTab( viewTP, tr( "Task" ), 0 );
          _taskListVisible = true;
          actionViewTaskpoints->setEnabled( true );
        }
    }

  // set the task list view at the current task
  viewTP->slot_setTask( _globalMapContents->getCurrentTask() );

  // quickDraw is enough, no need for sceduleRedraw() since
  // task line is on top, airspace and below structures not changed in preflight dlg
  viewMap->_theMap->quickDraw();
}

/** Opens the gps status dialog */
void CumulusApp::slotGpsStatusDialog()
{
  GpsStatusDialog *gpsd = new GpsStatusDialog( this );
  gpsd->show();
}


/** dynamicly updates view for reachable list */
void CumulusApp::slot_newReachList()
{
  viewRP->slot_newList(); //let the view know we have a new list
  viewMap->_theMap->quickDraw();
}


bool CumulusApp::eventFilter( QObject *o , QEvent *e )
{
  // qDebug("CumulusApp::eventFilter() is called with event type %d", e->type());

  if ( e->type() == QEvent::KeyPress )
    {
      QKeyEvent *k = ( QKeyEvent* ) e;
      qDebug( "Keycode of pressed key: %d, %%%X", k->key(), k->key() );

      if( k->key() == Qt::Key_Space )
        {
          slotToggleMenu();
        }
    }

  return QWidget::eventFilter( o, e ); // standard event processing;
}


void CumulusApp::slotNavigateHome()
{
  calculator->slot_WaypointChange( GeneralConfig::instance()->getHomeWp(), true );
}

void CumulusApp::slotToggleManualInFlight(bool on)
{
  // if we have lost the GPS fix, actionToggleManualInFlight is disabled from calculator
  // so we only can switch off if GPS fix available
  calculator->setManualInFlight(on);
  accManualNav->setEnabled( on );
  accGpsNav->setEnabled( !on );
}

/** Used to allow or disable user keys processing during map drawing. */
void CumulusApp::slotMapDrawEvent( bool drawEvent )
{
   if( drawEvent )
     {
      // Disable menu accelerator during drawing to avoid
      // event avalanche, if the user holds the key down longer.
      accMenuBar->setEnabled( false );
      
      if( view == mapView )
       {
         accManualNav->setEnabled( false );
         accGpsNav->setEnabled( false );
       }
     }
   else
     {
       accMenuBar->setEnabled( true );

       if( view == mapView )
         {
           accManualNav->setEnabled( !gps->getConnected() || calculator->isManualInFlight());
           accGpsNav->setEnabled( gps->getConnected() && !calculator->isManualInFlight() );
         }
     }
}


// resize the list view tabs, if requested
void CumulusApp::resizeEvent(QResizeEvent* event)
{
  qDebug("CumulusApp::resizeEvent(): w=%d, h=%d", event->size().width(), event->size().height() );
  // resize list view tabs, if current widget was modified

  if( listViewTabs )
    {
      listViewTabs->resize( event->size() );
    }
}

