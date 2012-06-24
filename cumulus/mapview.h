/***************************************************************************
                          mapview.h  -  This file is part of Cumulus.
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002      by André Somers
                               2008-2012 by Axel Pauli
    email                : axel@kflog.org

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

#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include <QWidget>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QDataStream>
#include <QBoxLayout>
#include <QColor>

#ifdef QSCROLLER1
#include <QScrollArea>
#include <QScroller>
#endif

#include "altitude.h"
#include "calculator.h"
#include "distance.h"
#include "gpsnmea.h"
#include "map.h"
#include "speed.h"

#ifdef FLARM
#include "flarmwidget.h"
#endif

/**
 * \class MapView
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Main view of the application.
 *
 * This is the main view of the application, providing the map and
 * other useful in flight information.
 *
 * \date 2002-2012
 *
 * \version: $Id$
 *
 */

class CuLabel;
class Map;
class MapInfoBox;
class MainWindow;

class MapView : public QWidget
{
   Q_OBJECT

  private:

   Q_DISABLE_COPY ( MapView )

  public:
    /**
     * Constructor
     */
    MapView( QWidget *parent=0 );

    /**
     * Destructor
     */
    virtual ~MapView();

    /**
     * @return the status bar object
     */
    QStatusBar* statusBar () const
      {
        return _statusbar;
      };

    /**
     * Writes a message into the status bar for the given time. Default
     * is 5s. If the time is zero, the message will never disappear.
     *
     * \param message The message to be displayed
     *
     * \param ms The display time in milli seconds.
     */
    void message( const QString &message, int ms=5000 );

    /**
     * @return the altitude widget
     */
    MapInfoBox* getAltitudeWidget () const
      {
        return _altitude;
      };

    /**
     * @return the Variometer widget
     */
    MapInfoBox* getVarioWidget () const
      {
        return _vario;
      };

#ifdef QSCROLLER1
    /**
     * Reset the QScroller
     */
    void resetScrolling();
#endif

#ifdef FLARM

    /**
     * @return the Flarm widget.
     */
    FlarmWidget* getFlarmWidget() const
      {
        return _flarmWidget;
      };

#endif

  protected:

    void showEvent(QShowEvent* event);

  public:

    /**
     * pointer to the map widget
     */
    Map* _theMap;

  public slots:
    /**
     * Called if speed has changed
     */
    void slot_Speed(const Speed& speed);

    /**
     * called if heading has changed
     */
    void slot_Heading(int head);

    /**
     * called if the waypoint is changed in the calculator
     */
    void slot_Waypoint(const Waypoint *wp);

    /**
     * This slot is called by calculator if a new ETA
     * (Estimated Time to Arrival, or the time that is approximately
     * needed to arrive at the waypoint) has been calculated.
     */
    void slot_ETA(const QTime& eta);

    /**
     * This slot is called by calculator if a new distance has
     * been calculated.
     */
    void slot_Distance(const Distance& distance);

    /**
     * This slot is called by calculator if a new bearing has
     * been calculated
     */
    void slot_Bearing(int bearing);

    /**
     * This slot is called by calculator if a new relative bearing
     * has been calculated
     */
    void slot_RelBearing(int relbearing);

    /**
     * This slot is called if a new position fix has been established.
     */
    void slot_Position(const QPoint& position, const int source);

    /**
     * This slot is called if a log entry has been made.
     */
    void slot_LogEntry();

    /**
     * This slot is called if the status of the GPS changes.
     */
    void slot_GPSStatus(GpsNmea::GpsStatus status);

    /**
     * This slot is called if the number of satellites changes.
     */
    void slot_SatCount( SatInfo& satInfo );

    /**
     * This slot is being called if the altitude has changed.
     */
    void slot_Altitude(const Altitude& alt);

    /**
     * This slot is changed when the "above glide path" value
     * has changed
     */
    void slot_GlidePath (const Altitude& above);

    /**
     * This slot is changed when the best speed value has changed
     */
    void slot_bestSpeed (const Speed& above);

    /**
     * This slot is called if a new McCready value has been set
     */
    void slot_Mc (const Speed& mc);

    /**
     * This slot is called if a new variometer value has been set
     */
    void slot_Vario (const Speed& vario);

    /**
     * This slot is called if a new wind value has been set
     */
    void slot_Wind (Vector& wind);

    /**
     * This slot is called if the current LD value has been modified
     */
    void slot_LD( const double& rLD, const double& cLD );

    /**
     * This slot is called if the glider selection has been modified
     */
    void slot_glider( const QString& glider );

    /**
     * This slot is called if a info message shall be displayed
     */
    void slot_info( const QString& info );

