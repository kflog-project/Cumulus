/***************************************************************************
 mainwindow.cpp  -  main application window
 ---------------------------------------------------------------------------
 begin                : Sun Jul 21 2002

 copyright            : (C) 2002-2007 by André Somers

                      : (C) 2007-2021 by Axel Pauli

 maintainer           : Axel Pauli <kflog.cumulus@gmail.com>

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

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <sys/ioctl.h>

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "aboutwidget.h"
#include "airfield.h"
#include "calculator.h"
#include "configwidget.h"
#include "generalconfig.h"
#include "gliderlistwidget.h"
#include "helpbrowser.h"
#include "layout.h"
#include "ListViewTabs.h"
#include "mainwindow.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "messagewidget.h"
#include "preflightwidget.h"
#include "sound.h"
#include "target.h"
#include "TaskFileManager.h"
#include "time_cu.h"
#include "waypoint.h"
#include "wgspoint.h"
#include "windanalyser.h"
#include "wpeditdialog.h"

#ifdef INTERNET
#include "DownloadManager.h"
#endif

#ifdef ANDROID

#include "androidevents.h"
#include "jnisupport.h"

#endif

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

/** Define the disclaimer version */
#define DISCLAIMER_VERSION 1

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

/**
 * Contains the root window flag.
 */
bool MainWindow::m_rootWindow = true;

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

MainWindow::MainWindow( Qt::WindowFlags flags ) :
  QMainWindow( 0, flags ),
  viewMap(0),
  viewWP(0),
  viewAF(0),
  viewHS(0),
  viewOL(0),
  viewNA(0),
  viewRP(0),
  viewTP(0),
  m_listViewTabs(0),
  view(mapView),
  actionToggleGps(0),
  actionManualNavUp(0),
  actionManualNavRight(0),
  actionManualNavDown(0),
  actionManualNavLeft(0),
  actionManualNavMove2Home(0),
  actionManualNavMove2WP(0),
  actionManualNavWPList(0),
  actionGpsNavUp(0),
  actionGpsNavDown(0),
  actionNav2Home(0),
  actionGpsNavWPList(0),
  actionGpsNavZoomIn(0),
  actionGpsNavZoomOut(0),
  actionOpenContextMenu(0),
  actionFileQuit(0),
#ifdef ANDROID
  actionHardwareMenu(0),
#endif
  actionViewInfo(0),
  actionViewWaypoints(0),
  actionViewAirfields(0),
  actionViewHotspots(0),
  actionViewNavAids(0),
  actionViewOutlandings(0),
  actionViewReachpoints(0),
  actionViewTaskpoints(0),
#ifdef FLARM
  actionViewFlarm(0),
#endif
  actionStatusGPS(0),
  actionStatusAirspace(0),
  actionZoomInZ(0),
  actionZoomOutZ(0),
  actionToggleStatusbar(0),
  actionToggleMapSidebar(0),
  actionToggleWindowSize(0),
  actionToggleAfLabels(0),
  actionToggleOlLabels(0),
  actionToggleNaLabels(0),
  actionToggleTpLabels(0),
  actionToggleWpLabels(0),
  actionToggleLabelsInfo(0),
  actionToggleLogging(0),
  actionToggleTrailDrawing(0),
  actionEnsureVisible(0),
  actionRemoveTarget(0),
  actionSelectTask(0),
  actionPreFlight(0),
  actionSetupConfig(0),
  actionSetupInFlight(0),
  actionHelpCumulus(0),
  actionHelpAboutApp(0),
  actionHelpAboutQt(0),
  actionStartFlightTask(0),
  scToggleMapSidebar(0),
  scExit(0),
  contextMenu(0),
  fileMenu(0),
  viewMenu(0),
  mapMenu(0),
  statusMenu(0),
  labelMenu(0),
  labelSubMenu(0),
  setupMenu(0),
  helpMenu(0),
  ws(0),
  splash(0),
  configView(0),
  m_menuBarVisible(false),
  m_logger(static_cast<IgcLogger *> (0)),
  m_reachpointListVisible(false),
  m_outlandingListVisible(false),
#ifdef INTERNET
  m_liveTrackLogger(0),
#endif
  m_firstStartup( false )
{
  _globalMainWindow = this;

#if defined ANDROID || defined MAEMO
  m_displayTrigger = static_cast<QTimer *> (0);
#endif

  // This is used to make it possible to reset some user configuration items once
  // or to execute a necessary migration.
  int rc = GeneralConfig::instance()->getResetConfiguration();

  if( rc != 3 )
    {
      GeneralConfig::instance()->setResetConfiguration(3);

      // Migrate glider list by using this constructor
      GliderListWidget glw;
    }

  // Rename the map/airfields directory to map/points. This check is always
  // as first executed.
  QStringList mapDirs = GeneralConfig::instance()->getMapDirectories();

  for( int i = 0; i < mapDirs.size(); ++i )
    {
      QString dirNameOld = mapDirs.at( i ) + "/airfields";

      QDir dir( dirNameOld );

      if( dir.exists() )
	{
	  QString dirNameNew = mapDirs.at( i ) + "/points";

	  int ok = rename( dirNameOld.toLatin1().data(), dirNameNew.toLatin1().data() );

	  if( ok == 0 )
	    {
	      qDebug() << "Renaming" << dirNameOld << "-->" << dirNameNew;
	    }
	}
    }

#ifdef ANDROID

  // Check Android Gui fonts for existence to force a recalculation if needed.
  QFontDatabase fdb;

  QString gfs = GeneralConfig::instance()->getGuiFont();

  if( gfs.isEmpty() == false )
    {
      QFont ft;
      bool ok = ft.fromString( gfs );

      if( ok == false )
        {
          // The defined font is not available. We reset it to force a recalculation.
          GeneralConfig::instance()->setGuiFont( "" );
        }
      else
        {
          QList<int> psl = fdb.pointSizes( ft.family(), fdb.styleString(ft) );

          qDebug() << "GuiFont" << gfs << "Points" << psl;

          if( psl.size() == 0 )
            {
              // The defined font is not available. We reset it to force a recalculation.
              GeneralConfig::instance()->setGuiFont( "" );

              qWarning() << "GuiFont" << gfs << "not available, force font reset!";
            }
        }
    }

  QString mfs = GeneralConfig::instance()->getGuiMenuFont();

  if( mfs.isEmpty() == false )
    {
      QFont tf;
      bool ok = tf.fromString( mfs );

      if( ok == false )
        {
          // The defined font is not available. We reset it to force a recalculation.
          GeneralConfig::instance()->setGuiMenuFont( "" );
        }
      else
        {
          QList<int> psl = fdb.pointSizes( tf.family(), fdb.styleString(tf) );

          qDebug() << "GuiMenuFont" << mfs << "Points" << psl;

          if( psl.size() == 0 )
            {
              // The defined font is not available. We reset it to force a recalculation.
              GeneralConfig::instance()->setGuiMenuFont( "" );

              qWarning() << "GuiMenuFont" << mfs << "not available, force font reset!";
            }
        }
    }

#endif

  // Get application font for user adaptions.
  QFont appFt = QApplication::font();

  qDebug( "Default QAppFont: Family %s, ptSize=%d, pxSize=%d, weight=%d, height=%dpx",
           appFt.family().toLatin1().data(),
           appFt.pointSize(),
           appFt.pixelSize(),
           appFt.weight(),
           QFontMetrics(appFt).height() );

  QString fontString = GeneralConfig::instance()->getGuiFont();
  QFont userFont;

  if( fontString.isEmpty() )
    {
      // No Gui font is defined, we try to define a senseful default.
      QFont appFont = QApplication::font();

#ifdef ANDROID

  // Android knows normally only two fonts:
  // a) Droid Sans
  // b) Roboto
  //
  // If a wrong font is set umlauts maybe not correct displayed!

  if( fdb.weight("Roboto Condensed", "Normal") != -1 )
    {
      // LG Nexus 5
      appFont.setFamily( "Roboto Condensed" );
    }
  else if( fdb.weight("Roboto-Regular", "Normal") != -1 )
    {
      // Samsung Galaxy S3
      appFont.setFamily( "Roboto-Regular" );
    }
  else if( fdb.weight("Roboto", "Normal") != -1 )
    {
      // Samsung Galaxy Tab 2, Dell Streak 5
      appFont.setFamily( "Roboto" );
    }
  else if( fdb.weight("Droid Serif-Regular", "Normal") != -1 )
    {
      appFont.setFamily( "Droid Serif-Regular" );
    }
  else if( fdb.weight("Droid Serif", "Normal") != -1 )
    {
      appFont.setFamily( "Droid Serif" );
    }
  else
    {
      qDebug() << "Android: No system font found, loading own font data from DroidSans";

      int res = QFontDatabase::addApplicationFont( ":/fonts/DroidSans.ttf" );

      if( res == -1 )
        {
          qWarning() << "Could not load font :/fonts/DroidSans.ttf";
        }
      else
        {
          appFont.setFamily( "DroidSans" );
        }

      res = QFontDatabase::addApplicationFont( ":/fonts/DroidSans-Bold.ttf" );

      if( res == -1 )
        {
          qWarning() << "Could not load font :/fonts/DDroidSans-Bold.ttf";
        }
    }

#else

#ifdef MAEMO
      appFont.setFamily("Nokia Sans");
#else
      appFont.setFamily("Sans Serif");
#endif

#endif

      appFont.setWeight( QFont::Normal );
      appFont.setStyle( QFont::StyleNormal );

      Layout::fitGuiFont( appFont );
      QApplication::setFont( appFont );
      GeneralConfig::instance()->setGuiFont( appFont.toString() );
    }
  else if( userFont.fromString( fontString ) )
    {
      // take the user's defined font
      QApplication::setFont( userFont );
    }

  appFt = QApplication::font();

  qDebug( "Used QAppFont: Family %s, ptSize=%d, pxSize=%d, weight=%d, fmHeight=%dpx",
           appFt.family().toLatin1().data(),
           appFt.pointSize(),
           appFt.pixelSize(),
           appFt.weight(),
           QFontMetrics(appFt).height() );

  Layout::fontPoints2Pixel( appFt );

  // For Maemo it's really better to adapt the size of some common widget
  // elements. That is done with the help of the class MaemoStyle.
  qDebug() << "GuiStyles:" << QStyleFactory::keys();

  // Set GUI style and style proxy.
  GeneralConfig::instance()->setOurGuiStyle();

#ifdef ANDROID

  // Overwrite some style items globally. Note the all new style items must be
  // defined in one string!
  QString style = "QDialog { background: lightgray }";
  qApp->setStyleSheet( style );

#endif

  qDebug() << "AutoSipEnabled:" << qApp->autoSipEnabled();

#ifdef MAEMO

  // That we do need for the location service. This service emits signals, which
  // are bound to a g_object. This call initializes the g_object handling.
  g_type_init();

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

#if defined MAEMO || defined ANDROID

  QSize dSize = QApplication::desktop()->availableGeometry().size();

  if( dSize == QSize(0, 0) )
    {
      // Qt5 for Android sets the available geometry sometimes to 0x0
      // Found out that the available geometry is still set after the widget
      // becomes visible.
      dSize = QApplication::desktop()->screenGeometry().size();
    }

  // Limit maximum size for Maemo and Android
  setMaximumSize( dSize );

#ifdef ANDROID
  setMinimumSize( dSize );
#endif

  resize( dSize );
#else
  // get last saved window geometric from GeneralConfig and set it again
  resize( GeneralConfig::instance()->getWindowSize() );
#endif

#ifdef MAEMO
  setWindowState(Qt::WindowFullScreen);
#endif

  qDebug() << "Cumulus Release:"
           << QCoreApplication::applicationVersion()
           << "Build date:"
           << GeneralConfig::instance()->getBuiltDate()
           << "based on Qt Version"
           << QT_VERSION_STR;

  qDebug( "ScreenGeometry is %dx%d, width=%d, height=%d",
           QApplication::desktop()->screenGeometry().width(),
           QApplication::desktop()->screenGeometry().height(),
           QApplication::desktop()->screenGeometry().width(),
           QApplication::desktop()->screenGeometry().height() );

  qDebug( "AvailableGeometry is %dx%d, width=%d, height=%d",
           QApplication::desktop()->availableGeometry().width(),
           QApplication::desktop()->availableGeometry().height(),
           QApplication::desktop()->availableGeometry().width(),
           QApplication::desktop()->availableGeometry().height() );

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
  char *Proxy = getenv( "HTTP_PROXY" );

  qDebug( "PWD=%s", pwd ? pwd : "NULL" );
  qDebug( "USER=%s", user ? user : "NULL" );
  qDebug( "HOME=%s", home ? home : "NULL" );
  qDebug( "LANG=%s", lang ? lang : "NULL" );
  qDebug( "LD_LIBRARY_PATH=%s", ldpath ? ldpath : "NULL" );
  qDebug( "QDir::homePath()=%s", QDir::homePath().toLatin1().data() );
  qDebug( "DISPLAY=%s", display ? display : "NULL" );
  qDebug( "http_proxy=%s", proxy ? proxy : "NULL" );
  qDebug( "HTTP_PROXY=%s", Proxy ? Proxy : "NULL" );

  qDebug( "UserDataDir=%s",
           GeneralConfig::instance()->getUserDataDirectory().toLatin1().data() );

  qDebug( "MapRootDir=%s",
           GeneralConfig::instance()->getMapRootDir().toLatin1().data() );

  setFocusPolicy( Qt::StrongFocus );
  setFocus();

  setWindowIcon( QIcon(GeneralConfig::instance()->loadPixmap("cumulus-desktop26x26.png")) );

  // As next setup a timer and return. That will start the QtMainLoop.
  // If that is not done in this way, some functionality of the GUI seems
  // not to be stable resp. not usable.
  if( GeneralConfig::instance()->getDisclaimerVersion() != DISCLAIMER_VERSION )
    {
      QTimer::singleShot(100, this, SLOT(slotCreateDisclaimer()));
    }
  else
    {
      QTimer::singleShot(100, this, SLOT(slotCreateSplash()));
    }
}

