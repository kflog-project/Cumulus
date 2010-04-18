/***************************************************************************
 mainwindow.cpp  -  main application class
 -----------------------------------------
 begin                : Sun Jul 21 2002
 copyright            : (C) 2002      by André Somers
 ported to Qt4.x/X11  : (C) 2007-2010 by Axel Pauli
 maintainer           : axel@kflog.org

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

/**
 *  This class is called by main to create all the widgets needed by this GUI
 *  and to initiate the load of the map and all other data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <QtCore>
#include <QtGui>

#ifdef MAEMO4
#include "maemostyle.h"
#endif

#include "generalconfig.h"
#include "mainwindow.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "calculator.h"
#include "windanalyser.h"
#include "configwidget.h"
#include "wpeditdialog.h"
#include "preflightwidget.h"
#include "wgspoint.h"
#include "waypoint.h"
#include "target.h"
#include "helpbrowser.h"
#include "sound.h"
#include "time_cu.h"

#ifdef MAEMO

extern "C" {

#include <glib-object.h>
#include <libosso.h>

}

// @AP: That is a little hack, to avoid the include of all glib
// functionality in the header file of this class. There are
// redefinitions of macros in glib and other cumulus header files.
static osso_context_t *ossoContext = static_cast<osso_context_t *> (0);

#endif

/**
 * Global available instance of this class
 */
MainWindow *_globalMainWindow = static_cast<MainWindow *> (0);

/**
 * Used for transforming the map items.
 */
MapMatrix *_globalMapMatrix = static_cast<MapMatrix *> (0);

/**
 * Contains all map elements and takes control over drawing or printing
 * the elements.
 */
MapContents *_globalMapContents = static_cast<MapContents *> (0);

/**
 * Contains all configuration-info for drawing and printing the elements.
 */
MapConfig *_globalMapConfig = static_cast<MapConfig *> (0);

/**
 * Contains all map view infos
 */
MapView *_globalMapView = static_cast<MapView *> (0);


// A signal SIGCONT has been catched. It is send out
// when the cumulus process was stopped and then reactivated.
// We have to renew the connection to our GPS Receiver.
static void resumeGpsConnection( int sig )
{
  if ( sig == SIGCONT )
    {
      GpsNmea::gps->forceReset();
      // @ee reinstall signal handler
      signal ( SIGCONT, resumeGpsConnection );
    }
}

MainWindow::MainWindow( Qt::WindowFlags flags ) : QMainWindow( 0, flags )
{
  _globalMainWindow = this;
  menuBarVisible = false;
  listViewTabs = 0;
  configView = 0;
  fileMenu = 0;
  viewMenu = 0;
  mapMenu = 0;
  setupMenu = 0;
  helpMenu = 0;
  logger = static_cast<IgcLogger *> (0);

  // Eggert: make sure the app uses utf8 encoding for translated widgets
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );

  // Get application font for user manipulations
  QFont appFt = QApplication::font();

//  qDebug("QAppFont family %s, pointSize=%d pixelSize=%d",
//         appFt.family().toLatin1().data(), appFt.pointSize(), appFt.pixelSize() );

#ifdef MAEMO

  // That we do need for the location service. This service emits signals, which
  // are bound to a g_object. This call initializes the g_object handling.
  g_type_init();

  // Show available input methods
  QStringList inputMethods = QInputContextFactory::keys();

  foreach( QString inputMethod, inputMethods )
    {
      qDebug() << "InputMethod: " << inputMethod;
    }

  // Check, if virtual keyboard support is enabled
  if( GeneralConfig::instance()->getVirtualKeyboard() )
    {
      // qApp->setInputContext(hildonInputContext);
    }

  // For Maemo it's really better to pre-set style and font
  // QApplication::setGlobalStrut( QSize(24,16) );

  // Set our own GUI style
  GeneralConfig::instance()->setOurGuiStyle();

#ifdef MAEMO4
  // N8x0 display has bad contrast for light shades, so make the (dialog)
  // background darker
  QPalette appPal = QApplication::palette();
  appPal.setColor(QPalette::Normal,QPalette::Window,QColor(236,236,236));
  appPal.setColor(QPalette::Normal,QPalette::Button,QColor(216,216,216));
  appPal.setColor(QPalette::Normal,QPalette::Base,Qt::white);
  appPal.setColor(QPalette::Normal,QPalette::AlternateBase,QColor(246,246,246));
  appPal.setColor(QPalette::Normal,QPalette::Highlight,Qt::darkBlue);
  QApplication::setPalette(appPal);
#endif

#endif

  // sets the user's selected font, if defined
  QString fontString = GeneralConfig::instance()->getGuiFont();
  QFont userFont;

  if( fontString != "" && userFont.fromString( fontString ) )
    {
      // take the user font
      QApplication::setFont( userFont );
    }
  else
    {
#ifdef MAEMO
      // If there is not defined a user font, we try to set some useful defaults for MAEMO
      // The Nokia font has excellent readability and less width than others
      appFt.setFamily("Nokia Sans");

      if( appFt.pointSize() < 14 )
        {
          // the system font size is too small, we are setting a bigger default
          appFt.setPointSize( 14 );
        }

      QApplication::setFont( appFt );
#endif
    }

  // get last saved window geometric from GeneralConfig and set it again
  resize( GeneralConfig::instance()->getWindowSize() );

  qWarning( "Cumulus, Release: %s, Build date:  %s based on Qt/X11 Version %s",
            CU_VERSION, __DATE__, QT_VERSION_STR );

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
  char *display = getenv( "DISPLAY" );
  char *proxy = getenv( "http_proxy" );

  qDebug( "PWD=%s", pwd ? pwd : "NULL" );
  qDebug( "USER=%s", user ? user : "NULL" );
  qDebug( "HOME=%s", home ? home : "NULL" );
  qDebug( "LANG=%s", lang ? lang : "NULL" );
  qDebug( "LD_LIBRARY_PATH=%s", ldpath ? ldpath : "NULL" );
  qDebug( "QDir::homePath()=%s", QDir::homePath().toLatin1().data() );
  qDebug( "DISPLAY=%s", display ? display : "NULL" );
  qDebug( "HTTP_PROXY=%s", proxy ? proxy : "NULL" );

  qDebug( "UserDataDir=%s",
          GeneralConfig::instance()->getUserDataDirectory().toLatin1().data() );

  qDebug( "MapRootDir=%s",
          GeneralConfig::instance()->getMapRootDir().toLatin1().data() );

  setFocusPolicy( Qt::StrongFocus );
  setFocus();

  this->installEventFilter( this );

  setWindowIcon( QIcon(GeneralConfig::instance()->loadPixmap("cumulus.png")) );
  setWindowTitle( "Cumulus" );

