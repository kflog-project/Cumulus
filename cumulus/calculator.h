/***********************************************************************
**
**   calculator.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QDateTime>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QTime>
#include <QTimer>

#include "altitude.h"
#include "basemapelement.h"
#include "distance.h"
#include "flighttask.h"
#include "generalconfig.h"
#include "glider.h"
#include "gpsnmea.h"
#include "limitedlist.h"
#include "polar.h"
#include "reachablelist.h"
#include "speed.h"
#include "taskpoint.h"
#include "vario.h"
#include "vector.h"
#include "waypoint.h"
#include "windstore.h"

class ReachableList;
class WindAnalyser;

/**
 * \class FlightSample
 *
 * \author Andrè Somers, Axel Pauli
 *
 * \brief A single sample from a flight data.
 *
 * This class represents a single sample of flight data obtained.
 *
 * \date 2002-2021
 *
 * \version 1.3
 */
class FlightSample
{

public:
  /**
   * Position in KFLog format
   */
  QPoint position;

  /**
   * User altitude of point. May be adapted by the user.
   */
  Altitude altitude;

  /**
   * Pressure Altitude of point, if available. Otherwise it is derived from
   * the GNSSAltitude.
   */
  Altitude STDAltitude;

  /**
   * GPS altitude of point
   */
  Altitude GNSSAltitude;

  /**
   * Speed and direction
   */
  Vector vector;

  /**
   * UTC date and time the sample was taken.
   */
  QDateTime time;

  /**
   * Current airspeed
   */
  Speed airspeed;

  /**
  * Unique marker. Can be used to reference a certain record/sample.
  */
  int marker;
};

/**
 * \class Calculator
 *
 * \author Andrè Somers, Axel Pauli
 *
 * \brief Flight calculator class
 *
 * This is the calculator for all flight-related parameters. It is
 * the heart of the Cumulus system. All data received from the GPS via
 * the @ref GpsNmea class is routed here first. The data received is
 * relayed to the other listeners, like the @ref Map and the @ref
 * MapView. Data not in the GPS signal is being estimated or
 * calculated here or by classes initiated from here, including final
 * glide information and wind information.
 *
 * This is a Singleton class.
 *
 * \date 2002-2015
 *
 * \version 1.3
 */
class Calculator : public QObject
{
  Q_OBJECT

private:
  /**
   * That macro forbids the copy constructor and the assignment operator.
   */
  Q_DISABLE_COPY( Calculator )

public:
  /**
   * Different possible flight modes
   */
  enum FlightMode {unknown=0, standstill, cruising, circlingL, circlingR};

  /**
   * Different sources for position data
   */
  enum positionFrom {GPS=0, MAN};

  const double infiniteTemperature;

public:
  /**
   * Constructor
   */
  Calculator( QObject* );

  /**
   * Destructor
   */
  virtual ~Calculator();

  /**
   * Read property of last temperature.
   */
  double getLastTemperature() const
   {
     return m_lastTemperature;
   }

  /**
   * Read property of QTime lastETA.
   */
  const QTime& getlastETA() const { return lastETA; }

  /**
   * Read property of integer lastBearing.
   */
  const int& getlastBearing() const { return lastBearing; }

  /**
   * Read property of double lastDistance.
   */
  const Distance& getlastDistance() const { return lastDistance; }

  /**
   * \return The selected target waypoint object.
   */
  const Waypoint* getTargetWp() const { return targetWp; }

  /**
   * Read property of Altitude lastAltitude.
   */
  const Altitude& getlastAltitude()const { return lastAltitude; }

  /**
   * Read property of Altitude lastSTDAltitude.
   */
  const Altitude& getlastSTDAltitude()const { return lastSTDAltitude; }

  /**
   * \return the last known altitude AGL (Above ground Level)
   */
  const Altitude& getlastAGLAltitude()
    {
      lastAGLAltitude = lastAltitude - lastElevation;
      return lastAGLAltitude;
    }