/**
 * Creates the disclaimer query widget.
 */
void MainWindow::slotCreateDisclaimer()
{
  m_firstStartup = true;

  setWindowTitle( tr("Disclaimer") );

  QString disclaimer =
      QObject::tr(
        "<html>"
        "This program comes with"
        "<div align=\"center\">"
        "<p><b>ABSOLUTELY NO WARRANTY!</b></p>"
        "</div>"
        "Do not rely on this software program as your "
        "primary source of navigation. As pilot in command you are "
        "responsible for using official aeronautical "
        "charts and proper methods for safe navigation. "
        "The information presented in this software "
        "program may be outdated or incorrect.<br><br>"
        "<div align=\"center\">"
        "<b>Do You accept these terms?</b>"
        "</div>"
        "</html>");

  MessageWidget *mw = new MessageWidget( disclaimer, this );
  connect( mw, SIGNAL(yesClicked()), SLOT(slotCreateSplash()) );
  connect( mw, SIGNAL(noClicked()), SLOT(slotDisclaimerQuit()));

  setCentralWidget( mw );
  setVisible( true );
}

void MainWindow::slotDisclaimerQuit()
{
  // This slot is called, if the disclaimer was rejected by the user.
  // We quit the application now.
  qWarning() << "Disclaimer was rejected by the user. Abort further processing!";

#ifdef ANDROID
      jniShutdown();
#endif

  hide();
  qApp->quit();
}

/**
 * Creates the splash screen.
 */
void MainWindow::slotCreateSplash()
{
  setWindowTitle( "Cumulus" );

  GeneralConfig *conf = GeneralConfig::instance();
  conf->setDisclaimerVersion( DISCLAIMER_VERSION );
  conf->save();

  splash = new Splash( this );
  setCentralWidget( splash );
  splash->setVisible( true );
  setVisible( true );

  ws = new WaitScreen(this);

#ifdef ANDROID
  // The waitscreen is not centered over the parent and not limited in
  // its size under Android. Therefore this must be done by our self.
  ws->setGeometry ( width() / 2 - 250, height() / 2 - 75,  500, 150 );
#endif

  ws->slot_SetText1( tr( "Starting Cumulus..." ) );

  QCoreApplication::flush();

  // Here we finish the base initialization and start a timer
  // to continue startup in another method. This is done, to return
  // to the window's manager event loop. Otherwise the behavior
  // of some widgets is undefined.

  // when the timer expires the cumulus startup is continued
  QTimer::singleShot(500, this, SLOT(slotCreateApplicationWidgets()));
}

/**
 * Creates the application widgets after the base initialization
 * of the core application window.
 */
