/***************************************************************************
                          mapview.h  -  This file is part of Cumulus.
                             -------------------
    begin                : Sun Jul 21 2002
    copyright            : (C) 2002 by Andre Somers, 2008 Axel Pauli
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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QWidget>
#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QDataStream>
#include <QBoxLayout>
#include <QColor>

#include "distance.h"
#include "speed.h"
#include "altitude.h"
#include "gpsnmea.h"
#include "calculator.h"
#include "map.h"

/**
 * This is the main view for the application, providing the map and
 * other useful in flight information.
 *
 * @author Andre Somers
 */

class MapInfoBox;
class CuLabel;
class Map;
class CumulusApp;

class MapView : public QWidget
  {
    Q_OBJECT

  public:
    /**
     * Constructor
     */
    MapView(QWidget *parent=0);

    /**
     * Destructor
     */
    ~MapView();

    /**
     * @returns the status bar object
     */
    QStatusBar* statusBar () const
      {
        return _statusbar;
      };

    /**
     * @writes a message into the status bar for the given time. Default
     * is 5s. If the time is zero, , the message will never disappear.
     */
    void message( const QString &message, int ms=5000 );

    /**
     * @returns the altitute widget
     */
    MapInfoBox* getAltitudeWidget () const
      {
        return _altitude;
      };

    /**
     * @returns the Vario widget
     */
    MapInfoBox* getVarioWidget () const
      {
        return _vario;
      };

  public: //public attributes

    /**
     * pointer to the map widget
     */
    Map * _theMap;
    CumulusApp * cuApp;

  public slots: // Public slots
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
    void slot_Waypoint(const wayPoint *wp);

    /**
     * This slot is called by calculator if a new ETA
     * (Estimated Time to Arrival, or the time that is approximatly
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
     * This slot is called if a new positionfix has been established.
     */
    void slot_Position(const QPoint& position, const int source);

    /**
     * This slot is called if a log entry has been made.
     */
    void slot_LogEntry();

    /**
     * This slot is called if the status of the GPS changes.
     */
    void slot_GPSStatus(GpsNmea::connectedStatus status);

    /**
     * This slot is called if the number of satelites changes.
     */
    void slot_SatConstellation();

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
     * This slot is called if a warning message shall be displayed
     */
    void slot_warning( const QString& warning );

    /**
     * This slot is called if the settings have been changed.
     * It refreshes all displayed data because units might have
     * been changed.
     */
    void slot_settingschange();

    /**
     * Format and set the FlightStatus string
     */
    void slot_setFlightStatus();

    /** Opens the inflight glider settings dialog. */
    void slot_gliderFlightDialog();

    /** Opens the GPS status dialog */
    void slot_gpsStatusDialog();

  signals: // Signals --------------------------------------------------
    /**
     * toggle LD compution on/off
     */
    void toggleLDCalculation( const bool );
    /**
     * toggle ETA compution on/off
     */
    void toggleETACalculation( const bool );

    /**
     * toggle vario compution on/off
     */
    void toggleVarioCalculation( const bool );

  private: // Private attributes
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
    /** reference to the flighttime label */
    //MapInfoBox* _flighttime;
    /** reference to the eta label */
    MapInfoBox* _eta;
    /** reference to the altitude label */
    MapInfoBox* _altitude;
    /** reference to the elevation label */
    MapInfoBox* _elevation;
    /** reference to the glide path label */
    MapInfoBox* _glidepath;
    /** reference to statusbar */
    QStatusBar* _statusbar;
    /** reference to GPS status */
    CuLabel* _statusGps;
    /** reference to flightstatus, including logging status and flightmode */
    QLabel* _statusFlightstatus;
    /** reference to position for statusbar */
    QLabel* _statusPosition;
    /** reference to selected glider for statusbar */
    QLabel* _statusGlider;
    /** reference to warning for statusbar */
    QLabel* _statusWarning;
    /** reference to menu toggle */
    CuLabel* _menuToggle;
    /** timer to reset font for logging indicator */
    QTimer* loggingTimer;
    /** index of mode select button 0: MSL,  1: GND */
    int _altimeterMode;
    /** bearing mode 0=invers bearing, 1=normal bearing */
    int _bearingMode;
    /** value of last got bearing */
    int _lastBearing;
    /** timer to reset invers bearing display */
    QTimer* _bearingTimer;
    /** default bg color */
    QColor _bearingBGColor;

    QPixmap _arrows;

    /** can be CuCalc::GPS or CuCalc::MAN */
    int lastPositionChangeSource;

  private slots:

    /**
     * toggle between distance and eta widget on mouse signal
     */
    void slot_toggleDistanceEta();

    /**
     * toggle between wind and ld widget on mouse signal
     */
    void slot_toggleWindAndLD();

    /**
     * toggle between QUJ and QTE
     */
    void slot_toggleBearing();

    /**
     * Reset invers bearing after a timeout
     */
    void slot_resetInversBearing();

    /** Opens the Variometer settings dialog. */
    void slot_VarioDialog();

    /** Opens the Altimeter settings dialog. */
    void slot_AltimeterDialog();

    /** Called, if altimeter mode has been changed */
    void slot_newAltimeterMode();
  };

#endif
