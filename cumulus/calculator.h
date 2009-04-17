/***********************************************************************
**
**   calculator.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDateTime>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QTime>

#include "altitude.h"
#include "basemapelement.h"
#include "distance.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "glider.h"
#include "gpsnmea.h"
#include "limitedlist.h"
#include "polar.h"
#include "speed.h"
#include "windstore.h"
#include "vario.h"
#include "vector.h"
#include "waypoint.h"
#include "reachablelist.h"

class ReachableList;
class WindAnalyser;

/**
 * @short A single sample from a flight
 *
 * This class represents a single sample of flight data obtained.
 *
 * @author Andrè Somers
 */

class flightSample
{
public:
    /**
     * Position in KFLog format
     */
    QPoint position;

    /**
     * Pressure altitude of point
     */
    Altitude altitude;

    /**
     * GPS altitude of point
     */
    Altitude GNSSAltitude;

    /**
     * Speed and direction
     */
    Vector vector;


    /**
     * Time the sample was taken.
     */
    QTime time;

    /**
     * Current airspeed
     */
    Speed airspeed;

    /**
    * Unique marker. Can be used to reference a certain record/sample.
    */
    int marker; //this marker can be used to reference a certain record...
};


/**
 * @short Flight calculator class
 *
 * This is the calculator for all flight-related parameters. It is at
 * the heart of the cumulus system. All data received from the GPS via
 * the @ref GPSNMEA class is routed here first. The data received is
 * relayed to the other listeners, like the @ref Map and the @ref
 * MapView. Data not in the GPS signal is being estimated or
 * calculated here or by classes initiated from here, including final
 * glide information and wind information.
 *
 * This is a Singleton class.
 *
 * @author Andrè Somers
 */

class Calculator : public QObject
{
    Q_OBJECT

public:
    /**
     * Different possible flightmodes
     */
    enum flightmode{unknown=0, standstill, cruising, circlingL, circlingR, wave};

    /**
     * Different sources for position data
     */
    enum positionFrom{GPS=0, MAN};

public:
    /**
     * Constructor
     */
    Calculator(QObject*);

    /**
     * Destructor
     */
    virtual ~Calculator();

    /**
     * Read property of QTime lastETA.
     */
    const QTime& getlastETA() const { return lastETA; };

    /**
     * Read property of int lastBearing.
     */
    const int& getlastBearing() const { return lastBearing; };

    /**
     * Read property of double lastDistance.
     */
    const Distance& getlastDistance() const { return lastDistance; };

    /**
     * Read property of wayPoint* selectedWp.
     */
    const wayPoint* getselectedWp() const { return selectedWp; };

    /**
     * Read property of Altitude lastAltitude.
     */
    const Altitude& getlastAltitude()const { return lastAltitude; };

    /**
     * @returns the last known altitude AGL (Above Ground Level)
     */
    const Altitude& getlastAGLAltitude()
      {
        lastAGLAltitude = lastAltitude - lastElevation;
        return lastAGLAltitude;
      };

    /**
     * Read property of known altitude GND (Above reference plain formed by waypoint)
     */
    const Altitude& getlastGNDAltitude();

    /**
     * Read property of Speed lastSpeed.
     */
    const Altitude& getAltimeterAltitude();
    const QString getAltimeterAltitudeText();

    /**
     * Read property of lastSpeed.
     */
    const AltitudeCollection& getAltitudeCollection();

    /**
     * Read property of lastSpeed.
     */
    Speed& getLastSpeed()
    {
      return lastSpeed;
    };

    /**
     * Read property of lastBestSpeed.
     */
    Speed& getlastBestSpeed()
    {
      return lastBestSpeed;
    };

    /**
     * Read property of lastGlidePath.
     */
    const Altitude& getlastGlidePath()
    {
      return lastGlidePath;
    }

    /**
     * Get required LD to target.
     */
    double getLastRequiredLD()
    {
      if ( selectedWp )
        {
          return lastRequiredLD;
        }
      else
        {
          return -1.0;
        }
    };