void MainWindow::slotCreateApplicationWidgets()
{
  qDebug() << "MainWindow::slotCreateApplicationWidgets()";

#ifdef MAEMO

  ossoContext = osso_initialize( "org.kflog.Cumulus",
                                 QCoreApplication::applicationVersion().toAscii().data(),
                                 false,
                                 0 );

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

  Airfield::createStaticIcons();

  calculator = new Calculator( this );

  connect( _globalMapMatrix, SIGNAL( displayMatrixValues( int, bool ) ),
           _globalMapConfig, SLOT( slotSetMatrixValues( int, bool ) ) );

  connect( _globalMapMatrix, SIGNAL( homePositionChanged() ),
           _globalMapContents, SLOT( slotReloadOpenAipPoi() ) );

  connect( _globalMapMatrix, SIGNAL( homePositionChanged() ),
           calculator, SLOT( slot_CheckHomeSiteSelection() ) );

  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           calculator, SLOT( slot_CheckHomeSiteSelection() ) );

  connect( _globalMapMatrix, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  ws->slot_SetText1( tr( "Creating views..." ) );

  // This is the main widget of Cumulus
  viewMap = new MapView( this );
  viewMap->setVisible( false );

  connect( viewMap, SIGNAL(openingSubWidget()), SLOT(slotSubWidgetOpened()) );
  connect( viewMap, SIGNAL(closingSubWidget()), SLOT(slotSubWidgetClosed()) );

  _globalMapView = viewMap;
  view = mapView;

  m_listViewTabs = new ListViewTabs( this );

  viewAF = m_listViewTabs->viewAF;
  viewHS = m_listViewTabs->viewHS;
  viewOL = m_listViewTabs->viewOL;
  viewNA = m_listViewTabs->viewNA;
  viewRP = m_listViewTabs->viewRP;
  viewWP = m_listViewTabs->viewWP;
  viewTP = m_listViewTabs->viewTP;

  connect( m_listViewTabs, SIGNAL(hidingWidget()), SLOT(slotSubWidgetClosed()) );

  // create GPS instance
  GpsNmea::gps = new GpsNmea( this );
  GpsNmea::gps->blockSignals( true );
  m_logger = IgcLogger::instance();

#ifdef INTERNET
  // create a live tracking instance
  m_liveTrackLogger = new LiveTrack24Logger( this );
#endif

  createActions();
  createContextMenu();

  ws->slot_SetText1( tr( "Setting up connections..." ) );

  // create connections between the components
  connect( _globalMapMatrix, SIGNAL( projectionChanged() ),
           _globalMapContents, SLOT( slotReloadMapData() ) );

  connect( _globalMapContents, SIGNAL( mapDataReloaded(Map::mapLayer) ),
           Map::instance, SLOT( slotRedraw(Map::mapLayer) ) );

  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewAF, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewHS, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewOL, SLOT( slot_reloadList() ) );
  connect( _globalMapContents, SIGNAL( mapDataReloaded() ),
           viewNA, SLOT( slot_reloadList() ) );
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
  connect( GpsNmea::gps, SIGNAL( newSatCount(SatInfo&) ),
           calculator->getWindAnalyser(), SLOT( slot_newConstellation(SatInfo&) ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           this, SLOT( slotGpsStatus( GpsNmea::GpsStatus ) ) );
  connect( GpsNmea::gps, SIGNAL( newSatConstellation(SatInfo&) ),
           m_logger, SLOT( slotConstellation(SatInfo&) ) );
  connect( GpsNmea::gps, SIGNAL( newSatConstellation(SatInfo&) ),
           calculator->getWindAnalyser(), SLOT( slot_newConstellation(SatInfo&) ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           calculator->getWindAnalyser(), SLOT( slot_gpsStatusChange( GpsNmea::GpsStatus ) ) );
  connect( GpsNmea::gps, SIGNAL( newSpeed(Speed&) ),
           calculator, SLOT( slot_Speed(Speed&) ) );
  connect( GpsNmea::gps, SIGNAL( newTas(const Speed&) ),
           calculator, SLOT( slot_GpsTas(const Speed&) ) );
  connect( GpsNmea::gps, SIGNAL( newPosition(QPoint&) ),
           calculator, SLOT( slot_Position(QPoint&) ) );
  connect( GpsNmea::gps, SIGNAL( newAltitude(Altitude&, Altitude&, Altitude&) ),
           calculator, SLOT( slot_Altitude(Altitude&, Altitude&, Altitude&) ) );
  connect( GpsNmea::gps, SIGNAL( newAndroidAltitude(const Altitude&) ),
           calculator, SLOT( slot_AndroidAltitude(const Altitude&) ) );
  connect( GpsNmea::gps, SIGNAL( newHeading(const double&) ),
           calculator, SLOT( slot_Heading(const double&) ) );
  connect( GpsNmea::gps, SIGNAL( newFix(const QDateTime&) ),
           calculator, SLOT( slot_newFix(const QDateTime&) ) );
  connect( GpsNmea::gps, SIGNAL( statusChange( GpsNmea::GpsStatus ) ),
           calculator, SLOT( slot_GpsStatus( GpsNmea::GpsStatus ) ) );

#ifdef FLARM
  connect( GpsNmea::gps, SIGNAL( newFlarmCount(int) ),
           viewMap, SLOT( slot_FlarmCount(int) ) );
#endif

  connect( viewWP, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewWP, SIGNAL( deleteWaypoint( Waypoint* ) ),
           calculator, SLOT( slot_WaypointDelete( Waypoint* ) ) );
  connect( viewWP, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewWP, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewWP, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewAF, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewAF, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewAF, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewAF, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewHS, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewHS, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewHS, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewHS, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewOL, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewOL, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewOL, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewOL, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewNA, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewNA, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewNA, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewNA, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewRP, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewRP, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( viewRP, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewRP, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  connect( viewTP, SIGNAL( newWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewTP, SIGNAL( info( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );

  connect( Map::instance, SIGNAL( isRedrawing( bool ) ),
           this, SLOT( slotMapDrawEvent( bool ) ) );
  connect( Map::instance, SIGNAL( firstDrawingFinished() ),
           this, SLOT( slotFinishStartUp() ) );
  connect( Map::instance, SIGNAL( showPoi( Waypoint* ) ),
           this, SLOT( slotSwitchToInfoView( Waypoint* ) ) );
  connect( Map::instance, SIGNAL( alarm( const QString&, const bool ) ),
           this, SLOT( slotAlarm( const QString&, const bool ) ) );
  connect( Map::instance, SIGNAL( notification( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );
  connect( Map::instance, SIGNAL( newPosition( QPoint& ) ),
           calculator, SLOT( slot_changePosition( QPoint& ) ) );
  connect( Map::instance, SIGNAL( userZoom() ),
           calculator, SLOT( slot_userMapZoom() ) );
  connect( Map::instance, SIGNAL( showInfoBoxes(bool) ),
           actionToggleMapSidebar, SLOT( setChecked(bool) ) );

#ifdef FLARM

  connect( Flarm::instance(), SIGNAL( flarmTrafficInfo( QString& ) ),
           Map::instance, SLOT( slotShowFlarmTrafficInfo( QString& )) );

  connect( Flarm::instance(), SIGNAL( flarmAlertZoneInfo( FlarmBase::FlarmAlertZone& ) ),
           _globalMapContents, SLOT( slotNewFlarmAlertZoneData( FlarmBase::FlarmAlertZone& )) );

#endif

  connect( viewMap, SIGNAL( toggleLDCalculation( const bool ) ),
           calculator, SLOT( slot_toggleLDCalculation(const bool) ) );
  connect( viewMap, SIGNAL( toggleMenu() ), SLOT( slotShowContextMenu() ) );
  connect( viewMap, SIGNAL( toggleVarioCalculation( const bool ) ),
           calculator, SLOT( slot_toggleVarioCalculation(const bool) ) );
  connect( viewMap, SIGNAL( toggleETACalculation( const bool ) ),
           calculator, SLOT( slot_toggleETACalculation(const bool) ) );

  connect( calculator, SIGNAL( newWaypoint( const Waypoint* ) ),
           viewMap, SLOT( slot_Waypoint( const Waypoint* ) ) );
  connect( calculator, SIGNAL( newBearing( int ) ),
           viewMap, SLOT( slot_Bearing( int ) ) );
  connect( calculator, SIGNAL( newRelBearing( int ) ),
           viewMap, SLOT( slot_RelBearing( int ) ) );
  connect( calculator, SIGNAL( newDistance( const Distance& ) ),
           viewMap, SLOT( slot_Distance( const Distance& ) ) );
  connect( calculator, SIGNAL( newETA( const QTime& ) ),
           viewMap, SLOT( slot_ETA( const QTime& ) ) );
  connect( calculator, SIGNAL( newHeading( int ) ),
           viewMap, SLOT( slot_Heading( int ) ) );
  connect( calculator, SIGNAL( newSpeed( const Speed& ) ),
           viewMap, SLOT( slot_Speed( const Speed& ) ) );
  connect( calculator, SIGNAL( newTas( const Speed& ) ),
           viewMap, SLOT( slot_Tas( const Speed& ) ) );
  connect( calculator, SIGNAL( newUserAltitude( const Altitude& ) ),
           viewMap, SLOT( slot_Altitude( const Altitude& ) ) );
  connect( calculator, SIGNAL( newPosition( const QPoint&, const int ) ),
           viewMap, SLOT( slot_Position( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( newPosition( const QPoint&, const int ) ),
           Map::getInstance(), SLOT( slotPosition( const QPoint&, const int ) ) );
  connect( calculator, SIGNAL( switchManualInFlight() ),
           Map::getInstance(), SLOT( slotSwitchManualInFlight() ) );
  connect( calculator, SIGNAL( switchMapScale(const double&) ),
           Map::getInstance(), SLOT( slotSetScale(const double&) ) );
  connect( calculator, SIGNAL( glidePath( const Altitude& ) ),
           viewMap, SLOT( slot_GlidePath( const Altitude& ) ) );
  connect( calculator, SIGNAL( bestSpeed( const Speed& ) ),
           viewMap, SLOT( slot_bestSpeed( const Speed& ) ) );
  connect( calculator, SIGNAL( newMc( const Speed& ) ),
           viewMap, SLOT( slot_Mc( const Speed& ) ) );
  connect( calculator, SIGNAL( newVario( const Speed& ) ),
           viewMap, SLOT( slot_Vario( const Speed& ) ) );
  connect( calculator, SIGNAL( newWind( Vector& ) ),
           viewMap, SLOT( slot_Wind( Vector& ) ) );
  connect( calculator, SIGNAL( newLD( const double&, const double&) ),
           viewMap, SLOT( slot_LD( const double&, const double&) ) );
  connect( calculator, SIGNAL( newGlider( const QString&) ),
           viewMap, SLOT( slot_glider( const QString&) ) );
  connect( calculator, SIGNAL( flightModeChanged(Calculator::FlightMode) ),
           viewMap, SLOT( slot_setFlightStatus(Calculator::FlightMode) ) );
  connect( calculator, SIGNAL( taskpointSectorTouched() ),
           m_logger, SLOT( slotTaskSectorTouched() ) );
  connect( calculator, SIGNAL( taskInfo( const QString&, const bool ) ),
           this, SLOT( slotNotification( const QString&, const bool ) ) );
  connect( calculator, SIGNAL( newSample() ),
           m_logger, SLOT( slotMakeFixEntry() ) );
  connect( calculator, SIGNAL( flightModeChanged(Calculator::FlightMode) ),
           m_logger, SLOT( slotFlightModeChanged(Calculator::FlightMode) ) );

  connect( ( QObject* ) calculator->getReachList(), SIGNAL( newReachList() ),
           this, SLOT( slotNewReachList() ) );

#ifdef INTERNET
  connect( calculator, SIGNAL( newSample() ),
           m_liveTrackLogger, SLOT( slotNewFixEntry() ) );
#endif

  connect( m_logger, SIGNAL( logging( bool ) ),
           viewMap, SLOT( slot_setLoggerStatus() ) );
  connect( m_logger, SIGNAL( logging( bool ) ),
            SLOT( slotLogging( bool ) ) );
  connect( m_logger, SIGNAL( madeEntry() ),
           viewMap, SLOT( slot_LogEntry() ) );
  connect( m_logger, SIGNAL( takeoffTime(QDateTime&) ),
            SLOT( slotTakeoff(QDateTime&) ) );
  connect( m_logger, SIGNAL( landingTime(QDateTime&) ),
            SLOT( slotLanded(QDateTime&) ) );

  calculator->setPosition( _globalMapMatrix->getMapCenter( false ) );

  slotReadconfig();

  // set the default glider to be the last one selected.
  calculator->setGlider( GliderListWidget::getUserSelectedGlider() );
  QString gt = calculator->gliderType();

  if ( !gt.isEmpty() )
    {
      setWindowTitle ( "Cumulus - " + gt );
    }

  calculator->newSites();  // New sites have been loaded in map draw
  // this call is responsible for setting correct AGL/STD for manual mode,
  // must be called after Map::instance->draw(), there the AGL info is loaded
  // I do not connect since it is never emitted, only called once here
  calculator->slot_changePosition(MapMatrix::NotSet);

  if( ! GeneralConfig::instance()->getAirspaceWarningEnabled() )
    {
      QMessageBox mb(this);

      mb.setWindowTitle( tr("Airspace Warnings") );
      mb.setIcon( QMessageBox::Warning );
      mb.setText( tr("<html><b>Airspace warnings are disabled!<br>"
                     "Enable now?</b></html>") );
      mb.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
      mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

      mb.show();
      QPoint pos = mapToGlobal(QPoint( width()/2 - mb.width()/2, height()/2 - mb.height()/2 ));
      mb.move( pos );

#endif

      if( mb.exec() == QMessageBox::Yes )
        {
          GeneralConfig::instance()->setAirspaceWarningEnabled(true);
        }

      QCoreApplication::flush();
      sleep(1);
    }

  splash->setVisible( true );
  ws->setVisible( true );

  QCoreApplication::flush();
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents|QEventLoop::ExcludeSocketNotifiers);

  Map::instance->setDrawing( true );
  viewMap->setVisible( true );
  viewMap->resize( size() );

  // set viewMap as central widget
  setCentralWidget( viewMap );

  // set the map view as the default widget
  setView( mapView );

  // Make the status bar visible. Maemo hides it per default.
  slotViewStatusBar( true );

  // Check, if a task file upgrade has to be done.
  TaskFileManager tfm;
  tfm.check4Upgrade();
}

/**
 * This slot is called by the map drawing method Map::__redrawMap, if the map
 * drawing has been done the first time. After that all map data should be
 * loaded.
 */
void MainWindow::slotFinishStartUp()
{
  qDebug() << "MainWindow::slotFinishStartUp()";

  GeneralConfig *conf = GeneralConfig::instance();

  if( conf->getLoggerAutostartMode() == true )
    {
      // set logger in standby mode
      m_logger->Standby();
    }

  setNearestOrReachableHeaders();

  // close wait screen
  ws->setScreenUsage( false );
  ws->setVisible( false );

  // closes and removes the splash screen
  splash->close();

  // Startup GPS client process now for data receiving
  GpsNmea::gps->blockSignals( false );

#ifndef ANDROID
  GpsNmea::gps->startGpsReceiver();
#endif

  // Get the language from the environment
  QString language = qgetenv("LANG");

#ifdef MAEMO

  if( ossoContext )
    {
      osso_display_blanking_pause( ossoContext );

      // setup timer to prevent screen blank
      m_displayTrigger = new QTimer(this);
      m_displayTrigger->setSingleShot(true);

      connect( m_displayTrigger, SIGNAL(timeout()),
               this, SLOT(slotDisplayTrigger()) );

      // start timer with 10s
      m_displayTrigger->start( 10000 );
    }
#endif

#ifdef ANDROID

  // setup timer to prevent screen blank
  m_displayTrigger = new QTimer(this);
  m_displayTrigger->setSingleShot(true);

  connect( m_displayTrigger, SIGNAL(timeout()),
           this, SLOT(slotDisplayTrigger()) );

  // start timer with 10s
  m_displayTrigger->start( 10000 );

  // Check, if the Android App was restarted after a OS kill. In this case
  // we have to restore some things.
  if( jniIsRestarted() )
    {
      if( _globalMapContents->restoreFlightTask() == true )
	{
	  slotPreFlightDataChanged();
        }

      calculator->restoreWaypoint();
    }
  else
    {
       // Remove a previous saved target waypoint.
      QString fn = conf->getUserDataDirectory() + "/target.wpt";
      Waypoint::write( static_cast<Waypoint *>(0), fn );

      // Reset a previous saved task.
      conf->setMapCurrentTask( "" );
      conf->save();
    }

  language = jniGetLanguage();

  // Enable JNI transfer now.
  jniShutdown( false );

#endif

  // On first startup we set some user defaults and download some default data
  // from the Internet.
  if( m_firstStartup == true )
    {
      // The letter 4 and 5 are the special country code of a land, if counting
      // starts with one.
      language = language.mid(3, 2).toUpper();

      if( language.size() == 2 )
        {
          if( conf->getHomeCountryCode().isEmpty() )
            {
              conf->setHomeCountryCode( language );
            }

          conf->setOpenAipAirfieldCountries( language.toLower() );
          conf->setOpenAipAirspaceCountries( language.toLower() );

          if( _globalMapContents->askUserForDownload() == true )
            {
              _globalMapContents->slotDownloadOpenAipPois( QStringList(language.toLower()) );
              _globalMapContents->slotDownloadAirspaces( QStringList(language.toLower()) );
            }
        }
    }

  qDebug( "End startup Cumulus" );
}

MainWindow::~MainWindow()
{
  // qDebug ("MainWindow::~MainWindow()");

#warning Question: Should we save the main window size on exit?
  // @AP: we do that later
  // save main window size
  // GeneralConfig::instance()->setWindowSize( size() );
  // GeneralConfig::instance()->save();

  if( m_logger )
    {
      delete m_logger;
    }

#ifdef MAEMO

  // stop Maemo screen saver off triggering
  if( ossoContext )
    {
      if( m_displayTrigger )
        {
          m_displayTrigger->stop();
        }

      osso_deinitialize( ossoContext );
    }
#endif

#ifdef ANDROID

  if( m_displayTrigger )
    {

      m_displayTrigger->stop();
    }

#endif

  _globalMainWindow = static_cast<MainWindow *> (0);
}

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

#ifndef ANDROID

  if( name && QString(name) == "notify" )
    {
      sound = GeneralConfig::instance()->getAppRoot() + "/sounds/Notify.wav";
    }
  else if( name && QString(name) == "alarm" )
    {
      sound = GeneralConfig::instance()->getAppRoot() + "/sounds/Alarm.wav";
    }
  else if( name )
    {
      sound = *name;
    }

  // The sound is played in an extra thread
  Sound *player = new Sound( sound );

  player->start( QThread::HighestPriority );

#else

  // Native method used to play sound via Android service
  int soundId = 0;

  if( name && QString(name) == "notify" )
    {
      soundId = 0;
    }
  else if( name && QString(name) == "alarm" )
    {
      soundId = 1;
    }
  else
    {
      // All other is unsupported
      return;
    }

  jniPlaySound( soundId );

#endif
}

void MainWindow::slotAlarm( const QString& msg, const bool sound )
{
  if( sound )
    {
      playSound("alarm");
    }

  if( ! msg.isEmpty() )
    {
      viewMap->slot_info( msg );
    }
}

void MainWindow::slotNotification( const QString& msg, const bool sound )
{
  if( sound )
    {
      playSound("notify");
    }

  if( ! msg.isEmpty() )
    {
      viewMap->slot_info( msg );
    }
}

void MainWindow::createContextMenu()
{
  contextMenu = new QMenu(this);
  contextMenu->setVisible(false);

  fileMenu = contextMenu->addMenu(tr("File") + "  ");
  fileMenu->addAction( actionFileQuit );

  viewMenu = contextMenu->addMenu(tr("View") + "  ");

#ifdef FLARM
  viewMenu->addAction( actionViewFlarm );
  viewMenu->addSeparator();
#endif

  viewMenu->addAction( actionViewReachpoints );
  viewMenu->addAction( actionViewAirfields );
  viewMenu->addAction( actionViewOutlandings );
  viewMenu->addAction( actionViewNavAids );
  viewMenu->addAction( actionViewHotspots );
  viewMenu->addAction( actionViewInfo );
  actionViewInfo->setEnabled( false );
  viewMenu->addAction( actionViewTaskpoints );
  actionViewTaskpoints->setEnabled( false );
  viewMenu->addAction( actionViewWaypoints );

  labelMenu = contextMenu->addMenu( tr("Toggles") + "  ");
  labelSubMenu = labelMenu->addMenu( tr("Labels") + "  ");
  labelSubMenu->addAction( actionToggleAfLabels );
  labelSubMenu->addAction( actionToggleOlLabels );
  labelSubMenu->addAction( actionToggleNaLabels );
  labelSubMenu->addAction( actionToggleTpLabels );
  labelSubMenu->addAction( actionToggleWpLabels );
  labelSubMenu->addAction( actionToggleLabelsInfo );
  labelMenu->addSeparator();

#ifndef ANDROID
  labelMenu->addAction( actionToggleGps );
#endif

  labelMenu->addAction( actionToggleLogging );
  labelMenu->addAction( actionToggleTrailDrawing );
  labelMenu->addSeparator();

#ifndef ANDROID
  labelMenu->addAction( actionToggleWindowSize );
#endif

  labelMenu->addAction( actionToggleMapSidebar );
  labelMenu->addAction( actionToggleStatusbar );

  mapMenu = contextMenu->addMenu(tr("Map") + "  ");
  mapMenu->addAction( actionSelectTask );
  mapMenu->addAction( actionManualNavMove2Home );
  mapMenu->addAction( actionNav2Home );
  mapMenu->addAction( actionEnsureVisible );
  mapMenu->addAction( actionRemoveTarget );

  statusMenu = contextMenu->addMenu(tr("Status") + "  ");
  statusMenu->addAction( actionStatusAirspace );
  statusMenu->addAction( actionStatusGPS );

  setupMenu = contextMenu->addMenu(tr("Setup") + "  ");
#ifdef ANDROID
  setupMenu->addAction( actionHardwareMenu );
#endif
  setupMenu->addAction( actionSetupConfig );
  setupMenu->addAction( actionPreFlight );
  setupMenu->addAction( actionSetupInFlight );

  helpMenu = contextMenu->addMenu(tr("Help") + "  ");
  helpMenu->addAction( actionHelpCumulus );
  helpMenu->addAction( actionHelpAboutApp );

#if ! defined ANDROID && ! defined MAEMO
  helpMenu->addAction( actionHelpAboutQt );
#endif
}

/** sets the menu fonts to a reasonable and usable value */
void MainWindow::slotSetMenuFontSize()
{
  // sets the user's selected menu font, if defined
  QString fontString = GeneralConfig::instance()->getGuiMenuFont();
  QFont userFont;

  qDebug() << "MainWindow::slotSetMenuFontSize(): fontString=" << fontString;

  if( fontString.isEmpty() || userFont.fromString( fontString ) == false )
    {
      qDebug() << "MainWindow::slotSetMenuFontSize() adapting font size";
      // take current font as alternative and adapt it.
      userFont = font();
      Layout::fitGuiMenuFont( userFont );

      // Don't save the temporary set menu font.
      // GeneralConfig::instance()->setGuiMenuFont( userFont.toString() );
      // GeneralConfig::instance()->save();
    }

  qDebug() << "MainWindow::slotSetMenuFontSize(): MenuFont PointSize=" << userFont.pointSize()
           << "FontHeight=" << QFontMetrics(userFont).height();

  if( contextMenu )
    {
      contextMenu->setFont( userFont );
    }
  else
    {
      menuBar()->setFont( userFont );
    }

  // maybe NULL, if not initialized
  if( fileMenu ) fileMenu->setFont( userFont );
  if( viewMenu ) viewMenu->setFont( userFont );
  if( mapMenu ) mapMenu->setFont( userFont );
  if( statusMenu ) statusMenu->setFont( userFont );
  if( setupMenu ) setupMenu->setFont( userFont );
  if( helpMenu ) helpMenu->setFont( userFont );
  if( labelMenu ) labelMenu->setFont( userFont );
  if( labelSubMenu ) labelSubMenu->setFont( userFont );
}

/** initializes all QActions of the application */
void MainWindow::createActions()
{
  ws->slot_SetText1( tr( "Setting up key shortcuts ..." ) );

  // Note: The ampersand in the QAction item's text "&File" sets Alt+F as a
  //       shortcut for this menu. You can use "&&" to get a real ampersand
  //       in the menu bar.

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

  actionManualNavMove2Home = new QAction( tr( "Goto home site" ), this );

#ifndef ANDROID
  QList<QKeySequence> acGoHomeKeys;
  acGoHomeKeys << QKeySequence(Qt::SHIFT + Qt::Key_H) << QKeySequence::MoveToStartOfLine;
  actionManualNavMove2Home->setShortcuts( acGoHomeKeys );
#endif

  addAction( actionManualNavMove2Home );
  connect( actionManualNavMove2Home, SIGNAL( triggered() ),
            calculator, SLOT( slot_changePositionHome() ) );

  actionManualNavMove2WP = new QAction( tr( "Move to waypoint" ), this );
  actionManualNavMove2WP->setShortcut( QKeySequence("C") );
  addAction( actionManualNavMove2WP );
  connect( actionManualNavMove2WP, SIGNAL( triggered() ),
            calculator, SLOT( slot_changePositionWp() ) );

  actionManualNavWPList = new QAction( tr( "Open waypoint list" ), this );

#ifndef ANDROID
  actionManualNavWPList->setShortcut( QKeySequence("F9") );
#endif

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

#ifndef ANDROID
  actionGpsNavWPList->setShortcut( QKeySequence("F9") );
#endif

  addAction( actionGpsNavWPList );
  connect( actionGpsNavWPList, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToWPListView() ) );

  // Set home site as target
  actionNav2Home = new QAction( tr( "Home site as target" ), this );

#ifndef ANDROID
  actionNav2Home->setShortcut( QKeySequence(Qt::Key_H) );
#endif

  addAction( actionNav2Home );
  connect( actionNav2Home, SIGNAL( triggered() ),
            this, SLOT( slotNavigate2Home() ) );

  // Zoom in map
  actionGpsNavZoomIn = new QAction( tr( "Zoom in" ), this );
  actionGpsNavZoomIn->setShortcut( QKeySequence("Right") );

  addAction( actionGpsNavZoomIn );
  connect( actionGpsNavZoomIn, SIGNAL( triggered() ),
            Map::instance, SLOT( slotZoomIn() ) );

  // Zoom out map
  actionGpsNavZoomOut = new QAction( tr( "Zoom out" ), this );
  actionGpsNavZoomOut->setShortcut( QKeySequence("Left") );
  addAction( actionGpsNavZoomOut );
  connect( actionGpsNavZoomOut, SIGNAL( triggered() ),
            Map::instance, SLOT( slotZoomOut() ) );

  // Show context menu
  actionOpenContextMenu = new QAction( tr( "Open menu" ), this );

  QList<QKeySequence> mBTSCList;
  mBTSCList << Qt::Key_M << Qt::Key_F4;

  actionOpenContextMenu->setShortcuts( mBTSCList );
  addAction( actionOpenContextMenu );
  connect( actionOpenContextMenu, SIGNAL( triggered() ),
           this, SLOT( slotShowContextMenu() ) );

  // Toggle window size
  actionToggleWindowSize = new QAction( tr( "Window" ), this );

  // Hardware Key F6 for maximize/normalize screen under Maemo 4.
  // Maemo 5 has no keys. Therefore the space key is activated for that.
#ifndef ANDROID
  QList<QKeySequence> wskList;
  wskList << Qt::Key_Space << Qt::Key_F6;
  actionToggleWindowSize->setShortcuts( wskList );
  actionToggleWindowSize->setCheckable( true );
  actionToggleWindowSize->setChecked( false );
  addAction( actionToggleWindowSize );
  connect( actionToggleWindowSize, SIGNAL( triggered() ),
            this, SLOT( slotToggleWindowSize() ) );
#endif

  actionFileQuit = new QAction( tr( "Exit" ), this );
#ifndef ANDROID
  actionFileQuit->setShortcut( QKeySequence("Shift+E") );
#endif
  addAction( actionFileQuit );
  connect( actionFileQuit, SIGNAL( triggered() ),
            this, SLOT( close() ) );

#ifdef ANDROID
  actionHardwareMenu = new QAction( tr( "Hardware" ), this );
  addAction( actionHardwareMenu );
  connect( actionHardwareMenu, SIGNAL( triggered() ),
           this, SLOT( slotOpenHardwareMenu() ) );
#endif

#ifdef FLARM
  actionViewFlarm = new QAction( tr( "Flarm Radar" ), this );
  addAction( actionViewFlarm );
  connect( actionViewFlarm, SIGNAL( triggered() ),
            viewMap, SLOT( slot_OpenFlarmWidget() ) );
#endif

  actionViewWaypoints = new QAction ( tr( "Waypoints" ), this );
  addAction( actionViewWaypoints );
  connect( actionViewWaypoints, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToWPListView() ) );

  actionViewAirfields = new QAction ( tr( "Airfields" ), this );
  addAction( actionViewAirfields );
  connect( actionViewAirfields, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToAFListView() ) );

  actionViewHotspots = new QAction ( tr( "Hotspots" ), this );
  addAction( actionViewHotspots );
  connect( actionViewHotspots, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToHSListView() ) );

  actionViewOutlandings = new QAction ( tr( "Fields" ), this );
  addAction( actionViewOutlandings );
  connect( actionViewOutlandings, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToOLListView() ) );

  actionViewNavAids = new QAction ( tr( "Navaids" ), this );
  addAction( actionViewNavAids );
  connect( actionViewNavAids, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToNavAidsListView() ) );

  actionViewReachpoints = new QAction ( tr( "Reachable" ), this );
#ifndef ANDROID
  actionViewReachpoints->setShortcut(Qt::Key_R);
#endif
  addAction( actionViewReachpoints );
  connect( actionViewReachpoints, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToReachListView() ) );

  actionViewTaskpoints = new QAction ( tr( "Task" ), this );
  addAction( actionViewTaskpoints );
  connect( actionViewTaskpoints, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToTaskListView() ) );

  // Show info about selected target
  actionViewInfo = new QAction( tr( "Target Info" ), this );

#ifndef ANDROID
  actionViewInfo->setShortcut(Qt::Key_I);
#endif

  addAction( actionViewInfo );
  connect( actionViewInfo, SIGNAL( triggered() ),
            this, SLOT( slotSwitchToInfoView() ) );

  actionToggleStatusbar = new QAction( tr( "Statusbar"), this );
  actionToggleStatusbar->setCheckable(true);
  actionToggleStatusbar->setChecked(true);
  addAction( actionToggleStatusbar );
  connect( actionToggleStatusbar, SIGNAL( toggled( bool ) ),
            this, SLOT( slotViewStatusBar( bool ) ) );

  actionStatusAirspace = new QAction( tr( "Airspace" ), this );
  addAction( actionStatusAirspace );
  connect( actionStatusAirspace, SIGNAL( triggered() ),
            Map::instance, SLOT( slotShowAirspaceStatus() ) );

  actionStatusGPS = new QAction( tr( "GPS" ), this );
#ifndef ANDROID
  actionStatusGPS->setShortcut(Qt::Key_G);
#endif
  addAction( actionStatusGPS );
  connect( actionStatusGPS, SIGNAL( triggered() ),
            viewMap, SLOT( slot_gpsStatusDialog() ) );

  // Consider qwertz keyboards y <-> z are interchanged
  // F7 is a Maemo hardware key for Zoom in
  actionZoomInZ = new QAction ( tr( "Zoom in" ), this );
  QList<QKeySequence> zInSCList;
  zInSCList << Qt::Key_Z << Qt::Key_Y << Qt::Key_F7;
  actionZoomInZ->setShortcuts( zInSCList );

  addAction( actionZoomInZ );
  connect ( actionZoomInZ, SIGNAL( triggered() ),
             Map::instance , SLOT( slotZoomIn() ) );

  // F8 is a Maemo hardware key for Zoom out
  actionZoomOutZ = new QAction ( tr( "Zoom out" ), this );
  QList<QKeySequence> zOutSCList;
  zOutSCList << Qt::Key_X << Qt::Key_F8;
  actionZoomOutZ->setShortcuts( zOutSCList );

  addAction( actionZoomOutZ );
  connect ( actionZoomOutZ, SIGNAL( triggered() ),
             Map::instance , SLOT( slotZoomOut() ) );

  actionToggleAfLabels = new QAction ( tr( "Airfield" ), this);
#ifndef ANDROID
  actionToggleAfLabels->setShortcut(Qt::Key_A);
#endif
  actionToggleAfLabels->setCheckable(true);
  actionToggleAfLabels->setChecked( GeneralConfig::instance()->getMapShowAirfieldLabels() );
  addAction( actionToggleAfLabels );
  connect( actionToggleAfLabels, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleAfLabels( bool ) ) );

  actionToggleNaLabels = new QAction ( tr( "NavAids" ), this);
