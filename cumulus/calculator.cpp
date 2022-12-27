/***********************************************************************
 **
 **   calculator.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2002      by André Somers
 **                  2008-2022 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   This class provides different calculator functionalities
 **
 ***********************************************************************/

#include <climits>
#include <cmath>
#include <cstdlib>

#include <QtWidgets>

#include "altimeterdialog.h"
#include "Atmosphere.h"
#include "calculator.h"
#include "flarmbase.h"
#include "generalconfig.h"
#include "gliderlistwidget.h"
#include "gpsnmea.h"
#include "layout.h"
#include "MainWindow.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "reachablelist.h"
#include "tpinfowidget.h"
#include "whatsthat.h"
#include "WindAnalyser.h"

#define MAX_MCCREADY 10.0
#define MAX_SAMPLECOUNT 600

Calculator *calculator = static_cast<Calculator *> (0);

extern MainWindow  *_globalMainWindow;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

Calculator::Calculator( QObject* parent ) :
  QObject(parent),
  infiniteTemperature(-300.0),
  samplelist( LimitedList<FlightSample>( MAX_SAMPLECOUNT ) ),
  m_turnRight(0),
  m_turnLeft(0),
  m_flyStraight(0)
{
  setObjectName( "Calculator" );
  GeneralConfig *conf = GeneralConfig::instance();

  manualAltitude.setMeters( conf->getManualNavModeAltitude() );

  lastGNSSAltitude     = manualAltitude;
  lastPressureAltitude = manualAltitude;
  lastAltitude         = manualAltitude;
  lastAGLAltitude      = manualAltitude;
  lastSTDAltitude      = manualAltitude;
  lastAHLAltitude      = manualAltitude;
  lastAGLAltitudeError.setMeters(0);

  lastPosition.setX( conf->getCenterLat() );
  lastPosition.setY( conf->getCenterLon() );

  lastSpeed.setMps(0);

  lastETA = QTime();
  lastBearing = -1;
  lastHeading = -1;
  lastMagneticHeading = -1.0;
  lastMagneticTrueHeading = -1.0;
  lastDistance = -1;
  lastRequiredLD = -1.0;
  lastCurrentLD = -1.0;
  lastDynamicPressure = -1.0;
  lastStaticPressure = -1.0;
  m_lastTemperature = infiniteTemperature;
  m_calculateVario = true,
  m_calculateLD = false;
  m_calculateETA = false;
  m_calculateTas = true;
  m_calculateWind = ! conf->isExternalWindEnabled();
  m_lastWind.wind = Vector();
  m_lastWind.altitude = lastAltitude;
  targetWp = static_cast<Waypoint *> (0);
  lastMc = GeneralConfig::instance()->getMcCready();
  lastExternalMc = GeneralConfig::instance()->getExternalMcCready();
  lastBugs = 0;
  lastExternalBugs = 0;
  lastBestSpeed.setInvalid();
  lastTas = -1.0;
  lastIas = -1.0;
  m_polar = 0;
  m_vario = new Vario (this);
  m_windAnalyser = new WindAnalyser(this);
  m_windInStraightFlight = new WindCalcInStraightFlight(this);
  m_reachablelist = new ReachableList(this);
  m_windStore = new WindStore(this);
  lastFlightMode = unknown;
  m_marker = 0;
  m_glider = static_cast<Glider*>( 0 );
  m_pastFirstFix = false;
  m_selectedWpInList = -1;
  m_taskEndReached = false;
  m_manualInFlight = false;
  m_cruiseDirection = -1;
  m_minimumAltitude = INT_MIN;
  m_lastTpPassageState = TaskPoint::Outside;
  m_lastZoomFactor = -1.0;

  m_resetAutoZoomTimer = new QTimer( this );
  m_resetAutoZoomTimer->setSingleShot( true );

  m_varioDataControl = new QTimer( this );
  m_varioDataControl->setSingleShot( true );

  connect( m_resetAutoZoomTimer, SIGNAL(timeout()),
           this, SLOT(slot_switchMapScaleBack()) );

  connect( m_varioDataControl, SIGNAL(timeout()),
           this, SLOT(slot_varioDataControl()) );

  // hook up the internal backend components
  connect( m_vario, SIGNAL(newVario(const Speed&)),
           this, SLOT(slot_Variometer(const Speed&)));

  // The former calls to slot_newSample and slot_newFlightMode of the wind
  // analyzer are replaced by direct calls, when a new sample or flight
  // mode is available and the wind calculation is enabled. Wind calculation
  // can be disabled when the Logger device delivers already wind data.
  connect( m_windAnalyser, SIGNAL(newMeasurement( Vector&, const Altitude&, float, int ) ),
           m_windStore, SLOT(slot_Measurement( Vector&, const Altitude&, float, int ) ));

  connect( m_windInStraightFlight, SIGNAL(newMeasurement( Vector&, const Altitude&, float, int ) ),
           m_windStore, SLOT(slot_Measurement( Vector&, const Altitude&, float, int ) ));

  connect( this, SIGNAL(newMeasurement( Vector&, const Altitude&, float, int ) ),
           m_windStore, SLOT(slot_Measurement( Vector&, const Altitude&, float, int ) ));

  connect( m_windStore, SIGNAL(newWind( Vector& )),
           this, SLOT(slot_Wind( Vector& )));

  connect( this, SIGNAL(newAltitude(const Altitude&)),
           m_windInStraightFlight, SLOT(slot_Altitude(const Altitude&)));
}

Calculator::~Calculator()
{
  if ( m_glider )
    {
      delete m_glider;
    }

  if ( targetWp )
    {
      delete targetWp;
    }

  GeneralConfig *conf = GeneralConfig::instance();

  // save last position as new center position of the map
  conf->setCenterLat(lastPosition.x());
  conf->setCenterLon(lastPosition.y());
}

/** Read property of Altitude for Altimeter display */
const Altitude& Calculator::getAltimeterAltitude()
{
  // qDebug("Calculator::getAltimeterAltitude(): %d",  _altimeter_mode );
  switch ( AltimeterDialog::mode() )
    {
    case 0:
      return lastAltitude; // MSL
      break;
    case 1:
      return lastSTDAltitude; // STD
      break;
    case 2:
      return lastAGLAltitude; // AGL
      break;
    case 3:
      return lastAHLAltitude; // AHL
      break;
    }

  return lastAltitude;
}

const AltitudeCollection& Calculator::getAltitudeCollection()
{
  lastAltCollection.gndAltitude=lastAGLAltitude;
  lastAltCollection.stdAltitude=lastSTDAltitude;
  lastAltCollection.gpsAltitude=lastAltitude;
  lastAltCollection.pressureAltitude=lastAltitude;
  lastAltCollection.gpsAltitudeError=0;
  lastAltCollection.gndAltitudeError=lastAGLAltitudeError;

  return lastAltCollection;
}