  /**
   * \return the last known altitude AHL (Above home level)
   */
  const Altitude& getlastAHLAltitude()
    {
      lastAHLAltitude = lastAltitude - GeneralConfig::instance()->getHomeElevation();
      return lastAHLAltitude;
    }

  /**
   * Read property of altitude.
   */
  const Altitude& getAltimeterAltitude();

  /**
   * Read property of altitude collection.
   */
  const AltitudeCollection& getAltitudeCollection();

  /**
   * Read property of lastStaticPressure in hPa.
   */
  double getLastStaticPressure()
  {
    return lastStaticPressure;
  }

  /**
   * Read property of lastDynamicPressure in Pa.
   */
  double getLastDynamicPressure()
  {
    return lastDynamicPressure;
  }

  /**
   * Read property of lastSpeed.
   */
  Speed& getLastSpeed()
  {
    return lastSpeed;
  }

  /**
   * Read property of lastBestSpeed.
   */
  Speed& getlastBestSpeed()
  {
    return lastBestSpeed;
  }

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
    if ( targetWp )
      {
        return lastRequiredLD;
      }
    else
      {
        return -1.0;
      }
  }

  /**
   * Get last current LD.
   */
  double getLastCurrentLD()
  {
    return lastCurrentLD;
  }

  /**
   * Read property of QPoint lastPosition.
   */
  const QPoint& getlastPosition()
  {
    return lastPosition;
  }

  /**
   * Read property of McCready setting
   */
  const Speed& getlastMc()
  {
    return lastMc;
  }

  /**
   * Read property of external McCready setting
   */
  const Speed& getlastExternalMc()
  {
    return lastExternalMc;
  }

  /**
   * Read property of TAS setting.
   */
  Speed& getlastTas()
  {
    return lastTas;
  }

  /**
   * Read property of IAS setting.
   */
  Speed& getlastIas()
  {
    return lastIas;
  }

  /**
   * Read property of variometer setting.
   */
  Speed& getlastVario()
  {
    return lastVario;
  }

  /**
   * Returns the last stored wind value.
   */
  Vector& getLastStoredWind()
  {
    return m_lastWind.wind;
  }

  /**
   * Gets the last known Wind from the wind store.
   */
  Vector& getLastWind();

  /**
   * Sets the last Wind.
   */
  void setLastWind( Vector& v )
  {
    m_lastWind.wind = v;
    m_lastWind.altitude = lastAltitude;
  }

  /**
   * Read property of lastHeading.
   */
  int getlastHeading()
  {
    return lastHeading;
  }

  /**
   * Read property of lastMagneticHeading.
   */
  int getlastMagneticHeading()
  {
    return lastMagneticHeading;
  }

  /**
   * Sets the current position to point newPos.
   */
  void setPosition(const QPoint& newPos);

  /**
   * Contains a list of samples from the flight
   */
  LimitedList<FlightSample> samplelist;

  /**
   * Returns the current flight mode
   */
  FlightMode currentFlightMode()
    {
      return lastFlightMode;
    }

  /**
   * Return the bug value contained in the gilder polar.
   */
  ushort getBugsFromPolar();

  /**
   * Write property of Glider glider.
   */
  void setGlider( Glider* glider);

  /**
   * Read property of Glider glider.
   */
  Glider* glider() const
    {
      return m_glider;
    }

  /**
   * get glider type
   */
  QString gliderType () const
  {
    return ( m_glider != 0 ) ? m_glider->type() : "";
  }

  /**
   * \return the arrival Altitude regarding wind and last altitude
   */
  bool glidePath(int aLastBearing, Distance aDistance,
                 Altitude aElevation, Altitude &arrival, Speed &BestSpeed );

  /**
   * \return the Glider Polar
   */
  Polar* getPolar()
  {
    return m_polar;
  }

  /**
   * \return the Reachable List
   */
  ReachableList* getReachList()
  {
      return m_reachablelist;
  }

  void clearReachable()
  {
      m_reachablelist->clearLists();
  }

  /**
   * \return the variometer object
   */
  const Vario* getVario()
  {
      return m_vario;
  }

  /**
   * recalculate reachable list as new sites came in
   */
  void newSites()
    {
      m_reachablelist->calculateNewList();
    }

  /**
   * \return the wind analyzer
   */
  const WindAnalyser* getWindAnalyser()
  {
      return m_windAnalyser;
  }

  /**
   * \return true if the current flight mode matches the given pseudo-mode
   */
  bool matchesFlightMode(GeneralConfig::UseInMode);

  void setManualInFlight(const bool);

  bool isManualInFlight()
    {
      return m_manualInFlight;
    }

  /**
   * @return True if we are in move otherwise fale.
   */
  bool moving();

  /**
   * @return The minimum altitude object.
   */
  Altitude getMinimumAltitude()
  {
    return m_minimumAltitude;
  }

  /**
   * @return The gained altitude object.
   */
  Altitude getGainedAltitude()
  {
    return m_gainedAltitude;
  }

  /**
   * \return The last flight sample.
   */
  const FlightSample& getLastFlightSample() const
  {
    return lastSample;
  }

  /**
   * \return The time when the last sample was taken.
   */
  const QDateTime& getLastSampleTime() const
  {
    return lastSample.time;
  }

  /**
   * \return The wind store
   */
  WindStore* getWindStore()
  {
    return m_windStore;
  }

  /**
   * Sets a new waypoint as target. The old waypoint instance is
   * deleted and a new one allocated.
   *
   * \param newTarget The new target waypoint
   */
  void setTargetWp( Waypoint* newTarget );

  /**
   * Restores a saved waypoint instance, if available.
   *
   * \return True, if waypoint instance could be restored otherwise false.
   */
  bool restoreWaypoint();

 public slots:

  /**
   * Checks, if the a selected waypoint to the home site exists
   * and if the home site has been changed. In this case the
   * selection is renewed to the new position.
   */
  void slot_CheckHomeSiteSelection();

  /**
   * Called if a new waypoint has been selected.
   */
  void slot_WaypointChange(Waypoint *, bool);

  /**
   * Called if a waypoint has to be deleted.
   */
  void slot_WaypointDelete(Waypoint *);

  /**
   * called if a new position-fix has been established.
   */
  void slot_Position( QPoint& newPosition );

  /**
   * called if a new speed fix has been received
   */
  void slot_Speed( Speed& newSpeed );

  /**
   * called on GNSS altitude change.
   */
  void slot_GnssAltitude( Altitude& gnns );

  /**
   * called on pressure altitude change.
   */
  void slot_PressureAltitude( Altitude& pressure );

  /**
   * Called if the position is changed manually.
   */
  void slot_changePosition(int direction);

  /**
   * Called if a new GPS heading has been obtained
   */
  void slot_Heading( const double& newHeading );

  /**
   * Called if a new magnetic heading has been obtained
   */
  void slot_MagneticHeading( const double& newHeading );

  /**
   * Change position to the North
   */
  void slot_changePositionN();

  /**
   * Change position to the South
   */
  void slot_changePositionS();

  /**
   * Change position to the East
   */
  void slot_changePositionE();

  /**
   * Change position to the West
   */
  void slot_changePositionW();

  /**
   * Change position to the Homesite
   */
  void slot_changePositionHome();

  /**
   * Change position to the selected waypoint
   */
  void slot_changePositionWp();

  /**
   * Change position to the new coordinate point. Called by move map with mouse.
   */
  void slot_changePosition(QPoint& newPosition);

  /**
   * Set McCready value, delivered by Mc dialog of Cumulus.
   */
  void slot_Mc(const Speed&);

  /**
   * Set McCready value, delivered by external device.
   */
  void slot_ExternalMc(const Speed&);

  /**
   * Set water value, delivered by Mc dialog, used by glider polare.
   */
  void slot_Water( const int water );

  /**
   * Set bug value, delivered by Mc dialog, used by glider polare.
   */
  void slot_Bugs( const int bugs );

  /**
   * Set bug value used by glider polare and delivered from an external device.
   */
  void slot_ExternalBugs( const unsigned short bugs );

  /**
   * Called if a new TAS is available from an external device.
   */
  void slot_ExternalTas( const Speed& speed );

  /**
   * increment McCready value
   */
  void slot_McUp();

  /**
   * decrement McCready value; don't get negative
   */
  void slot_McDown();

  /**
   * GPS variometer lift receiver. The internal variometer
   * calculation can be switched off, if we got values via this slot.
   */
  void slot_ExternalVariometer(const Speed&);

  /**
   * Variometer lift receiver and distributor to map display.
   */
  void slot_Variometer(const Speed&);

  /**
   * settings have been changed
   */
  void slot_settingsChanged();
  /**
   * This slot is called by the NMEA interpreter if a new fix has been received.
   */
  void slot_newFix( const QDateTime& newFixTime );
  /**
   * Called if the status of the GPS changes.
   */
  void slot_GpsStatus(GpsNmea::GpsStatus);

  /**
   * Called to switch on/off LD calculation
   */
  void slot_toggleLDCalculation(const bool newVal)
  {
    m_calculateLD = newVal;
  }

  /**
   * Called to switch on/off ETA calculation
   */
  void slot_toggleETACalculation(const bool newVal)
  {
    m_calculateETA = newVal;
  }

  /**
   * Called, if a new temperature value is available from an external device
   * (OpenVario or XCVario).
   * Temperature in degree Celsius.
   */
  void slot_Temperature( const double temperature );

  /** Called if a new wind measurement is available from an external device */
  void slot_ExternalWind(const Speed& speed, const short direction);

  /**
   * Called if the Cumulus wind analyzer has a new measurement.
   */
  void slot_Wind(Vector&);

  /**
   * Called, if the user has activated manual wind and has changed the
   * wind values.
   */
  void slot_ManualWindChanged( bool enabled );

  /**
   * Called to select the start point of a loaded task.
   * That will also activate the automatic task point switch.
   */
  void slot_startTask();

  /**
   * Called when a new AHRS data set is available.
   */
  void slot_AHRSInfo( const double rollAngle,
                      const double pitchAngle,
                      const double accelarationX,
                      const double accelarationY,
                      const double accelarationZ );

  /**
   * Called to set the minimum altitude to the current altitude value.
   * That also resets the gained altitude.
   */
  void slot_setMinimumAltitude()
  {
    m_gainedAltitude = 0;
    m_minimumAltitude = lastAltitude;
  }

  /**
   * Called to set the static pressure value from an external device
   * (OpenVario or XCVario).
   * Pressure in hPa. From the static pressure value the STD altitude is
   * calculated.
   *
   * @param pressure Static pressure value in hPa.
   */
  void slot_staticPressure( const double pressure );

  /**
   * Called to set the dynamic pressure value from an external device
   * (OpenVario or XCVario).
   * Pressure in Pa.
   *
   * @param pressure Static pressudouble re value in Pa.
   */
  void slot_dynamicPressure( const double pressure );

  /**
   * Called, if the user has zoomed the map. That will reset the auto zoom value
   * to prevent an unwanted auto zoom.
   */
  void slot_userMapZoom();

  /**
   * Called by a timer event, to zoom back the map to the old zoom value after
   * passing a task point.
   */
  void slot_switchMapScaleBack();

  /**
   * Called, if in the glider flight dialog this option is changed.
   *
   */
  void slot_ExternalData4McAndBugs( const bool flag );

 private slots:

 /**
  * Called, if the timer m_varioDataControl expires.
  */
  void slot_varioDataControl();

 signals:

  /**
   * Sent if a new waypoint has been selected.
   */
  void newWaypoint(const Waypoint *);

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
   * Sent if a new altitude has been obtained. The altitude is related to MSL.
   */
  void newAltitude(const Altitude&);

  /**
   * Sent if a new altitude has been obtained. The altitude is related to the
   * user selection. That can be MSL, STD, AGL, AHL.
   */
  void newUserAltitude(const Altitude&);

  /**
   * Sent if a new speed has been obtained
   */
  void newSpeed(const Speed&);

  /**
  * Sent if a new true airspeed has been obtained
  */
  void newTas(const Speed&);

  /**
  * Sent if a new indicated airspeed has been obtained
  */
  void newIas(const Speed&);

  /**
   * Sent if a new heading has been obtained
   */
  void newHeading(int);

  /**
   * Sent if a new magneticheading has been obtained
   */
  void newMagneticHeading(int);

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
  void newMc (const Speed& mc);

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
   * Sent if the flight mode has changed.
   */
  void flightModeChanged(Calculator::FlightMode);

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

  /**
   * Sent if a new gained altitude is available.
   */
  void newGainedAltitude( const Altitude& );

  /**
   * Sent if the map shall change the zoom factor
   */
  void switchMapScale(const double& newScale);

  /**
   * Send, if a new static pressure value is avialable in hPa.
   */
  void newStaticPressure( const double pressure );

  /**
   * Send, if a new dynamic pressure value is avialable in Pa.
   */
  void newDynamicPressure( const double pressure );

  /**
   * Send, if a new temperature value is available in degree Celsius.
   */
  void newTemperature( const double pressure );