    /**
     * Get last current LD.
     */
    double getLastCurrentLD()
    {
      return lastCurrentLD;
    };

    /**
     * Read property of QPoint lastPosition.
     */
    const QPoint& getlastPosition()
    {
      return lastPosition;
    };

    /**
     * Read property of McCready setting
     */
    const Speed& getlastMc()
    {
      return lastMc;
    };

    /**
     * Read property of int lastWind.
     */
    const Vector& getlastWind() const { return lastWind; };

    /**
     * Read property of int lastHeading.
     */
    int getlastHeading()
    {
      return lastHeading;
    };

    /**
     * Sets the current position to point newPos.
     */
    void setPosition(const QPoint& newPos);

    /**
     * Contains a list of samples from the flight
     */
    LimitedList<flightSample> samplelist;

    /**
     * Returns the current flightmode
     */
    flightmode currentFlightMode()
      {
        return lastFlightMode;
      };

    /**
     * Read property of Glider glider.
     */
    Glider* glider() const
      {
        return _glider;
      };

    /**
     * get glider type
     */
    QString gliderType () const
    {
      return ( _glider != 0 ) ? _glider->type() : QString::null;
    };

    /**
     * @returns the arrival Altitude regarding wind and last altitude
     */
    bool glidePath(int aLastBearing, Distance aDistance,
                   Altitude aElevation, Altitude &arrival, Speed &BestSpeed );

    /**
     * @returns the Glider Polar
     */
    Polar * getPolar()
    {
      return _polar;
    };

    /**
     * @returns the Reachable List
     */
    ReachableList * getReachList()
    {
        return _reachablelist;
    };

    void clearReachable()
    {
        _reachablelist->clearLists();
    };

    /**
     * @returns the vario object
     */
    const Vario* getVario()
    {
        return _vario;
    };

    /**
     * recalculate reachable list as new sites came in
     */
    void newSites()
      {
        _reachablelist->calculateNewList();
      };

    /**
     * @returns the wind analyzer
     */
    const WindAnalyser* getWindAnalyser()
    {
        return _windAnalyser;
    };

    /**
     * @returns true if the current flight mode matches the
     * given pseudo-mode
     */
    bool matchesFlightMode(GeneralConfig::UseInMode);

    void setManualInFlight(const bool);

    bool isManualInFlight()
      {
        return manualInFlight;
      };

public slots: // Public slots
    /**
     * Called if a new waypoint has been selected.
     */
    void slot_WaypointChange(wayPoint *, bool);

    /**
     * Called if a waypoint has to be deleted.
     */
    void slot_WaypointDelete(wayPoint *);

    /**
     * called if a new position-fix has been established.
     */
    void slot_Position();

    /**
     * called if a new speed fix has been received
     */
    void slot_Speed();
    /**
     * called on altitude change
     */
    void slot_Altitude();
    /**
     * Called if the position is changed manually.
     */
    void slot_changePosition(int direction);
    /**
     * Called if a new heading has been obtained
     */
    void slot_Heading();
    /**
     * Chanched position to the North
     */
    void slot_changePositionN();
    /**
     * Chanched position to the South
     */
    void slot_changePositionS();
    /**
     * Chanched position to the East
     */
    void slot_changePositionE();
    /**
     * Chanched position to the West
     */
    void slot_changePositionW();
    /**
     * Chanched position to the Homesite
     */
    void slot_changePositionHome();
    /**
     * Chanched position to the selected waypoint
     */
    void slot_changePositionWp();
    /**
     * set McCready value
     */
    void slot_Mc(const Speed&);
    /**
     * increment McCready value
     */
    void slot_McUp();
    /**
     * decrement McCready value; don't get negative
     */
    void slot_McDown();
    /**
     * Variometer lift receiver and distributor to map display.
     */
    void slot_Variometer(const Speed&);
    /**
     * GPS variometer lift receiver. The internal variometer
     * calculation can be switched off, if we got values vis this slot.
     */
    void slot_GpsVariometer(const Speed&);
    /**
     * settings have been changed
     */
    void slot_settingsChanged();
    /**
     * This slot is called by the NMEA interpreter if a new fix has been received.
     */
    void slot_newFix();
    /**
     * Called if the status of the GPS changes.
     */
    void slot_GpsStatus(GpsNmea::GpsStatus);
    /**
     * Write property of Glider glider.
     */
    void setGlider( Glider* _newVal);