/** called on a GNNS altitude change */
void Calculator::slot_GnssAltitude( Altitude& altitude )
{
  if( lastGNSSAltitude != altitude )
    {
      lastGNSSAltitude = altitude;

      if( GeneralConfig::instance()->getGpsAltitude() == GpsNmea::GPS )
        {
          // Altitude correction value set by the user.
          Altitude _userAltitudeCorrection =
              GeneralConfig::instance()->getGpsUserAltitudeCorrection();

          lastAltitude.setMeters( altitude.getMeters() + _userAltitudeCorrection.getMeters() );
          lastSTDAltitude = altitude;
          lastAGLAltitude  = lastAltitude - lastElevation;
          lastAGLAltitudeError = lastElevationError;
          lastAHLAltitude  = lastAltitude - GeneralConfig::instance()->getHomeElevation();

          emit newAltitude( lastAltitude );
          emit newUserAltitude( getAltimeterAltitude() );

          // Calculate GPS altitude gain.
          calcAltitudeGain();
        }
    }

  // The GNNS altitude has been changed. Check, if a wind information has to be
  // distributed. It is get from the wind store. If we are in still stand or
  // circling, that is not necessary.
  if( GeneralConfig::instance()->isManualWindEnabled() == false &&
      lastFlightMode == cruising )
    {
      // Only in cruise mode we have to retrieve the wind from the wind store
      // and to distribute.
      Vector v = getWindStore()->getWind( lastAltitude );

      if( v.isValid() == true )
        {
          setLastWind( v );
          emit newWind( v ); // forwards the wind info to the MapView
        }
    }

  // The glide path is only calculated with the GPS altitude, which is
  // independently from the pressure variation.
  calcGlidePath();
}

/** called on a pressure altitude change */
void Calculator::slot_PressureAltitude( Altitude& altitude )
{
  if( lastPressureAltitude != altitude )
    {
      lastPressureAltitude = altitude;

      if( GeneralConfig::instance()->getGpsAltitude() == GpsNmea::PRESSURE )
        {
          // Altitude correction value set by the user.
          Altitude _userAltitudeCorrection =
              GeneralConfig::instance()->getGpsUserAltitudeCorrection();

          lastAltitude.setMeters( altitude.getMeters() + _userAltitudeCorrection.getMeters() );
          lastSTDAltitude = altitude;
          lastAGLAltitude  = lastAltitude - lastElevation;
          lastAGLAltitudeError = lastElevationError;
          lastAHLAltitude  = lastAltitude - GeneralConfig::instance()->getHomeElevation();

          emit newAltitude( lastAltitude );
          emit newUserAltitude( getAltimeterAltitude() );

          // Calculate pressure altitude gain
          calcAltitudeGain();
        }
    }
}

/** Called if a new heading has been obtained */
void Calculator::slot_Heading( const double& newHeadingValue )
{
  if( lastSpeed.getMps() <= 0.3 )
    {
      // @AP: don't forward values, when the speed is nearly
      // zero. Arrow will stay in last position. Same does make
      // Garmin-Pilot III
      return;
    }

  lastHeading = static_cast<int>( rint( newHeadingValue ) );

  emit newHeading( lastHeading );

  // if we have no bearing, lastBearing is -1;
  // this is only a small mistake, relBearing points to north
  //@JD: but wrong showing of new relative bearing icon, so bail out
  if( lastBearing < 0 )
    {
      return;
    }

  emit newRelBearing (lastBearing - lastHeading);
}

/**
 * Called if a new compass magnetic heading has been obtained.
 */
void Calculator::slot_MagneticHeading( const double& newHeading )
{
  lastMagneticHeading = newHeading;
  emit newMagneticHeading( newHeading );
}

/**
 * Called if a new compass true magnetic heading has been obtained
 */
void Calculator::slot_MagneticTrueHeading( const double& newHeading )
{
  // Call always the in straight flight wind calculator, when external TAS
  // is available
  if( m_calculateTas == false )
    {
      m_windInStraightFlight->slot_trueCompassHeading( newHeading );
    }

  lastMagneticTrueHeading = newHeading;
  emit newMagneticTrueHeading( newHeading );
}

/** called if a new speed fix has been received */
void Calculator::slot_Speed( Speed& newSpeedValue )
{
  lastSpeed = newSpeedValue;
  emit newSpeed(newSpeedValue);
}

/** called if a new position-fix has been established. */
void Calculator::slot_Position( QPoint& newPositionValue )
{
  lastGPSPosition = newPositionValue;

  if( ! m_manualInFlight )
    {
      lastPosition = lastGPSPosition;
    }

  lastElevation = Altitude( _globalMapContents->findElevation(lastPosition, &lastElevationError) );
  emit newPosition(lastGPSPosition, Calculator::GPS);
  calcDistance();
  calcETA();
  calcTas();
  calcGlidePath();
  // Calculate List of reachable items
  m_reachablelist->calculate(false);
}

/**
 * Called when a new AHRS data set is available.
 */
void Calculator::slot_AHRSInfo( const double rollAngle,
                                const double pitchAngle,
                                const double accelarationX,
                                const double accelarationY,
                                const double accelarationZ )
{

}

/** Called if a new waypoint has been selected. If user action is
    true, change was done by an user interaction.*/
void Calculator::slot_WaypointChange(Waypoint *newWp, bool userAction)
{
  if ( targetWp && newWp &&
       targetWp->taskPointIndex != -1 && newWp->taskPointIndex == -1 )
    {
      // A user action will overwrite a task point. That will stop the
      // automatic task point switch. We will notice the user about that fact.
      QString text = tr( "Replace current task point?" );

      QString infoText = tr( "<html>"
                              "A flight task is activated!<br>"
                              "This selection will stop the automatic task point switch."
                              "To avoid that make a selection from task menu."
                              "<br>Do You really want to replace?"
                              "</html>" );

      int ret = Layout::messageBox( QMessageBox::Question,
                                    text,
                                    infoText,
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No,
                                    QApplication::desktop() );

      if( ret != QMessageBox::Yes )
        {
          // do nothing change
          return;
        }
    }

  // Map new waypoint instance to current projection to be sure, that
  // all is correct before distribution.
  if ( newWp )
    {
      newWp->projPoint = _globalMapMatrix->wgsToMap( newWp->wgsPoint );
    }

  // save new target waypoint
  setTargetWp( newWp );

  FlightTask *task = _globalMapContents->getCurrentTask();

  if ( newWp == nullptr )
    {
      if ( task != static_cast<FlightTask *> (0) )
        {
          task->resetTimes();
        }

      lastBestSpeed.setInvalid();
      emit bestSpeed( lastBestSpeed );

      // reset LD display
      emit newLD( lastRequiredLD=-1, lastCurrentLD=-1 );
      m_selectedWpInList = -1;
      m_taskEndReached = false;
      //@JD: reset bearing, and no more calculations
      lastBearing = -1;
      return;
    }

  if ( targetWp && userAction && targetWp->taskPointIndex != -1 )
    {
      // This was not an automatic switch, it was made manually by the
      // user in the tasklistview or initiated by pressing accept in the
      // preflight dialog.
      m_taskEndReached = false;
      m_selectedWpInList = -1;

      if ( task != static_cast<FlightTask *> (0) )
        {
          // Clear all times in the selected task.
          task->resetTimes();

          QList<TaskPoint>& tpList = task->getTpList();

          // Tasks with less 2 entries are incomplete!
          for ( int i=0; i < tpList.count() && tpList.count() >= 2; i++ )
            {
              if ( targetWp->wgsPoint == tpList.at(i).getWGSPosition() &&
                   targetWp->taskPointIndex == tpList.at(i).getFlightTaskListIndex() )
                {
                  m_selectedWpInList = i;
                  break;
                }
            }

          emit taskInfo( tr("Task started"), true );
        }
    }

  if( userAction )
    {
      calcDistance( !userAction );
    }
  else
    {
      calcBearing();
    }

  calcETA();
  calcGlidePath();
  // for debug purpose trigger manually a calculation by selecting a waypoint
}

/**
 * Called if a waypoint has to be deleted.
 */
