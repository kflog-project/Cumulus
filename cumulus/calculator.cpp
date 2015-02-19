/***********************************************************************
 **
 **   calculator.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2002      by André Somers
 **                  2008-2015 by Axel Pauli
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

#ifndef QT_5
#include <QtGui>
#else
#include <QtWidgets>
#endif

#include "altimeterdialog.h"
#include "calculator.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "reachablelist.h"
#include "tpinfowidget.h"
#include "whatsthat.h"
#include "windanalyser.h"

#define MAX_MCCREADY 10.0
#define MAX_SAMPLECOUNT 600

Calculator *calculator = static_cast<Calculator *> (0);

extern MainWindow  *_globalMainWindow;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

Calculator::Calculator(QObject* parent) :
  QObject(parent),
  samplelist( LimitedList<FlightSample>( MAX_SAMPLECOUNT ) )
{
  setObjectName( "Calculator" );
  GeneralConfig *conf = GeneralConfig::instance();

  manualAltitude.setMeters( conf->getManualNavModeAltitude() );

  lastAltitude     = manualAltitude;
  lastAGLAltitude  = manualAltitude;
  lastSTDAltitude  = manualAltitude;
  lastGNSSAltitude = manualAltitude;
  lastAHLAltitude  = manualAltitude;
  lastAGLAltitudeError.setMeters(0);

  lastPosition.setX( conf->getCenterLat() );
  lastPosition.setY( conf->getCenterLon() );

  lastSpeed.setMps(0);

  lastETA=QTime();
  lastBearing=-1;
  lastHeading=-1;
  lastDistance=-1;
  lastRequiredLD = -1.0;
  lastCurrentLD = -1.0;
  m_calculateLD = false;
  m_calculateETA = false;
  m_calculateVario = true;
  m_calculateTas = true;
  m_androidPressureAltitude = false;
  m_calculateWind = true;
  m_lastWind.wind = Vector(0.0, 0.0);
  m_lastWind.altitude = lastAltitude;
  targetWp = static_cast<Waypoint *> (0);
  lastMc = GeneralConfig::instance()->getMcCready();
  lastBestSpeed.setInvalid();
  lastTas = 0.0;
  m_polar = 0;
  m_vario = new Vario (this);
  m_windAnalyser = new WindAnalyser(this);
  m_reachablelist = new ReachableList(this);
  m_windStore = new WindStore(this);
  lastFlightMode=unknown;
  m_marker=0;
  m_glider=static_cast<Glider *> (0);
  m_pastFirstFix=false;
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
  connect (m_windAnalyser, SIGNAL(newMeasurement(const Vector&, int)),
           m_windStore, SLOT(slot_Measurement(const Vector&, int)));

  connect (m_windStore, SIGNAL(newWind(Vector&)),
           this, SLOT(slot_Wind(Vector&)));

  connect (this, SIGNAL(newAltitude(const Altitude&)),
           m_windStore, SLOT(slot_Altitude(const Altitude&)));
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

/** called on altitude change */
void Calculator::slot_Altitude(Altitude& user, Altitude& std, Altitude& gnns)
{
  lastAltitude         = user;
  lastSTDAltitude      = std;
  lastGNSSAltitude     = gnns;
  lastAGLAltitude      = lastAltitude - lastElevation;
  lastAGLAltitudeError = lastElevationError;

  lastAHLAltitude  = lastAltitude - GeneralConfig::instance()->getHomeElevation();
  emit newAltitude( lastAltitude );
  emit newUserAltitude( getAltimeterAltitude() );

  calcGlidePath();
  calcAltitudeGain();
}