#ifdef MAEMO
  setWindowState(Qt::WindowFullScreen);
#endif

  splash = new Splash( this );

  setCentralWidget( splash );
  splash->show();
  show();

  ws = new WaitScreen(this);
  ws->show();

  qApp->processEvents();

  // Here we do finish the base initialization and start a timer
  // to continue startup in another method. This is done, to get
  // running the window manager event loop. Otherwise the behavior
  // of some widgets is undefined.

  // when the timer expires the cumulus startup is continued
  QTimer::singleShot(400, this, SLOT(slotCreateApplicationWidgets()));
 }

/** creates the application widgets after the base initialization
 *  of the core application window.
*/
void MainWindow::slotCreateApplicationWidgets()
{
  // qDebug( "MainWindow::slotCreateApplicationWidgets()" );

#ifdef MAEMO

  ossoContext = osso_initialize( "org.kflog.Cumulus", CU_VERSION, false, 0 );

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

  calculator = new Calculator( this );

  connect( _globalMapMatrix, SIGNAL( displayMatrixValues( int, bool ) ),
           _globalMapConfig, SLOT( slotSetMatrixValues( int, bool ) ) );

  connect( _globalMapMatrix, SIGNAL( homePositionChanged() ),
           _globalMapContents, SLOT( slotReloadWelt2000Data() ) );

  connect( _globalMapMatrix, SIGNAL( homePositionChanged() ),
           calculator, SLOT( slot_CheckHomeSiteSelection() ) );

  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           calculator, SLOT( slot_CheckHomeSiteSelection() ) );

  connect( _globalMapMatrix, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  _globalMapMatrix->slotInitMatrix();

  ws->slot_SetText1( tr( "Creating views..." ) );

  qDebug( "Main window size is %dx%d, width=%d, height=%d",
          size().width(),
          size().height(),
          size().width(),
          size().height() );

  // This is the main widget of Cumulus
  viewMap = new MapView( this );
  viewMap->hide();

  _globalMapView = viewMap;
  view = mapView;

#ifndef MAEMO
  QFont fnt( "Helvetica", 14 );
#else
  QFont fnt = font();
#endif

  fnt.setBold(true);

  listViewTabs = new QTabWidget( this );
  listViewTabs->setObjectName("listViewTabs");
  listViewTabs->resize( this->size() );
  listViewTabs->setFont( fnt );

  viewWP = new WaypointListView( this );

  QVector<enum MapContents::MapContentsListID> itemList;
  itemList << MapContents::AirfieldList << MapContents::GliderSiteList;
  viewAF = new AirfieldListView( itemList, this ); // airfields

  itemList.clear();
  itemList << MapContents::OutLandingList;
  viewOL = new AirfieldListView( itemList, this ); // outlandings

  viewRP = new ReachpointListView( this );
  viewTP = new TaskListView( this );

  viewWP->setFont( fnt );
  viewAF->setFont( fnt );
  viewOL->setFont( fnt );
  viewRP->setFont( fnt );
  viewTP->setFont( fnt );

  viewCF = new QWidget( this );

  // set visibility of lists to false
  _taskListVisible       = false;
  _reachpointListVisible = false;
  _outlandingListVisible = false;

  listViewTabs->addTab( viewWP, tr( "Waypoints" ) );
  listViewTabs->addTab( viewAF, tr( "Airfields" ) );
  // listViewTabs->addTab( viewRP, tr( "Reachable" ) ); --> added in slotReadconfig
  // listViewTabs->addTab( viewOL, tr( "Outlandings" ) ); --> added in slotReadconfig

  // waypoint info widget
  viewInfo = new WPInfoWidget( this );

  viewWP->listWidget()->fillWpList();

  // create GPS object
  GpsNmea::gps = new GpsNmea( this );
  GpsNmea::gps->blockSignals( true );
  logger = IgcLogger::instance();

  initActions();
  initMenuBar();

  ws->slot_SetText1( tr( "Setting up connections..." ) );

  // create connections between the components
  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           _globalMapContents, SLOT( slotReloadMapData() ) );

  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewMap->_theMap, SLOT( slotRedraw() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewAF, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewOL, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewWP, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewTP, SLOT( slot_updateTask() ) );

  connect( GpsNmea::gps, SIGNAL( newVario(const Speed&) ),
           calculator, SLOT( slot_GpsVariometer(const Speed&) ) );
  connect( GpsNmea::gps, SIGNAL( newMc(const Speed&) ),
           calculator, SLOT( slot_Mc(const Speed&) ) );
  connect( GpsNmea::gps, SIGNAL( newWind(const Speed&, const short) ),
           calculator, SLOT( slot_GpsWind(const Speed&, const short) ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           viewMap, SLOT( slot_GPSStatus( GpsNmea::GpsStatus ) ) );
  connect( GpsNmea::gps, SIGNAL( newSatCount(SatInfo&) ),
           viewMap, SLOT( slot_SatCount(SatInfo&) ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           this, SLOT( slotGpsStatus( GpsNmea::GpsStatus ) ) );
  connect( GpsNmea::gps, SIGNAL( newSatConstellation() ),
           logger, SLOT( slotConstellation() ) );
  connect( GpsNmea::gps, SIGNAL( newSatConstellation() ),
           calculator->getWindAnalyser(), SLOT( slot_newConstellation() ) );
  connect( GpsNmea::gps, SIGNAL( newSpeed() ),
           calculator, SLOT( slot_Speed() ) );
  connect( GpsNmea::gps, SIGNAL( newPosition() ),
           calculator, SLOT( slot_Position() ) );
  connect( GpsNmea::gps, SIGNAL( newAltitude() ),
           calculator, SLOT( slot_Altitude() ) );
  connect( GpsNmea::gps, SIGNAL( newHeading() ),
           calculator, SLOT( slot_Heading() ) );
  connect( GpsNmea::gps, SIGNAL( newFix() ),
           calculator, SLOT( slot_newFix() ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           calculator, SLOT( slot_GpsStatus( GpsNmea::GpsStatus ) ) );

  connect( viewWP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewWP, SIGNAL( deleteWaypoint( wayPoint* ) ),
           calculator, SLOT( slot_WaypointDelete( wayPoint* ) ) );
  connect( viewWP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewWP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewWP, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewWP, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewAF, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewAF, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewAF, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewAF, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewAF, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewOL, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewOL, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewOL, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewOL, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewOL, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewRP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewRP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewRP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewRP, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewRP, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewTP, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewTP, SIGNAL( done() ),
           this, SLOT( slotSwitchToMapView() ) );
  connect( viewTP, SIGNAL( info( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );

  connect( viewMap->_theMap, SIGNAL( isRedrawing( bool ) ),
           this, SLOT( slotMapDrawEvent( bool ) ) );
  connect( viewMap->_theMap, SIGNAL( waypointSelected( wayPoint* ) ),
           this, SLOT( slotSwitchToInfoView( wayPoint* ) ) );
  connect( viewMap->_theMap, SIGNAL( airspaceWarning( const QString&, const bool ) ),
           this, SLOT( slotAlarm( const QString&, const bool ) ) );
  connect( viewMap, SIGNAL( toggleLDCalculation( const bool ) ),
           calculator, SLOT( slot_toggleLDCalculation(const bool) ) );

  connect( viewInfo, SIGNAL( waypointAdded( wayPoint& ) ),
           viewWP, SLOT( slot_wpAdded( wayPoint& ) ) );
  connect( viewInfo, SIGNAL( waypointSelected( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );
  connect( viewInfo, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewInfo, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( listViewTabs, SIGNAL( currentChanged( int ) ),
           this, SLOT( slot_tabChanged( int ) ) );

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
           Map::getInstance(), SLOT( slotPosition( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( switchManualInFlight() ),
           Map::getInstance(), SLOT( slotSwitchManualInFlight() ) );
  connect( calculator, SIGNAL( glidePath( const Altitude& ) ),
           viewMap, SLOT( slot_GlidePath( const Altitude& ) ) );
  connect( calculator, SIGNAL( bestSpeed( const Speed& ) ),
           viewMap, SLOT( slot_bestSpeed( const Speed& ) ) );
  connect( calculator, SIGNAL( newMc( const Speed& ) ),
           viewMap, SLOT( slot_Mc( const Speed& ) ) );
  connect( calculator, SIGNAL( newVario( const Speed& ) ),
           viewMap, SLOT( slot_Vario( const Speed& ) ) );
  connect( viewMap, SIGNAL( toggleVarioCalculation( const bool ) ),
           calculator, SLOT( slot_toggleVarioCalculation(const bool) ) );
  connect( calculator, SIGNAL( newWind( Vector& ) ),
           viewMap, SLOT( slot_Wind( Vector& ) ) );
  connect( calculator, SIGNAL( newLD( const double&, const double&) ),
           viewMap, SLOT( slot_LD( const double&, const double&) ) );
  connect( calculator, SIGNAL( newGlider( const QString&) ),
           viewMap, SLOT( slot_glider( const QString&) ) );
  connect( calculator, SIGNAL( flightModeChanged( Calculator::flightmode ) ),
           viewMap, SLOT( slot_setFlightStatus() ) );

  connect( calculator, SIGNAL( taskpointSectorTouched() ),
           logger, SLOT( slotTaskSectorTouched() ) );
  connect( calculator, SIGNAL( taskInfo( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );
  connect( calculator, SIGNAL( newSample() ),
           logger, SLOT( slotMakeFixEntry() ) );

  connect( ( QObject* ) calculator->getReachList(), SIGNAL( newReachList() ),
           this, SLOT( slotNewReachList() ) );

  connect( logger, SIGNAL( logging( bool ) ),
           viewMap, SLOT( slot_setLoggerStatus() ) );
  connect( logger, SIGNAL( logging( bool ) ),
           this, SLOT( slot_Logging( bool ) ) );
  connect( logger, SIGNAL( madeEntry() ),
           viewMap, SLOT( slot_LogEntry() ) );

  calculator->setPosition( _globalMapMatrix->getMapCenter( false ) );

  slotReadconfig();

  // set the default glider to be the last one selected.
  calculator->setGlider( GliderListWidget::getStoredSelection() );
  QString gt = calculator->gliderType();

  if ( !gt.isEmpty() )
    {
      setWindowTitle ( "Cumulus - " + gt );
    }

  // @AP: That's a trick here! We call resize and make the drawing
  viewMap->_theMap->setDrawing( true );
  viewMap->_theMap->resize( 584, 460 );

  // This actions initiates the map loading procedures
  viewMap->_theMap->slotDraw();
  viewMap->_theMap->setDrawing( false );

  calculator->newSites();  // New sites have been loaded in map draw
  // this call is responsible for setting correct AGL/STD for manual mode,
  // must be called after viewMap->_theMap->draw(), there the AGL info is loaded
  // I do not connect since it is never emitted, only called once here
  calculator->slot_changePosition(MapMatrix::NotSet);

  if( ! GeneralConfig::instance()->getAirspaceWarningEnabled() )
    {
      int answer= QMessageBox::warning( this,tr("Airspace Warnings"),
                                        tr("<html><b>Airspace warnings are disabled!<br>"
                                           "Enable now?</b></html>"),
                                        QMessageBox::Yes | QMessageBox::No );

      if( answer == QMessageBox::Yes )
        {
          GeneralConfig::instance()->setAirspaceWarningEnabled(true);
        }

      QCoreApplication::processEvents();
      sleep(1);
    }

  setNearestOrReachableHeaders();

#ifdef MAEMO

  if( ossoContext )
    {
      osso_display_blanking_pause( ossoContext );

      // setup timer to prevent screen blank
      ossoDisplayTrigger = new QTimer(this);
      ossoDisplayTrigger->setSingleShot(true);

      connect( ossoDisplayTrigger, SIGNAL(timeout()),
               this, SLOT(slotOssoDisplayTrigger()) );

      // start timer with 10s
      ossoDisplayTrigger->start( 10000 );
    }

#endif

  splash->show();
  ws->slot_SetText1( tr( "Initializing GPS" ) );
  ws->show();

  QCoreApplication::processEvents();

  // Startup GPS client process now for data receiving
  GpsNmea::gps->blockSignals( false );
  GpsNmea::gps->startGpsReceiver();

  if( GeneralConfig::instance()->getLoggerAutostartMode() == true )
    {
      // set logger in standby mode
      logger->Standby();
    }

  // close wait screen
  ws->setScreenUsage( false );
  ws->hide();
  // closes and removes the splash screen
  splash->close();

  viewMap->_theMap->setDrawing( true );

  // set viewMap as central widget
  setCentralWidget( viewMap );

  // show map view as the central widget
  setView( mapView );

  // Make the status bar visible. Maemo do hide it per default.
  slotViewStatusBar( true );

  qDebug( "End startup CmulusApp" );
}

MainWindow::~MainWindow()
{
  // qDebug ("MainWindow::~MainWindow()");

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
void MainWindow::playSound( const char *name )
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

  player->start( QThread::HighestPriority );
}

void MainWindow::slotNotification( const QString& msg, const bool sound )
{
  if ( sound )
    {
      playSound("notify");
    }

  viewMap->slot_info( msg );
}

void MainWindow::slotAlarm( const QString& msg, const bool sound )
{
  if ( msg.isEmpty() )
    {
      return ;
    }

  if ( sound )
    {
      playSound("alarm");
    }

  viewMap->slot_info( msg );
}

void MainWindow::initMenuBar()
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
  // add a submenu for Label management
  labelMenu = viewMenu->addMenu( tr("Labels"));
  labelMenu->addAction( actionToggleAfLabels );
  labelMenu->addAction( actionToggleOlLabels );
  labelMenu->addAction( actionToggleTpLabels );
  labelMenu->addAction( actionToggleWpLabels );
  viewMenu->addSeparator();
  labelMenu->addAction( actionToggleLabelsInfo );

  viewMenu->addAction( actionToggleStatusbar );
  viewMenu->addSeparator();
  viewMenu->addAction( actionViewGPSStatus );

  mapMenu = menuBar()->addMenu(tr("&Map"));
  mapMenu->addAction( actionToggleLogging );
  mapMenu->addAction( actionSelectTask );
  mapMenu->addAction( actionManualNavHome );
  mapMenu->addAction( actionNav2Home );
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

  slotSetMenuBarFontSize();
}

/** set menubar font size to a reasonable and usable value */
void MainWindow::slotSetMenuBarFontSize()
{
  int minFontSize = 8;

  // sets the user's selected menu font, if defined
  QString fontString = GeneralConfig::instance()->getGuiMenuFont();
  QFont userFont;

  if( fontString == "" || userFont.fromString( fontString ) == false )
    {
      // take current font as alternative
      userFont = font();
    }

  if( userFont.pointSize() < minFontSize )
    {
      userFont.setPointSize( minFontSize );
    }

  menuBar()->setFont( userFont );

  // maybe NULL, if not initialized
  if( fileMenu ) fileMenu->setFont( userFont );
  if( viewMenu ) viewMenu->setFont( userFont );
  if( mapMenu ) mapMenu->setFont( userFont );
  if( setupMenu ) setupMenu->setFont( userFont );
  if( helpMenu ) helpMenu->setFont( userFont );
  if( labelMenu ) labelMenu->setFont( userFont );
}

/** initializes all QActions of the application */
void MainWindow::initActions()
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

  actionManualNavHome = new QAction( tr( "Goto home site" ), this );
  QList<QKeySequence> acGoHomeKeys;
  acGoHomeKeys << QKeySequence(Qt::SHIFT + Qt::Key_H) << QKeySequence::MoveToStartOfLine;
  actionManualNavHome->setShortcuts( acGoHomeKeys );

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

  actionGpsNavWPList = new QAction( tr( "Open waypoint list" ), this );
  actionGpsNavWPList->setShortcut( QKeySequence("F9") );
  addAction( actionGpsNavWPList );
  connect( actionGpsNavWPList, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToWPListView() ) );

  // Select home site as target
  actionNav2Home = new QAction( tr( "Select home site" ), this );
  actionNav2Home->setShortcut( QKeySequence(Qt::Key_H) );
  addAction( actionNav2Home );
  connect( actionNav2Home, SIGNAL( triggered() ),
           this, SLOT( slotNavigate2Home() ) );

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
  mBTSCList << Qt::Key_M << Qt::Key_Space << Qt::Key_F4;
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
  addAction( actionViewWaypoints );
  connect( actionViewWaypoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToWPListView() ) );

  actionViewAirfields = new QAction ( tr( "&Airfields" ), this );
  addAction( actionViewAirfields );
  connect( actionViewAirfields, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToAFListView() ) );

  actionViewReachpoints = new QAction ( tr( "&Reachable" ), this );
  actionViewReachpoints->setShortcut(Qt::Key_R);
  addAction( actionViewReachpoints );
  connect( actionViewReachpoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToReachListView() ) );

  actionViewTaskpoints = new QAction ( tr( "&Task" ), this );
  addAction( actionViewTaskpoints );
  connect( actionViewTaskpoints, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToTaskListView() ) );

  // Show info about selected target
  actionViewInfo = new QAction( tr( "&Info Target" ), this );
  actionViewInfo->setShortcut(Qt::Key_I);
  addAction( actionViewInfo );
  connect( actionViewInfo, SIGNAL( triggered() ),
           this, SLOT( slotSwitchToInfoView() ) );

  actionToggleStatusbar = new QAction( tr( "Status Bar" ), this );
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

  actionToggleAfLabels = new QAction ( tr( "Airfield labels" ), this);
  actionToggleAfLabels->setShortcut(Qt::Key_A);
  actionToggleAfLabels->setCheckable(true);
  actionToggleAfLabels->setChecked( GeneralConfig::instance()->getMapShowAirfieldLabels() );
  addAction( actionToggleAfLabels );
  connect( actionToggleAfLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleAfLabels( bool ) ) );

  actionToggleOlLabels = new QAction ( tr( "Outlanding labels" ), this);
  actionToggleOlLabels->setShortcut(Qt::Key_O);
  actionToggleOlLabels->setCheckable(true);
  actionToggleOlLabels->setChecked( GeneralConfig::instance()->getMapShowOutLandingLabels() );
  addAction( actionToggleOlLabels );
  connect( actionToggleOlLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleOlLabels( bool ) ) );

  actionToggleTpLabels = new QAction ( tr( "Taskpoint labels" ), this);
  actionToggleTpLabels->setShortcut(Qt::Key_T);
  actionToggleTpLabels->setCheckable(true);
  actionToggleTpLabels->setChecked( GeneralConfig::instance()->getMapShowTaskPointLabels() );
  addAction( actionToggleTpLabels );
  connect( actionToggleTpLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleTpLabels( bool ) ) );

  actionToggleWpLabels = new QAction ( tr( "Waypoint labels" ), this);
  actionToggleWpLabels->setShortcut(Qt::Key_W);
  actionToggleWpLabels->setCheckable(true);
  actionToggleWpLabels->setChecked( GeneralConfig::instance()->getMapShowWaypointLabels() );
  addAction( actionToggleWpLabels );
  connect( actionToggleWpLabels, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleWpLabels( bool ) ) );

  actionToggleLabelsInfo = new QAction (  tr( "Labels extra info" ), this);
  actionToggleLabelsInfo->setShortcut(Qt::Key_E);
  actionToggleLabelsInfo->setCheckable(true);
  actionToggleLabelsInfo->setChecked( GeneralConfig::instance()->getMapShowLabelsExtraInfo() );
  addAction( actionToggleLabelsInfo );
  connect( actionToggleLabelsInfo, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleLabelsInfo( bool ) ) );

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

  actionStartFlightTask = new QAction( tr( "Start flight task" ), this );
  actionStartFlightTask->setShortcut(Qt::Key_B);
  addAction( actionStartFlightTask );
  connect ( actionStartFlightTask, SIGNAL( triggered() ),
            calculator, SLOT( slot_startTask() ) );

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

