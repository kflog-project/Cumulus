/***************************************************************************
 cumulusapp.cpp  -  main application class
                          -------------------
 begin                : Sun Jul 21 2002
 copyright            : (C) 2002 by Andre Somers
 ported to Qt4.x/X11  : (C) 2008 by Axel pauli
 email                : axel@kflog.org

  This file is distributed under the terms of the General Public
  Licence. See the file COPYING for more information.

  $Id$

****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <QTextCodec>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QList>
#include <QMessageBox>

#ifdef MAEMO
#include <QInputContext>
#include <QInputContextFactory>
#include <QtDebug>
#endif

#include "generalconfig.h"
#include "cumulusapp.h"
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
#include "target.h"
#include "helpbrowser.h"
#include "sound.h"

#ifdef MAEMO

#include <libosso.h>

// @AP: That is a little hack, to avoid the include of all glib
// functionality in the header file of this class. There are
// redefinitions of macros in glib and other cumulus header files.
static osso_context_t *ossoContext = static_cast<osso_context_t *> (0);

#endif

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

CumulusApp::CumulusApp( QMainWindow *parent, Qt::WindowFlags flags ) :
  QMainWindow( parent, flags )
{
  _globalCumulusApp = this;
  menuBarVisible = false;
  listViewTabs = 0;
  configView = 0;

  // Eggert: make sure the app uses utf8 encoding for translated widgets
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );

#warning FIXME should we have a menu entry for the application font size?
  // Check the font size and set it bigger if it was to small
  QFont appFt = QApplication::font();

//  qDebug("QAppFont family %s, pointSize=%d pixelSize=%d",
//         appFt.family().toLatin1().data(), appFt.pointSize(), appFt.pixelSize() );

#ifdef MAEMO

  // activate Hildon Input Method for Maemo
  QInputContext *hildonInputContext = 0;
  
  QStringList inputMethods = QInputContextFactory::keys();
  
  foreach( QString inputMethod, inputMethods )
  {
    qDebug() << "InputMethod: " << inputMethod;
    
    if ( inputMethod == "hildon-input-method" )
    {
      hildonInputContext = QInputContextFactory::create( "hildon-input-method", 0 );
      break;
    }
  }
  
  if ( !hildonInputContext )
    {
      qWarning( "QHildonInputMethod plugin not loadable!" );
    }
  else
    {
      // app.setInputContext(hildonInputContext);
    }

  // For MAEMO it's really better to pre-set style and font
  // To resize tiny buttons (does not work everywhere though)
  QApplication::setGlobalStrut( QSize(24,16) );

  // N8x0 display has bad contrast for light shades, so make the (dialog)
  // background darker
  QPalette appPal = QApplication::palette();
  appPal.setColor(QPalette::Normal,QPalette::Window,QColor(236,236,236));
  appPal.setColor(QPalette::Normal,QPalette::Button,QColor(216,216,216));
  appPal.setColor(QPalette::Normal,QPalette::Base,Qt::white);
  appPal.setColor(QPalette::Normal,QPalette::AlternateBase,QColor(246,246,246));
  appPal.setColor(QPalette::Normal,QPalette::Highlight,Qt::darkBlue);
  QApplication::setPalette(appPal);

  // The Nokia font has excellent readability and less width than others
  appFt.setFamily("Nokia Sans");

#endif

  // CleanLooks is best for tabs and edit fields but bad for
  // spin buttons. I've found no ideal solution. Try out.
  // Well, CleanLooks has bugs. So much about that. Back to Plastique
  QApplication::setStyle( GeneralConfig::instance()->getGuiStyle() );

  appFt.setPointSize( GeneralConfig::instance()->getGuiFontSize() );
  QApplication::setFont(appFt);

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
  qDebug( "QDir::homePath()=%s", QDir::homePath().toLatin1().data() );
  qDebug( "DISPLAY=%s", qwsdisplay ? qwsdisplay : "NULL" );

  // Check, if in users home a cumulus application directory exists,
  // otherwise create it.
  QDir cuApps( QDir::homePath() + "/cumulus" );

  if ( ! cuApps.exists() )
    {
      cuApps.mkdir( QDir::homePath() + "/cumulus" );
    }

  setFocusPolicy( Qt::StrongFocus );
  setFocus();

  //grabKeyboard(); // @AP: make problems on Qt4.3/X11

  this->installEventFilter( this );

  setWindowIcon( QIcon(GeneralConfig::instance()->loadPixmap("cumulus.png")) );
  setWindowTitle( "Cumulus" );

#ifdef MAEMO
  setWindowState(Qt::WindowFullScreen);
#endif

  show();
  
  ws = new WaitScreen(this);
  ws->show();

  // Here we finished the base initialization and start a timer
  // to continue startup in another method. This is done, to get
  // running the window manager event loop. Otherwise the behaviour
  // of some widgets is undefined.
  
  // when the timer expires the cumulus startup is continued
  QTimer::singleShot(100, this, SLOT(slotCreateApplicationWidgets()));
}

/** creates the application widgets after the base initialization
 *  of the core application window.
*/
void CumulusApp::slotCreateApplicationWidgets()
{
  // qDebug( "CumulusApp::slotCreateApplicationWidgets()" );

#ifdef MAEMO

  ossoContext = osso_initialize( "cumulus", CU_VERSION, false, 0 );

  if( ! ossoContext )
    {
      qWarning("Could not initialize Osso Library");
    }
  else
    {
      // prevent screen blanking
      osso_display_blanking_pause( ossoContext );
    }

#endif

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

  ws->slot_SetText1( tr( "Creating views..." ) );

  view = mapView;

  qDebug( "Main window size is %dx%d, width=%d, height=%d",
          size().width(),
          size().height(),
          size().width(),
          size().height() );

  // This is the main widget of cumulus
  viewMap = new MapView( this );
  _globalMapView = viewMap;

  setCentralWidget( viewMap );

#ifndef MAEMO
  QFont fnt( "Helvetica", 14, QFont::Bold );
#else
  QFont fnt = font();
#endif

  fnt.setBold(true);

  listViewTabs = new QTabWidget( this );
  listViewTabs->setObjectName("listViewTabs");
  listViewTabs->resize( this->size() );
  listViewTabs->setFont( fnt );

  viewWP = new WaypointListView( this );
  viewAF = new AirfieldListView( this );
  viewRP = new ReachpointListView( this );
  viewTP = new TaskListView( this );
  viewWP->setFont( fnt );
  viewAF->setFont( fnt );
  viewRP->setFont( fnt );
  viewTP->setFont( fnt );

  viewCF = new QWidget( this );

  _taskListVisible = false;
  _reachpointListVisible = false;
  listViewTabs->addTab( viewWP, tr( "Waypoints" ) );
  //listViewTabs->addTab( viewRP, tr( "Reachable" ) ); --> added in slotReadconfig
  listViewTabs->addTab( viewAF, tr( "Airfields" ) );

  // waypoint info widget
  viewInfo = new WPInfoWidget( this );

  viewWP->listWidget()->fillWpList();

  //create global objects
  gps = new GPSNMEA( this );
  gps->blockSignals( true );
  logger = IgcLogger::instance();

  initActions();
  initMenuBar();

  ws->slot_SetText1( tr( "Setting up connections..." ) );

  // create connections between the components
  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           _globalMapContents, SLOT( slotReloadMapData() ) );

  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewMap->_theMap, SLOT( slotDraw() ) );

  connect( viewMap->_theMap, SIGNAL( isRedrawing( bool ) ),
           this, SLOT( slotMapDrawEvent( bool ) ) );

  connect( calculator, SIGNAL( newAirspeed( const Speed& ) ),
           calculator->getVario(), SLOT( slotNewAirspeed( const Speed& ) ) );

  connect( gps, SIGNAL( statusChange( GPSNMEA::connectedStatus ) ),
           viewMap, SLOT( slot_GPSStatus( GPSNMEA::connectedStatus ) ) );
  connect( gps, SIGNAL( newSatConstellation() ),
           viewMap, SLOT( slot_SatConstellation() ) );
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
           this, SLOT( slotAlarm( const QString&, const bool ) ) );
  connect( viewMap, SIGNAL( toggleLDCalculation( const bool ) ),
           calculator, SLOT( slot_toggleLDCalculation(const bool) ) );

  connect( viewInfo, SIGNAL( waypointAdded( wayPoint* ) ),
           viewWP, SLOT( slot_wpAdded( wayPoint* ) ) );
  connect( viewInfo, SIGNAL( waypointSelected( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewInfo, SIGNAL( newHomePosition( const QPoint* ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint* ) ) );

  connect( listViewTabs, SIGNAL( currentChanged( int ) ),
           this, SLOT( slot_tabChanged( int ) ) );

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
           viewMap->_theMap, SLOT( slotPosition( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( switchManualInFlight() ),
           viewMap->_theMap, SLOT( slotSwitchManualInFlight() ) );
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
  connect( calculator, SIGNAL( newGlider( const QString&) ),
           viewMap, SLOT( slot_glider( const QString&) ) );
  connect( calculator, SIGNAL( flightModeChanged( CuCalc::flightmode ) ),
           viewMap, SLOT( slot_setFlightStatus() ) );
  connect( calculator, SIGNAL( flightModeChanged( CuCalc::flightmode ) ),
           logger, SLOT( slotFlightMode( CuCalc::flightmode ) ) );
  connect( calculator, SIGNAL( taskpointSectorTouched() ),
           logger, SLOT( slotTaskSectorTouched() ) );
  connect( calculator, SIGNAL( taskInfo( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );

  connect( ( QObject* ) calculator->getReachList(), SIGNAL( newReachList() ),
           this, SLOT( slot_newReachList() ) );

  connect( logger, SIGNAL( logging( bool ) ),
           viewMap, SLOT( slot_setFlightStatus() ) );
  connect( logger, SIGNAL( logging( bool ) ),
           this, SLOT( slot_Logging( bool ) ) );
  connect( logger, SIGNAL( madeEntry() ),
           viewMap, SLOT( slot_LogEntry() ) );

  calculator->setPosition( _globalMapMatrix->getMapCenter( false ) );

  slotReadconfig();
  setView( mapView );

  // set the default glider to be the last one selected.
  calculator->setGlider( GliderListWidget::getStoredSelection() );
  QString gt = calculator->gliderType();

  if ( !gt.isEmpty() )
    {
      setWindowTitle ( "Cumulus - " + gt );
    }

  viewMap->_theMap->setDrawing( true );

  // This actions initiates the map loading procedures
  viewMap->_theMap->slotDraw();

  calculator->newSites();  // New sites have been loaded in map draw
  // this call is responsible for setting correct AGL/STD for manual mode,
  // must be called after viewMap->_theMap->draw(), there the AGL info is loaded
  // I do not connect since it is never emitted, only called once here
  calculator->slot_changePosition(MapMatrix::NotSet);

  ws->setScreenUsage( false );
  ws->hide();

  if( ! GeneralConfig::instance()->getAirspaceWarningEnabled() )
    {
      int answer= QMessageBox::warning( this,tr("Airspace Warnings"),
                                       tr("<html><b>Airspace warnings are disabled!<br>"
                                           "Enable now?</b></html>"),
                                       QMessageBox::Yes | QMessageBox::No );

      if (answer==QMessageBox::Yes)
        {
          GeneralConfig::instance()->setAirspaceWarningEnabled(true);
        }
    }

#ifdef MAEMO

  if( ossoContext )
    {
      osso_display_blanking_pause( ossoContext );

      // setup timer to prevent screen blank
      ossoDisplayTrigger = new QTimer(this);
      ossoDisplayTrigger->setSingleShot(true);

      connect( ossoDisplayTrigger, SIGNAL(timeout()),
               this, SLOT(slot_ossoDisplayTrigger()) );

      // start timer with 10s
      ossoDisplayTrigger->start( 10000 );
    }

#endif

  // Startup GPS client process now for data receiving
  gps->blockSignals( false );
  gps->startGpsReceiver();

  qDebug( "End startup cumulusapp" );
}

CumulusApp::~CumulusApp()
{
  // qDebug ("CumulusApp::~CumulusApp()");

#warning Question: Should we save the main window size on exit?
  // @AP: we do that later
  // save main window size
  // GeneralConfig::instance()->setWindowSize( size() );
  // GeneralConfig::instance()->save();

  delete logger;

#ifdef MAEMO

  // stop maemo screen saver off triggering
  if( ossoContext )
    {
      ossoDisplayTrigger->stop(); 
      osso_deinitialize( ossoContext );
    }

#endif

}

/** As the name tells ...
  *
  */
void CumulusApp::playSound( const char *name )
{
  if ( ! GeneralConfig::instance()->getAlarmSoundOn() )
    {
      return ;
    }

  if ( name && QString(name) == "beep" )
    {
       QApplication::beep();
       return;
    }
  
  QString sound;

  if( name && QString(name) == "notify" )
    {
      sound = GeneralConfig::instance()->getInstallRoot() + "/sounds/Notify.wav";
    }
  else if( name && QString(name) == "alarm" )
    {
      sound = GeneralConfig::instance()->getInstallRoot() + "/sounds/Alarm.wav";
    }
  else if( name )
    {
      sound = *name;
    }

  // The sound is played in an extra thread
  Sound *player = new Sound( sound );

  player->start( QThread::TimeCriticalPriority );
}

void CumulusApp::slotNotification( const QString& msg, const bool sound )
{
  if ( sound )
    {
      playSound("notify");
    }

  viewMap->slot_warning( msg );
}

void CumulusApp::slotAlarm( const QString& msg, const bool sound )
{
  if ( msg.isEmpty() )
    {
      return ;
    }

  if ( sound )
    {
      playSound("alarm");
    }

  viewMap->slot_warning( msg );
}

void CumulusApp::initMenuBar()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction( actionFileQuit );

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction( actionViewTaskpoints );
  actionViewTaskpoints->setEnabled( false );
  viewMenu->addAction( actionViewWaypoints );
  viewMenu->addAction( actionViewReachpoints );
  viewMenu->addAction( actionViewAirfields );
  viewMenu->addAction( actionViewInfo );
  actionViewInfo->setEnabled( false );
  viewMenu->addSeparator();
  viewMenu->addAction( actionToggleStatusbar );
  viewMenu->addAction( actionToggleWpLabels );
  viewMenu->addAction( actionToggleWpLabelsEI );
  viewMenu->addSeparator();
  viewMenu->addAction( actionViewGPSStatus );

  mapMenu = menuBar()->addMenu(tr("&Map"));
  mapMenu->addAction( actionToggleLogging );
  mapMenu->addAction( actionRememberWaypoint );
  mapMenu->addAction( actionSelectTask );
  mapMenu->addAction( actionToggleManualInFlight );
  mapMenu->addSeparator();
  mapMenu->addAction( actionEnsureVisible );
  mapMenu->addAction( actionZoomInZ );
  mapMenu->addAction( actionZoomOutZ );

  setupMenu = menuBar()->addMenu(tr("&Setup"));
  setupMenu->addAction( actionSetupConfig );
  setupMenu->addAction( actionPreFlight );
  setupMenu->addAction( actionSetupInFlight );

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction( actionHelpCumulus );
  helpMenu->addAction( actionHelpAboutApp );
  helpMenu->addAction( actionHelpAboutQt );

  menuBar()->hide();

  if( font().pointSize() < 15 )
    {
      QFont cf = font();
      cf.setPointSize( 15 );

      menuBar()->setFont( cf );
      fileMenu->setFont( cf );
      viewMenu->setFont( cf );
      mapMenu->setFont( cf );
      setupMenu->setFont( cf );
      helpMenu->setFont( cf );
    }
}


/** initializes all QActions of the application */
void CumulusApp::initActions()
{
  ws->slot_SetText1( tr( "Setting up key shortcuts ..." ) );

  // most shortcuts are now QActions these could also go as
  // QAction, even when not in a menu
  // @JD done. Uff!

  // Manual navigation shortcuts. Only available if no GPS connection
  actionManualNavUp = new QAction( tr( "Move up" ), this );
  actionManualNavUp->setShortcut ( QKeySequence("Up") );
  addAction( actionManualNavUp );
  connect( actionManualNavUp, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionN() ) );

  actionManualNavRight = new QAction( tr( "Move right" ), this );
  actionManualNavRight->setShortcut( QKeySequence("Right") );
  addAction( actionManualNavRight );
  connect( actionManualNavRight, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionE() ) );

  actionManualNavDown = new QAction( tr( "Move down" ), this );
  actionManualNavDown->setShortcut( QKeySequence("Down") );
  addAction( actionManualNavDown );
  connect( actionManualNavDown, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionS() ) );

  actionManualNavLeft = new QAction( tr( "Move left" ), this );
  actionManualNavLeft->setShortcut( QKeySequence("Left") );
  addAction( actionManualNavLeft );
  connect( actionManualNavLeft, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionW() ) );

  actionManualNavHome = new QAction( tr( "Move to home site" ), this ); 
  actionManualNavHome->setShortcut( QKeySequence("H") );
  addAction( actionManualNavHome );
  connect( actionManualNavHome, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionHome() ) );

  actionManualNavWP = new QAction( tr( "Move to waypoint" ), this ); 
  actionManualNavWP->setShortcut( QKeySequence("C") );
  addAction( actionManualNavWP );
  connect( actionManualNavWP, SIGNAL( triggered() ),
           calculator, SLOT( slot_changePositionWp() ) );

  actionManualNavWPList = new QAction( tr( "Open waypoint list" ), this ); 
  actionManualNavWPList->setShortcut( QKeySequence("F9") );
  addAction( actionManualNavWPList );
  connect( actionManualNavWPList, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToWPListView() ) );

  // GPS navigation shortcuts. Only available with GPS connected

  actionGpsNavUp = new QAction( tr( "McCready up" ), this );
  actionGpsNavUp->setShortcut( QKeySequence("Up") );
  addAction( actionGpsNavUp );
  connect( actionGpsNavUp, SIGNAL( triggered() ),
           calculator, SLOT( slot_McUp() ) );

  actionGpsNavDown = new QAction( tr( "McCready down" ), this );
  actionGpsNavDown->setShortcut( QKeySequence("Down") );
  addAction( actionGpsNavDown );
  connect( actionGpsNavDown, SIGNAL( triggered() ),
           calculator, SLOT( slot_McDown() ) );

  actionGpsNavHome = new QAction( tr( "Set home site waypoint" ), this );
  actionGpsNavHome->setShortcut( QKeySequence("H") );
  addAction( actionGpsNavHome );
  connect( actionGpsNavHome, SIGNAL( triggered() ),
           this, SLOT( slotNavigateHome() ) );

  actionGpsNavWPList = new QAction( tr( "Open waypoint list" ), this );
  actionGpsNavWPList->setShortcut( QKeySequence("F9") );
  addAction( actionGpsNavWPList );
  connect( actionGpsNavWPList, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToWPListView() ) );

  // Zoom in map
  actionGpsNavZoomIn = new QAction( tr( "Zoom in" ), this );
  actionGpsNavZoomIn->setShortcut( QKeySequence("Right") );

  addAction( actionGpsNavZoomIn );
  connect( actionGpsNavZoomIn, SIGNAL( triggered() ),
           viewMap->_theMap, SLOT( slotZoomIn() ) );

  // Zoom out map
  actionGpsNavZoomOut = new QAction( tr( "Zoom out" ), this );
  actionGpsNavZoomOut->setShortcut( QKeySequence("Left") );
  addAction( actionGpsNavZoomOut );
  connect( actionGpsNavZoomOut, SIGNAL( triggered() ),
           viewMap->_theMap, SLOT( slotZoomOut() ) );

  // Toggle menu bar
  actionMenuBarToggle = new QAction( tr( "Toggle menu" ), this );
  QList<QKeySequence> mBTSCList;
  mBTSCList << Qt::Key_F4 << Qt::Key_M << Qt::Key_Space;
  actionMenuBarToggle->setShortcuts( mBTSCList );
  addAction( actionMenuBarToggle );
  connect( actionMenuBarToggle, SIGNAL( triggered() ),
                           this, SLOT( slotToggleMenu() ) );

  actionFileQuit = new QAction( tr( "&Exit" ), this );
  actionFileQuit->setShortcut( QKeySequence("Shift+E") );
  addAction( actionFileQuit );
  connect( actionFileQuit, SIGNAL( triggered() ),
           this, SLOT( slotFileQuit() ) );

  actionViewWaypoints = new QAction ( tr( "&Waypoints" ), this );
  actionViewWaypoints->setShortcut(Qt::Key_W);
  addAction( actionViewWaypoints );
  connect( actionViewWaypoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToWPListView() ) );

  actionViewAirfields = new QAction ( tr( "&Airfields" ), this );
  actionViewAirfields->setShortcut(Qt::Key_O);
  addAction( actionViewAirfields );
  connect( actionViewAirfields, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToAFListView() ) );

  actionViewReachpoints = new QAction ( tr( "&Reachable" ), this );
  actionViewReachpoints->setShortcut(Qt::Key_E);
  addAction( actionViewReachpoints );
  connect( actionViewReachpoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToReachListView() ) );

  actionViewTaskpoints = new QAction ( tr( "&Task" ), this );
  actionViewTaskpoints->setShortcut(Qt::Key_T);
  addAction( actionViewTaskpoints );
  connect( actionViewTaskpoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToTaskListView() ) );

  actionViewInfo = new QAction( tr( "&Info" ), this );
  actionViewInfo->setShortcut(Qt::Key_I);
  addAction( actionViewInfo );
  connect( actionViewInfo, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToInfoView() ) );

  actionToggleStatusbar = new QAction( tr( "&Statusbar" ), this );
  actionToggleStatusbar->setCheckable(true);
  actionToggleStatusbar->setChecked(true);
  addAction( actionToggleStatusbar );
  connect( actionToggleStatusbar, SIGNAL( toggled( bool ) ),
           this, SLOT( slotViewStatusBar( bool ) ) );

  actionViewGPSStatus = new QAction( tr( "GPS Status" ), this );
  actionViewGPSStatus->setShortcut(Qt::Key_G);
  addAction( actionViewGPSStatus );
  connect( actionViewGPSStatus, SIGNAL( triggered() ),
           viewMap, SLOT( slot_gpsStatusDialog() ) );

  // Consider qwertz keyboards y <-> z are interchanged
  // F7 is a Maemo hardware key for Zoom in
  actionZoomInZ = new QAction ( tr( "Zoom in" ), this );
  QList<QKeySequence> zInSCList;
  zInSCList << Qt::Key_Z << Qt::Key_Y << Qt::Key_F7;
  actionZoomInZ->setShortcuts( zInSCList );

  addAction( actionZoomInZ );
  connect ( actionZoomInZ, SIGNAL( triggered() ),
            viewMap->_theMap , SLOT( slotZoomIn() ) );

  // F8 is a Maemo hardware key for Zoom out
  actionZoomOutZ = new QAction ( tr( "Zoom out" ), this );
  QList<QKeySequence> zOutSCList;
  zOutSCList << Qt::Key_X << Qt::Key_F8;
  actionZoomOutZ->setShortcuts( zOutSCList );
  
  addAction( actionZoomOutZ );
  connect ( actionZoomOutZ, SIGNAL( triggered() ),
            viewMap->_theMap , SLOT( slotZoomOut() ) );

  actionToggleWpLabels = new QAction ( tr( "Waypoint labels" ), this);
  actionToggleWpLabels->setShortcut(Qt::Key_A);
  actionToggleWpLabels->setCheckable(true);
  actionToggleWpLabels->setChecked( _globalMapConfig->getShowWpLabels() );
  addAction( actionToggleWpLabels );
  connect( actionToggleWpLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleWpLabels( bool ) ) );

  actionToggleWpLabelsEI = new QAction (  tr( "Waypoint extra info" ), this);
  actionToggleWpLabelsEI->setShortcut(Qt::Key_S);
  actionToggleWpLabelsEI->setCheckable(true);
  actionToggleWpLabelsEI->setChecked( _globalMapConfig->getShowWpLabelsExtraInfo() );
  addAction( actionToggleWpLabelsEI );
  connect( actionToggleWpLabelsEI, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleWpLabelsExtraInfo( bool ) ) );

  actionToggleLogging = new QAction( tr( "Logging" ), this );
  actionToggleLogging->setShortcut(Qt::Key_L);
  actionToggleLogging->setCheckable(true);
  addAction( actionToggleLogging );
  connect ( actionToggleLogging, SIGNAL( triggered() ),
            logger, SLOT( slotToggleLogging() ) );

  actionEnsureVisible = new QAction ( tr( "Ensure waypoint visible" ), this );
  actionEnsureVisible->setShortcut(Qt::Key_V);
  addAction( actionEnsureVisible );
  connect ( actionEnsureVisible, SIGNAL( triggered() ),
            this, SLOT( slotEnsureVisible() ) );

  actionSelectTask = new QAction( tr( "Select task" ), this );
  actionSelectTask->setShortcut(Qt::Key_T + Qt::SHIFT);
  addAction( actionSelectTask );
  connect ( actionSelectTask, SIGNAL( triggered() ),
            this, SLOT( slotPreFlightTask() ) );

  actionToggleManualInFlight = new QAction( tr( "Manual" ), this );
  actionToggleManualInFlight->setShortcut(Qt::Key_M + Qt::SHIFT);
  actionToggleManualInFlight->setEnabled(false);
  actionToggleManualInFlight->setCheckable(true);
  addAction( actionToggleManualInFlight );
  connect( actionToggleManualInFlight, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleManualInFlight( bool ) ) );

  actionPreFlight = new QAction( tr( "Pre-flight setup" ), this );
  actionPreFlight->setShortcut(Qt::Key_P);
  addAction( actionPreFlight );
  connect ( actionPreFlight, SIGNAL( triggered() ),
            this, SLOT( slotPreFlightGlider() ) );

  actionRememberWaypoint = new QAction( tr( "Remember waypoint" ), this );
  actionRememberWaypoint->setShortcut(Qt::Key_R);
  addAction( actionRememberWaypoint );
  connect( actionRememberWaypoint, SIGNAL( triggered() ),
           this, SLOT( slotRememberWaypoint() ) );

  actionSetupConfig = new QAction( tr ( "General setup" ), this );
  actionSetupConfig->setShortcut(Qt::Key_S + Qt::SHIFT);
  addAction( actionSetupConfig );
  connect ( actionSetupConfig, SIGNAL( triggered() ),
            this, SLOT( slotConfig() ) );

  actionSetupInFlight = new QAction( tr ( "In flight" ), this );
  actionSetupInFlight->setShortcut(Qt::Key_F);
  addAction( actionSetupInFlight );
  connect ( actionSetupInFlight, SIGNAL( triggered() ),
            viewMap, SLOT( slot_gliderFlightDialog() ) );

  actionHelpCumulus = new QAction( tr("Help" ), this );
  actionHelpCumulus->setShortcut(Qt::Key_Question);
  addAction( actionHelpCumulus );
  connect( actionHelpCumulus, SIGNAL(triggered()), this, SLOT(slotHelp()) );

  // actionWhatsThis = new QAction( tr( "What's this ?" ), this );
  // connect ( actionWhatsThis, SIGNAL( triggered() ), this, SLOT( whatsThis() ) );

  actionHelpAboutApp = new QAction( tr( "About Cumulus" ), this );
  actionHelpAboutApp->setShortcut(Qt::Key_V + Qt::SHIFT);
  addAction( actionHelpAboutApp );
  connect( actionHelpAboutApp, SIGNAL( triggered() ),
           this, SLOT( slotVersion() ) );

  actionHelpAboutQt = new QAction( tr( "About Qt" ), this );
  actionHelpAboutQt->setShortcut(Qt::Key_Q + Qt::SHIFT);
  addAction( actionHelpAboutQt );
  connect( actionHelpAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );

  // Cumulus can be closed by using Escape key. This key is also as
  // hardware key available under Maemo.
  scExit = new QShortcut( this );
  scExit->setKey( Qt::Key_Escape );
  connect( scExit, SIGNAL(activated()), this, SLOT( close() ));
}