private:

  /**
   * Calculates the distance to the currently selected waypoint and
   * emits newDistance if the distance has changed.  If a flight
   * task has been activated, the automatic switch from one task point
   * to the next is also controlled and handled here.
   */
  void calcDistance(const bool autoWpSwitch=true);

  /**
   * Calculates the bearing to the currently selected waypoint, and
   * emits signal newBearing if the bearing has changed.
   */
  void calcBearing();

  /**
   * Calculates the direction of heading and emits the signal
   * newHeading. This method is only called in the manually mode.
   *
   * \param previousPosition previous position
   *
   * \param newPosition new position
   */
  void calcHeading( const QPoint& previousPosition, const QPoint& newPosition );

  /**
   * Calculates the ETA (Estimated Time to Arrival) to the current
   * waypoint and emits a signal newETA if the value has changed.
   */
  void calcETA();

  /**
   * Calculates the TAS (True Air Speed) from the last provided heading and wind.
   */
  void calcTas();

  /**
   * Calculates the glide path to the current waypoint and the needed height
   */
  void calcGlidePath();

  /**
   * Calculates the current and required LD to the selected waypoint
   */
  void calcLD();

  /**
   * Calculates the altitude gain.
   */
  void calcAltitudeGain();

  /**
   * Determines the status of the flight:
   * unknown, standstill, cruising, circlingL, circlingR or wave
   */
  void determineFlightStatus();

  /**
   * Distributes a flight mode change.
   */
  void newFlightMode(Calculator::FlightMode);

  /**
   * Auto zoom in into the map, if the feature is enabled and a waypoint is
   * selected.
   */
  void autoZoomInMap();