void  MainWindow::toggleActions( const bool toggle )
{
  actionViewWaypoints->setEnabled( toggle );
  actionViewAirfields->setEnabled( toggle );
  actionViewGPSStatus->setEnabled( toggle );
  actionZoomInZ->setEnabled( toggle );
  actionZoomOutZ->setEnabled( toggle );
  actionToggleAfLabels->setEnabled( toggle );
  actionToggleOlLabels->setEnabled( toggle );
  actionToggleTpLabels->setEnabled( toggle );
  actionToggleWpLabels->setEnabled( toggle );
  actionToggleLabelsInfo->setEnabled( toggle );
  actionEnsureVisible->setEnabled( toggle );
  actionSelectTask->setEnabled( toggle );
  actionStartFlightTask->setEnabled( toggle );
  actionPreFlight->setEnabled( toggle );
  actionSetupConfig->setEnabled( toggle );
  actionSetupInFlight->setEnabled( toggle );
  actionHelpCumulus->setEnabled( toggle );
  actionHelpAboutApp->setEnabled( toggle );
  actionHelpAboutQt->setEnabled( toggle );
  actionToggleLogging->setEnabled( toggle );
  actionNav2Home->setEnabled( toggle );
  scExit->setEnabled( toggle );

  // do not toggle actionToggleManualInFlight, status may not be changed
  if( toggle )
    {
      GeneralConfig * conf = GeneralConfig::instance();
      actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );

      if( calculator->getselectedWp() )
        {
          // allow action only if a waypoint is selected
          actionViewInfo->setEnabled( toggle );
        }

      if ( _globalMapContents->getCurrentTask() )
        {
          // allow action only if a task is defined
          actionViewTaskpoints->setEnabled( toggle );
        }
    }
  else
    {
      actionViewReachpoints->setEnabled( toggle );
      actionViewInfo->setEnabled( toggle );
      actionViewTaskpoints->setEnabled( toggle );
    }
}