/**
 * Toggle on/off all actions which have key shortcuts defined.
 */

void  CumulusApp::toggleActions( const bool toggle )
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
  actionHelpCumulus->setEnabled( toggle );
  actionHelpAboutApp->setEnabled( toggle );
  actionHelpAboutQt->setEnabled( toggle );
  actionToggleLogging->setEnabled( toggle );
  scExit->setEnabled( toggle );
  // do not toggle actionToggleManualInFlight, status may not be changed
}

/**
 * Toggle actions depending on GPS connection.
 */
void CumulusApp::toggleManualNavActions( const bool toggle )
{
  actionManualNavUp->setEnabled( toggle );
  actionManualNavRight->setEnabled( toggle );
  actionManualNavDown->setEnabled( toggle );
  actionManualNavLeft->setEnabled( toggle );
  actionManualNavHome->setEnabled( toggle );
  actionManualNavWP->setEnabled( toggle );
  actionManualNavWPList->setEnabled( toggle );
}

void CumulusApp::toggleGpsNavActions( const bool toggle )
{
  actionGpsNavUp->setEnabled( toggle );
  actionGpsNavDown->setEnabled( toggle );
  actionGpsNavHome->setEnabled( toggle );
  actionGpsNavWPList->setEnabled( toggle );
  actionGpsNavZoomIn->setEnabled( toggle );
  actionGpsNavZoomOut->setEnabled( toggle );
}