#ifndef ANDROID
  actionToggleNaLabels->setShortcut(Qt::Key_N);
#endif
  actionToggleNaLabels->setCheckable(true);
  actionToggleNaLabels->setChecked( GeneralConfig::instance()->getMapShowNavAidsLabels() );
  addAction( actionToggleNaLabels );
  connect( actionToggleNaLabels, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleNaLabels( bool ) ) );

  actionToggleOlLabels = new QAction ( tr( "Outlanding" ), this);
#ifndef ANDROID
  actionToggleOlLabels->setShortcut(Qt::Key_O);
#endif
  actionToggleOlLabels->setCheckable(true);
  actionToggleOlLabels->setChecked( GeneralConfig::instance()->getMapShowOutLandingLabels() );
  addAction( actionToggleOlLabels );
  connect( actionToggleOlLabels, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleOlLabels( bool ) ) );


  actionToggleTpLabels = new QAction ( tr( "Taskpoint" ), this);
#ifndef ANDROID
  actionToggleTpLabels->setShortcut(Qt::Key_T);
#endif
  actionToggleTpLabels->setCheckable(true);
  actionToggleTpLabels->setChecked( GeneralConfig::instance()->getMapShowTaskPointLabels() );
  addAction( actionToggleTpLabels );
  connect( actionToggleTpLabels, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleTpLabels( bool ) ) );

  actionToggleWpLabels = new QAction ( tr( "Waypoint" ), this);