/**
 * Toggle actions depending on GPS connection.
 */
void MainWindow::toggleManualNavActions( const bool toggle )
{
  actionManualNavUp->setEnabled( toggle );
  actionManualNavRight->setEnabled( toggle );
  actionManualNavDown->setEnabled( toggle );
  actionManualNavLeft->setEnabled( toggle );
  actionManualNavHome->setEnabled( toggle );
  actionManualNavWP->setEnabled( toggle );
  actionManualNavWPList->setEnabled( toggle );
}

void MainWindow::toggleGpsNavActions( const bool toggle )
{
  actionGpsNavUp->setEnabled( toggle );
  actionGpsNavDown->setEnabled( toggle );
  actionGpsNavWPList->setEnabled( toggle );
  actionGpsNavZoomIn->setEnabled( toggle );
  actionGpsNavZoomOut->setEnabled( toggle );
}

void MainWindow::slotFileQuit()
{
  close();
}

/**
 * Make sure the user really wants to quit
 */
void MainWindow::closeEvent( QCloseEvent* evt )
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

  QMessageBox mb( QMessageBox::Question,
                  tr( "Terminating?" ),
                  tr( "Terminating Cumulus<br><b>Are you sure?</b>" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this,
                  Qt::Dialog );

  mb.setDefaultButton( QMessageBox::No );

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


void MainWindow::slotToggleMenu()
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

void MainWindow::slotToggleAfLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowAirfieldLabels( toggle );
  GeneralConfig::instance()->save();
  viewMap->_theMap->scheduleRedraw(Map::airports);
}