void CumulusApp::slotFileQuit()
{
  close();
}


/**
 * Make sure the user really wants to quit
 */
void CumulusApp::closeEvent( QCloseEvent* evt )
{
  // @AP: All close events will be ignored, if we are not in the map
  // view to avoid any possibility of confusion with the two close
  // buttons.
  if( view != mapView )
    {
      evt->ignore();
      return;
    }

  playSound("notify");

  QMessageBox mb( QMessageBox::Warning,
                  tr( "Are You Sure?" ),
                  tr( "Terminating Cumulus<br><b>Are you sure?</b>" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this,
                  Qt::Dialog );

  mb.setDefaultButton( QMessageBox::Yes );

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
  viewMap->_theMap->scheduleRedraw(Map::waypoints);
}


void CumulusApp::slotToggleWpLabelsExtraInfo( bool toggle )
{
  _globalMapConfig->setShowWpLabelsExtraInfo( toggle );
  viewMap->_theMap->scheduleRedraw(Map::waypoints);
}


void CumulusApp::slotViewStatusBar( bool toggle )
{
  if ( toggle )
    viewMap->statusBar()->show();
  else
    viewMap->statusBar()->hide();
}


/** Called if the logging is actually toggled */
void CumulusApp::slot_Logging ( bool logging )
{
  actionToggleLogging->blockSignals( true );
  actionToggleLogging->setChecked( logging );
  actionToggleLogging->blockSignals( false );
}



/** Read property of enum view. */
const CumulusApp::appView CumulusApp::getView()
{
  return view;
}


/** Called if the user clicks on a tab with of a list view */
void CumulusApp::slot_tabChanged( int index )
{
  // qDebug("CumulusApp::slot_tabChanged(): NewIndex=%d", index );
  
  //switch to the correct view
  if ( index == listViewTabs->indexOf(viewWP) )
    {
      setView( wpView );
    }
  else if ( index == listViewTabs->indexOf(viewTP) )
    {
      setView( tpView );
    }
  else if ( index == listViewTabs->indexOf(viewRP) )
    {
      setView( rpView );
    }
  else if ( index == listViewTabs->indexOf(viewAF) )
    {
      setView( afView );
    }
  else
    {
      qWarning("CumulusApp::slot_tabChanged(): Cannot switch to index %d", index );
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

      toggleManualNavActions( !gps->getConnected() || calculator->isManualInFlight());
      toggleGpsNavActions( gps->getConnected() && !calculator->isManualInFlight() );
      actionMenuBarToggle->setEnabled( true );

      // Switch on all action shortcuts in this view
      toggleActions( true );
      viewMap->statusBar()->clearMessage(); // remove temporary statusbar messages

      break;

    case wpView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      listViewTabs->setCurrentWidget( viewWP );
      listViewTabs->show();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

      break;

    case rpView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      listViewTabs->setCurrentWidget( viewRP );
      listViewTabs->show();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

      break;

    case afView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      viewAF->listWidget()->fillWpList();
      listViewTabs->setCurrentWidget( viewAF );
      listViewTabs->show();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

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
      listViewTabs->setCurrentWidget( viewTP );
      listViewTabs->show();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

      break;

    case infoView:

      if ( ! wp )
        {
          return;
        }

      menuBar()->hide();
      viewMap->hide();
      listViewTabs->hide();
      viewInfo->showWP( view, wp );

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

      break;

    case tpSwitchView:

      menuBar()->hide();
      viewMap->hide();
      listViewTabs->hide();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );

      break;

    case cfView:
      menuBar()->hide();
      viewMap->hide();
      listViewTabs->hide();

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionMenuBarToggle->setEnabled( false );
      toggleActions( false );
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
//  qDebug("CumulusApp::slotSwitchToInfoView()");
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
      setView( infoView, viewAF->getSelectedWaypoint() );
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


/** Opens the config "dialog" */
void CumulusApp::slotConfig()
{
  setWindowTitle( "Cumulus Settings" );
  ConfigDialog *cDlg = new ConfigDialog( this );
  cDlg->resize( size() );

  setView( cfView );

  connect( cDlg, SIGNAL( settingsChanged() ),
           this, SLOT( slotReadconfig() ) );
  connect( cDlg, SIGNAL( closeConfig() ),
           this, SLOT( slotCloseConfig() ) );
  connect( cDlg,  SIGNAL( welt2000ConfigChanged() ),
           _globalMapContents, SLOT( slotReloadWelt2000Data() ) );
  configView = (QWidget*) cDlg;
  cDlg->show();
}


/** Closes the config or pre-flight "dialog" */
void CumulusApp::slotCloseConfig()
{
  setView( mapView );
//  qDebug("* closing configView:  %s", calculator->gliderType().toLatin1().data() );
  if ( !calculator->gliderType().isEmpty() )
    setWindowTitle ( "Cumulus - " + calculator->gliderType() );
  else
    setWindowTitle( "Cumulus" );

  configView = 0;
}

/** Shows version and copyright information */
void CumulusApp::slotVersion()
{
  QMessageBox::about ( this,
                       "Cumulus",
                       QString(tr(
                                 "<html>"
                                 "Cumulus X11 version %1<br>"
                                 "<font size=-1>(compiled at %2 with QT %3)</font><br>"
                                 "&copy; 2002-2008 by<br>"
                                 "Andr&eacute; Somers, Eggert Ehmke<br>"
                                 "Axel Pauli, Eckhard V&ouml;llm<br>"
                                 "Josua Dietze, Michael Enke<br>"
                                 "Hendrik M&uuml;ller, Florian Ehinger<br>"
                                 "Heiner Lamprecht<br>"
                                 "<a href=\"http://www.kflog.org/cumulus\">http://www.kflog.org/cumulus</a><br>"
                                 "Published under the GPL<br>"
                                 "</html>" ).arg( QString(CU_VERSION) ).arg( QString( __DATE__ )).arg( QString(QT_VERSION_STR) ) ) );
}


/** opens help documentation in browser. */
void CumulusApp::slotHelp()
{
  HelpBrowser *hb = new HelpBrowser(this);
  hb->setAttribute(Qt::WA_DeleteOnClose);
  hb->resize( GeneralConfig::instance()->getWindowSize() );
  hb->show();
}

void CumulusApp::slotWaypointChanged( const wayPoint *newWp )
{
  // qDebug("CumulusApp::slotWaypointChanged() is called" );

  if( newWp != 0  )
    {
      actionViewInfo->setEnabled( true );
      actionViewInfo->setShortcut( Qt::Key_I ); // re-enable shortcut
    }
  else
    {
      actionViewInfo->setEnabled( false );
      actionViewInfo->setShortcut( 0 ); // disable shortcut
    }
}


void CumulusApp::slotRememberWaypoint()
{
  static uint count = 1;
  QString name;

  name = tr( "WP%1-%2" ).arg( count ).arg( QTime::currentTime().toString().left( 5 ) );

  // @AP: let us check, if user waypoint is already known from its
  // position. In this case we will reject the insertion to avoid senseless
  // duplicates.

  QPoint pos = calculator->getlastPosition();
  QList<wayPoint*>* wpList = _globalMapContents->getWaypointList();

  for ( int i = 0; i < wpList->count(); i++ )
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
  wp->description = tr( "user created" );
  wp->comment = tr("created by remember action at ") +
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
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
void CumulusApp::slotReadconfig()
{
  // @AP: WARNING POPUP crash test statement, please let it commented out here
  // viewMap->theMap->checkAirspace(calculator->getlastPosition());

  GeneralConfig *conf = GeneralConfig::instance();

  // configure units
  Distance::setUnit( Distance::distanceUnit( conf->getUnitDist() ) );
  Speed::setHorizontalUnit( Speed::speedUnit( conf->getUnitSpeed() ) );
  Speed::setVerticalUnit( Speed::speedUnit( conf->getUnitVario() ) );
  Altitude::setUnit( Altitude::altitude( conf->getUnitAlt() ) );
  WGSPoint::setFormat( WGSPoint::Format( conf->getUnitPos() ) );

  // other config changes
  _globalMapConfig->slotReadConfig();
  viewMap->slot_settingschange();
  calculator->slot_settingschanged();
  viewTP->slot_updateTask();
  viewRP->fillRpList();
  viewAF->listWidget()->configRowHeight();
  viewAF->listWidget()->slot_Done();
  viewWP->listWidget()->configRowHeight();
  viewWP->listWidget()->slot_Done();

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
              listViewTabs->insertTab( _taskListVisible ? 2 : 1, viewRP, tr( "Reachable" ) );
              calculator->newSites();
              _reachpointListVisible = true;
            }
        }
      else
        {
          if(_reachpointListVisible)
            {
              // changes in listViewTabs trigger  (if viewRP was last active),
              // this slot calls setView and tries to set the view to viewRP
              // but since this doesn't exist (removeWidget), sets the view to the next one
              // which is viewAF; that's the reason we have to a) call setView(mapView);
              // or b) disconnect before removeWidget and connect again behind
              disconnect( listViewTabs, SIGNAL( currentChanged( int index ) ),
                          this, SLOT( slot_tabChanged( int index ) ) );
              listViewTabs->removeTab( listViewTabs->indexOf(viewRP) );

              connect( listViewTabs, SIGNAL( currentChanged( int index ) ),
                       this, SLOT( slot_tabChanged( int index ) ) );
              calculator->clearReachable();
              viewRP->fillRpList();   // this clears the listView
              viewMap->_theMap->scheduleRedraw(Map::waypoints);
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

  playSound("notify");

  if ( ( status < GPSNMEA::validFix || calculator->isManualInFlight()) && ( view == mapView ) )
    {  // no GPS data
      toggleManualNavActions( true );
      toggleGpsNavActions( false );
    }
  else
    {  // GPS data valid
      toggleManualNavActions( false );
      toggleGpsNavActions( true );
    }
}


/** This slot is called if the user presses C in manual navigation mode. It centers
  * the map on the current waypoint. */
void CumulusApp::slotCenterToWaypoint()
{

  if ( calculator->getselectedWp() )
    {
      _globalMapMatrix->centerToLatLon( calculator->getselectedWp() ->origP );
      viewMap->_theMap->scheduleRedraw();

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


/** Opens the pre-flight "dialog" and brings the selected tabulator to the front */
void CumulusApp::slotPreFlight(const char *tabName)
{
  setWindowTitle( "Pre-Flight Settings" );
  PreFlightDialog* cDlg = new PreFlightDialog( this, tabName );
  cDlg->setObjectName("PreFlightDialog");
  cDlg->resize( size() );

  setView( cfView );

  connect( cDlg, SIGNAL( settingsChanged() ),
           this, SLOT( slotPreFlightDataChanged() ) );

  connect( cDlg, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );

  connect( cDlg, SIGNAL( closeConfig() ),
           this, SLOT( slotCloseConfig() ) );

  cDlg->show();
  configView = (QWidget*) cDlg;
}


void CumulusApp::slotPreFlightDataChanged()
{

  if ( _globalMapContents->getCurrentTask() == 0 )
    {
      if ( _taskListVisible )
        {
          // see comment for removeTab( viewRP )
          disconnect( listViewTabs, SIGNAL( currentChanged( int index ) ),
                      this, SLOT( slot_tabChanged( int index ) ) );
          listViewTabs->removeTab( listViewTabs->indexOf(viewTP) );
          
          connect( listViewTabs, SIGNAL( currentChanged( int index ) ),
                   this, SLOT( slot_tabChanged( int index ) ) );
          actionViewTaskpoints->setEnabled( false );
          _taskListVisible = false;
        }
    }
  else
    {
      if ( !_taskListVisible )
        {
          listViewTabs->insertTab( 0, viewTP, tr( "Task" ) );
          _taskListVisible = true;
          actionViewTaskpoints->setEnabled( true );
        }
    }

  // set the task list view at the current task
  viewTP->slot_setTask( _globalMapContents->getCurrentTask() );
  viewMap->_theMap->scheduleRedraw(Map::task);
}

/** dynamicly updates view for reachable list */
void CumulusApp::slot_newReachList()
{
  viewRP->slot_newList(); //let the view know we have a new list
  viewMap->_theMap->scheduleRedraw(Map::waypoints);
}


bool CumulusApp::eventFilter( QObject *o , QEvent *e )
{
  // qDebug("CumulusApp::eventFilter() is called with event type %d", e->type());

  if ( e->type() == QEvent::KeyPress )
    {
      QKeyEvent *k = ( QKeyEvent* ) e;
      
      qDebug( "Keycode of pressed key: %d, %X", k->key(), k->key() );

      if( k->key() == Qt::Key_F6 )
        {
          // hardware Key F6 for maximize/normalize screen under Maemo
          setWindowState(windowState() ^ Qt::WindowFullScreen);
          return true;
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
  toggleManualNavActions( on );
  toggleGpsNavActions( !on );
}

/** Used to allow or disable user keys processing during map drawing. */
void CumulusApp::slotMapDrawEvent( bool drawEvent )
{
   if( drawEvent )
     {
      // Disable menu shortcut during drawing to avoid
      // event avalanche, if the user holds the key down longer.
      actionMenuBarToggle->setEnabled( false );

      if( view == mapView )
       {
         toggleManualNavActions( false );
         toggleGpsNavActions( false );
       }
     }
   else
     {
       actionMenuBarToggle->setEnabled( true );

       if( view == mapView )
         {
           toggleManualNavActions( !gps->getConnected() || calculator->isManualInFlight());
           toggleGpsNavActions( gps->getConnected() && !calculator->isManualInFlight() );
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

  if( configView )
    {
      configView->resize( event->size() );
    }
}

#ifdef MAEMO

/** Called to prevent the switch off of the screen display */
void CumulusApp::slot_ossoDisplayTrigger()
{
  // If the speed is greater or equal 10 km/h and we have a connected
  // gps we switch off the screen saver. Otherwise we let all as it
  // is.

  if( calculator->getlastSpeed().getKph() >= 10.0 && gps->getConnected() )
    {
      // tell maemo that we are in move to avoid blank screen
      osso_return_t ret = osso_display_blanking_pause( ossoContext );

      if( ret != OSSO_OK )
        {
          qWarning( "osso_display_blanking_pause() call failed" );
        }
    }
  
  // Restart the timer because we use a single shot timer to avoid
  // multiple triggering in case of delays. Next trigger is in 10s.
  ossoDisplayTrigger->start( 10000 );
}

#endif