#ifndef ANDROID
  actionToggleWpLabels->setShortcut(Qt::Key_W);
#endif
  actionToggleWpLabels->setCheckable(true);
  actionToggleWpLabels->setChecked( GeneralConfig::instance()->getMapShowWaypointLabels() );
  addAction( actionToggleWpLabels );
  connect( actionToggleWpLabels, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleWpLabels( bool ) ) );

  actionToggleLabelsInfo = new QAction (  tr( "Extra Info" ), this);
#ifndef ANDROID
  actionToggleLabelsInfo->setShortcut(Qt::Key_E);
#endif
  actionToggleLabelsInfo->setCheckable(true);
  actionToggleLabelsInfo->setChecked( GeneralConfig::instance()->getMapShowLabelsExtraInfo() );
  addAction( actionToggleLabelsInfo );
  connect( actionToggleLabelsInfo, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleLabelsInfo( bool ) ) );

  actionToggleLogging = new QAction( tr( "Logging" ), this );
#ifndef ANDROID
  actionToggleLogging->setShortcut(Qt::Key_L);
#endif
  actionToggleLogging->setCheckable(true);
  addAction( actionToggleLogging );
  connect ( actionToggleLogging, SIGNAL( triggered() ),
             m_logger, SLOT( slotToggleLogging() ) );

  actionToggleTrailDrawing = new QAction( tr( "Flight trail" ), this );
  actionToggleTrailDrawing->setCheckable(true);
  actionToggleTrailDrawing->setChecked( GeneralConfig::instance()->getMapDrawTrail() );
  addAction( actionToggleTrailDrawing );
  connect ( actionToggleTrailDrawing, SIGNAL( toggled( bool ) ),
             this, SLOT( slotToggleTrailDrawing(bool) ) );

  actionEnsureVisible = new QAction ( tr( "Visualize target" ), this );
#ifndef ANDROID
  actionEnsureVisible->setShortcut(Qt::Key_V);
#endif
  addAction( actionEnsureVisible );
  connect ( actionEnsureVisible, SIGNAL( triggered() ),
             this, SLOT( slotEnsureVisible() ) );

  actionRemoveTarget = new QAction ( tr( "Remove target" ), this );
#ifndef ANDROID
  actionRemoveTarget->setShortcut(Qt::Key_R + Qt::SHIFT);
#endif
  addAction( actionRemoveTarget );
  connect ( actionRemoveTarget, SIGNAL( triggered() ),
             this, SLOT( slotRemoveTarget() ) );


  actionSelectTask = new QAction( tr( "Select task" ), this );
#ifndef ANDROID
  actionSelectTask->setShortcut(Qt::Key_T + Qt::SHIFT);
#endif
  addAction( actionSelectTask );
  connect ( actionSelectTask, SIGNAL( triggered() ),
             this, SLOT( slotOpenPreFlightConfig() ) );

  actionStartFlightTask = new QAction( tr( "Start flight task" ), this );
  actionStartFlightTask->setShortcut(Qt::Key_B);
  addAction( actionStartFlightTask );
  connect ( actionStartFlightTask, SIGNAL( triggered() ),
             calculator, SLOT( slot_startTask() ) );

  actionToggleGps = new QAction( tr( "GPS On/Off" ), this );
#ifndef ANDROID
  actionToggleGps->setShortcut(Qt::Key_G + Qt::SHIFT);
#endif
  actionToggleGps->setCheckable(true);
  actionToggleGps->setChecked(true);
  actionToggleGps->setEnabled(true);

#ifndef ANDROID

  addAction( actionToggleGps );
  connect( actionToggleGps, SIGNAL( toggled( bool ) ),
            this, SLOT( slotToggleGps( bool ) ) );
#endif

  actionPreFlight = new QAction( tr( "Flying" ), this );
#ifndef ANDROID
  actionPreFlight->setShortcut(Qt::Key_P);
#endif
  addAction( actionPreFlight );
  connect ( actionPreFlight, SIGNAL( triggered() ),
            this, SLOT( slotOpenPreFlightConfig() ) );

  actionSetupConfig = new QAction( tr ( "General" ), this );
#ifndef ANDROID
  actionSetupConfig->setShortcut(Qt::Key_S);
#endif
  addAction( actionSetupConfig );
  connect ( actionSetupConfig, SIGNAL( triggered() ),
            this, SLOT( slotOpenConfig() ) );

  actionSetupInFlight = new QAction( tr ( "In flight" ), this );
#ifndef ANDROID
  actionSetupInFlight->setShortcut(Qt::Key_F);
#endif
  addAction( actionSetupInFlight );
  connect ( actionSetupInFlight, SIGNAL( triggered() ),
             viewMap, SLOT( slot_gliderFlightDialog() ) );

  actionHelpCumulus = new QAction( tr("Help" ), this );
#ifndef ANDROID
  actionHelpCumulus->setShortcut(Qt::Key_Question);
#endif
  addAction( actionHelpCumulus );
  connect( actionHelpCumulus, SIGNAL(triggered()), this, SLOT(slotHelp()) );

  actionHelpAboutApp = new QAction( tr( "About Cumulus" ), this );
#ifndef ANDROID
  actionHelpAboutApp->setShortcut(Qt::Key_V + Qt::SHIFT);
#endif
  addAction( actionHelpAboutApp );
  connect( actionHelpAboutApp, SIGNAL( triggered() ),
            this, SLOT( slotVersion() ) );

#if ! defined ANDROID && ! defined MAEMO
  // The Qt about is too big for small screens. Therefore it is undefined for
  // Android and Maemo
  actionHelpAboutQt = new QAction( tr( "About Qt" ), this );
  actionHelpAboutQt->setShortcut(Qt::Key_Q);
  addAction( actionHelpAboutQt );
  connect( actionHelpAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );
#endif

  actionToggleMapSidebar = new QAction( tr( "Map Info Boxes"), this );
  actionToggleMapSidebar->setCheckable(true);
  actionToggleMapSidebar->setChecked(true);
  addAction( actionToggleMapSidebar );
  connect( actionToggleMapSidebar, SIGNAL( toggled( bool ) ),
           viewMap, SLOT( slot_showInfoBoxes(bool) ) );

  scToggleMapSidebar = new QShortcut( this );
  scToggleMapSidebar->setKey(Qt::Key_D);
  connect( scToggleMapSidebar, SIGNAL( activated() ),
           actionToggleMapSidebar, SLOT( toggle() ) );

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
#ifdef FLARM
  actionViewFlarm->setEnabled( toggle );