/** Called if a new heading has been obtained */
void Calculator::slot_Heading( const double& newHeadingValue )
{
  if ( lastSpeed.getMps() <= 0.3 )
    {
      // @AP: don't forward values, when the speed is nearly
      // zero. Arrow will stay in last position. Same does make
      // Garmin-Pilot III
      return;
    }

  lastHeading = static_cast<int> (rint(newHeadingValue));

  emit newHeading(lastHeading);

  // if we have no bearing, lastBearing is -1;
  // this is only a small mistake, relBearing points to north
  //@JD: but wrong showing of new relative bearing icon, so bail out
  if( lastBearing < 0 )
    {
      return;
    }

  emit newRelBearing (lastBearing - lastHeading);
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
  calcBearing();
  calcETA();
  calcTas();
  calcGlidePath();
  // Calculate List of reachable items
  m_reachablelist->calculate(false);
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
      QMessageBox mb( QMessageBox::Question,
                      tr( "Replace current task point?" ),
                      tr( "<html>"
                          "A flight task is activated!<br>"
                          "This selection will stop the automatic task point switch."
                          "To avoid that make a selection from task menu."
                          "<br>Do You really want to replace?"
                          "</html>" ),
                      QMessageBox::Yes | QMessageBox::No,
                      QApplication::desktop() );

      mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

      mb.show();
      QPoint pos = QApplication::desktop()->mapToGlobal( QPoint( QApplication::desktop()->width()/2 - mb.width()/2,
                                                                 QApplication::desktop()->height()/2 - mb.height()/2 ) );
      mb.move( pos );

#endif

      if ( mb.exec() != QMessageBox::Yes )
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

  if ( newWp == 0 )
    {
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
      // this was not an automatic switch, it was made manually by the
      // user in the tasklistview or initiated by pressing accept in the
      // preflight dialog.
      m_taskEndReached = false;
      m_selectedWpInList = -1;

      FlightTask *task = _globalMapContents->getCurrentTask();

      if ( task != static_cast<FlightTask *> (0) )
        {
          QList<TaskPoint *> tpList = task->getTpList();

          // Tasks with less 4 entries are incomplete!
          for ( int i=0; i < tpList.count() && tpList.count() > 3; i++ )
            {
              if ( targetWp->wgsPoint == tpList.at(i)->getWGSPosition() &&
                   targetWp->taskPointIndex == tpList.at(i)->getFlightTaskListIndex() )
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

  calcBearing();
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

/** Calculates the distance to the currently selected waypoint and
    emits newDistance if the distance has been changed. If a flight
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

  Distance curDistance;

  curDistance.setKilometers( MapCalc::dist(double(lastPosition.x()),
                                           double(lastPosition.y()),
                             targetWp->wgsPoint.lat(),
                             targetWp->wgsPoint.lon()) );

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
  QList<TaskPoint *> tpList = task->getTpList();

  // Load active task switch scheme. Switch to next TP can be executed
  // by nearest to TP or touched TP sector/circle.
  enum GeneralConfig::ActiveTaskSwitchScheme ntScheme =
    GeneralConfig::instance()->getActiveTaskSwitchScheme();

  enum TaskPoint::PassageState passageState = TaskPoint::Outside;

  TaskPoint* tp = tpList.at( targetWp->taskPointIndex );

  passageState = tp->checkPassage( curDistance, lastPosition );

  if( passageState == TaskPoint::Near )
    {
      if( m_lastTpPassageState != passageState )
        {
          // Auto zoom in to make the task point figure better visible.
          autoZoomInMap();

          if( tp->getActiveTaskPointFigureScheme() == GeneralConfig::Line )
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
              tp->getActiveTaskPointFigureScheme() != GeneralConfig::Line )
            {
              // Announce task point touch only, if nearest switch scheme is
              // chosen by the user and no line figure is active to avoid to
              // much info for him.
              emit taskInfo( tr("TP in sight"), true );
            }

          // Send a signal to the IGC logger to increase logging interval
          emit taskpointSectorTouched();
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
      // b) the landing task point is selected
      // c) the end and landing task point are identically
      TaskPoint* tp = tpList.at( targetWp->taskPointIndex );

      bool tpEndEqLanding = false;
      int sidx            = targetWp->taskPointIndex;

      if( sidx > 1 && sidx < tpList.size() - 1 )
        {
          TaskPoint* tpNext = tpList.at( sidx + 1 );

          if( tp->getTaskPointType() == TaskPointTypes::End &&
              tpNext->getTaskPointType() == TaskPointTypes::Landing &&
              tp->getWGSPosition() == tpNext->getWGSPosition() )
            {
              // End and landing point are identically
              tpEndEqLanding = true;
            }
        }

      if ( m_taskEndReached == false &&
           ( tpEndEqLanding == true ||
             tp->getTaskPointType() == TaskPointTypes::Landing ) )
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
        }
      else if( m_taskEndReached == false )
        {
          if ( tpList.count() > m_selectedWpInList + 1 )
            {
              // this loop excludes the last WP
              TaskPoint *lastWp = tpList.at(m_selectedWpInList);
              m_selectedWpInList++;
              TaskPoint *nextWp = tpList.at(m_selectedWpInList);

              // calculate the distance to the next waypoint
              Distance dist2Next( MapCalc::dist( double(lastPosition.x()),
                                                 double(lastPosition.y()),
                                                 nextWp->getWGSPosition().lat(),
                                                 nextWp->getWGSPosition().lon() ) * 1000);
              curDistance = dist2Next;

              // announce task point change as none user interaction
              slot_WaypointChange( nextWp->getWaypointObject(), false );

              // Here we send a notice to the user about the task point switch.
              emit taskInfo( tr("TP passed"), true );

              if( GeneralConfig::instance()->getReportTpSwitch() == true )
                {
                  // Show a detailed switch info, if the user has configured that.
                  TPInfoWidget *tpInfo = new TPInfoWidget( _globalMainWindow );

                  tpInfo->prepareSwitchText( lastWp->getFlightTaskListIndex(),
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
  if ( targetWp == 0 ||
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

  if( ! wind.getSpeed().isValid() )
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
bool Calculator::glidePath(int aLastBearing, Distance aDistance,
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

  // we add wind because of the negative direction
  Vector airspeed = groundspeed + getLastWind();
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
void Calculator::calcBearing()
{
  int iresult = -1;
  int lH = -1;

  if( lastHeading == -1 )
    {
      lH = 0;
    }

  if( targetWp == 0 )
    {
      if (iresult != lastBearing)
        {
          lastBearing = iresult;

          emit newBearing(-1000);

          // if we have no waypoint, let the rel bearing arrow point to north
          emit newRelBearing(0);
        }
    }
  else
    {
      double result = MapCalc::getBearing( lastPosition, targetWp->wgsPoint );

      iresult = int (rint(result * 180./M_PI) );

      if( iresult != lastBearing )
        {
          lastBearing = iresult;
          emit newBearing(lastBearing);
          emit newRelBearing (lastBearing - lH);
        }
    }
}

/** Called if the position is changed manually. */
void Calculator::slot_changePosition(int direction)
{
  extern MapMatrix * _globalMapMatrix;

  double distance=_globalMapMatrix->getScale(MapMatrix::CurrentScale)*10;
  double kmPerDeg;

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

  lastAltitude    = manualAltitude;  // provide support to config altitude in settings
  lastAGLAltitude = lastAltitude - lastElevation;
  lastAHLAltitude = lastAltitude - GeneralConfig::instance()->getHomeElevation();
  lastSTDAltitude = manualAltitude;

  GeneralConfig *conf = GeneralConfig::instance();
  int qnhDiff = 1013 - conf->getQNH();

  if ( qnhDiff != 0 )
    {
      // Calculate altitude correction in meters from pressure difference.
      // The common approach is to expect a pressure difference of 1 hPa per
      // 30ft until 18.000ft. 30ft are 9.1437m
      int delta = (int) rint( qnhDiff * 9.1437 );

      // qDebug("Calculator::slot_changePosition(): QNH=%d, Delta=%d", conf->getQNH(), delta);

      lastSTDAltitude.setMeters(manualAltitude.getMeters() + delta );
    }

  emit newAltitude(lastAltitude);
  emit newUserAltitude( getAltimeterAltitude() );

  calcDistance();
  calcBearing();
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
  lastPosition = newPosition;
  slot_changePosition(MapMatrix::Position);
}


/** increment McCready value */
void Calculator::slot_McUp()
{
  if( m_glider && lastMc.getMps() <= MAX_MCCREADY - 0.5 )
    {
      lastMc.setMps( lastMc.getMps() + 0.5 );
      GeneralConfig::instance()->setMcCready(lastMc);
      calcGlidePath();
      emit newMc( lastMc );
    }
}

/** set McCready value */
void Calculator::slot_Mc(const Speed& mc)
{
  if( m_glider && lastMc.getMps() != mc.getMps() )
    {
      lastMc = mc;
      GeneralConfig::instance()->setMcCready(lastMc);
      calcGlidePath();
      emit newMc( mc );
    }
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
 * set water and bug values used by glider polare.
 */
void Calculator::slot_WaterAndBugs( const int water, const int bugs )
{
  if( m_glider )
    {
      if(m_glider->polar()->water() != water )
	{
	  m_glider->polar()->setWater( water );
	}

      if( m_glider->polar()->bugs() != bugs )
	{
	  m_glider->polar()->setBugs( bugs );
	}

      calcGlidePath();
    }
}

void Calculator::slot_GpsTas(const Speed& tas)
{
  // We get the TAS from another source. Therefore it must not be calculated by us.
  m_calculateTas = false;
  lastTas.setMps( tas.getMps() );
  emit newTas( lastTas );
}

/**
 * Variometer lift receiver and distributor to map display.
 */
void Calculator::slot_Variometer(const Speed& lift)
{
  if (lastVario != lift)
    {
      lastVario = lift;
      emit newVario (lift);
    }
}

/**
 * This slot triggers a variometer calculation based on Android pressure
 * altitude values.
 */
void Calculator::slot_AndroidAltitude(const Altitude& altitude)
{
  // qDebug() << "slot_AndroidAltitude: m_calculateVario="
  //         << m_calculateVario << altitude.getMeters();

  if( m_calculateVario == false )
    {
      // Variometer data from another external device are used.
      // Abort calculation.
      return;
    }

  m_androidPressureAltitude = true;
  m_vario->newPressureAltitude( altitude, lastTas );
  m_varioDataControl->start( 5000 );
}

/**
 * GPS variometer lift receiver. The internal variometer
 * calculation can be switched off, if we got values via this slot.
 */
void Calculator::slot_GpsVariometer(const Speed& lift)
{
  // Hey we got a variometer value directly from the GPS.
  // Therefore internal calculation is not needed and can be
  // switched off.
  m_calculateVario = false;

  if (lastVario != lift)
    {
      lastVario = lift;
      emit newVario (lift);
    }

  m_varioDataControl->start( 5000 );
}

/** Sets the current position to point newPos. */
void Calculator::setPosition(const QPoint& newPos)
{
  lastPosition = newPos;
  emit newPosition(lastPosition, Calculator::MAN);

  calcDistance();
  calcBearing();
  calcETA();
  calcGlidePath();
}

void Calculator::slot_varioDataControl()
{
  // Variometer control timer expired. Reset variometer status variables
  // to ensure normal calculation from GPS altitude.
  m_calculateVario = true;
  m_androidPressureAltitude = false;
}

/** Resets some internal items to the initial state */
void Calculator::slot_settingsChanged()
{
  // Send last known wind to MapView for update of speed. User maybe
  // changed the speed unit or has assigned its own wind.
  GeneralConfig* conf = GeneralConfig::instance();

  // Reset last wind information
  m_lastWind.wind = Vector(0.0, 0.0);
  m_lastWind.altitude = lastAltitude;

  slot_ManualWindChanged( conf->isManualWindEnabled() );

  // Switch on the internal variometer lift and wind calculation.
  // User could be changed the GPS device.
  m_calculateVario = true;
  m_calculateWind  = true;
  m_calculateTas   = true;

  m_androidPressureAltitude = false;

  slot_CheckHomeSiteSelection();
}

/** This slot is called by the NMEA interpreter if a new fix has been received.  */
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

  //qDebug("Speed in sample: %d kph", int(lastSpeed.getKph()));
  //qDebug("Direction in sample: %d degrees", int(lastHeading));
  Vector groundspeed (lastHeading, lastSpeed);
  // qDebug ("groundspeed: %d/%f", groundspeed.getAngleDeg(), groundspeed.getSpeed().getKph());
  // qDebug ("wind: %d/%f", lastWind.getAngleDeg(), lastWind.getSpeed().getKph());

  Vector& lastWind = getLastWind();
  Vector airspeed = groundspeed + lastWind;
  // qDebug ("airspeed: %d/%f", airspeed.getAngleDeg(), airspeed.getSpeed().getKph());
  if ( lastWind.getSpeed().getKph() != 0 )
    {
      sample.airspeed = airspeed.getSpeed();
    }

  // add to the samplelist
  samplelist.add(sample);
  lastSample = sample;

  // Call variometer calculation derived from GPS altitude. Can be switched off,
  // when an external device delivers variometer information derived from a
  // baro sensor.
  if ( m_calculateVario == true && m_androidPressureAltitude == false )
    {
      m_vario->newAltitude();
    }

  // Call wind analyzer calculation if required. Can be switched off,
  // when GPS delivers wind information.
  if ( m_calculateWind == true )
    {
      m_windAnalyser->slot_newSample();
    }

  // Calculate LD
  calcLD();

  // start analyzing...
  // determine if we are standing still, cruising, circling or doing something else
  determineFlightStatus();

  // let the world know we have added a new sample to our sample list
  emit newSample();
}


/** Determines the status of the flight: unknown, standstill, cruising, circlingL, circlingR */
void Calculator::determineFlightStatus()
{
  /*
    WARNING: THIS CODE IS HIGHLY EXPERIMENTAL AND UNTESTED IN FLIGHT!

    In principle, we are determining the flight status from the last 6
    seconds of flight. We may need to adjust this, as this is highly
    experimental code. However, if we already have a flight status, we
    only need to check if it has changed. If that is the case, we set
    the status to "Unknown" so the next fix will trigger a normal
    analysis again.

    Further optimizations could include using the airspeed to
    determine the flight mode. This however requires a good working
    wind estimation, which requires a good working flight status.... etc.
    Another approach would be to keep a counter that could allow for,
    say, one or two 'errors' in the data before a flight mode is
    changed to 'unknown'.
  */

  /*
    We define a maximum deviation of 15 degrees from the cruising course.
    This should be allowed for course corrections.
  */
#define MAXCRUISEANGDIFF 15 //see above

  /*
    We define a minimum turn rate of 4 degrees per second (that is,
    one turn takes at most one and a half minute).

    This can be a bit more advanced to allow for temporary changes in
    circling speed in order to better center a thermal.
  */
#define MINTURNANGDIFF 4 //see above

  /*
    We define a maximum altitude average drift to refine the detection of a
    standstill. Because GPS altitudes tend to drift, we can't rely
    on the exact same altitude over any length of time. We need to
    allow for some drift.
  */
#define MAXALTDRIFTAVG 2 //see above

  /*
    We define an analysis time frame. This time frame determines how
    far back we're going in history (in seconds) for our
    analysis. Note that the actual time difference between the first
    and the last sample used may differ from this time frame.
  */
#define TIMEFRAME 6

  if( samplelist.count() < TIMEFRAME )
    {
      // We need to have some samples in order to be able to analyze anything.
      return;
    }

  FlightMode flightMode = unknown;

  // get headings from the last two samples
  int lastHead = samplelist[0].vector.getAngleDeg();
  int prevHead = samplelist[1].vector.getAngleDeg();

  // get the time difference between these samples
  int timediff = samplelist[1].time.secsTo(samplelist[0].time);

  if (timediff == 0)
    {
      // If the time difference is 0, return (just to be sure). This will only cause problems...
      return;
    }

  // We are not doing a full analysis if we already have a flight mode.
  // It suffices to check some basic criteria.
  switch (lastFlightMode)
    {
    case standstill: // we are not moving at all!

      if ( (samplelist[0].position == samplelist[1].position ||
          ( MapCalc::dist(&samplelist[0].position, &samplelist[1].position) / double(timediff) ) < 0.005) &&
           lastSpeed.getMps() <= 0.5 )
        {
          // may be too ridged, GPS errors could cause problems here
          return;
        }
      else
        {
          flightMode = unknown;
        }
      break;

    case cruising: // we are flying from point A to point B
      if (abs(MapCalc::angleDiff(lastHead, m_cruiseDirection)) <=  MAXCRUISEANGDIFF &&
          samplelist[0].vector.getSpeed().getMps() > 0.5 )
        {
          return;
        }
      else
        {
          // We left the cruising corridor. A new analysis has to be started.
          flightMode = unknown;
          break;
        }

    case circlingR:
      //turning left means: the heading is decreasing
      if (MapCalc::angleDiff(prevHead, lastHead) > (-MINTURNANGDIFF * timediff))
        {
          flightMode = unknown;
          break;
        }
      else
        {
          return; //no change in flight mode
        }

    case circlingL:
      //turning right means: the heading is increasing
      if (MapCalc::angleDiff(prevHead, lastHead) < (MINTURNANGDIFF * timediff))
        {
          flightMode = unknown;
          break;
        }
      else
        {
          return; //no change in flight mode
        }

    default:
      flightMode = unknown;
      break;
    }

  if ( flightMode == unknown )
    {
      // qDebug() << "Flight mode unknown --> Start Analysis";

      // we need some real analysis
      QDateTime refTime = samplelist[0].time.addSecs(-TIMEFRAME);
      int samples = 1;

      while ((samplelist[samples].time > refTime) && ( samples < samplelist.count() - 1) )
        {
          samples++;
        }

      if( samples < 2 )
        {
          // At least we need 2 samples in our time window.
          return;
        }

      // samples now contains the index of the oldest sample we will use for this analysis.
      // Newer samples have lower indices!

      //initialize some values we will be needing...
      bool mayBeL = true;     //this may be a left turn (that is, no big dir change to the right)
      bool mayBeR = true;     //this may be a right turn (that is, no big dir change to the left)
      int totalDirChange = 0; //total heading change. If cruising, this will be low, if turning, it will be high
      double maxSpeed = 0.0;  //maximum speed obtained in this set of samples
      int angDiff = 0;        //difference in heading between two samples
      int totalAltChange = 0; //total change in altitude (absolute)
      int maxAltChange = 0;   //maximum change of altitude between samples.
      int altChange = 0;
      bool break_analysis = false; //flag to indicate we can stop further analysis.

      // loop through the samples to get some basic data we can use to distinguish flight modes
      for (int i = 0; i < samples; i++)
        {
          if( i < (samples - 1) )
            {
              // angDiff can be positive or negative according to the turn direction
              angDiff = (int) rint(MapCalc::angleDiff( samplelist[i].vector.getAngleDeg(), samplelist[i+1].vector.getAngleDeg() ));

              altChange = int( samplelist[i].altitude.getMeters() - samplelist[i+1].altitude.getMeters() );
              //qDebug("analysis: position=(%d, %d)", samplelist->at(i)->position.x(),samplelist->at(i)->position.y() );
            }
          else
            {
              angDiff = 0;
              altChange = 0;
            }

          // That is the average value of the direction change in degree.
          // Can be positive or negation.
          totalDirChange += angDiff;

          maxSpeed = qMax( maxSpeed, samplelist[i].vector.getSpeed().getMps() );
          totalAltChange += altChange;
          maxAltChange = qMax(abs(altChange), maxAltChange);

          if ( angDiff > MINTURNANGDIFF )
            {
              mayBeR = false;
            }
          else if ( angDiff < -MINTURNANGDIFF )
            {
              mayBeL = false;
            }

#if 0
          qDebug("#Analysis(%d): angle1=%d, angle2=%d, angDiff=%d, speed=%f, alt1=%f, alt2=%f, altDiff=%d, tac=%d, tdc=%d, Vmax=%f",
                 i,
                 samplelist[i].vector.getAngleDeg(),
                 (i < samples - 1) ? samplelist[i+1].vector.getAngleDeg() : 0,
                 angDiff,
                 samplelist[i].vector.getSpeed().getMps(),
                 samplelist[i].altitude.getMeters(),
                 (i < samples - 1) ? samplelist[i+1].altitude.getMeters() : 0,
                 altChange,
                 totalAltChange,
                 totalDirChange,
                 maxSpeed);
#endif
        }

      // try standstill. We are using a value > 0 because of possible GPS errors.
      /*
        The detection of stand stills may be extended further by checking if the altitude matches the terrain altitude. If not
        (or no where near), we can not assume a standstill. This is probably wave flying.
      */
      if ( maxSpeed < 0.5 ) // if we get under 0.5m/s for maximum speed, we may be standing still
        {
          // check if we had any significant altitude changes
          if ( (abs(totalAltChange) / (samples-1)) <= MAXALTDRIFTAVG )
            {
              flightMode = standstill;
              break_analysis = true;
              // qDebug() << "-->StandStill";
            }
        }

      if (!break_analysis)
        {
          // Get the time difference between the first and the last sample.
          // This might not be the 20 secs we were planning to use at all!
          timediff = samplelist[samples-1].time.secsTo(samplelist[0].time);

          // So, we are not standing still, nor are we cruising. Circling then maybe?
          if ( abs(totalDirChange) > (MINTURNANGDIFF * timediff) )
            {
              // if circling, we should have a minimum average turn rate of
              // MINTURNANGDIFF degrees per second
              if (mayBeL && !mayBeR)
                {
                  flightMode = circlingL;
                  break_analysis = true;
                  // qDebug() << "-->circlingL";
                }

              if (mayBeR && !mayBeL)
                {
                  flightMode = circlingR;
                  break_analysis = true;
                  // qDebug() << "-->circlingR";
                }

              // we have a new mode: we are circling! If neither of the above are true, we are doing something
              // funny, and the flight mode 'unknown' still applies, so we may also quit analyzing...
            }
        }

      if ( !break_analysis )
        {
          // If all other modes are not true we assume the cruise mode.
          flightMode = cruising;
          break_analysis = true;

          // save current heading for cruise check.
          m_cruiseDirection = samplelist[0].vector.getAngleDeg();
          // qDebug("-->Cruise direction: %d.", _cruiseDirection);
        }
    }

  if (flightMode != lastFlightMode)
    {
      lastFlightMode = flightMode;
      samplelist[0].marker = ++m_marker;
      newFlightMode( flightMode );
      // qDebug( "new FlightMode: %d", lastFlightMode );
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
    }
}

/** This function is used internally to emit the flight mode signal with the marker value */
void Calculator::newFlightMode(Calculator::FlightMode fm)
{
  if ( m_calculateWind )
    {
      // Wind calculation can be disabled when the Logger device
      // delivers already wind data.
      m_windAnalyser->slot_newFlightMode( fm );
    }

  emit flightModeChanged( fm );
}

/** Called if a new wind measurement is delivered by the GPS/Logger device */
void Calculator::slot_GpsWind( const Speed& speed, const short direction )
{
  // Hey we got a wind value directly from the GPS.
  // Therefore internal calculation is not needed and can be switched off.
  m_calculateWind = false;

  Vector v;
  v.setAngle( direction );
  v.setSpeed( speed );

  // Add new wind with the best quality 5 to the windStore.
  m_windStore->slot_Measurement( v, 5 );
}

/** Called if the wind measurement changes */
void Calculator::slot_Wind(Vector& v)
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
      WindMeasurementList& wml = getWindStore()->getWindMeasurementList();

      v = wml.getWind( lastAltitude, 1800 );

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

  if ( task == static_cast<FlightTask *> (0) )
    {
      // no task defined, ignore request
      return;
    }

  QList<TaskPoint *> tpList = task->getTpList();

  // Tasks with less 4 entries are incomplete! The selection
  // of the start point is also senseless, request is ignored.
  if( tpList.count() < 4 )
    {
      return;
    }

  TaskPoint *tp2Taken;

  // The start point of a task depends of the type. If first and
  // second point are identical, we take always the second one
  // otherwise the first.
  if( tpList.at(0)->getWGSPosition() != tpList.at(1)->getWGSPosition() )
    {
      // take first task point
      // tp2Taken = tpList.at(0);

      // 01.02.2013 AP: Always the begin point is selected as first point.
      // The former start point has only a comment function.
      tp2Taken = tpList.at(1);
    }
  else
    {
      // take second task point
      tp2Taken = tpList.at(1);
    }

  if( targetWp )
    {
      // Check, if another task point is already selected. In this case ask the
      // user if the first point shall be really selected.
      if( targetWp->taskPointIndex != -1 &&
          targetWp->taskPointIndex != tp2Taken->getFlightTaskListIndex() )
        {
          QMessageBox mb( QMessageBox::Question,
                          tr( "Restart current task?" ),
                          tr( "<html>"
                              "A flight task is running!<br>"
                              "This command will start the<br>"
                              "task again at the beginning."
                              "<br>Do You really want to restart?"
                              "</html>" ),
                          QMessageBox::Yes | QMessageBox::No,
                          QApplication::desktop() );

          mb.setDefaultButton( QMessageBox::Yes );

#ifdef ANDROID

          mb.show();
          QPoint pos = QApplication::desktop()->mapToGlobal( QPoint( QApplication::desktop()->width()/2 - mb.width()/2,
                                                                          QApplication::desktop()->height()/2 - mb.height()/2 ) );
          mb.move( pos );

#endif
          if ( mb.exec() == QMessageBox::No )
            {
              return;
            }
        }
    }

  slot_WaypointChange( tp2Taken->getWaypointObject(), true );
}

/**
 * @returns true if we move us over the user's defined speed limit.
 */
bool Calculator::moving()
{
  // set speed limit in m/s
  const double SpeedLimit = GeneralConfig::instance()->getAutoLoggerStartSpeed() * 1000.0 / 3600.0;
  const double TimeLimit = 5; // time limit in seconds

  if( samplelist.count() <= TimeLimit )
    {
      // We need to have some samples in order to be able to analyze speed.
      return false;
    }

  double speed = 0.0;

  for( int i = 0; i < TimeLimit; i++ )
    {
      // average speed about TimeLimit seconds
      speed += samplelist[i].vector.getSpeed().getMps();
    }

  if( (speed / TimeLimit) > SpeedLimit )
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
 * Gets the last Wind.
 */
Vector& Calculator::getLastWind()
{
  if( GeneralConfig::instance()->isManualWindEnabled() )
    {
      // User has manual wind preselected.
      return m_lastWind.wind;
    }

  if( fabs(m_lastWind.altitude.getMeters() - lastAltitude.getMeters()) < 200.0 )
    {
      // In a 200m band we assume that the wind changes are moderate only.
      return m_lastWind.wind;
    }

  // Try to get a wind info from the wind store.
  WindMeasurementList& wml = getWindStore()->getWindMeasurementList();

  Vector v = wml.getWind( lastAltitude, 1800, 400 );

  if( v.isValid() )
    {
      // Update last wind
      setLastWind(v);
      emit newWind(v); // forwards the wind info to the MapView
    }
  else
    {
      m_lastWind.altitude = lastAltitude;
    }

  return m_lastWind.wind;
}

bool Calculator::restoreWaypoint()
{
  qDebug() << "Calculator::restoreSavedWaypoint()";

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