    /**
     * Connected to the signal flightModeChanged and used to re-emit with marker value
     */
    void slot_flightModeChanged(Calculator::flightmode);
    /**
     * Called to switch on/off LD calculation
     */
    void slot_toggleLDCalculation(const bool newVal)
    {
      _calculateLD = newVal;
    };
    /**
     * Called to switch on/off Vario calculation
     */
    void slot_toggleVarioCalculation(const bool newVal)
    {
      _calculateVario = newVal;
    };
    /**
     * Called to switch on/off ETA calculation
     */
    void slot_toggleETACalculation(const bool newVal)
    {
      _calculateETA = newVal;
    };

    /** Called if a new wind measurement is available
     * by the GPS/Logger device */
    void slot_GpsWind(const Speed& speed, const short direction);
    /**
     * Called if the cumulus wind analyzer has a new measurement.
     */
    void slot_Wind(Vector&);

signals: // Signals

    /**
     * Sent if a new waypoint has been selected.
     */
    void newWaypoint(const wayPoint *);

    /**
     * Sent if a new distance has been calculated. Negative if invalid.
     */
    void newDistance(const Distance&);

    /**
     * Sent if a new bearing has been calculated. Negative if invalid.
     */
    void newBearing(int);

    /**
     * Sent if a new relative bearing has been calculated. Negative if valid.
     */
    void newRelBearing(int);

    /**
     * Sent if a new ETA has been calculated.
     */
    void newETA(const QTime&);

    /**
     * Sent if a new altitude has been obtained
     */
    void newAltitude(const Altitude&);

    /**
     * Sent if a new speed has been obtained
     */
    void newSpeed(const Speed&);

    /**
    * Sent if a new airspeed has been obtained
    */
    void newAirspeed(const Speed&);

    /**
     * Sent if a new heading has been obtained
     */
    void newHeading(int);

    /**
     * Sent if a new position has been selected, either manually or by GPS
     */
    void newPosition(const QPoint&, const int);

    /**
     * Sent if a new best speed for glide path has been calculated
     */
    void bestSpeed (const Speed& speed);

    /**
     * Sent if a new glide path value has been calculated
     */
    void glidePath (const Altitude& above);

    /**
     * Sent if a new current LD value has been calculated
     */
    void newLD( const double& rLD, const double& cLD );

    /**
     * Sent if a new McCready value has been set
     */
    void newMc (const Speed&);

    /**
     * Sent if a new variometer value has been set
     */
    void newVario (const Speed&);

    /**
     * Sent if a new wind has been obtained
     */
    void newWind (Vector&);

    /**
     * Sent the name of the glider type, if a new glider has been set.
     */
    void newGlider(const QString&);

    /**
     * Sent if the flight mode changes
     */
    void flightModeChanged(Calculator::flightmode);

    /**
     * Sent if the flight mode changes
     */
    void flightModeChanged(Calculator::flightmode, int marker);

    /**
     * Sent if a new sample has been added to the sample list
     */
    void newSample();

    /**
     * Sent to inform the user about the task progress.
     */
    void taskInfo( const QString&, const bool );

    /**
     * Sent if a task point sector is touched. Will be used by the IGC
     * logger to increase logger sequence for a certain time.
     */
    void taskpointSectorTouched();

    /** sent on activate/deactivate of manualInFlight mode */
    void switchManualInFlight();

private: // Private methods
    /**
     * Sets a new selected waypoint. The old waypoint instance is
     * deleted and a new one allocated.
     */
    void setSelectedWp( const wayPoint* newWp );

    /**
     * Calculates the distance to the currently selected waypoint and
     * emits newDistance if the distance has changed.
     */
    void calcDistance(const bool autoWpSwitch=true);