#endif

  actionViewWaypoints->setEnabled( toggle );

  if( toggle == true &&
      (_globalMapContents->getListLength( MapContents::AirfieldList ) > 0 ||
       _globalMapContents->getListLength( MapContents::GliderfieldList ) > 0) )
    {
      actionViewAirfields->setEnabled( true );
    }
  else
    {
      actionViewAirfields->setEnabled( false );
    }

  if( toggle == true &&
      _globalMapContents->getListLength( MapContents::OutLandingList ) > 0 )
    {
      actionViewOutlandings->setEnabled( true );
    }
  else
    {
      actionViewOutlandings->setEnabled( false );
    }

  if( toggle == true &&
      _globalMapContents->getListLength( MapContents::RadioList ) > 0 )
    {
      actionViewNavAids->setEnabled( true );
    }
  else
    {
      actionViewNavAids->setEnabled( false );
    }

  actionStatusAirspace->setEnabled( toggle );
  actionStatusGPS->setEnabled( toggle );
  actionZoomInZ->setEnabled( toggle );
  actionZoomOutZ->setEnabled( toggle );
  actionToggleAfLabels->setEnabled( toggle );
  actionToggleOlLabels->setEnabled( toggle );
  actionToggleNaLabels->setEnabled( toggle );
  actionToggleTpLabels->setEnabled( toggle );
  actionToggleWpLabels->setEnabled( toggle );
  actionToggleLabelsInfo->setEnabled( toggle );
  actionToggleWindowSize->setEnabled( toggle );
  actionEnsureVisible->setEnabled( toggle );
  actionRemoveTarget->setEnabled( toggle );
  actionSelectTask->setEnabled( toggle );
  actionStartFlightTask->setEnabled( toggle );
  actionPreFlight->setEnabled( toggle );
  actionSetupConfig->setEnabled( toggle );
  actionSetupInFlight->setEnabled( toggle );
  actionHelpCumulus->setEnabled( toggle );
  actionHelpAboutApp->setEnabled( toggle );

#if ! defined ANDROID && ! defined MAEMO
  actionHelpAboutQt->setEnabled( toggle );
#endif

  actionToggleLogging->setEnabled( toggle );
  actionNav2Home->setEnabled( toggle );
  scToggleMapSidebar->setEnabled( toggle );
  scExit->setEnabled( toggle );

  if( toggle )
    {
      GeneralConfig * conf = GeneralConfig::instance();
      actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );

      if( calculator->getTargetWp() )
        {
          // allow action only if a waypoint is selected
          actionViewInfo->setEnabled( toggle );
        }

      if( _globalMapContents->getCurrentTask() )
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
  actionManualNavMove2Home->setEnabled( toggle );
  actionManualNavMove2WP->setEnabled( toggle );
  actionManualNavWPList->setEnabled( toggle );
  actionToggleGps->setEnabled( toggle );
}

void MainWindow::toggleGpsNavActions( const bool toggle )
{
  actionGpsNavUp->setEnabled( toggle );
  actionGpsNavDown->setEnabled( toggle );
  actionGpsNavWPList->setEnabled( toggle );
  actionGpsNavZoomIn->setEnabled( toggle );
  actionGpsNavZoomOut->setEnabled( toggle );
}

void MainWindow::slotToggleTrailDrawing( bool toggle )
{
  actionToggleTrailDrawing->setChecked( toggle );
  GeneralConfig::instance()->setMapDrawTrail( toggle );
  GeneralConfig::instance()->save();
}

/**
 * Make sure the user really wants to quit
 */
void MainWindow::closeEvent( QCloseEvent* event )
{
  // Flag to signal a deferred close. It is used by the LiveTrackLogger
  // to give it the possibility to send an end record and to cancel and
  // finishing running downloads.
  static bool deferredClose = false;

  // Counter to wait for termination of background threads.
  static int deferredCounter = 10;

  if( _globalMapView == 0 )
    {
      event->accept();
      return QMainWindow::closeEvent(event);
    }

  if( deferredClose == true )
    {
      deferredCounter--;

      //      qDebug() << "Close deferred: deferredCounter=" << deferredCounter
      //	         << "runningDownloads=" << DownloadManager::runningDownloads()
      //               << "livetrackWorkingState=" << m_liveTrackLogger->livetrackWorkingState();

      if( deferredCounter > 0 && DownloadManager::runningDownloads() > 0 &&
          m_liveTrackLogger->livetrackWorkingState ())
        {
          // Trigger a recall of this slot to check again background processes.
          QTimer::singleShot( 250, this, SLOT(close()) );
          event->ignore ();
          return;
        }

#ifdef ANDROID
      jniShutdown();
#endif

      event->accept();
      return QMainWindow::closeEvent(event);
    }

  // @AP: All close events will be ignored, if we are not in the map
  // view to avoid any possibility of confusion with the two close buttons.
  if( view != mapView || ! isRootWindow() )
    {
      event->ignore();
      return;
    }

  playSound("notify");

  QMessageBox mb( QMessageBox::Question,
                  tr( "Terminating?" ),
                  tr( "Terminating Cumulus<br><b>Are you sure?</b>" ),
                  QMessageBox::Yes | QMessageBox::No,
                  this );

  mb.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

  mb.show();
  QPoint pos = mapToGlobal(QPoint( width()/2  - mb.width()/2,
                                   height()/2 - mb.height()/2 ));
  mb.move( pos );

#endif

  switch ( mb.exec() )
    {
    case QMessageBox::Yes:
      // Stop GPS data forwarding to stop any other logging actions.
      GpsNmea::gps->blockSignals( true );

#ifdef INTERNET

      {
          // Stop all downloads.
          // Qt::HANDLE tid = QThread::currentThreadId();
          // qDebug() << "Closing Thread=" << tid;

          DownloadManager::setStopFlag (true);

          if (DownloadManager::runningDownloads () > 0)
            {
              deferredClose = true;
            }
      }

      // Stop live tracking, if is is running
      if( m_liveTrackLogger->sessionStatus() == true )
        {
          deferredClose = true;
          m_liveTrackLogger->slotFinishLogging();
        }

      if( deferredClose == true)
        {
          // Trigger a recall of this slot to check again for running
          // background processes.
          QTimer::singleShot( 250, this, SLOT(close()) );

          // Hide the main window
          setVisible( false );
          event->ignore();
          return;
        }

#endif

#ifdef ANDROID

      deferredClose = true;

      // Wait 250ms before termination of application to give the main loop
      // time to process delayed keypad events.
      QTimer::singleShot( 250, this, SLOT(close()) );

      // Hide the main window
      setVisible( false );
      event->ignore();
      return;

#endif

      // accept close event and exit
      event->accept();
      QMainWindow::closeEvent(event);
      break;

    case QMessageBox::No:
    case QMessageBox::Cancel:
    default:

      // don't save and don't exit
      event->ignore();
      break;
    }
}

void MainWindow::slotShowContextMenu()
{
  // We switch off the visibility all actions, which should not be to seen
  // by the user.
  if (_globalMapContents->getListLength( MapContents::AirfieldList ) > 0 ||
      _globalMapContents->getListLength( MapContents::GliderfieldList ) > 0 )
    {
      actionViewAirfields->setVisible( true );
      actionToggleAfLabels->setVisible( true );
    }
  else
    {
      actionViewAirfields->setVisible( false );
      actionToggleAfLabels->setVisible( false );
    }

  if( _globalMapContents->getListLength( MapContents::OutLandingList ) > 0 )
    {
      actionViewOutlandings->setVisible( true );
      actionToggleOlLabels->setVisible( true );
    }
  else
    {
      actionViewOutlandings->setVisible( false );
      actionToggleOlLabels->setVisible( false );
    }

  if( _globalMapContents->getListLength( MapContents::RadioList ) > 0 )
    {
      actionViewNavAids->setVisible( true );
      actionToggleNaLabels->setVisible( true );
    }
  else
    {
      actionViewNavAids->setVisible( false );
      actionToggleNaLabels->setVisible( false );
    }

  GeneralConfig * conf = GeneralConfig::instance();
  actionViewReachpoints->setVisible( conf->getNearestSiteCalculatorSwitch() );

  if( calculator->getTargetWp() )
    {
      // show action only if a waypoint is selected
      actionViewInfo->setVisible( true );
      actionEnsureVisible->setVisible( true );
      actionRemoveTarget->setVisible( true );
    }
  else
    {
      actionViewInfo->setVisible( false );
      actionEnsureVisible->setVisible( false );
      actionRemoveTarget->setVisible( false );
    }

  if( _globalMapContents->getCurrentTask() )
    {
      // show action only if a task is defined
      actionViewTaskpoints->setVisible( true );
      actionToggleTpLabels->setVisible( true );
    }
  else
    {
      actionViewTaskpoints->setVisible( false );
      actionToggleTpLabels->setVisible( false );
    }

#ifndef ANDROID

  actionToggleGps->setEnabled( ! calculator->moving() );
  actionToggleGps->setVisible( ! calculator->moving() );

#endif

  contextMenu->exec(mapToGlobal(QPoint(0,0)));
}

void MainWindow::slotToggleMapSidebar()
{
  actionToggleMapSidebar->toggle();
}

void MainWindow::slotToggleAfLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowAirfieldLabels( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::airfields);
}

void MainWindow::slotToggleNaLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowNavAidsLabels( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::navaids);
}

void MainWindow::slotToggleOlLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowOutLandingLabels( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::outlandings);
}

void MainWindow::slotToggleTpLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowTaskPointLabels( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::task);
}

void MainWindow::slotToggleWpLabels( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowWaypointLabels( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::waypoints);
}

void MainWindow::slotToggleLabelsInfo( bool toggle )
{
  // save configuration change
  GeneralConfig::instance()->setMapShowLabelsExtraInfo( toggle );
  GeneralConfig::instance()->save();
  Map::instance->scheduleRedraw(Map::airfields);
}


void MainWindow::slotToggleWindowSize()
{
  setWindowState( windowState() ^ Qt::WindowFullScreen );
}

void MainWindow::slotViewStatusBar( bool toggle )
{
  if ( toggle )
    viewMap->statusBar()->setVisible( true );
  else
    viewMap->statusBar()->setVisible( false );
}

/** Called if the logging is actually toggled */
void MainWindow::slotLogging ( bool logging )
{
  actionToggleLogging->blockSignals( true );
  actionToggleLogging->setChecked( logging );
  actionToggleLogging->blockSignals( false );
}

void MainWindow::setView( const AppView newView )
{
  switch ( newView )
    {
    case mapView:

      // save new view value
      view = newView;

      setRootWindow( true );

      // @AP: set focus to MainWindow widget, otherwise F-Key events will
      // not routed to it
      setFocus();

      fileMenu->setEnabled( true );
      mapMenu->setEnabled( true );
      viewMenu->setEnabled( true );
      setupMenu->setEnabled( true );
      helpMenu->setEnabled( true );

      viewMap->setVisible( true );

      toggleManualNavActions( GpsNmea::gps->getGpsStatus() != GpsNmea::validFix ||
                              calculator->isManualInFlight() );

      toggleGpsNavActions( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
                           !calculator->isManualInFlight() );

      actionOpenContextMenu->setEnabled( true );

      // Switch on all action shortcuts in this view
      toggleActions( true );

      // remove temporary status bar messages
      viewMap->statusBar()->clearMessage();

      // If we returned to the map view, we should schedule a redraw of the
      // airspaces and the navigation layer. Map is not drawn, when invisible
      // and airspace filling, edited waypoints or tasks can be outdated in the
      // meantime.
      Map::instance->scheduleRedraw( Map::aeroLayer );
      break;

    case tpView:

      // only allow switching to this view if there is anything to see
      if( _globalMapContents->getCurrentTask() == static_cast<FlightTask *>(0) )
        {
          break;
        }

      /* no break is desired here */
      [[fallthrough]];

    case afView:
    case hsView:
    case naView:
    case olView:
    case rpView:
    case wpView:

      setRootWindow( false );
      m_listViewTabs->show();
      m_listViewTabs->setView( newView );

      if( newView == rpView )
        {
          setNearestOrReachableHeaders();
        }

      break;

    case flarmView:
      // called if Flarm view is created

      // save new view value
      view = newView;

      setRootWindow( false );

      toggleManualNavActions( false );
      toggleGpsNavActions( false );
      actionOpenContextMenu->setEnabled( false );
      toggleActions( false );
      break;

    default:
      qWarning( "MainWindow::setView(): unknown view %d to be set", newView );
      return;
    }
}