    /**
     * This slot is called if the settings have been changed.
     * It refreshes all displayed data because units might have
     * been changed.
     */
    void slot_settingsChange();

    /**
     * Format and set the FlightStatus string
     */
    void slot_setFlightStatus( Calculator::FlightMode fm );

    /*
     * Sets the logger status in the status bar.
     */
    void slot_setLoggerStatus();

    /** Opens the in flight glider settings dialog. */
    void slot_gliderFlightDialog();

    /** Opens the GPS status dialog */
    void slot_gpsStatusDialog();

#ifdef FLARM

    /** This slot is called if the number of received Flarms has been changed.*/
    void slot_FlarmCount( int flarmCount );

    /** Opens the Flarm widget. */
    void slot_OpenFlarmWidget();

    /** Opens the Map view. */
    void slot_OpenMapView();

#endif

  signals:
    /**
     * toggle LD calculation on/off
     */
    void toggleLDCalculation( const bool );
    /**
     * toggle ETA calculation on/off
     */
    void toggleETACalculation( const bool );

    /**
     * toggle variometer calculation on/off
     */
    void toggleVarioCalculation( const bool );

    /**
     * toggle menu in main window.
     */
    void toggleMenu();

  private:

    /** Widget for info boxes */
    QWidget* _sideWidget;
    /** reference to the heading label */
    MapInfoBox* _heading;
    /** reference to the bearing label */
    MapInfoBox* _bearing;
    /** reference to the relative bearing label */
    MapInfoBox* _rel_bearing;
    /** reference to the distance label */
    MapInfoBox* _distance;
    /** reference to the speed label */
    MapInfoBox* _speed;
    /** reference to the best speed label */
    MapInfoBox* _speed2fly;
    /** reference to the McCready label */
    MapInfoBox* _mc;
    /** reference to the variometer label */
    MapInfoBox* _vario;
    /** reference to the wind label */
    MapInfoBox* _wind;
    /** reference to the LD label */
    MapInfoBox* _ld;
    /** reference to the waypoint label */
    MapInfoBox* _waypoint;
    /** reference to the ETA label */
    MapInfoBox* _eta;
    /** reference to the altitude label */
    MapInfoBox* _altitude;
    /** reference to the glide path label */
    MapInfoBox* _glidepath;
    /** reference to the menu label */
    MapInfoBox* _menuToggle;
    /** reference to status bar */
    QStatusBar* _statusbar;
    /** reference to GPS status */
    CuLabel* _statusGps;

#ifdef QSCROLLER1
    /** reference to map container */
    QScrollArea* mapArea;
    /** reference to map scroller */
    QScroller* mapScroller;
#endif

#ifdef FLARM
    /** reference to Flarm status */
    CuLabel* _statusFlarm;
    /** Reference to Flarm widget */
    FlarmWidget* _flarmWidget;
#endif

    /** reference to flight status, including logging status and flight mode */
    QLabel* _statusFlightstatus;
    /** reference to position for status bar */
    QLabel* _statusPosition;
    /** reference to selected glider for status bar */
    QLabel* _statusGlider;
    /** reference to status bar info */
    QLabel* _statusInfo;
    /** index of mode select button 0: MSL,  1: GND */
    int _altimeterMode;
    /** bearing mode 0=inverse bearing, 1=normal bearing */
    int _bearingMode;
    /** value of last got bearing */
    int _lastBearing;
    /** timer to reset inverse bearing display */
    QTimer* _bearingTimer;
    /** default bg color */
    QColor _bearingBGColor;
    /** default bg color */
    QColor _glidepathBGColor;
    /** Pixmap containing arrows to be drawn*/
    QPixmap _arrows;
    /** can be CuCalc::GPS or CuCalc::MAN */
    int lastPositionChangeSource;
    /** pointer to main window */
    MainWindow *_mainWindow;

  private slots:

    /**
     * toggle between distance and ETA widget on mouse signal
     */
    void slot_toggleDistanceEta();

    /**
     * toggle between wind and LD widget on mouse signal
     */
    void slot_toggleWindAndLD();

    /**
     * toggle between QUJ and QTE
     */
    void slot_toggleBearing();

    /**
     * Reset inverse bearing after a timeout
     */
    void slot_resetInversBearing();

    /** Opens the variometer settings dialog. */
    void slot_VarioDialog();

    /** Opens the altimeter settings dialog. */
    void slot_AltimeterDialog();

    /** Called, if altimeter mode has been changed */
    void slot_newAltimeterMode();

    /** Called to toggle the menu of the main window. */
    void slot_toggleMenu();

#ifdef QSCROLLER1
    /** Process status changes during map drag and release. */
    void slot_scrollerStateChanged(QScroller::State new_s);
#endif

};

#endif