    /**
     * Calculates the bearing to the currently selected waypoint, and
     * emits signal newBearing if the bearing has changed.
     */
    void calcBearing();

    /**
     * Calculates the ETA (Estimated Time to Arrival) to the current
     * waypoint and emits a signal newETA if the value has changed.
     */
    void calcETA();

    /**
     * Calculates the glide path to the current waypoint and the needed height
     */
    void calcGlidePath();

    /**
     * Calculates the current and required LD to the selected waypoint
     */
    void calcLD();

    /**
     * Determines the status of the flight:
     * unknown, standstill, cruising, circlingL, circlingR or wave
     */
    void determineFlightStatus();


private: // Private attributes
    /** Contains the last flight sample */
    flightSample lastSample;
    /** Contains the last calculated bearing */
    int lastBearing;
    /** Contains the last calculated distance to the waypoint */
    Distance lastDistance;
    /** Reference to the selected waypoint */
    wayPoint* selectedWp;
    /** Contains the last calculated ETA */
    QTime lastETA;
    /** contains the current state of ETA calculation */
    bool _calculateETA;
    /** Contains the last known speed */
    Speed lastSpeed;
    /** Contains the last known best speed */
    Speed lastBestSpeed;
    /** Contains the last McCready setting */
    Speed lastMc;
    /** Contains the last variometer value */
    Speed lastVario;
    /** Contains the last known glide path information */
    Altitude lastGlidePath;
    /** Contains the last known altitude */
    Altitude lastAltitude;
    /** Contains the last known AGL altitude */
    Altitude lastAGLAltitude;
    /** Contains the last known STD altitude */
    Altitude lastSTDAltitude;
    /** Contains the last known Error margin for the AGL altitude */
    Distance lastAGLAltitudeError;
    /** Contains the last known altitude */
    Altitude lastGNSSAltitude;
    /** Contains the last known altitude */
    Altitude lastGNDAltitude;
    /** Contains the altitude used for manual navigation mode */
    Altitude manualAltitude;
    /** Contains the error margin for the lastGNDAltitude */
    Distance lastElevationError;
    /** Contains the last known elevation */
    Altitude lastElevation;
    /** Contains the last returned altitude collection */
    AltitudeCollection lastAltCollection;
    /** Contains the last known position, either obtained from the GPS or modified by manual input. */
    QPoint lastPosition, lastGPSPosition;
    /** contains the last known heading */
    int lastHeading;
    /** contains the last known wind */
    Vector lastWind;
    /** contains the last required LD to target */
    double lastRequiredLD;
    /** contains the last current LD */
    double lastCurrentLD;
    /** contains the current state of LD calculation */
    bool _calculateLD;
    /** contains the polar object of the selected glider */
    Polar* _polar;
    /** contains some functions to provide variometer data */
    Vario* _vario;
    /** contains the current state of vario calculation */
    bool _calculateVario;
    /** Contains the last known flight mode */
    flightmode lastFlightMode;
    /** Last marker value used */
    int _marker;
    /** contains the current state of wind calculation */
    bool _calculateWind;
    /** contains functions to analyze the wind */
    WindAnalyser * _windAnalyser;
    /** contains functions to analyze the wind */
    ReachableList * _reachablelist;
    /** maintains wind measurements and returns new wind values */
    WindStore * _windStore;
    /** Info on the selected glider. */
    Glider * _glider;
    /** Did we already receive a complete sentence? */
    bool _pastFirstFix;
    /** Direction of cruise if we are in cruising mode */
    int _cruiseDirection;
    /** index of mode select button 0: MSL,  1: GND */
    int _altimeter_mode;
    /** the index in wpList for the actual selected WP */
    int selectedWpInList;
    /** waypoint touch flag */
    bool wpTouched;
    /** waypoint touch counter */
    int wpTouchCounter;
    /** task end touch flag */
    bool taskEndReached;

    bool manualInFlight;
};

extern Calculator* calculator;

#endif