/**
 * Set nearest or reachable headers.
 */
void MainWindow::setNearestOrReachableHeaders()
{
  // Set the tabulator header according to calculation result.
  // If a glider is known, reachables by L/D are shown
  // otherwise the nearest points in 75 km radius are shown.
  QString header = QString(tr( "Reachables" ));

  if( calculator->getReachList()->getCalcMode() == ReachableList::distance )
    {
      // nearest sites are calculated
      header = QString(tr( "Nearest" ));
    }

  // update menu display
  actionViewReachpoints->setText( header );

  // update list view tabulator header
  m_listViewTabs->setTextRp( header );
}

/** Switches to the WaypointList View */
void MainWindow::slotSwitchToWPListView()
{
  setView( wpView );
}

/**
 * Called from the mapView, if the waypoint label is clicked.
 */
void MainWindow::slotSwitchToWPListViewExt()
{
  if ( _globalMapContents->getCurrentTask() )
    {
      // Switch to the task view, if a task is set.
      setView( tpView );
      return;
    }

  if( calculator->moving() &&
      GeneralConfig::instance()->getNearestSiteCalculatorSwitch() )
    {
      // Cumulus is moving and site calculator switch is enabled.
      // We are switching to the reachable list view.
      setView( rpView );
      return;
    }

  setView( wpView );
}

/** Switches to the AirfieldList View */
void MainWindow::slotSwitchToAFListView()
{
  setView( afView );
}

/** Switches to the HotspotList View */
void MainWindow::slotSwitchToHSListView()
{
  setView( hsView );
}

/** Switches to the OutlandingList View */
void MainWindow::slotSwitchToOLListView()
{
  setView( olView );
}

/** Switches to the list with all the nav aids. */
void MainWindow::slotSwitchToNavAidsListView()
{
  setView( naView );
}

/** Switches to the ReachablePointList View */
void MainWindow::slotSwitchToReachListView()
{
  setView( rpView );
}

/** Switches to the Task List View */
void MainWindow::slotSwitchToTaskListView()
{
  setView( tpView );
}

/** This slot is called to switch to the info view. */
void MainWindow::slotSwitchToInfoView()
{
  if ( view == wpView )
    {
      slotSwitchToInfoView( viewWP->getCurrentEntry() );
    }
  else if ( view == rpView )
    {
      slotSwitchToInfoView( viewRP->getCurrentEntry() );
    }
  else if ( view == afView )
    {
      slotSwitchToInfoView( viewAF->getCurrentEntry() );
    }
  else if ( view == olView )
    {
      slotSwitchToInfoView( viewOL->getCurrentEntry() );
    }
  else if ( view == tpView )
    {
      slotSwitchToInfoView( viewTP->getCurrentEntry() );
    }
  else if ( view == naView )
    {
      slotSwitchToInfoView( viewNA->getCurrentEntry() );
    }
  else if ( view == hsView )
    {
      slotSwitchToInfoView( viewHS->getCurrentEntry() );
    }
  else
    {
      // That is a bad solution with that cast but a fast workaround.
      slotSwitchToInfoView( const_cast<Waypoint *>(calculator->getTargetWp()) );
    }
}