void Calculator::slot_WaypointDelete(Waypoint* newWp)
{
  // @AP: check, if waypoint to be deleted is the set target. In this case a
  // deselection must be done
  if ( newWp && targetWp && *targetWp == *newWp )
    {
      m_taskEndReached = false;
      setTargetWp(0);
      calcBearing();
      calcETA();
      lastBestSpeed.setInvalid();
      emit bestSpeed( lastBestSpeed );
      emit newLD( lastRequiredLD=-1, lastCurrentLD=-1 );
    }
}

/**
  Calculates the distance and the bearing to the currently selected waypoint
  and emits newDistance if the distance has been changed. If a flight
  task has been activated, the automatic switch from one task point
  to the next is also controlled and handled here.
*/
void Calculator::calcDistance( bool autoWpSwitch )
{
  if ( ! targetWp )
    {
      // There is no waypoint selected.
      return;
    }

  QPair<double, double> p = MapCalc::distVinc( double(lastPosition.x()),
                                               double(lastPosition.y()),
                                               targetWp->wgsPoint.lat(),
                                               targetWp->wgsPoint.lon() );
  // pass bearing to related method
  calcBearing( p.second );

  Distance curDistance( p.first * 1000.0 );

  if ( curDistance == lastDistance )
    {
      // no changes in the meantime
      return;
    }

  // get active task
  FlightTask *task = _globalMapContents->getCurrentTask();

  if ( ! task || targetWp->taskPointIndex == -1 || autoWpSwitch == false )
    {
      // no task active,
      // no selected waypoint from a task
      // no automatic waypoint switch required
      // emit new distance only
      lastDistance = curDistance;
      emit newDistance(lastDistance);
      return;
    }

  // load task point list from task
  QList<TaskPoint>& tpList = task->getTpList();

  // Load active task switch scheme. Switch to next TP can be executed
  // by nearest to TP or touched TP sector/circle.
  enum GeneralConfig::ActiveTaskSwitchScheme ntScheme =
    GeneralConfig::instance()->getActiveTaskSwitchScheme();

  enum TaskPoint::PassageState passageState = TaskPoint::Outside;

  TaskPoint& tp = tpList[ targetWp->taskPointIndex ];

  passageState = tp.checkPassage( curDistance, lastPosition );

  if( passageState == TaskPoint::Near )
    {
      if( m_lastTpPassageState != passageState )
        {
          // Auto zoom in to make the task point figure better visible.
          autoZoomInMap();

          if( tp.getActiveTaskPointFigureScheme() == GeneralConfig::Line )
            {
              // Announce task point in sight for a line only, if nearest
              // position is reached.
              emit taskInfo( tr("TP in sight"), true );
            }
        }

      m_lastTpPassageState = passageState;
    }

  if( passageState == TaskPoint::Touched )
    {
      if( m_lastTpPassageState != passageState )
        {
          // Auto zoom in to make the task point figure better visible.
          autoZoomInMap();

          if( ntScheme == GeneralConfig::Nearst &&
              tp.getActiveTaskPointFigureScheme() != GeneralConfig::Line )
            {
              // Announce task point touch only, if nearest switch scheme is
              // chosen by the user and no line figure is active to avoid to
              // much info for him.
              emit taskInfo( tr("TP in sight"), true );
            }

          // Send a signal to the IGC logger to increase logging interval
          emit taskpointSectorTouched();

          // Send a pilot event to the Flarm, if Flarm is recognized.
          // That increases the IGC logger interval to 1s for 30s.
          if( FlarmBase::getFlarmStatus().valid == true )
            {
              const QString pilotEvent = "$PFLAI,PILOTEVENT";
              GpsNmea::gps->sendSentence( pilotEvent );
            }
        }

      m_lastTpPassageState = passageState;
    }

  if( passageState == TaskPoint::Passed )
    {
      m_lastTpPassageState = TaskPoint::Passed;

      // setup a timer to reset a task point approach zoom after 10s of passage
      if( m_lastZoomFactor > 0)
        {
          m_resetAutoZoomTimer->start( 10000 );
        }

      // Display a task end message under the following conditions:
      // a) we touched/passed the target radius
      // b) the finish task point is selected
      TaskPoint& tp = tpList[ targetWp->taskPointIndex ];

      if( targetWp->taskPointIndex == 0 )
        {
          // The first task point has been passed, set start time of task.
          task->setStartTime();
        }

      // Set pass time of task point
      tp.setPassTime();

      if( tp.getTaskPointType() == TaskPointTypes::Finish )
        {
          m_taskEndReached = true;
          emit taskInfo( tr("Task ended"), true );

          QString text = QString("<html>") +
                         "<table cellpadding=2 cellspacing=0>" +
                         "<tr><th>" +
                         tr("Task Target") +
                         "</th></tr>" +
                         "<tr><td>" +
                         targetWp->name + " (" + targetWp->description + ")" +
                         "</td></tr>" +
                         "<tr><td align=center>" +
                         tr("reached") +
                         "</td></tr>" +
                         "</table" +
                         "</html>";

          // fetch info show time from config and compute it as milli seconds
          int showTime = GeneralConfig::instance()->getInfoDisplayTime() * 1000;

          WhatsThat *box = new WhatsThat( _globalMainWindow, text,  showTime );
          box->show();

          // Reset the task point index of the selected waypoint. That stops
          // the further automatic task point switch.
          targetWp->taskPointIndex = -1;

          // Set the end date-time in the current task
          task->setEndTime();
        }
      else
        {
          if( tpList.count() > m_selectedWpInList + 1 )
          {
            // this loop excludes the last WP
            TaskPoint& lastWp = tpList[ m_selectedWpInList ];
            m_selectedWpInList++;
            TaskPoint& nextWp = tpList[ m_selectedWpInList ];

            // calculate the distance to the next waypoint
            QPair<double, double> p =
                MapCalc::distVinc( &lastPosition, nextWp.getWGSPositionPtr() );

            Distance dist2Next( p.first * 1000.0 );
            curDistance = dist2Next;

            // announce task point change as none user interaction
            slot_WaypointChange( nextWp.getWaypointObject(), false );

            // Here we send a notice to the user about the task point switch.
            emit taskInfo( tr("TP passed"), true );

            if( GeneralConfig::instance()->getReportTpSwitch() == true )
              {
                // Show a detailed switch info, if the user has configured that.
                TPInfoWidget *tpInfo = new TPInfoWidget( _globalMainWindow );

                tpInfo->prepareSwitchText( lastWp.getFlightTaskListIndex(),
                                           dist2Next.getKilometers() );
                tpInfo->showTP();
              }
          }
        }
    }

  lastDistance = curDistance;
  emit newDistance( lastDistance );
}

/** Calculates the ETA (Estimated Time to Arrival) to the current
    waypoint and emits a signal newETA if the value has changed. */
void Calculator::calcETA()
{
  if ( targetWp == nullptr ||
       lastSpeed.getMps() <= 0.3 || ! GpsNmea::gps->getConnected() )
    {
      if( lastETA.isValid() )
        {
	  // Set ETA to invalid
          emit newETA(QTime());
          lastETA = QTime();
        }

      return;
    }

  int eta = (int) rint(lastDistance.getMeters() / lastSpeed.getMps());

  QTime etaNew(0, 0);
  etaNew = etaNew.addSecs(eta);

  if ( lastETA.hour() == etaNew.hour() &&
       lastETA.minute() == etaNew.minute() )
    {
      // changes in seconds only will be ignored
      return;
    }

  if ( etaNew != lastETA )
    {
      lastETA = etaNew;

      if ( eta > 99*3600 )
        {
          // Don't emit times greater as 99 hours. ETA will be set to
          // undefined in such a case to avoid a problem in the map
          // display with to large values.
          QTime invalid;
          emit newETA(invalid);
        }
      else
        {
          emit newETA(lastETA);
        }
    }

  // qDebug("New ETA=%s", etaNew.toString().toLatin1().data());
}

