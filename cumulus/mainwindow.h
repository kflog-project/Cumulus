/***************************************************************************
                          mainwindow.h  -  main application object
                             -------------------
   begin                : Sun Jul 21 2002
   copyright            : (C) 2002      by André Somers
   ported to Qt4.x/X11  : (C) 2007-2009 by Axel Pauli
   email                : axel@kflog.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   $Id$

 ***************************************************************************/
/**
 * \author André Somers, Axel Pauli
 *
 * \brief This class provides the main window of Cumulus.
 *
 * This class provides the main window of Cumulus. All needed other stuff
 * is initialized and handled here:
 *
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
                   cfView=8 };      // configuration

  public: //methods
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
    virtual void setView (const appView& _newVal, const wayPoint* wp = 0);

    /**
     * @returns the view type
     */
    virtual appView getView();

    /**
     * play some sound
     */
    void playSound(const char *name=0);

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
    WPInfoWidget *viewInfo;     // waypoint info
    QTabWidget *listViewTabs;

    /** empty view for config "dialog" */
    QWidget *viewCF;

  public slots: // Public slots
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
     * Exits Cumulus
     */
    void slotFileQuit();
    /** Switches to mapview. */
    void slotSwitchToMapView();
    /** This slot is called to switch to the info view. */
    void slotSwitchToInfoView();
    /** @ee This slot is called to switch to the info view with selected waypoint. */
    void slotSwitchToInfoView(wayPoint*);
    /** Opens the config "dialog". */
    void slotConfig();
    /** Opens the pre-flight "dialog". */
    void slotPreFlight(const char *tabName);
    /** This slot is called if the configuration has changed and at the start of the program to read the initial configuration. */
    void slotReadconfig();
    /** Called if the status of the GPS changes, and controls the availability of manual navigation. */
    void slotGpsStatus(GpsNmea::GpsStatus status);
    /** Opens the pre flight dialog */
    void slotPreFlightGlider();
    void slotPreFlightTask();
    /** shows resp. signals a notifiation */
    void slotNotification (const QString&, const bool sound=true);
    /** shows resp. signals an alarm */
    void slotAlarm (const QString&, const bool sound=true);
    /** updates the list of reachable points  */
    void slotNewReachList();
    /** use manual navigation even if GPS signal received */
    void slotToggleManualInFlight(bool);
    /** used to allow or disable user keys processing during map drawing */
    void slotMapDrawEvent(bool);
    /** closes the config or pre-flight "dialog" */
    void slotCloseConfig();
    /** set menubar font size to a reasonable and useable value */
    void slotSetMenuBarFontSize();

  protected: //methods
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject*, QEvent*);

    /**
     * Redefinition of the resizeEvent.
     */
    virtual void resizeEvent(QResizeEvent* event);

    /**
     * No descriptions
     */
    void initActions();

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
     * No descriptions
     */
    void initMenuBar();

    /**
     * Make sure the user really wants to quit by asking
     * for confirmation
     */
    virtual void closeEvent (QCloseEvent*);


  protected: //members
    /** contains the currently selected viewmode */
    appView view;

  private slots: // Private slots
    /**
     * This slot is called if the user presses C in manual
     * navigation mode. It centers the map on the current waypoint.
     */
    void slotCenterToWaypoint();

    /**
     * Called if the user pressed V in mapview. Adjusts the
     * zoomfactor so that the currently selected waypoint is displayed
     * as good as possible.
     */
    void slotEnsureVisible();

    void slotToggleMenu();

    void slotViewStatusBar(bool toggle);

    /**
     * Called if the logging is actually toggled
     */
    void slot_Logging (bool logging);

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
     * Called if new prefight data were set
     */
    void slotPreFlightDataChanged();

    /**
     * Called if the user clicks on a tab to select a different
     * list-type view
     */
    void slot_tabChanged( int index );

    /** shows version and copyright. */
    void slotVersion();

    /** opens help documentation in browser. */
    void slotHelp();

    /** creates the application widgets after the base initialization
     *  of the core application window.
     */
    void slotCreateApplicationWidgets();

  private:
    /**
     * set nearest or reachable headers
     */
    void setNearestOrReachableHeaders();

  public:

    /** use manual navigation even if GPS signal received */
    QAction* actionToggleManualInFlight;

  private:

    QAction* actionManualNavUp;
    QAction* actionManualNavRight;
    QAction* actionManualNavDown;
    QAction* actionManualNavLeft;
    QAction* actionManualNavHome;
    QAction* actionManualNavWP;
    QAction* actionManualNavWPList;
    QAction* actionGpsNavUp;
    QAction* actionGpsNavDown;
    QAction* actionNav2Home;
    QAction* actionGpsNavWPList;
    QAction* actionGpsNavZoomIn;
    QAction* actionGpsNavZoomOut;
    QAction* actionMenuBarToggle;
    QAction* actionToggleMenu;
    QAction* actionFileQuit;
    QAction* actionViewInfo;
    QAction* actionViewWaypoints;
    QAction* actionViewAirfields;
    QAction* actionViewReachpoints;
    QAction* actionViewTaskpoints;
    QAction* actionViewGPSStatus;
    QAction* actionToggleStatusbar;
    QAction* actionZoomInZ;
    QAction* actionZoomOutZ;

    QAction* actionToggleAfLabels;
    QAction* actionToggleOlLabels;
    QAction* actionToggleTpLabels;
    QAction* actionToggleWpLabels;
    QAction* actionToggleLabelsInfo;

    QAction* actionToggleLogging;
    QAction* actionEnsureVisible;
    QAction* actionSelectTask;
    QAction* actionPreFlight;
    QAction* actionSetupConfig;
    QAction* actionSetupInFlight;
    QAction* actionHelpCumulus;
    QAction* actionHelpAboutApp;
    QAction* actionHelpAboutQt;
    QAction* actionStartFlightTask;

    /* shortcut for exit application */
    QShortcut* scExit;

    /** fileMenu contains all items of the menubar entry "File" */
    QMenu *fileMenu;
    /** viewMenu contains all items of the menubar entry "View" */
    QMenu *viewMenu;
    /** mapMenu contains all items of the menubar entry "Map" */
    QMenu *mapMenu;
    /** labelMenu contains all items of the menubar subentry "Label" */
    QMenu *labelMenu;
    /** setupMenu contains all items of the menubar entry "Setup" */
    QMenu *setupMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    QMenu *helpMenu;
    // Wait screen
    QPointer<WaitScreen> ws;
    // Holds temporary the config or pre-flight widgets
    QPointer<QWidget> configView;
    // visibility of menu bar
    bool menuBarVisible;
    // Splash screen
    QPointer<Splash> splash;
    // instance of IGC logger
    IgcLogger *logger;
    // Store here, if the lists are visible or not.
    bool _taskListVisible;
    bool _reachpointListVisible;
    bool _outlandingListVisible;

#ifdef MAEMO

  private:

    /** Timer for triggering display on. */
    QTimer *ossoDisplayTrigger;

  private slots:

    /** Called to prevent the switch off of the display */
    void slotOssoDisplayTrigger();

#endif

};

#endif