/** This slot is called to switch to the info view and to show the waypoint data. */
void MainWindow::slotSwitchToInfoView( Waypoint* wp )
{
  if( ! wp )
    {
      return;
    }

  WPInfoWidget* viewInfo = new WPInfoWidget( this );

  connect( viewInfo, SIGNAL( addWaypoint( Waypoint& ) ),
           viewWP, SLOT( slot_addWp( Waypoint& ) ) );
  connect( viewInfo, SIGNAL( deleteWaypoint( Waypoint& ) ),
           viewWP, SLOT( slot_deleteWp( Waypoint& ) ) );
  connect( viewInfo, SIGNAL( waypointEdited( Waypoint& ) ),
           viewWP, SLOT( slot_wpEdited( Waypoint& ) ) );
  connect( viewInfo, SIGNAL( selectWaypoint( Waypoint*, bool ) ),
           calculator, SLOT( slot_WaypointChange( Waypoint*, bool ) ) );
  connect( viewInfo, SIGNAL( newHomePosition( const QPoint& ) ),
           _globalMapMatrix, SLOT( slotSetNewHome( const QPoint& ) ) );
  connect( viewInfo, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  // Note, this widget can be opened from different dialog widgets.
  if( isRootWindow() )
    {
      setRootWindow( false );
      connect( viewInfo, SIGNAL( closingWidget() ),
               SLOT( slotSubWidgetClosed() ) );
    }

  viewInfo->showWP( view, *wp );
  viewInfo->show();
}

/** Opens the configuration widget */
void MainWindow::slotOpenConfig()
{
  ConfigWidget *cDlg = new ConfigWidget( this );
  configView = static_cast<QWidget *> (cDlg);

  connect( cDlg, SIGNAL( closeConfig() ), this, SLOT( slotCloseConfig() ) );
  connect( cDlg, SIGNAL( closeConfig() ), this, SLOT( slotSubWidgetClosed() ) );
  connect( cDlg, SIGNAL( gotoHomePosition() ),
           calculator, SLOT( slot_changePositionHome() ) );

  setRootWindow( false );
  cDlg->setVisible( true );
}

/** Closes the configuration or Flying widget */
void MainWindow::slotCloseConfig()
{
  if ( !calculator->gliderType().isEmpty() )
    {
      setWindowTitle ( "Cumulus - " + calculator->gliderType() );
    }
  else
    {
      setWindowTitle( "Cumulus" );
    }

  setNearestOrReachableHeaders();
  setView( mapView );
}

/** Shows version, copyright, license and team information */
void MainWindow::slotVersion()
{
  AboutWidget *aw = new AboutWidget( this );

  aw->setWindowIcon( QIcon(GeneralConfig::instance()->loadPixmap("cumulus-desktop26x26.png")) );
  aw->setWindowTitle( tr( "About Cumulus") );
  aw->setHeaderIcon( GeneralConfig::instance()->loadPixmap("cumulus-desktop48x48.png") );

  QString header( tr("<html>Cumulus %1, &copy; 2002-2021, The Cumulus-Team</html>").arg( QCoreApplication::applicationVersion() ) );

  aw->setHeaderText( header );

  QString about( tr(
          "<hml>"
          "Cumulus %1, compiled at %2 with QT %3<br><br>"
          "Homepage: <a href=\"http://www.kflog.org/cumulus/\">www.kflog.org/cumulus/</a><br><br>"
          "Software Repository: <a href=\"https://github.com/kflog-project/Cumulus\">github.com/kflog-project/Cumulus</a><br><br>"
          "ChangeLog <a href=\"https://github.com/kflog-project/Cumulus/blob/%1/ChangeLog\">%1</a><br><br>"
          "Report bugs to: <a href=\"mailto:kflog.cumulus&#64;gmail.com\">kflog.cumulus&#64;gmail.com</a> or to <a href=\"https://github.com/kflog-project/Cumulus/issues\">GitHub</a><br><br>"
          "Published under the <a href=\"http://www.gnu.org/licenses/licenses.html#GPL\">GPL</a>"
          "</html>" ).arg( QCoreApplication::applicationVersion() )
                     .arg( GeneralConfig::instance()->getBuiltDate() )
                     .arg( QT_VERSION_STR ) );

  aw->setAboutText( about );

  QString team( tr(
          "<hml>"
          "<b>Project Leader</b>"
          "<blockquote>"
          "Axel Pauli &lt;<a href=\"mailto:kflog.cumulus&#64;gmail.com\">kflog.cumulus&#64;gmail.com</a>&gt;"
          "</blockquote>"
          "<b>Developers</b>"
          "<blockquote>"
          "Axel Pauli (Developer, Maintainer)<br>"
          "Andr&eacute; Somers (Core-developer)<br>"
          "Eggert Ehmke (Core-developer)<br>"
          "Eckhard V&ouml;llm (Developer, NMEA Simulator)<br>"
          "Josua Dietze (Developer)<br>"
          "Michael Enke (Developer)<br>"
          "Hendrik Hoeth (Developer)<br>"
          "Florian Ehinger (KFLog-developer)<br>"
          "Harald Maier (KFLog-developer)<br>"
          "Heiner Lamprecht (KFLog-developer)<br>"
          "Thomas Nielsen (KFLog-developer)"
          "</blockquote>"
          "<b>Contributors</b>"
          "<blockquote>"
          "Robin King (Help pages)<br>"
          "Peter Turczak (Code Optimizations)<br>"
          "Hendrik M&uuml;ller<br>"
          "Stephan Danner<br>"
          "Derrick Steed"
          "</blockquote>"
          "<b>Server Sponsor</b>"
          "<blockquote>"
          "Heiner Lamprecht &lt;<a href=\"mailto:heiner&#64;kflog.org\">heiner&#64;kflog.org</a>&gt;"
          "</blockquote>"
          "Thanks to all, who have made available this software!"
          "<br></html>" ));

  aw->setTeamText( team );

  QString disclaimer( tr(
            "<hml>"
            "This program comes with"
            "<p><b>ABSOLUTELY NO WARRANTY!</b></p>"
            "Do not rely on this software program as your "
            "primary source of navigation. As pilot in command you are "
            "responsible for using official aeronautical "
            "charts and proper methods for safe navigation. "
            "The information presented by this application "
            "may be outdated or incorrect."
            "<p><b>Don't use this program if you cannot accept the disclaimer!</b></p>"
            "</html>" ));

  aw->setDisclaimerText( disclaimer );

  aw->resize( size() );
  aw->setVisible( true );
}

/** opens help documentation in browser. */
void MainWindow::slotHelp()
{
  HelpBrowser *hb = new HelpBrowser(this);
  hb->resize( size() );
  hb->setVisible( true );
}

void MainWindow::slotRememberWaypoint()
{
  static uint count = 1;
  QString name;

  name = QString( tr("W%1-%2") ).arg( count ).arg( QTime::currentTime().toString("HH:mm") );

  // @AP: let us check, if the user waypoint is already known from its
  // position. In this case we will reject the insertion to avoid senseless
  // duplicates.

  QPoint pos = calculator->getlastPosition();

  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  for ( int i = 0; i < wpList.count(); i++ )
    {
      const Waypoint &wpItem = wpList.at(i);

      if ( wpItem.wgsPoint == pos )
        {
          return ; // we have such position already
        }
    }

  count++;

  Waypoint wp;

  wp.name = name;
  wp.wgsPoint = calculator->getlastPosition();
  wp.projPoint = _globalMapMatrix->wgsToMap( wp.wgsPoint );
  wp.description = tr( "user created" );
  wp.comment = tr("created by remember action at") + QString(" ") +
  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
  wp.priority = Waypoint::High; // high to make sure it is visible
  AltitudeCollection alt = calculator->getAltitudeCollection();
  wp.elevation = int ( ( alt.gpsAltitude - alt.gndAltitude ).getMeters() );
  wp.type = BaseMapElement::UserPoint;
  wp.country = GeneralConfig::instance()->getHomeCountryCode();

  viewWP->slot_addWp( wp );

  // qDebug("WP lat=%d, lon=%d", wp.origP.lat(), wp.origP.lon() );
}

/** This slot is called if the configuration has been changed and at the
    start of the program to read the initial configuration. */
void MainWindow::slotReadconfig()
{
  // qDebug() << "MainWindow::slotReadconfig()";

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
  viewHS->listWidget()->configRowHeight();
  viewNA->listWidget()->configRowHeight();
  viewOL->listWidget()->configRowHeight();
  viewWP->listWidget()->configRowHeight();

  GeneralConfig *conf = GeneralConfig::instance();

  // Update action settings
  actionToggleAfLabels->setChecked( conf->getMapShowAirfieldLabels() );
  actionToggleOlLabels->setChecked( conf->getMapShowOutLandingLabels() );
  actionToggleTpLabels->setChecked( conf->getMapShowTaskPointLabels() );
  actionToggleWpLabels->setChecked( conf->getMapShowWaypointLabels() );
  actionToggleLabelsInfo->setChecked( conf->getMapShowLabelsExtraInfo() );
  actionToggleTrailDrawing->setChecked( conf->getMapDrawTrail() );

  // configure reconnect of GPS receiver in case of process stop
  QString device = conf->getGpsDevice();

#ifndef ANDROID
  if( device.startsWith("/dev/" ) )
    {
      // @ee install signal handler
      signal( SIGCONT, resumeGpsConnection );
    }
#endif

  GpsNmea::gps->slot_reset();

  // update menubar font size
  slotSetMenuFontSize();

  actionViewReachpoints->setEnabled( conf->getNearestSiteCalculatorSwitch() );

 // Check, if reachable list is to show or not
  if( conf->getNearestSiteCalculatorSwitch() )
    {
      if( ! m_reachpointListVisible )
        {
          // list was not visible before
          calculator->newSites();
          m_reachpointListVisible = true;
        }
    }
  else
    {
      if( m_reachpointListVisible )
        {
          calculator->clearReachable();
          viewRP->clearList(); // this clears the reachable list in the view
          Map::instance->scheduleRedraw(Map::waypoints);
          m_reachpointListVisible = false;
        }
    }

  // Check, if outlanding list is to show or not
  if( m_outlandingListVisible )
    {
      viewRP->clearList();  // this clears the outlanding list in the view
      Map::instance->scheduleRedraw(Map::outlandings);
      m_outlandingListVisible = false;
    }

  Map::instance->scheduleRedraw();
}

void MainWindow::slotGpsStatus( GpsNmea::GpsStatus status )
{
  static bool onePlay = false;

  if ( ( status != GpsNmea::validFix || calculator->isManualInFlight()) && ( view == mapView ) )
    {  // no GPS data
      toggleManualNavActions( true );
      toggleGpsNavActions( false );
    }
  else
    {  // GPS data valid
      if( onePlay == false )
        {
          // Only the first connect is reported via sound after the startup.
          // That shall prevent multiple notifications if the GPS receiver
          // plays ping pong (have a fix, have not a fix...) That can be very
          // annoying. The current GPS state is signaled optical now.
          playSound("notify");
          onePlay = true;
        }

      toggleManualNavActions( false );
      toggleGpsNavActions( true );
    }
}

void MainWindow::slotCenterToTarget()
{

  if ( calculator->getTargetWp() )
    {
      _globalMapMatrix->centerToLatLon( calculator->getTargetWp()->wgsPoint );
      Map::instance->scheduleRedraw();
    }
}

void MainWindow::slotEnsureVisible()
{
  if ( calculator->getTargetWp() )
    {
      double newScale = _globalMapMatrix->ensureVisible( calculator->getTargetWp()->wgsPoint );

      if ( newScale > 0 )
        {
          Map::instance->slotSetScale( newScale );
        }
      else
        {
          viewMap->message( tr( "Waypoint out of map range." ) );
        }
    }
}

void MainWindow::slotRemoveTarget()
{
  calculator->setTargetWp( 0 );
}

void MainWindow::slotOpenPreFlightConfig()
{
  PreFlightWidget* cDlg = new PreFlightWidget( this );
  configView = static_cast<QWidget *> (cDlg);

  connect( cDlg, SIGNAL( closeConfig() ), this, SLOT( slotCloseConfig() ) );
  connect( cDlg, SIGNAL( closeConfig() ), this, SLOT( slotSubWidgetClosed() ) );

  setRootWindow( false );
  cDlg->setVisible( true );
}

void MainWindow::slotPreFlightDataChanged()
{
  // set the task list view at the current task
  viewTP->slot_setTask( _globalMapContents->getCurrentTask() );
  Map::instance->scheduleRedraw(Map::task);
}

/** Dynamically updates view for reachable list */
void MainWindow::slotNewReachList()
{
  viewRP->slot_newList();
  Map::instance->scheduleRedraw(Map::waypoints);
}

void MainWindow::keyPressEvent( QKeyEvent* event )
{
#ifdef ANDROID

  // qDebug( "MW KeyPress Key=%x", event->key() );

  // Sent by native method "nativeKeypress"
  if( event->key() == Qt::Key_F11 )
    {
      // Open setup from Android menu
      if ( isRootWindow() )
        {
          slotOpenConfig();
        }

      return;
    }

  if( event->key() == Qt::Key_F12 )
    {
      // Open Flying setup from Android menu
      if ( isRootWindow() )
        {
          slotOpenPreFlightConfig();
        }

      return;
    }

  if( event->key() == Qt::Key_F13 )
    {
      // Open GPS status window from Android menu
      if ( isRootWindow() )
        {
          actionStatusGPS->activate(QAction::Trigger);
        }

      return;
    }

  if( event->key() == Qt::Key_D )
    {
      // toggle left map sidebar
      if ( isRootWindow() )
        {
          slotToggleMapSidebar();
        }

      return;
    }


#endif

  QWidget::keyPressEvent( event );
}

void MainWindow::keyReleaseEvent( QKeyEvent* event )
{
#ifdef ANDROID

  // qDebug( "MW KeyRelease Key=%x", event->key() );

  if( event->key() == Qt::Key_Close )
    {
      // Quit application is requested from Android menu to ensure a safe shutdown.
      if ( isRootWindow() )
        {
          close();
          return;
        }
    }

#endif

  QWidget::keyReleaseEvent( event );
}

/** Called to select the home site position */
void MainWindow::slotNavigate2Home()
{
  Waypoint wp;

  wp.name = GeneralConfig::instance()->getHomeName();
  wp.description = tr("Home Site");
  wp.wgsPoint.setPos( GeneralConfig::instance()->getHomeCoord() );
  wp.elevation = GeneralConfig::instance()->getHomeElevation().getMeters();
  wp.country = GeneralConfig::instance()->getHomeCountryCode();

  calculator->slot_WaypointChange( &wp, true );
}

void MainWindow::slotToggleGps( bool on )
{
  if( calculator->moving() )
    {
      // Do not switch off GPS when we are in move.
      return;
    }

  if( on == false )
    {
      GpsNmea::gps->fixNOK( "User" );
      GpsNmea::gps->blockSignals( true );
    }
  else
    {
      GpsNmea::gps->blockSignals( false );
      GpsNmea::gps->fixOK( "User" );
    }

  toggleManualNavActions( !on );
  toggleGpsNavActions( on );
}

/** Used to allow or disable user keys processing during map drawing. */
void MainWindow::slotMapDrawEvent( bool drawEvent )
{
   if( drawEvent )
     {
      // Disable menu shortcut during drawing to avoid
      // event avalanche, if the user holds the key down longer.
      actionOpenContextMenu->setEnabled( false );

      if( view == mapView )
       {
         toggleManualNavActions( false );
         toggleGpsNavActions( false );
       }
     }
   else
     {
       actionOpenContextMenu->setEnabled( true );

       if( view == mapView )
         {
           toggleManualNavActions( GpsNmea::gps->getGpsStatus() != GpsNmea::validFix ||
                                   calculator->isManualInFlight() );

           toggleGpsNavActions( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix &&
                                !calculator->isManualInFlight() );
         }
     }
}

/**
 * Called if a subwidget is opened.
 */
void MainWindow::slotSubWidgetOpened()
{
  // Reset the root window flag
  m_rootWindow = false;
}

/**
 * Called if an opened subwidget is closed.
 */
void MainWindow::slotSubWidgetClosed()
{
  // Set the root window flag
  m_rootWindow = true;
}

/**
 * Called if logger recognized takeoff.
 */
void MainWindow::slotTakeoff( QDateTime& dt )
{
  slotNotification( tr("takeoff") + dt.time().toString(" HH:mm"), true );
}
/**
 * Called if logger recognized landing.
 */
void MainWindow::slotLanded( QDateTime& dt )
{
  slotNotification( tr("landed")+ dt.time().toString(" HH:mm"), true );
}

void MainWindow::slotCloseSip()
{
  qDebug() << "MainWindow::slotCloseSip()" << QApplication::focusWidget();

  // Get the widget which has the keyboard focus assigned
  QWidget *widget = QApplication::focusWidget();

  if( ! widget )
    {
      // No widget has the focus.
      return;
    }

#ifdef ANDROID

  if( jniShutdownFlag() == true )
    {
      // We are in shutdown
      return;
    }

#endif

  // Request the SIP closing from the focused widget
  QEvent event( QEvent::CloseSoftwareInputPanel );
  QApplication::sendEvent( widget, &event );
}

bool MainWindow::isRootWindow()
{
  if( mainWindow() == 0 || _globalMapView == 0 )
    {
      return false;
    }

  if( _globalMapView->getMap()->isVisible() && m_rootWindow == true )
    {
      return true;
    }
  else
    {
      return false;
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
  qDebug("MainWindow::resizeEvent(): w=%d, h=%d", event->size().width(), event->size().height() );
  // resize list view tabs, if current widget was modified
  if( m_listViewTabs )
    {
      m_listViewTabs->resize( event->size() );
    }

  if( configView )
    {
      configView->resize( event->size() );
    }

  QMainWindow::resizeEvent(event);
}

#ifdef MAEMO

/** Called to prevent the switch off of the screen display */
void MainWindow::slotDisplayTrigger()
{
  // If the speed is greater or equal as the limit and we have a connected
  // GPS we switch off the screen saver. Otherwise we let all as it is.
  double speedLimit = GeneralConfig::instance()->getScreenSaverSpeedLimit();

  if( speedLimit == 0.0 ||
      ( calculator->getLastSpeed().getKph() >= speedLimit &&
      GpsNmea::gps->getConnected() ) )
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
  m_displayTrigger->start( 10000 );
}

#endif

#ifdef ANDROID

/** Called to control the brightness of the the screen display */
void MainWindow::slotDisplayTrigger()
{
  // If the speed is greater or equal as the limit and we have a connected
  // GPS we switch off the screen saver. Otherwise we let all as it is.
  double speedLimit = GeneralConfig::instance()->getScreenSaverSpeedLimit();

  if( speedLimit == 0.0 ||
      ( calculator->getLastSpeed().getKph() >= speedLimit &&
      GpsNmea::gps->getConnected() ) )
    {
      // tells Android CumulusActivity to brighten the screen
      jniDimmScreen( false );
    }
  else
    {
      // tells Android CumulusActivity to dimm the screen
      jniDimmScreen( true );
    }

  // Restart the timer because we use a single shot timer to avoid
  // multiple triggering in case of delays. Next trigger is in 10s.
  m_displayTrigger->start( 10000 );
}

void MainWindow::slotOpenHardwareMenu()
{
  jniOpenHardwareMenu();
}

#endif
