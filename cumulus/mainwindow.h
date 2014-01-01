/***************************************************************************
                          mainwindow.h  -  main application object
                             -------------------
   begin                : Sun Jul 21 2002
   copyright            : (C) 2002      by André Somers
   ported to Qt4.x/X11  : (C) 2007-2013 by Axel Pauli
   email                : Axel Pauli <kflog.cumulus@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   $Id$

 ***************************************************************************/

/**
 * \class MainWindow
 *
 * \author André Somers, Axel Pauli
 *
 * \brief This class provides the main window of Cumulus.
 *
 * This class provides the main window of Cumulus. All needed stuff
 * is initialized and handled here.
 *
 * \date 2002-2013
 *
 * \version $Id$
 */

#ifndef _MainWindow_h
#define _MainWindow_h

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QEvent>
#include <QTabWidget>
#include <QResizeEvent>
#include <QShortcut>
#include <QPointer>

#include "igclogger.h"
#include "mapview.h"
#include "waypointlistview.h"
#include "airfieldlistview.h"
#include "reachpointlistview.h"
#include "tasklistview.h"
#include "wpinfowidget.h"
#include "gpsnmea.h"
#include "mapinfobox.h"
#include "waitscreen.h"
#include "splash.h"

#ifdef INTERNET
#include "LiveTrack24Logger.h"
#endif

extern MainWindow  *_globalMainWindow;

class MainWindow : public QMainWindow
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MainWindow )

public: // application view types

  enum appView { mapView=0,       // map
                 wpView=1,        // waypoint
                 infoView=2,      // info
                 rpView=3,        // reachable
                 afView=4,        // airfield
                 olView=5,        // outlanding
                 tpView=6,        // taskpoint
                 tpSwitchView=7,  // taskpoint switch
                 cfView=8,        // configuration
                 flarmView=9 };   // flarm view

public:
  /**
   * Constructor
   */
  MainWindow( Qt::WindowFlags flags = 0 );

  /**
   * Destructor
   */
  virtual ~MainWindow();

  /**
   * Sets the view type
   */
  void setView(const appView& _newVal, const Waypoint* wp = 0);

  /**
   * @returns the view type
   */
  appView getView() const
  {
    return view;
  }

  /**
   * plays some sound.
   */
  void playSound(const char *name=0);

  /**
   * Returns the pointer to this class.
   *
   * \return A pointer to the main window instance.
   */
  static MainWindow* mainWindow()
  {
    return _globalMainWindow;
  };

  /**
   * \return The state of the root window flag.
   */
  static bool isRootWindow()
  {
    return m_rootWindow;
  };

  static void setRootWindow( bool value)
  {
    m_rootWindow = value;
  };

#ifdef INTERNET

  /**
   * Returns the LiveTrack24 logger instance.
   *
   * \return A LiveTrack24 logger instance
   */
  LiveTrack24Logger* getLiveTrack24Logger() const
  {
    return m_liveTrackLogger;
  };

#endif

#ifdef ANDROID
  void forceFocus();
#endif

public:
  /**
   * Reference to the Map pages
   */
  MapView *viewMap;
  WaypointListView *viewWP;   // waypoints
  AirfieldListView *viewAF;   // airfields
  AirfieldListView *viewOL;   // outlandings
  ReachpointListView *viewRP; // reachable points
  TaskListView *viewTP;       // task points
  WPInfoWidget *viewInfo;     // POI info
  QTabWidget *listViewTabs;

  /** empty view for config "dialog" */
  QWidget *viewCF;

