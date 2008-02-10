/***************************************************************************
                          cumulusapp.h  -  main application object
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli
    email                : andre@kflog.org, axel@kflog.org

    This file is distributed under the terms of the General Public
    Licence. See the file COPYING for more information.

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CUMULUSAPP_H
#define CUMULUSAPP_H

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <Q3Action>
#include <QMenuBar>
#include <Q3PopupMenu>
#include <QEvent>
#include <Q3Accel>
#include <QTabWidget>
#include <QResizeEvent>

#include "mapview.h"
#include "waypointlistview.h"
#include "airfieldlistview.h"
#include "reachpointlistview.h"
#include "tasklistview.h"
#include "wpinfowidget.h"
#include "gpsnmea.h"
#include "preflightdialog.h"
#include "gliderflightdialog.h"
#include "altimetermodedialog.h"
#include "variomodedialog.h"
#include "mapinfobox.h"
#include "waitscreen.h"

/**
 * @short This class provides the main application for Cumulus.
 * @author Andre Somers
 */

class CumulusApp : public QMainWindow
  {
    Q_OBJECT

  public: // application view types

    enum appView { mapView=0,
                   wpView=1,
                   infoView=2,
                   rpView=3,
                   afView=4,
                   tpView=5,
                   tpSwitchView=6 };

  public: //methods
    /**
     * Constructor
     */
    CumulusApp( QMainWindow *parent = 0, Qt::WindowFlags flags = 0 );

    /**
     * Destructor
     */
    virtual ~CumulusApp();

    /**
     * Sets the view type
     */
    virtual void setView (const appView& _newVal, const wayPoint* wp = 0);

    /**
     * @returns the view type
     */
    virtual const appView getView();

    /**
     * play some sound
     */
    static void playSound(const QString *name=0);

  public: // Public attributes
    /**
     * Reference to the Map page
     */
    MapView *viewMap;
    WaypointListView *viewWP;
    AirfieldListView *viewAF;
    ReachpointListView *viewRP;
    TaskListView *viewTP;
    WPInfoWidget *viewInfo;
    QTabWidget *listViewTabs;
    /** use manual navigation even if GPS signal received */
    Q3Action* actionToggleManualInFlight;

    Q3Accel *accAfView;
    Q3Accel *accInfoView;
    Q3Accel *accManualNav;
    Q3Accel *accGpsNav;
    Q3Accel *accRpView;
    Q3Accel *accTpView;
    Q3Accel *accWpView;
    Q3Accel *accMenuBar; // toggle for bing up/down menu bar

  public slots: // Public slots
    /** Switches to the WaypointList View */
    void slotSwitchToWPListView();
    /**
     * Switches to the WaypointList View if there is
     * no task, and to the task list if there is.
     */
    void slotSwitchToWPListViewExt();
    /**
     * Switches to the list with all loaded fields
     */
    void slotSwitchToAFListView();
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
     * Informs about a waypoint selection change, used for
     * activation/deactivation of info menu action
     */
    void slotWaypointChanged(const wayPoint* newWp);
    /**
     * Navigates to the home site (only in manual mode)
     */
    void slotNavigateHome();
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
    /** Opens the configdialog. */
    void slotConfig();
    /** Opens the inflight glider settings dialog. */
    void slotGliderFlight();
    /** Opens the inflight Altimeter settings dialog. */
    void slotAltimeterMode();
    /** Called if inflight Altimeter settings have changed */
    void slotNewAltimeterMode();
    /** Opens the Variometer settings dialog. */
    void slotVarioMode();
    /** This slot is called if the configuration has changed and at the start of the program to read the initial configuration. */
    void slot_readconfig();
    /** shows version and copyright. */
    void slotVersion();
    /** Called if the status of the GPS changes, and controls the availability of manual navigation. */
    void slotGpsStatus(GPSNMEA::connectedStatus status);
    /** Opens the pre flight dialog */
    void slotPreFlightGlider();
    void slotPreFlightTask();
    /** shows resp. signals a notifiation */
    void slotNotification (const QString&, const bool sound=true);
    /** updates the list of reachable points  */
    void slot_newReachList();
    /** shows the GPS status dialog */
    void slotGpsStatusDialog();
    /** use manual navigation even if GPS signal received */
    void slotToggleManualInFlight(bool);
    /** used to allow or disable user keys processing during map drawing */
    void slotMapDrawEvent(bool);

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
     * Toggle on/off all actions, which have a key accelerator defined.
     */
    void toggelActions( const bool toggle );

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

    PreFlightDialog* _preFlightDialog;

    GliderFlightDialog* _gliderFlightDlg;

    AltimeterModeDialog* _altimeterModeDlg;

    VarioModeDialog* _varioModeDlg;

    bool _taskListVisible, _reachpointListVisible;

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
    void slotToggleWpLabels (bool toggle);

    /**
     * Called if the extra label info displaying is actually toggled
     */
    void slotToggleWpLabelsExtraInfo (bool toggle);

    /**
     * Called if new prefight data were set
     */
    void slotPreFlightDataChanged();

    /**
     * Called if the user clicks on a tab to select a different
     * list-type view
     */
    void slot_tabChanged(QWidget *);


  private:

    void slotPreFlight(const char *tabName);

    Q3Action* actionToggleMenu;
    Q3Action* actionFileQuit;
    Q3Action* actionViewInfo;
    Q3Action* actionViewWaypoints;
    Q3Action* actionViewAirfields;
    Q3Action* actionViewReachpoints;
    Q3Action* actionViewTaskpoints;
    Q3Action* actionViewGPSStatus;
    Q3Action* actionToggleStatusbar;
    Q3Action* actionZoomInZ;
    Q3Action* actionZoomOutZ;
    Q3Action* actionToggleWpLabels;
    Q3Action* actionToggleWpLabelsEI;
    Q3Action* actionToggleLogging;
    Q3Action* actionEnsureVisible;
    Q3Action* actionSelectTask;
    Q3Action* actionPreFlight;
    Q3Action* actionRememberWaypoint;
    Q3Action* actionSetupConfig;
    Q3Action* actionSetupInFlight;
    Q3Action* actionHelp;
    Q3Action* actionHelpAboutApp;
    Q3Action* actionWhatsThis;
    /** file_menu contains all items of the menubar entry "File" */
    Q3PopupMenu *fileMenu;
    /** view_menu contains all items of the menubar entry "View" */
    Q3PopupMenu *viewMenu;
    /** view_menu contains all items of the menubar entry "Map" */
    Q3PopupMenu *mapMenu;
    /** setupMenu contains all items of the menubar entry "Setup" */
    Q3PopupMenu *setupMenu;
    /** view_menu contains all items of the menubar entry "Help" */
    Q3PopupMenu *helpMenu;
    // Wait screen
    WaitScreen *ws;
    // visibility of menu bar
    bool menuBarVisible;
  };

#endif