void Calculator::calcTas()
{
  // \param [in] tk Track, course over ground in degrees
  // \param [in] gs Ground speed
  // \param [in] wd Wind direction in degrees
  // \param [in] ws Wind speed
  // \param [out] etas Estimated true air speed.
  // \param [out] eth Estimated true heading

  if( m_calculateTas == false )
    {
      return;
    }

  if( ! lastSpeed.isValid() || lastSpeed.getMps() <= 0.0 )
    {
      return;
    }

  Vector& wind = getLastWind();

  if( ! wind.isValid() )
    {
      return;
    }

  int eth = -1;

  MapCalc::calcETAS( lastHeading,
                     lastSpeed,
                     wind.getAngleDeg(),
                     wind.getSpeed(),
                     lastTas,
                     eth );

  emit newTas( lastTas );
}

/** Calculates the required LD and the current LD about the last seconds,
    if required */
void Calculator::calcLD()
{
  if ( ! targetWp || m_calculateLD == false || samplelist.count() < 2 )
    {
      return;
    }

  const FlightSample *start = 0;
  const FlightSample *end = &samplelist.at(0);
  int timeDiff = 0;
  double distance = 0.0;
  double newCurrentLD = -1.0;
  double newRequiredLD = -1.0;
  bool notify = false;

  GeneralConfig *conf = GeneralConfig::instance();

  // Get the configured calculation time span.
  int ldCalcTime = conf->getLDCalculationTime();

  // first calculate current LD
  for ( int i = 1; i < samplelist.count(); i++ )
    {

      timeDiff = (samplelist.at(i).time).msecsTo(end->time);

      if ( timeDiff >= ldCalcTime * 1000 )
        {
          break;
        }

      // summarize single distances from speed
      distance += samplelist[i].vector.getSpeed().getMps();

      // qDebug( "i=%d, dist=%f", i, distance );
      // store start record
      start = &samplelist[i];
    }

  if ( ! start )
    {
      // time distance too short
      lastCurrentLD = -1.0;
    }
  else
    {
      // calculate altitude difference
      double altDiff = start->altitude.getMeters() - end->altitude.getMeters();

      if ( altDiff <= 0.2 )
        {
          // we climbed in the last time, therefore the result will become huge
          newCurrentLD = 999.0;
        }
      else
        {

          newCurrentLD = distance / altDiff ;

          if ( newCurrentLD  > 999.0 )
            {
              newCurrentLD = 999.0;
            }
        }
    }

  // calculate required LD, we consider elevation and security altitude
  double altDiff = lastAltitude.getMeters() - targetWp->elevation -
                   conf->getSafetyAltitude().getMeters();

  if ( altDiff <= 10.0 )
    {
      newRequiredLD = -1.0;
    }
  else
    {
      newRequiredLD = lastDistance / altDiff;
    }

  if ( newRequiredLD != lastRequiredLD )
    {
      lastRequiredLD = newRequiredLD;
      notify = true;
    }

  if ( newCurrentLD != lastCurrentLD )
    {
      lastCurrentLD = newCurrentLD;
      notify = true;
    }

  if ( notify )
    {
      emit newLD( lastRequiredLD, lastCurrentLD );
      // qDebug("SIGNAL: rLD=%f, cLD=%f", lastRequiredLD, lastCurrentLD);
    }
}


/** Calculates the glide path to the current waypoint and the needed height */
bool Calculator::glidePath( int aLastBearing, Distance aDistance,
                            Altitude aElevation, Altitude &arrivalAlt,
                            Speed &bestSpeed )
{

  if (!m_polar)
    {
      arrivalAlt.setInvalid();
      bestSpeed.setInvalid();
      return false;
    }

  //  qDebug("Glider=%s", _glider->type().toLatin1().data());

  // we use the method described by Bob Hansen
  // get best speed for zero wind V0
  Speed speed = m_polar->bestSpeed(0.0, 0.0, lastMc);
  //qDebug ("rough best speed: %f", speed.getKph());

  // wind has a negative vector!
  //qDebug ("wind: %d/%f", lastWind.getAngleDeg(), lastWind.getSpeed().getKph());

  // assume we are heading for the wp
  Vector groundspeed (aLastBearing, speed);
  //qDebug ("groundspeed: %d/%f", groundspeed.getAngleDeg(), groundspeed.getSpeed().getKph());

  // get last known wind.
  Vector lastWind = getLastWind();

  if( lastWind.isValid() == false )
    {
      lastWind = Vector( 0.0, 0.0 );
    }

  // we add wind because of the negative direction
  Vector airspeed = groundspeed + lastWind;
  //qDebug ("airspeed: %d/%f", airspeed.getAngleDeg(), airspeed.getSpeed().getKph());

  // this is the first iteration of the Bob Hansen method
  Speed headwind = groundspeed.getSpeed() - airspeed.getSpeed() ;
  //qDebug ("headwind: %f", headwind.getKph());

  Altitude minimalArrival( GeneralConfig::instance()->getSafetyAltitude().getMeters() );
  Altitude givenAlt (lastAltitude - Altitude (aElevation) - minimalArrival);

  // improved speed for wind V1
  speed = m_polar->bestSpeed(headwind, 0.0, lastMc);
  //qDebug ("improved best speed: %f", speed.getKph());
  bestSpeed = speed;
  // the ld is over ground, so we take groundspeed
  double ld =m_polar->bestLD(speed, groundspeed.getSpeed(), 0.0);

  arrivalAlt = (givenAlt - (aDistance / ld));

  //qDebug ("ld = %f", ld);
  //qDebug ("bestSpeed: %f", speed.getKph());
  //  qDebug ("lastSpeed: %f", lastSpeed.getKph());

  return true;
}

void Calculator::calcGlidePath()
{
  Speed speed;
  Altitude arrivalAlt;

  // Calculate new glide path, if a waypoint is selected and a glider is defined.
  if ( ! targetWp || ! m_glider )
    {
      lastRequiredLD = -1;
      return;
    }

  int tpIdx = targetWp->taskPointIndex;
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( tpIdx != -1 && // selected waypoint is a task point
      task != 0 &&   // a flight task is defined
      GeneralConfig::instance()->getArrivalAltitudeDisplay() == GeneralConfig::landingTarget )
    {
      // Calculates arrival altitude above landing target.
      task->calculateFinalGlidePath( tpIdx, arrivalAlt, speed );
    }
  else
    {
      // Calculates arrival altitude above selected target.
      glidePath( lastBearing, lastDistance, targetWp->elevation, arrivalAlt, speed );
    }

  if( speed != lastBestSpeed )
    {
      lastBestSpeed = speed;
      emit bestSpeed( speed );
    }

  if( arrivalAlt != lastGlidePath )
    {
      lastGlidePath = arrivalAlt;
      emit glidePath( arrivalAlt );
    }
}

/**
 * Calculates the bearing to the currently selected waypoint and emits signal
 * newBearing if the bearing has changed.
 */