public slots:

  /** Switches to the WaypointList View */
  void slotSwitchToWPListView();
  /**
   * Switches to the WaypointList View if there is
   * no task, and to the task list if there is.
   */
  void slotSwitchToWPListViewExt();
  /**
   * Switches to the list with all loaded airfields
   */
  void slotSwitchToAFListView();
  /**
   * Switches to the list with all loaded outlandings
   */
  void slotSwitchToOLListView();
  /**
   * Switches to the list with all the reachable fields
   */
  void slotSwitchToReachListView();
  /**
   * Switches to the list with waypoints in this task
   */
  void slotSwitchToTaskListView();
  /**
   * Makes Cumulus store the current location as a waypoint
   */
  void slotRememberWaypoint ();
  /**
   * Navigates to the home site.
   */
  void slotNavigate2Home();
  /**
   * Exits Cumulus, if the disclaimer was rejected..
   */
  void slotDisclaimerQuit();
  /**
   * Switches to the mapview.
   */
  void slotSwitchToMapView();
  /** This slot is called to switch to the info view. */
  void slotSwitchToInfoView();
  /** This slot is called to switch to the info view and to show the waypoint data. */
  void slotSwitchToInfoView(Waypoint* wp);

  /** Opens the general configuration dialog. */
  void slotOpenConfig();

  /** Opens the pre-flight dialog. */
  void slotOpenPreFlightConfig();

  /**
   * This slot is called if the configuration has changed and at the start
   * of the program to read the initial configuration.
   */
  void slotReadconfig();

  /**
   * Called if the status of the GPS changes, and controls the availability
   * of manual navigation.
   */
  void slotGpsStatus(GpsNmea::GpsStatus status);
  /** shows resp. signals a notification */
  void slotNotification (const QString&, const bool sound=true);
  /** shows resp. signals an alarm */
  void slotAlarm (const QString&, const bool sound=true);
  /** updates the list of reachable points  */
  void slotNewReachList();
  /** switch on/off GPS data processing */
  void slotToggleGps(bool);
  /** used to allow or disable user keys processing during map drawing */
  void slotMapDrawEvent(bool);
  /** closes the config or pre-flight "dialog" */
  void slotCloseConfig();
  /** sets the menu fonts to a reasonable and usable value */
  void slotSetMenuFontSize();
  /**
   * Called if a subwidget is opened.
   */
  void slotSubWidgetOpened();
  /**
   * Called if an opened subwidget is closed.
   */
  void slotSubWidgetClosed();
  /**
   * Called if logger recognized takeoff.
   */
  void slotTakeoff( QDateTime& dt );
  /**
   * Called if logger recognized landing.
   */
  void slotLanded( QDateTime& dt );

protected:

  virtual void keyPressEvent( QKeyEvent* event );

  virtual void keyReleaseEvent( QKeyEvent* event );

  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

  /**
   * No descriptions
   */
  void createActions();

  /**
   * Toggle on/off all actions which have a key accelerator defined.
   */
  void toggleActions( const bool toggle );

  /**
   * Toggle on/off all GPS dependent actions.
   */
  void toggleManualNavActions( const bool toggle );
  void toggleGpsNavActions( const bool toggle );

  /**
   * Creates the menu bar
   */
  void createMenuBar();

  /**
   * Creates the context menu
   */
  void createContextMenu();

  /**
   * Make sure the user really wants to quit by asking
   * for confirmation
   */
  virtual void closeEvent (QCloseEvent*);


protected:

  /** contains the currently selected view mode */
  appView view;

private slots:
  /**
   * This slot is called if the user presses C in manual
   * navigation mode. It centers the map on the current waypoint.
   */
  void slotCenterToWaypoint();

  /**
   * Called if the user pressed V in map view. Adjusts the
   * zoom factor so that the currently selected waypoint is displayed
   * as good as possible.
   */
  void slotEnsureVisible();

  /**
   * Called to toggle the menu bar.
   */
  void slotToggleMenu();

  /**
   * Called to show the context menu.
   */
  void slotShowContextMenu();

  /**
   * Called to toggle the window size.
   */
  void slotToggleWindowSize();

  /**
   * Called to toggle the left map sidebar size.
   */
  void slotToggleMapSidebar();

  /**
   * Called to show or hide the status bar.
   */
  void slotViewStatusBar(bool toggle);

  /**
   * Called if the logging is actually toggled
   */
  void slotLogging (bool logging);

  /**
   * Called if the label displaying is actually toggled
   */
  void slotToggleAfLabels (bool toggle);

  /**
   * Called if the label displaying is actually toggled
   */
  void slotToggleOlLabels (bool toggle);

  /**
   * Called if the label displaying is actually toggled
   */
  void slotToggleTpLabels (bool toggle);

  /**
   * Called if the label displaying is actually toggled
   */
  void slotToggleWpLabels (bool toggle);

  /**
   * Called if the extra label info displaying is actually toggled
   */
  void slotToggleLabelsInfo (bool toggle);

  /**
   * Called if the trail drawing is toggled
   */
  void slotToggleTrailDrawing( bool toggle );

  /**
   * Called if new prefight data were set
   */
  void slotPreFlightDataChanged();

  /**
   * Called if the user clicks on a tab to select a different
   * list-type view
   */
  void slotTabChanged( int index );

  /** shows version and copyright. */
  void slotVersion();

  /** opens help documentation in browser. */
  void slotHelp();

  /**
   * Creates the application widgets after the base initialization
   * of the core application window.
   */
  void slotCreateApplicationWidgets();

  /**
   * Creates the disclaimer query widget.
   */
  void slotCreateDisclaimer();

  /**
   * Creates the splash screen.
   */
  void slotCreateSplash();

  /**
   * Finishes the startup after the map drawing.
   */
  void slotFinishStartUp();

  /**
   * Called to check for Welt2000 updates.
   */
  void slotCheck4Updates();

  /**
   * Called to open the Android hardware menu.
   */