void MainWindow::slotToggleOlLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowOutLandingLabels( toggle );
  GeneralConfig::instance()->save();
  viewMap->_theMap->scheduleRedraw(Map::outlandingSites);
}

void MainWindow::slotToggleTpLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowTaskPointLabels( toggle );
  GeneralConfig::instance()->save();
  viewMap->_theMap->scheduleRedraw(Map::task);
}

void MainWindow::slotToggleWpLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowWaypointLabels( toggle );
  GeneralConfig::instance()->save();
  viewMap->_theMap->scheduleRedraw(Map::waypoints);
}

void MainWindow::slotToggleLabelsInfo( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowLabelsExtraInfo( toggle );
  GeneralConfig::instance()->save();
  viewMap->_theMap->scheduleRedraw(Map::airports);
}

void MainWindow::slotViewStatusBar( bool toggle )
{
  if ( toggle )
    viewMap->statusBar()->show();
  else
    viewMap->statusBar()->hide();
}

/** Called if the logging is actually toggled */
void MainWindow::slot_Logging ( bool logging )
{
  actionToggleLogging->blockSignals( true );
  actionToggleLogging->setChecked( logging );
  actionToggleLogging->blockSignals( false );
}

/** Read property of enum view. */
MainWindow::appView MainWindow::getView()
{
  return view;
}