void Calculator::calcBearing( double bearingIn )
{
  int iresult = -1;
  int lH = lastHeading;

  if( lastHeading == -1 )
    {
      lH = 0;
    }

  if( targetWp == nullptr )
    {
      if( iresult != lastBearing )
        {
          lastBearing = iresult;

          emit newBearing(-1000);

          // if we have no waypoint, let the rel bearing arrow point to north
          emit newRelBearing(0);
        }
    }
  else
    {
      double bearing = 0.0;

      if( bearingIn != -1.0 )
        {
          // We got a valid bearing
          bearing = bearingIn;
        }
      else
        {
          // Calculate bearing
          bearing = MapCalc::getBearing( lastPosition, targetWp->wgsPoint );
        }

      iresult = static_cast<int>( rint(bearing * 180./M_PI) );

      if( iresult != lastBearing )
        {
          lastBearing = iresult;
          emit newBearing( lastBearing );
          emit newRelBearing (lastBearing - lH);
        }
    }
}

void Calculator::calcHeading( const QPoint& previousPosition, const QPoint& newPosition )
{
  // Calculate the heading from the previous to the current position.
  if( previousPosition == newPosition )
    {
      return;
    }

  double heading = MapCalc::getBearingWgs( previousPosition, newPosition );

  int iHeading = static_cast<int> (rint(heading * 180.0 / M_PI));

  emit newHeading( iHeading );
}

/** Called if the position is changed manually. */
void Calculator::slot_changePosition(int direction)
{
  extern MapMatrix * _globalMapMatrix;

  double distance=_globalMapMatrix->getScale(MapMatrix::CurrentScale)*10;
  double kmPerDeg;

  // Store previous position for heading calculation.
  QPoint previousPosition = lastPosition;

  switch (direction)
    {
    case MapMatrix::North:
      kmPerDeg = MapCalc::dist(lastPosition.x(), lastPosition.y(), lastPosition.x()+600000, lastPosition.y());
      lastPosition = QPoint((int) (lastPosition.x()+(distance/kmPerDeg) * 600), lastPosition.y());
      break;
    case MapMatrix::West:
      kmPerDeg = MapCalc::dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
      lastPosition = QPoint(lastPosition.x(), (int) (lastPosition.y()-(distance/kmPerDeg) * 600));
      break;
    case MapMatrix::East:
      kmPerDeg = MapCalc::dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
      lastPosition = QPoint(lastPosition.x(), (int) (lastPosition.y()+(distance/kmPerDeg) * 600));
      break;
    case MapMatrix::South:
      kmPerDeg = MapCalc::dist(lastPosition.x(), lastPosition.y(), lastPosition.x()+600000, lastPosition.y());
      lastPosition = QPoint((int) (lastPosition.x()-(distance/kmPerDeg) * 600), lastPosition.y());
      break;
    case MapMatrix::Home:
      lastPosition = QPoint(_globalMapMatrix->getHomeCoord());
      break;
    case MapMatrix::Waypoint:
      if (targetWp)
        {
          lastPosition = targetWp->wgsPoint;
        }
      break;
    case MapMatrix::Position:
      // Nothing to do, new position was already set.
      break;
    }

  // Check new position that it lays inside the earth coordinates +-90, +-180 degrees
  if ( lastPosition.x() > 90*600000 )
    {
      lastPosition.setX(90*600000);
    }
  else if ( lastPosition.x() < -90*600000 )
    {
      lastPosition.setX(-90*600000);
    }

  if ( lastPosition.y() > 180*600000 )
    {
      lastPosition.setY(180*600000);
    }
  else if ( lastPosition.y() < -180*600000 )
    {
      lastPosition.setY(-180*600000);
    }

  lastElevation = Altitude( _globalMapContents->findElevation(lastPosition, &lastElevationError) );
  emit newPosition(lastPosition, Calculator::MAN);

  // In manual mode an STD altitude of 1000 m is set.
  lastSTDAltitude = manualAltitude;
  lastGNSSAltitude = manualAltitude;

  // Altitude correction value set by the user.
  Altitude _userAltitudeCorrection =
      GeneralConfig::instance()->getGpsUserAltitudeCorrection();

  // The last altitude is set by considering the user defined offset.
  lastAltitude = manualAltitude + _userAltitudeCorrection;

  lastAGLAltitude = lastAltitude - lastElevation;
  lastAHLAltitude = lastAltitude - GeneralConfig::instance()->getHomeElevation();

  emit newAltitude(lastAltitude);
  emit newUserAltitude( getAltimeterAltitude() );

  calcDistance();
  calcHeading( previousPosition, lastPosition );
  calcGlidePath();

  // calculate always when moving manually (big delta !)
  m_reachablelist->calculate(true);
  //qDebug("Elevation: %d m",_globalMapContents->findElevation(lastPosition) );
}

/** No descriptions */
void Calculator::slot_changePositionN()
{
  slot_changePosition(MapMatrix::North);
}

/** No descriptions */
void Calculator::slot_changePositionS()
{
  slot_changePosition(MapMatrix::South);
}

/** No descriptions */
void Calculator::slot_changePositionE()
{
  slot_changePosition(MapMatrix::East);
}

/** No descriptions */
void Calculator::slot_changePositionW()
{
  slot_changePosition(MapMatrix::West);
}

/** No descriptions */
void Calculator::slot_changePositionHome()
{
  slot_changePosition(MapMatrix::Home);
}

/** No descriptions */
void Calculator::slot_changePositionWp()
{
  slot_changePosition(MapMatrix::Waypoint);
}

/** Called if position was moved by using mouse. */
void Calculator::slot_changePosition(QPoint& newPosition)
{
  calcHeading( lastPosition, newPosition );
  lastPosition = newPosition;
  slot_changePosition(MapMatrix::Position);
}

/**
 * Called to set the static pressure value from an external device.
 * Pressure in hPa. From the static pressure value the STD altitude is
 * calculated.
 *
 * @param pressure Static pressure value in hPa.
 */
void Calculator::slot_staticPressure( const double pressure )
{
  if( pressure <= 0.0 )
    {
      // undefined static pressure
      return;
    }

  if( lastStaticPressure != pressure )
    {
      lastStaticPressure = pressure;
      emit newStaticPressure( pressure );
    }
}

/**
 * Called to set the dynamic pressure value from an external device
 * (OpenVario or XCVario).
 * Pressure in Pa.
 *
 * @param pressure Static pressure value in Pa.
 */
void Calculator::slot_dynamicPressure( const double pressure )
{
  if( pressure < 0.0 )
    {
      // undefined dynamic pressure
      return;
    }

  if( lastDynamicPressure != pressure )
    {
      lastDynamicPressure = pressure;
      emit newDynamicPressure( lastDynamicPressure );
    }

  // calculate IAS in mps from dynamic pressure in Pa
  double dIas = Atmosphere::pascal2mps( pressure );

  Speed ias( dIas );

  if( lastIas != ias )
    {
      lastIas = ias;
      emit newIas( ias );
    }

  double temp = m_lastTemperature;

  if( temp == infiniteTemperature )
    {
      // assume default temperature of 15 degree Celsius
      temp = 15.0;
    }

  if( lastStaticPressure < 0.0 )
    {
      // undefined static pressure
      return;
    }

  // Calculate the TAS in m/s from the IAS.
  double dTas = Atmosphere::tas( dIas,
                                 lastStaticPressure,
                                 temp );
  Speed tas( dTas );

  if( lastTas != tas )
    {
      lastTas = tas;
      emit newTas( tas );
    }
}

/**
 * Called, if a new temperature value is available from an external device
 * (OpenVario or XCVario).
 * Temperature in degree Celsius.
 */