#ifdef ANDROID
  void slotOpenHardwareMenu();
#endif

private:

  /**
   * set nearest or reachable headers
   */
  void setNearestOrReachableHeaders();

private:

  /** use manual navigation even if GPS signal is received */
  QAction* actionToggleGps;
  QAction* actionManualNavUp;
  QAction* actionManualNavRight;
  QAction* actionManualNavDown;
  QAction* actionManualNavLeft;
  QAction* actionManualNavMove2Home;
  QAction* actionManualNavMove2WP;
  QAction* actionManualNavWPList;
  QAction* actionGpsNavUp;
  QAction* actionGpsNavDown;
  QAction* actionNav2Home;
  QAction* actionGpsNavWPList;
  QAction* actionGpsNavZoomIn;
  QAction* actionGpsNavZoomOut;
  QAction* actionMenuBarToggle;
  QAction* actionOpenContextMenu;
  QAction* actionFileQuit;

#ifdef ANDROID
  QAction* actionHardwareMenu;
#endif

  QAction* actionViewInfo;
  QAction* actionViewWaypoints;
  QAction* actionViewAirfields;
  QAction* actionViewReachpoints;
  QAction* actionViewTaskpoints;

#ifdef FLARM
  QAction* actionViewFlarm;
#endif

  QAction* actionStatusGPS;
  QAction* actionStatusAirspace;

  QAction* actionZoomInZ;
  QAction* actionZoomOutZ;

  QAction* actionToggleStatusbar;
  QAction* actionToggleMapSidebar;
  QAction* actionToggleWindowSize;
  QAction* actionToggleAfLabels;
  QAction* actionToggleOlLabels;
  QAction* actionToggleTpLabels;
  QAction* actionToggleWpLabels;
  QAction* actionToggleLabelsInfo;
  QAction* actionToggleLogging;
  QAction* actionToggleTrailDrawing;

  QAction* actionEnsureVisible;
  QAction* actionSelectTask;
  QAction* actionPreFlight;
  QAction* actionSetupConfig;
  QAction* actionSetupInFlight;
  QAction* actionHelpCumulus;
  QAction* actionHelpAboutApp;
  QAction* actionHelpAboutQt;
  QAction* actionStartFlightTask;

  /* shortcut to toggle the left map sidebar. */
  QShortcut* scToggleMapSidebar;

  /* shortcut for exit application */
  QShortcut* scExit;

  /** Context main menu */
  QMenu *contextMenu;

  /** fileMenu contains all items of the menu entry "File" */
  QMenu *fileMenu;
  /** viewMenu contains all items of the menu entry "View" */
  QMenu *viewMenu;
  /** mapMenu contains all items of the menu entry "Map" */
  QMenu *mapMenu;
  /** statusMenu contains all items of the menu entry "Status" */
  QMenu *statusMenu;
  /** labelMenu is a main menu */
  QMenu *labelMenu;
  /** labelSubMenu contains all items of the menu entry "Label" */
  QMenu* labelSubMenu;
  /** setupMenu contains all items of the menu entry "Setup" */
  QMenu *setupMenu;
  /** helpMemu contains all items of the menu entry "Help" */
  QMenu *helpMenu;

  // Wait screen
  QPointer<WaitScreen> ws;
  // Splash screen
  QPointer<Splash> splash;
  // Holds temporary the configuration or pre-flight widgets
  QPointer<QWidget> configView;

  // visibility of menu bar
  bool m_menuBarVisible;

  // instance of IGC logger class
  IgcLogger* m_logger;

  // Store here, if the lists are visible or not.
  bool m_taskListVisible;
  bool m_reachpointListVisible;
  bool m_outlandingListVisible;

  // Flag to store if the root window is visible or not. Used by Android for the
  // menu display. Android popups only its menu if the main window is active and
  // not covered by another widget.
  static bool m_rootWindow;

#ifdef INTERNET
  /** LiveTrack24 logger object. */
  LiveTrack24Logger* m_liveTrackLogger;
#endif

#if defined ANDROID || defined MAEMO

private:

  /** Timer for triggering display on. */
  QTimer* m_displayTrigger;

private slots:

  /** Called to prevent the switch off of the display */
  void slotDisplayTrigger();

#endif

#ifdef ANDROID
private:
  QPoint forceFocusPoint;
#endif
};

#endif