/** Called if the user clicks on a tabulator of the list view */
void MainWindow::slot_tabChanged( int index )
{
  // qDebug("MainWindow::slot_tabChanged(): NewIndex=%d", index );

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
  else if ( index == listViewTabs->indexOf(viewOL) )
    {
      setView( olView );
    }
  else
    {
      qWarning("MainWindow::slot_tabChanged(): Cannot switch to index %d", index );
    }
}


/** Write property of internal view. */
void MainWindow::setView( const appView& newVal, const wayPoint* wp )
{
  // qDebug("MainWindow::setView called with argument %d", newVal);

  switch ( newVal )
    {

    case mapView:

      // @AP: set focus to MainWindow widget, otherwise F-Key events will
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

      toggleManualNavActions( GpsNmea::gps->getGpsStatus() != GpsNmea::validFix ||
                              calculator->isManualInFlight() );

      toggleGpsNavActions( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
                           !calculator->isManualInFlight() );

      actionMenuBarToggle->setEnabled( true );

      // Switch on all action shortcuts in this view
      toggleActions( true );
      viewMap->statusBar()->clearMessage(); // remove temporary status bar messages

      // If we returned to the map view, we should schedule a redraw of the
      // air spaces and the navigation layer. Map is not drawn, when invisible
      // and airspace filling, edited waypoints or tasks can be outdated in the
      // meantime.
      viewMap->_theMap->scheduleRedraw( Map::aeroLayer );

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
      {
        menuBar()->hide();
        viewMap->hide();
        viewInfo->hide();

        setNearestOrReachableHeaders();

        listViewTabs->setCurrentWidget( viewRP );
        listViewTabs->show();

        toggleManualNavActions( false );
        toggleGpsNavActions( false );
        actionMenuBarToggle->setEnabled( false );
        toggleActions( false );
      }

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

    case olView:

      menuBar()->hide();
      viewMap->hide();
      viewInfo->hide();
      viewOL->listWidget()->fillWpList();
      listViewTabs->setCurrentWidget( viewOL );
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
      viewInfo->showWP( view, *wp );

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
      // called if configuration or preflight widget was created
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
      qWarning( "MainWindow::setView(): unknown view %d to be set", newVal );
      return;
    }

  // save new view value
  view = newVal;
}

/**
 * Set nearest or reachable headers.
 */
void MainWindow::setNearestOrReachableHeaders()
{
  // Set the tabulator header according to calculation result.
  // If a glider is known, reachables by L/D are shown
  // otherwise the nearest points in 75 km radius are shown.
  QString header = QString(tr( "Reachable" ));

  if( calculator->getReachList()->getCalcMode() == ReachableList::distance )
    {
      // nearest site are calculated
      header = QString(tr( "Nearest" ));
    }

  // update menu display
  actionViewReachpoints->setText( QString("&") + header );

  // update list view tabulator header
  listViewTabs->setTabText( _taskListVisible ? 2 : 1, header );
}

/** Switches to mapview. */
void MainWindow::slotSwitchToMapView()
{
  setView( mapView );
}


/** Switches to the WaypointList View */
void MainWindow::slotSwitchToWPListView()
{
  setView( wpView );
}


/** Switches to the WaypointList View if there is
 * no task, and to the task list if there is .
 */
void MainWindow::slotSwitchToWPListViewExt()
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
void MainWindow::slotSwitchToAFListView()
{
  setView( afView );
}

/** Switches to the OutlandingList View */
void MainWindow::slotSwitchToOLListView()
{
  setView( olView );
}

/** Switches to the ReachablePointList View */
void MainWindow::slotSwitchToReachListView()
{
  setView( rpView );
}


/** Switches to the WaypointList View */
void MainWindow::slotSwitchToTaskListView()
{
  setView( tpView );
}