private: // Private attributes
  /** Contains the last flight sample */
  FlightSample lastSample;
  /** Contains the last calculated bearing */
  int lastBearing;
  /** Contains the last calculated distance to the waypoint */
  Distance lastDistance;
  /** The selected target waypoint */
  Waypoint* targetWp;
  /** Contains the last calculated ETA */
  QTime lastETA;
  /** contains the current state of ETA calculation */
  bool m_calculateETA;
  /** Contains the last known speed */
  Speed lastSpeed;
  /** Contains the last known best speed */
  Speed lastBestSpeed;
  /** Contains the last McCready setting */
  Speed lastMc;
  /** Contains the last external McCready setting */
  Speed lastExternalMc;
  /** Contains the last bug setting */
  ushort lastBugs;
  /** Contains the last external bug setting */
  ushort lastExternalBugs;
  /** Contains the last TAS value */
  Speed lastTas;
  /** Contains the last IAS value */
  Speed lastIas;
  /** contains the current state of TAS calculation */
  bool m_calculateTas;
  /** Contains the last variometer value */
  Speed lastVario;
  /** Contains the last known glide path information */
  Altitude lastGlidePath;

  /** Contains the last known altitude */
  Altitude lastAltitude;

  /** Contains the last known GNSS altitude */
  Altitude lastGNSSAltitude;
  /** Contains the last known pressure altitude */
  Altitude lastPressureAltitude;

  /** Contains the last known AGL altitude */
  Altitude lastAGLAltitude;
  /** Contains the last known STD altitude */
  Altitude lastSTDAltitude;
  /** Contains the last known Error margin for the AGL altitude */
  Distance lastAGLAltitudeError;
  /** Contains the last known altitude about home level. */
  Altitude lastAHLAltitude;
  /** Contains the altitude used for manual navigation mode */
  Altitude manualAltitude;

  /** Contains the error margin for the lastElevation */
  Distance lastElevationError;
  /** Contains the last known elevation */
  Altitude lastElevation;
  /** Contains the last returned altitude collection */
  AltitudeCollection lastAltCollection;
  /** Contains the last known position, either obtained from the GPS or modified by manual input. */
  QPoint lastPosition, lastGPSPosition;
  /** contains the last known GPS heading */
  int lastHeading;
  /** contains the last known magnetic heading */
  int lastMagneticHeading;
  /** Contains the last know static pressure in hPa */
  double lastStaticPressure;
  /** Contains the last know dynamic pressure in Pa */
  double lastDynamicPressure;

  /** contains the last known wind */
  struct
  {
    Vector wind;
    Altitude altitude;
  } m_lastWind;

  /** contains the last required LD to target */
  double lastRequiredLD;
  /** contains the last current LD */
  double lastCurrentLD;
  /** contains the current state of LD calculation */
  bool m_calculateLD;
  /** contains the polar object of the selected glider */
  Polar* m_polar;
  /** contains some functions to provide variometer data */
  Vario* m_vario;
  /** contains the current state of vario calculation */
  bool m_calculateVario;
  /** Contains the last known flight mode */
  FlightMode lastFlightMode;
  /** Last marker value used */
  int m_marker;
  /** contains the current state of wind calculation */
  bool m_calculateWind;
  /** contains functions to analyze the wind */
  WindAnalyser* m_windAnalyser;
  /** contains functions to analyze the wind */
  ReachableList* m_reachablelist;
  /** maintains wind measurements and returns new wind values */
  WindStore* m_windStore;
  /** Info on the selected glider. */
  Glider* m_glider;
  /** Did we already receive a complete sentence? */
  bool m_pastFirstFix;
  /** Direction of cruise if we are in cruising mode */
  int m_cruiseDirection;
  /** the index of the selected taskpoint in the flight task list. */
  int m_selectedWpInList;
  /** task end touch flag */
  bool m_taskEndReached;

  bool m_manualInFlight;

  /** minimum altitude for determining altitude gain. */
  Altitude m_minimumAltitude;

  /** Altitude gain. */
  Altitude m_gainedAltitude;

  /** Last got temperature in degree Celsius */
  double m_lastTemperature;

  /** Last task point passage state. */
  enum TaskPoint::PassageState m_lastTpPassageState;

  /** Last used zoom factor. */
  double m_lastZoomFactor;

  /**
   * Timer to reset an auto map zoom after a taskpoint passage.
   */
  QTimer* m_resetAutoZoomTimer;

  /**
   * Timer to supervise external vaiometer data.
   */
  QTimer* m_varioDataControl;
};

extern Calculator* calculator;

#endif