void Calculator::slot_Temperature( const double temperature )
{
  if( m_lastTemperature != temperature )
    {
      m_lastTemperature = temperature;
      emit newTemperature( temperature );
    }
}

/** increment McCready value */
void Calculator::slot_McUp()
{
  lastMc.setMps( lastMc.getMps() + 0.5 );
  GeneralConfig::instance()->setMcCready(lastMc);
  calcGlidePath();
  emit newMc( lastMc );
}

/**
 * Called, if in the glider flight dialog this option is changed.
 *
 */
void Calculator::slot_ExternalData4McAndBugs( const bool flag )
{
  if( flag == true )
    {
      // Call external mc and bug methods to switch to those data
      slot_ExternalMc( lastExternalMc );
      slot_ExternalBugs( lastExternalBugs );
    }
}

/** Set McCready value, delivered by external device. */
void Calculator::slot_ExternalMc( const Speed &mc )
{
  if( GeneralConfig::instance()->getUseExternalMcAndBugs() == false )
    {
      return;
    }

  lastExternalMc = mc;
  GeneralConfig::instance()->setExternalMcCready( lastExternalMc );
  calcGlidePath();
  emit newMc( mc );
}

/** Set McCready value, delivered by Mc dialog. */
void Calculator::slot_Mc( const Speed &mc )
{
  if( GeneralConfig::instance()->getUseExternalMcAndBugs() == true )
    {
      return;
    }

  lastMc = mc;
  GeneralConfig::instance()->setMcCready( lastMc );
  calcGlidePath();
  emit newMc( mc );
}

/** decrement McCready value; don't get negative */
void Calculator::slot_McDown()
{
  if( m_glider && lastMc.getMps() >= 0.5 )
    {
      lastMc.setMps( lastMc.getMps() - 0.5 );
      GeneralConfig::instance()->setMcCready(lastMc);
      calcGlidePath();
      emit newMc( lastMc );
    }
}

/**
 * Set water value, delivered by Mc dialog, used by glider polare.
 */
void Calculator::slot_Water( const int water )
{
  if( m_glider->polar()->water() != water )
    {
      m_glider->polar()->setWater( water );
    }

  calcGlidePath();
}

/**
 * Set bug value, delivered by Mc dialog, used by glider polare.
 */
void Calculator::slot_Bugs( const int bugs )
{
  if( m_glider->polar()->bugs() != bugs )
    {
      m_glider->polar()->setBugs( bugs );
    }

  calcGlidePath();
}

/**
 * Set bug value used by glider polare and delivered from an external device.
 */
void Calculator::slot_ExternalBugs( const unsigned short bugs )
{
  lastExternalBugs = bugs;

  if( GeneralConfig::instance()->getUseExternalMcAndBugs() == false )
    {
      return;
    }

  if( m_glider )
    {
       if( m_glider->polar()->bugs() != bugs )
        {
          m_glider->polar()->setBugs( bugs );
          calcGlidePath();
        }
    }
}

/**
 * Return the bug value contained in the gilder polar.
 */
ushort Calculator::getBugsFromPolar()
{
  if( m_glider )
    {
      return m_glider->polar()->bugs();
    }

  // No glider is defined, return zero.
  return 0;
}

void Calculator::slot_ExternalTas( const Speed& tas )
{
  // We get the TAS from an external device. Therefore it must not be calculated
  // by Cumulus.
  m_calculateTas = false;
  lastTas.setMps( tas.getMps() );
  emit newTas( lastTas );
}

/**
 * Variometer lift receiver and distributor to map display.
 */
void Calculator::slot_Variometer( const Speed &lift )
{
  if( lastVario != lift )
    {
      lastVario = lift;
      emit newVario( lift );
    }
}

/**
 * External variometer lift receiver. The internal variometer
 * calculation can be switched off, if we got values via this slot.
 */
void Calculator::slot_ExternalVariometer( const Speed &lift )
{
  // Hey we got a variometer value directly from the external device.
  // Therefore internal calculation is not needed and can be switched off.
  m_calculateVario = false;

  if( lastVario != lift )
    {
      lastVario = lift;
      emit newVario( lift );
    }

  m_varioDataControl->start( 5000 );
}

/** Sets the current position to point newPos. */
void Calculator::setPosition( const QPoint &newPos )
{
  lastPosition = newPos;
  emit newPosition( lastPosition, Calculator::MAN );

  calcDistance();
  calcETA();
  calcGlidePath();
}

void Calculator::slot_varioDataControl()
{
  // Variometer control timer expired. Reset variometer status variables
  // to ensure normal calculation from GPS altitude.
  m_calculateVario = true;
}

/** Resets some internal items to the initial state */
void Calculator::slot_settingsChanged()
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Reset last wind information
  m_lastWind.wind = Vector();
  m_lastWind.altitude = lastAltitude;

  slot_ManualWindChanged( conf->isManualWindEnabled() );

  // Switch on the internal variometer lift and wind calculation.
  // User could be changed the GPS device.
  m_calculateVario = true;
  m_calculateWind  = ! conf->isExternalWindEnabled();
  m_calculateTas   = true;

  slot_CheckHomeSiteSelection();

  // Update the glider selected by the user.
  setGlider( GliderListWidget::getUserSelectedGlider() );
}

/**
 * This slot is called by the NMEA interpreter if a new fix in GPRMC has been
 * received.
 */
void Calculator::slot_newFix( const QDateTime& newFixTime )
{
  // before we start making samples, let's be sure we have all the
  // data we need for that. So, we wait for the second Fix.
  if (!m_pastFirstFix)
    {
      m_pastFirstFix = true;
      return;
    }

  // create a new sample structure
  FlightSample sample;

  // fill it with the relevant data
  sample.time = newFixTime;
  sample.altitude.setMeters(lastAltitude.getMeters());
  sample.STDAltitude.setMeters(lastSTDAltitude.getMeters());
  sample.GNSSAltitude.setMeters(lastGNSSAltitude.getMeters());
  sample.position=lastPosition;
  sample.vector.setAngleAndSpeed(lastHeading, lastSpeed);

  //qDebug("Direction in sample: %d degrees", int(lastHeading));
  Vector groundspeed (lastHeading, lastSpeed);
  // qDebug ("groundspeed: %d/%f", groundspeed.getAngleDeg(), groundspeed.getSpeed().getKph());
  // qDebug ("wind: %d/%f", lastWind.getAngleDeg(), lastWind.getSpeed().getKph());

  if( m_calculateTas == false )
    {
      // We have a real TAS
      sample.airspeed = lastTas;
    }
  else
    {
      // Try to calculate a TAS, when wind is available.
      Vector lastWind = getLastWind();

      if( lastWind.isValid() == false )
        {
          // work around, set TAS equal to ground speed, if wind is not available.
          lastWind = Vector( 0.0, 0.0 );
        }

      Vector airspeed = groundspeed + lastWind;
      // qDebug ("airspeed: %d/%f", airspeed.getAngleDeg(), airspeed.getSpeed().getKph());
      sample.airspeed = airspeed.getSpeed();
    }

  // add to the samplelist
  samplelist.add(sample);

  lastSample = sample;

  // start analyzing...
  // determine if we are standing still, cruising, circling or doing something else
  determineFlightStatus();

  // Call variometer calculation derived from GPS altitude. Can be switched off,
  // when an external device delivers variometer information derived from a
  // baro sensor.
  if ( m_calculateVario == true )
    {
      m_vario->newAltitude();
    }

  // Call wind analyzer calculation if required. Can be switched off,
  // when an external device delivers wind information.
  if ( m_calculateWind == true )
    {
      m_windAnalyser->slot_newSample();
    }

  // Calculate LD
  calcLD();

  // let the world know we have added a new sample to our sample list
  emit newSample();
}