/** This slot is called to switch to the info view. */
void MainWindow::slotSwitchToInfoView()
{
//  qDebug("MainWindow::slotSwitchToInfoView()");
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
  if ( view == olView )
    {
      setView( infoView, viewOL->getSelectedWaypoint() );
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
void MainWindow::slotSwitchToInfoView( wayPoint* wp )
{
  if( wp )
    {
      setView( infoView, wp );
    }
}


/** Opens the configuration widget */
void MainWindow::slotConfig()
{
  setWindowTitle( "Cumulus Settings" );
  ConfigWidget *cDlg = new ConfigWidget( this );
  cDlg->resize( size() );
  configView = static_cast<QWidget *> (cDlg);

  setView( cfView );

  connect( cDlg, SIGNAL( settingsChanged() ),
           this, SLOT( slotReadconfig() ) );
  connect( cDlg, SIGNAL( closeConfig() ),
           this, SLOT( slotCloseConfig() ) );
  connect( cDlg,  SIGNAL( welt2000ConfigChanged() ),
           _globalMapContents, SLOT( slotReloadWelt2000Data() ) );
  connect( cDlg, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  cDlg->show();
}


/** Closes the configuration or pre-flight widget */
void MainWindow::slotCloseConfig()
{
  setView( mapView );
//  qDebug("* closing configView:  %s", calculator->gliderType().toLatin1().data() );
  if ( !calculator->gliderType().isEmpty() )
    setWindowTitle ( "Cumulus - " + calculator->gliderType() );
  else
    setWindowTitle( "Cumulus" );
}

/** Shows version and copyright information */
void MainWindow::slotVersion()
{
  QMessageBox::about ( this,
                       "Cumulus",
                       QString(tr(
                                 "<html>"
                                 "Cumulus X11 version %1<br>"
                                 "<font size=-1>(compiled at %2 with QT %3)</font><br>"
                                 "&copy; 2002-2010 by<br>"
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
void MainWindow::slotHelp()
{
  HelpBrowser *hb = new HelpBrowser(this);
  hb->setAttribute(Qt::WA_DeleteOnClose);
  hb->resize( this->size() );
  hb->setWindowState( windowState() );
  hb->show();
}

void MainWindow::slotRememberWaypoint()
{
  static uint count = 1;
  QString name;

  name = tr( "WP%1-%2" ).arg( count ).arg( QTime::currentTime().toString().left( 5 ) );

  // @AP: let us check, if the user waypoint is already known from its
  // position. In this case we will reject the insertion to avoid senseless
  // duplicates.

  QPoint pos = calculator->getlastPosition();

  QList<wayPoint>& wpList = _globalMapContents->getWaypointList();

  for ( int i = 0; i < wpList.count(); i++ )
    {
      const wayPoint &wpItem = wpList.at(i);

      if ( wpItem.origP == pos )
        {
          return ; // we have such position already
        }
    }

  count++;

  wayPoint wp;

  wp.name = name;
  wp.origP = calculator->getlastPosition();
  wp.projP = _globalMapMatrix->wgsToMap( wp.origP );
  wp.description = tr( "user created" );
  wp.comment = tr("created by remember action at ") +
  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  wp.importance = wayPoint::High; // high to make sure it is visible
  wp.frequency = 0.0;
  wp.runway = 0;
  wp.length = 0;
  AltitudeCollection alt = calculator->getAltitudeCollection();
  wp.elevation = int ( ( alt.gpsAltitude - alt.gndAltitude ).getMeters() );
  wp.type = BaseMapElement::Turnpoint;
  wp.isLandable = false;

  viewWP->slot_wpAdded( wp );

  // qDebug("WP lat=%d, lon=%d", wp.origP.lat(), wp.origP.lon() );
}

/** This slot is called if the configuration has been changed and at the
    start of the program to read the initial configuration. */
void MainWindow::slotReadconfig()
{
  // qDebug("MainWindow::slotReadconfig()");

  // other configuration changes
  _globalMapMatrix->slotInitMatrix();
  viewMap->slot_settingsChange();
  calculator->slot_settingsChanged();
  viewTP->slot_updateTask();

  if ( _globalMapContents->getCurrentTask() != static_cast<FlightTask *> (0) )
    {
      // set the current task again, time zone could be changed
      viewTP->slot_setTask( _globalMapContents->getCurrentTask() );
    }

  viewRP->fillRpList();
  viewAF->listWidget()->configRowHeight();
  viewAF->listWidget()->slot_Done();
  viewOL->listWidget()->configRowHeight();
  viewOL->listWidget()->slot_Done();
  viewWP->listWidget()->configRowHeight();
  viewWP->listWidget()->slot_Done();

  GeneralConfig *conf = GeneralConfig::instance();

  // Update action settings
  actionToggleAfLabels->setChecked( conf->getMapShowAirfieldLabels() );
  actionToggleOlLabels->setChecked( conf->getMapShowOutLandingLabels() );
  actionToggleTpLabels->setChecked( conf->getMapShowTaskPointLabels() );
  actionToggleWpLabels->setChecked( conf->getMapShowWaypointLabels() );
  actionToggleLabelsInfo->setChecked( conf->getMapShowLabelsExtraInfo() );

  // configure reconnect of GPS receiver in case of process stop
  QString device = conf->getGpsDevice();

  if( device.startsWith("/dev/" ) )
    {
      // @ee install signal handler
      signal( SIGCONT, resumeGpsConnection );
    }

  GpsNmea::gps->slot_reset();

  // update menubar font size
  slotSetMenuBarFontSize();

  actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );

 // Check, if reachable list is to show or not
  if( conf->getNearestSiteCalculatorSwitch() )
    {
      if( ! _reachpointListVisible )
        {
          // list was not visible before
          listViewTabs->insertTab( _taskListVisible ? 2 : 1, viewRP, tr( "Reachable" ) );
          calculator->newSites();
          _reachpointListVisible = true;
        }
    }
  else
    {
      if( _reachpointListVisible )
        {
          // changes in listViewTabs trigger  (if viewRP was last active),
          // this slot calls setView and tries to set the view to viewRP
          // but since this doesn't exist (removeWidget), sets the view to the next one
          // which is viewAF; that's the reason we have to a) call setView(mapView);
          // or b) disconnect before removeWidget and connect again behind
          listViewTabs->blockSignals( true );
          listViewTabs->removeTab( listViewTabs->indexOf(viewRP) );
          listViewTabs->blockSignals( false );

          calculator->clearReachable();
          viewRP->clearList(); // this clears the reachable list in the view
          viewMap->_theMap->scheduleRedraw(Map::waypoints);
          _reachpointListVisible = false;
        }
    }

  // Check, if outlanding list is to show or not
  if( conf->getWelt2000LoadOutlandings() )
    {
      if( ! _outlandingListVisible )
        {
          // list was not visible before, add it to the end of the tabs
          listViewTabs->addTab( viewOL, tr( "Outlandings" ) );
          _outlandingListVisible = true;
        }
    }
  else
    {
      if( _outlandingListVisible )
        {
          listViewTabs->blockSignals( true );
          listViewTabs->removeTab( listViewTabs->indexOf(viewOL) );
          listViewTabs->blockSignals( false );
          viewRP->clearList();  // this clears the outlanding list in the view
          viewMap->_theMap->scheduleRedraw(Map::outlandingSites);
          _outlandingListVisible = false;
        }
    }

  viewMap->_theMap->scheduleRedraw();
}


/** Called if the status of the GPS changes, and controls the availability of manual navigation. */
void MainWindow::slotGpsStatus( GpsNmea::GpsStatus status )
{
  switch ( status )
    {
    case GpsNmea::validFix:
      viewMap->message( tr( "GPS fix established" ) );
      break;
    case GpsNmea::noFix:
      viewMap->message(tr( "GPS connection - no fix" ) );
      break;
    default:
      viewMap->message( tr( "GPS lost" ) );
    }

  playSound("notify");

  if ( ( status != GpsNmea::validFix || calculator->isManualInFlight()) && ( view == mapView ) )
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
void MainWindow::slotCenterToWaypoint()
{

  if ( calculator->getselectedWp() )
    {
      _globalMapMatrix->centerToLatLon( calculator->getselectedWp()->origP );
      viewMap->_theMap->scheduleRedraw();

    }
}


/** Called if the user pressed V in mapview.
  * Adjusts the zoom factor so that the currently selected waypoint
  * is displayed as good as possible. */
void MainWindow::slotEnsureVisible()
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
void MainWindow::slotPreFlightGlider()
{
  slotPreFlight("gliderselection");
}


/** Opens the preflight dialog and brings the selected tabulator in foreground */
void MainWindow::slotPreFlightTask()
{
  slotPreFlight("taskselection");
}


/** Opens the pre-flight widget and brings the selected tabulator to the front */
void MainWindow::slotPreFlight(const char *tabName)
{
  setWindowTitle( "Pre-Flight Settings" );
  PreFlightWidget* cDlg = new PreFlightWidget( this, tabName );
  cDlg->setObjectName("PreFlightDialog");
  cDlg->resize( size() );
  configView = static_cast<QWidget *> (cDlg);

  setView( cfView );

  connect( cDlg, SIGNAL( settingsChanged() ),
           this, SLOT( slotPreFlightDataChanged() ) );

  connect( cDlg, SIGNAL( settingsChanged() ),
           IgcLogger::instance(), SLOT( slotReadConfig() ) );

  connect( cDlg, SIGNAL( newWaypoint( wayPoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( wayPoint*, bool ) ) );

  connect( cDlg, SIGNAL( closeConfig() ),
           this, SLOT( slotCloseConfig() ) );

  cDlg->show();
}


void MainWindow::slotPreFlightDataChanged()
{
  if ( _globalMapContents->getCurrentTask() == static_cast<FlightTask *> (0) )
    {
      if ( _taskListVisible )
        {
          // see comment for removeTab( viewRP )
          listViewTabs->blockSignals( true );
          listViewTabs->removeTab( listViewTabs->indexOf(viewTP) );
          listViewTabs->blockSignals( false );
          _taskListVisible = false;
        }
    }
  else
    {
      if ( !_taskListVisible )
        {
          listViewTabs->insertTab( 0, viewTP, tr( "Task" ) );
          _taskListVisible = true;
        }
    }

  // set the task list view at the current task
  viewTP->slot_setTask( _globalMapContents->getCurrentTask() );
  viewMap->_theMap->scheduleRedraw(Map::task);
}

/** Dynamically updates view for reachable list */
void MainWindow::slotNewReachList()
{
  // qDebug( "MainWindow::slotNewReachList() is called" );

  viewRP->slot_newList(); //let the view know we have a new list
  viewMap->_theMap->scheduleRedraw(Map::waypoints);
}

bool MainWindow::eventFilter( QObject *o , QEvent *e )
{
  // qDebug("MainWindow::eventFilter() is called with event type %d", e->type());

  if ( e->type() == QEvent::KeyPress )
    {
      QKeyEvent *k = ( QKeyEvent* ) e;

      qDebug( "Keycode of pressed key: %d, %X", k->key(), k->key() );

      if( k->key() == Qt::Key_F6 )
        {
          // Hardware Key F6 for maximize/normalize screen under Maemo.
          setWindowState(windowState() ^ Qt::WindowFullScreen);
          return true;
        }
    }

  return QWidget::eventFilter( o, e ); // standard event processing;
}

/** Called to select the home site position */
void MainWindow::slotNavigate2Home()
{
  wayPoint wp;

  wp.name = tr("Home");
  wp.description = tr("Home Site");
  wp.origP.setPos( GeneralConfig::instance()->getHomeCoord() );

  calculator->slot_WaypointChange( &wp, true );
}

void MainWindow::slotToggleManualInFlight(bool on)
{
  // if we have lost the GPS fix, actionToggleManualInFlight is disabled from calculator
  // so we only can switch off if GPS fix available
  calculator->setManualInFlight(on);
  toggleManualNavActions( on );
  toggleGpsNavActions( !on );
}

/** Used to allow or disable user keys processing during map drawing. */
void MainWindow::slotMapDrawEvent( bool drawEvent )
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
           toggleManualNavActions( GpsNmea::gps->getGpsStatus() != GpsNmea::validFix ||
                                   calculator->isManualInFlight() );

           toggleGpsNavActions( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
                                !calculator->isManualInFlight() );
         }
     }
}

// resize the list view tabs, if requested
void MainWindow::resizeEvent(QResizeEvent* event)
{
  qDebug("MainWindow::resizeEvent(): w=%d, h=%d", event->size().width(), event->size().height() );
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
void MainWindow::slotOssoDisplayTrigger()
{
  // If the speed is greater or equal 10 km/h and we have a connected
  // gps we switch off the screen saver. Otherwise we let all as it
  // is.
  double speedLimit = GeneralConfig::instance()->getScreenSaverSpeedLimit();

  if( calculator->getLastSpeed().getKph() >= speedLimit &&
      GpsNmea::gps->getConnected() )
    {
      // tells Maemo that we are in move enough to switch off or avoid blank screen
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
