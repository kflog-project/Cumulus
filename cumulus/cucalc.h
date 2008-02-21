/***********************************************************************
**
**   cucalc.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CUCALC_H
#define CUCALC_H

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
 * This class represents a single sample of flightdata obtained.
 *
 * @author André Somers
 */

class flightsample
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
 * @author André Somers
 */

class CuCalc : public QObject
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
    CuCalc(QObject*);

    /**
     * Destructor
     */
    virtual ~CuCalc();

    /**
     * Read property of QTime lastETA.
     */
    virtual const QTime& getlastETA() const { return lastETA; };

    /**
     * Read property of int lastBearing.
     */
    virtual const int& getlastBearing() const { return lastBearing; };

    /**
     * Read property of double lastDistance.
     */
    virtual const Distance& getlastDistance() const { return lastDistance; };

    /**
     * Read property of wayPoint* selectedWp.
     */
    virtual const wayPoint* getselectedWp() const { return selectedWp; };

    /**
     * Read property of Altitude lastAltitude.
     */
    virtual const Altitude& getlastAltitude()const { return lastAltitude; };

    /**
     * @returns the last known altitude AGL (Above Ground Level)
     */
    virtual const Altitude& getlastAGLAltitude()
      {
	lastAGLAltitude = lastAltitude - lastElevation;
	return lastAGLAltitude;
      };

    /**
     * Read property of known altitude GND (Above reference plain formed by waypoint)
     */
    virtual const Altitude& getlastGNDAltitude();

    /**
     * Read property of Speed lastSpeed.
     */
    virtual const Altitude& getAltimeterAltitude();
    virtual const QString getAltimeterAltitudeText();

    /**
     * Read property of Speed lastSpeed.
     */
    virtual const AltitudeCollection& getAltitudeCollection();

    /**
     * Read property of Speed lastSpeed.
     */
    virtual Speed& getlastSpeed();

    /**
     * Read property of Speed lastBestSpeed.
     */
    virtual Speed& getlastBestSpeed();

    /**
     * Read property of Speed lastGlidePath.
     */
    virtual const Altitude& getlastGlidePath();

    /**
     * Get required LD to target.
     */
    virtual const double getLastRequiredLD();

    /**
     * Get current LD.
     */
    virtual const double getLastCurrentLD();

    /**
     * Read property of QPoint lastPosition.
     */
    virtual const QPoint& getlastPosition();

    /**
     * Read property of McCready setting
     */
    const Speed& getlastMc();

    /**
     * Read property of int lastWind.
     */
    const Vector& getlastWind() const { return lastWind; };

    /**
     * Read property of int lastHeading.
     */
    virtual int getlastHeading();

    /**
     * Sets the current position to point newPos.
     */
    void setPosition(const QPoint& newPos);

    /**
     * Contains a list of samples from the flight
     */
    LimitedList<flightsample> * samplelist;

    /**
     * Returns the current flightmode
     */
    flightmode currentFlightMode();

    /**
     * Read property of Glider glider.
     */
    virtual Glider * glider() const;

    /**
     * get glider type
     */
    QString gliderType () const;

    /**
     * @returns the arrival Altitude regarding wind and last altitude
     */
    bool glidePath(int aLastBearing, Distance aDistance,
                   Altitude aElevation, Altitude &arrival, Speed &BestSpeed );

    /**
     * @returns the Glider Polar
     */
    Polar * getPolar();

    /**
     * @returns the Reachable List
     */
    ReachableList * getReachList()
    {
        return _reachablelist;
    };

    void clearReachable()
    {
        _reachablelist->clearList();
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
    void newSites();

    /**
     * @returns the windcalculator
     */
    const WindAnalyser* getWindAnalyser()
    {
        return _windAnalyser;
    };

    /**
     * @returns true if the current flightmode matches the
     * given pseudo-mode
     */
    bool matchesFlightMode(GeneralConfig::UseInMode);

    void setManualInFlight(const bool);
    bool isManualInFlight();

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
     * No descriptions
     */
    void slot_Variometer(const Speed&);
    /**
     * settings have changed
     */
    void slot_settingschanged();
    /**
     * This slot is called by the NMEA interpretter if a new fix has been received.
     */
    void slot_newFix();
    /**
     * Called if the status of the GPS changes.
     */
    void slot_GpsStatus(GPSNMEA::connectedStatus);
    /**
     * Write property of Glider glider.
     */
    virtual void setGlider( Glider * _newVal);

    /**
     * Connected to the signal flightModeChanged, and used to re-emit with markervalue
     */
    void _slot_flightModeChanged(CuCalc::flightmode);
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
    /**
     * Called if the wind measurement changes
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
     * Sent if a new position has been selected, either manually or by gps
     */
    void newPosition(const QPoint&, const int);

    /**
     * Sent if a new best speed for glide path has been calculated
     */
    void bestSpeed (const Speed& speed);

    /**
     * Sent if a new glidepath value has been calculated
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

    void newWind (Vector&);
    /**
     * Sent if the flightmode changes
     */
    void flightModeChanged(CuCalc::flightmode);

    /**
     * Sent if the flightmode changes
     */
    void flightModeChanged(CuCalc::flightmode, int marker);

    /**
     * Sent if a new sample has been added to the samplelist
     */
    void newSample();

    /**
     * Sent to inform the user about the task progress.
     */
    void taskInfo( const QString&, const bool );

    /**
     * Sent if a task point sector is touched. Will be used by the igc
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
     * unkown, standstill, cruising, circlingL, circlingR or wave
     */
    void determineFlightStatus();


private: // Private attributes
    /** Contains the last flight sample */
    flightsample * lastSample;
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
    /** Contains the last known Errormargin for the AGL altitude */
    Distance lastAGLAltitudeError;
    /** Contains the last known altitude */
    Altitude lastGNSSAltitude;
    /** Contains the last known altitude */
    Altitude lastGNDAltitude;
    /** Contains the altitude used for manual navigation mode */
    Altitude manualAltitude;
    /** Contains the errormargin for the lastGNDAltitude */
    Distance lastElevationError;
    /** Contains the last known elevation */
    Altitude lastElevation;
    /** Contains the last returned altitude collection */
    AltitudeCollection lastAltCollection;
    /** Contains the last known position, either obtained from the gps or modified by manual input. */
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
    /** Contains the last known flightmode */
    flightmode lastFlightMode;
    /** Last markervalue used */
    int _marker;
    /** contains funtions to analyze the wind */
    WindAnalyser * _windAnalyser;
    /** contains funtions to analyze the wind */
    ReachableList * _reachablelist;
    /** maintains windmeasurements and returns new windvalues */
    WindStore * _windStore;
    /** Info on the selected glider. */
    Glider * _glider;
    /** Did we allready receive a complete sentence? */
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

extern CuCalc* calculator;

#endif