/** Determines the status of the flight: unknown, standstill, cruising, circlingL, circlingR */
void Calculator::determineFlightStatus()
{
  /*
    We define a minimum turn rate of 4 degrees per second (that is,
    one turn takes at most one and a half minute).

    This can be a bit more advanced to allow for temporary changes in
    circling speed in order to better center a thermal.
  */
#define MINTURNANGDIFF 4 //see above

  if( samplelist.count() < 2 )
    {
      // We need to have two samples in order to be able to analyze anything.
      // qDebug() << "determineFlightStatus: samples < 2";
      return;
    }

  // get headings from the last two samples
  int lastHead = samplelist[0].vector.getAngleDeg();
  int prevHead = samplelist[1].vector.getAngleDeg();

  // calculate the heading difference between the last two samples
  int headingDiff = MapCalc::angleDiff( prevHead, lastHead );

  // get the time difference between these samples
  int timediff = samplelist[1].time.secsTo(samplelist[0].time);

#if 0
  qDebug() << "determineFlightStatus:"
           << "LastFlightMode=" << lastFlightMode
           << "headingDiff=" << headingDiff
           << "timeDiff=" << timediff
           << "lastSpeed" << lastSpeed.getMps() << "m/s";
#endif

  if (timediff == 0)
    {
      // If the time difference is 0, return. This will only cause problems.
      // qDebug() << "determineFlightStatus: sample timeDiff = 0";
      return;
    }

  // Check for standstill at first
  if( samplelist[0].position == samplelist[1].position ||
      lastSpeed.getMps() <= 1.0 )
    {
      if( lastFlightMode != standstill )
        {
          qDebug() << "determineFlightStatus: standstill detected";
          m_turnRight = m_turnLeft = m_flyStraight = 0;
          lastFlightMode = standstill;
          newFlightMode( lastFlightMode );
        }

       return;
    }

  if( headingDiff > MINTURNANGDIFF )
    {
      if( m_turnRight < 4 )  // hold down
        {
          m_turnRight++;
        }

      m_turnLeft = m_flyStraight = 0;

      // right circling is assumed
      if( lastFlightMode != circlingR && m_turnRight > 2 )
        {
          qDebug() << "determineFlightStatus: circlingR detected,"
                   << "HeadingDiff=" << headingDiff
                   << "Vm/s=" << lastSpeed.getMps();
          lastFlightMode = circlingR;
          newFlightMode( lastFlightMode );
        }
    }
  else if( headingDiff < -MINTURNANGDIFF )
    {
      if( m_turnLeft < 4)
        {
          m_turnLeft++;
        }

      m_turnRight = m_flyStraight = 0;

        // left circling is assumed
      if( lastFlightMode != circlingL && m_turnLeft > 2 )
        {
          qDebug() << "determineFlightStatus: circlingL detected"
                   << "HeadingDiff=" << headingDiff
                   << "Vm/s=" << lastSpeed.getMps();
          lastFlightMode = circlingL;
          newFlightMode( lastFlightMode );
        }
    }
  else
    {
      if( m_flyStraight < 4 )
        {
          m_flyStraight++;
        }

      m_turnLeft = m_turnRight = 0;

      if( lastFlightMode != cruising  && m_flyStraight > 2 )
        {
          qDebug() << "determineFlightStatus: cruising detected"
                   << "HeadingDiff=" << headingDiff
                   << "Vm/s=" << lastSpeed.getMps();

          // cruising is assumed, set heading start point
          m_cruiseDirection = lastHead;
          lastFlightMode = cruising;
          newFlightMode( lastFlightMode );
        }
    }
}

/** Called if the status of the GPS changes. */
void Calculator::slot_GpsStatus(GpsNmea::GpsStatus newState)
{
  // qDebug("connection status changed...");
  FlightMode flightMode = unknown;

  if (flightMode != lastFlightMode)
    {
      lastFlightMode       = flightMode;
      samplelist[0].marker = ++m_marker;
      // qDebug("new FlightMode: %d",lastFlightMode);
      newFlightMode(flightMode);
    }

  if ( newState != GpsNmea::validFix )
    {
      // Reset LD display
      emit newLD( -1.0, -1.0 );
      // reset first fix passed
      m_pastFirstFix = false;
    }

  if( newState == GpsNmea::notConnected )
    {
      // Reset ETA in calculator and on map display.
      calcETA();

      // reset flight detection
      m_turnRight = m_turnLeft = m_flyStraight = 0;
    }
}

/** This function is used internally to emit the flight mode signal with the marker value */
void Calculator::newFlightMode( Calculator::FlightMode fm )
{
  if( m_calculateWind == true )
    {
      // Wind calculation can be disabled when the Logger device
      // delivers already wind data.
      m_windAnalyser->slot_newFlightMode( fm );
    }

  emit flightModeChanged( fm );
}

/** Called if a new wind measurement is delivered by an external device */
void Calculator::slot_ExternalWind( const Speed& speed, const short direction )
{
  // Hey we got a wind value directly from an external device.
  if( m_calculateWind == true )
    {
      // User has disabled the usage of external wind.
      return;
    }

  Vector v;
  v.setAngle( direction );
  v.setSpeed( speed );

  // Add new wind with the best quality 5 to the windStore.
  m_windStore->slot_Measurement( v, lastAltitude, 5.0, 1 );
}

/** Called if the wind measurement changes */
void Calculator::slot_Wind( Vector& v )
{
  if( GeneralConfig::instance()->isManualWindEnabled() )
    {
      // User has manual wind preselected.
      return;
    }

  setLastWind(v);
  emit newWind(v); // forwards the wind info to the MapView
  calcGlidePath();
}

void Calculator::slot_ManualWindChanged( bool enabled )
{
  GeneralConfig* conf = GeneralConfig::instance();

  Vector v( 0.0, 0.0 );

  if( enabled == true )
    {
      v.setAngle( conf->getManualWindDirection() );
      v.setSpeed( conf->getManualWindSpeed() );
   }
  else
    {
      // Try to get a wind info from the wind store.
      v = getWindStore()->getWind( lastAltitude );

      if( v.isValid() == false )
        {
          v = Vector( 0.0, 0.0 );
        }
    }

  setLastWind(v);
  emit newWind(v); // forwards the wind info to the MapView
  calcGlidePath();
}

void Calculator::slot_userMapZoom()
{
  // Reset last zoom factor, if the user did a map zoom.
  m_lastZoomFactor = -1.0;
}

void Calculator::slot_switchMapScaleBack()
{
  if( m_lastZoomFactor != -1.0 )
    {
      // Zoom back to the zoom factor before the task point has been touched.
      emit switchMapScale( m_lastZoomFactor );

      // Reset zoom factor
      m_lastZoomFactor = -1.0;
    }
}

/** Store the property of a new Glider. */
void Calculator::setGlider(Glider* glider)
{
  if (m_glider)
    {
      delete m_glider;

      // reset glider and polar object to avoid senseless calculations
      m_glider = 0;
      m_polar  = 0;
    }

  if (glider)
    {
      m_glider = glider;
      m_polar = m_glider->polar();
      calcETA();
      calcGlidePath();
      emit newGlider( m_glider->type() );
    }
  else
    {
      lastMc.setInvalid();
      GeneralConfig::instance()->setMcCready( lastMc );
      lastBestSpeed.setInvalid();
      emit bestSpeed( lastBestSpeed );
      emit newGlider( "" );
    }

  // Switches on/off Mc in map view
  emit newMc( lastMc );

  // Recalculate the nearest reachable sites because
  // glider has been modified resp. removed.
  m_reachablelist->calculateNewList();
}

bool Calculator::matchesFlightMode(GeneralConfig::UseInMode mode)
{
  switch (mode)
  {
    case GeneralConfig::always:
        return true;
    case GeneralConfig::never:
        return false;
    case GeneralConfig::standstill:
        return lastFlightMode == standstill;
    case GeneralConfig::cruising:
        return lastFlightMode == cruising;
    case GeneralConfig::circling:
        return (lastFlightMode == circlingL || lastFlightMode == circlingR);
    default:
        return false;
  }
}

void Calculator::setTargetWp( Waypoint* newTarget )
{
  // delete old waypoint selection
  if ( targetWp != 0 )
    {
      delete targetWp;
      targetWp = 0;
    }

  // Save new target waypoint permanently into a file.
  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/target.wpt";

  Waypoint::write( newTarget, fn );

  // make a deep copy of the new waypoint to be set
  if ( newTarget != 0 )
    {
      targetWp = new Waypoint( *newTarget );
    }

  // Reset task point passage state
  m_lastTpPassageState = TaskPoint::Outside;

  // inform mapView about the change
  emit newWaypoint(newTarget);
}

void Calculator::setManualInFlight(bool switchOn)
{
  m_manualInFlight = switchOn;
  // immediately put glider into center if outside
  // we can only switch off if GPS data coming in
  emit switchManualInFlight();
}

/**
 * Checks, if the a selected waypoint to the home site exists
 * and if the home site has been changed. In this case the
 * selection is renewed to the new position.
 */
void Calculator::slot_CheckHomeSiteSelection()
{
  // Check, if home position has been changed and the selected waypoint is
  // the home position. In this case the home position selection must be
  // renewed.
  GeneralConfig *conf = GeneralConfig::instance();

  if( targetWp && targetWp->name == tr("Home") &&
      targetWp->wgsPoint != conf->getHomeCoord() )
    {
      Waypoint wp;

      wp.name = tr("Home");
      wp.description = tr("Home Site");
      wp.wgsPoint.setPos( conf->getHomeCoord() );

      slot_WaypointChange( &wp, true );
    }
}

/**
 * Called to select the start point of a loaded task.
 * That will also activate the automatic task point switch.
 */
void Calculator::slot_startTask()
{
  FlightTask *task = _globalMapContents->getCurrentTask();

  if ( task == nullptr )
    {
      // no task defined, ignore request
      return;
    }

  QList<TaskPoint> tpList = task->getTpList();

  // Tasks with less 2 points are incomplete, request is ignored.
  if( tpList.count() < 2 )
    {
      return;
    }

  // Take the start point of the task.
  TaskPoint& tp2Taken = tpList[0];

  if( targetWp )
    {
      // Check, if another task point is already selected. In this case ask the
      // user if the first point shall be really selected.
      if( targetWp->taskPointIndex != -1 &&
          targetWp->taskPointIndex != tp2Taken.getFlightTaskListIndex() )
        {
          QString text = tr( "Restart current task?" );
          QString infoText = tr( "<html>"
                                 "A flight task is running!<br>"
                                 "This command will start the<br>"
                                 "task again at the beginning."
                                 "<br>Do You really want to restart?"
                                 "</html>" );

          int ret = Layout::messageBox( QMessageBox::Question,
                                        text,
                                        infoText,
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::Yes,
                                        QApplication::desktop() );

          if( ret == QMessageBox::No )
            {
              return;
            }

          // All time data are reset in the current selected task.
          task->resetTimes();
        }
    }

  slot_WaypointChange( tp2Taken.getWaypointObject(), true );
}

/**
 * @returns true if we move us over the user's defined speed limit.
 */
bool Calculator::moving()
{
  // The speed limit is in m/s
  const double SpeedLimit = GeneralConfig::instance()->getAutoLoggerStartSpeed() * 1000.0 / 3600.0;
  const int TimeLimit     = 5; // time limit in seconds

  if( samplelist.size() <= TimeLimit )
    {
      // We need to have some samples in order to be able to analyze speed.
      return false;
    }

  double speed = 0.0;

  // Note, that the newest samples are inserted at the list beginning.
  for( int i = 0; i < TimeLimit && samplelist.size(); i++ )
    {
      speed += samplelist[i].vector.getSpeed().getMps();
    }

  if( (speed / double(TimeLimit)) > SpeedLimit )
    {
      return true;
    }

  return false;
}

/**
 * Calculates the altitude gain. The variable m_minimumAltitude must be set
 * to a senseful value before, to enable the calculation.
 */
void Calculator::calcAltitudeGain()
{
  if( m_minimumAltitude.getMeters() == double(INT_MIN) )
    {
      // Gain calculation is not desired..
      return;
    }

  if( lastAltitude < m_minimumAltitude )
    {
      // We have sunken and store the new deep point.
      m_minimumAltitude = lastAltitude;
    }
  else if( lastAltitude > m_minimumAltitude )
    {
      // We are about the stored minimum altitude. That means we had a lift.
      Altitude lift = lastAltitude - m_minimumAltitude;

      if( lift > m_gainedAltitude )
        {
          // Store the new gained altitude value.
          m_gainedAltitude = lift;
          emit newGainedAltitude( m_gainedAltitude );
        }
    }
}

void Calculator::autoZoomInMap()
{
  // Auto zoom in to make the task point figure better visible, if:
  // -feature is enabled
  // -selected waypoint is not a null pointer
  // -there is no zoom already active

  // Stop reset zoom timer
  m_resetAutoZoomTimer->stop();

  if( GeneralConfig::instance()->getTaskPointAutoZoom() == true &&
      targetWp != 0 &&
      m_lastZoomFactor == -1 )
    {
      m_lastZoomFactor = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

      QPoint tpWgs(targetWp->wgsPoint.lat(), targetWp->wgsPoint.lon() );

      double newZoomFactor = _globalMapMatrix->ensureVisible( tpWgs, lastPosition );

      if( newZoomFactor > 0.0 && m_lastZoomFactor != newZoomFactor )
        {
          // Set map center to the current position
          _globalMapMatrix->centerToLatLon( lastPosition );

          // Zoom into the map
          emit switchMapScale(newZoomFactor);

          // Notice user about the zoom
          emit taskInfo( tr("TP zoom"), true );
        }
    }
}

/**
 * Gets the last Wind from the wind store. If no wind is available, the
 * returned wind vector is invalid.
 */
Vector& Calculator::getLastWind()
{
  if( GeneralConfig::instance()->isManualWindEnabled() )
    {
      // User has manual wind preselected.
      return m_lastWind.wind;
    }

  // Try to get a wind info from the wind store. If no wind is available,
  // the wind vector is invalid.
  Vector v = m_windStore->getWind( lastAltitude );
  setLastWind( v );

  return m_lastWind.wind;
}

bool Calculator::restoreWaypoint()
{
  Waypoint wp;

  // Filename of last saved target waypoint.
  QString fn = GeneralConfig::instance()->getUserDataDirectory() + "/target.wpt";

  if( wp.read( &wp, fn ) )
    {
      setTargetWp( &wp );
      return true;
    }

  return false;
}
