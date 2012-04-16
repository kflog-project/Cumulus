/***********************************************************************
 **F
 **   calculator.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2002      by André Somers
 **                  2008-2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 **   This class provides different calculator functionalities
 **
 ***********************************************************************/

#include <cmath>
#include <cstdlib>

#include <QtGui>

#include "generalconfig.h"
#include "calculator.h"
#include "mapcalc.h"
#include "gpsnmea.h"
#include "mapmatrix.h"
#include "windanalyser.h"
#include "reachablelist.h"
#include "altimetermodedialog.h"
#include "tpinfowidget.h"
#include "mainwindow.h"
#include "whatsthat.h"

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

  lastETA=QTime(0,0);
  lastBearing=-1;
  lastHeading=-1;
  lastDistance=-1;
  lastRequiredLD = -1.0;
  lastCurrentLD = -1.0;
  _calculateLD = false;
  _calculateETA = false;
  _calculateVario = true;
  _calculateWind = true;
  selectedWp = static_cast<Waypoint *> (0);
  lastMc.setInvalid();
  lastBestSpeed.setInvalid();
  lastTas = 0.0;
  _polar = 0;
  _vario = new Vario (this);
  _windAnalyser = new WindAnalyser(this);
  _reachablelist = new ReachableList(this);
  _windStore = new WindStore(this);
  lastFlightMode=unknown;
  _marker=0;
  _glider=static_cast<Glider *> (0);
  _pastFirstFix=false;
  selectedWpInList = -1;
  wpTouched = false;
  wpTouchCounter = 0;
  taskEndReached = false;
  manualInFlight = false;

  // hook up the internal backend components
  connect (_vario, SIGNAL(newVario(const Speed&)),
           this, SLOT(slot_Variometer(const Speed&)));

  // The former calls to slot_newSample and slot_newFlightMode of the wind
  // analyzer are replaced by direct calls, when a new sample or flight
  // mode is available and the wind calculation is enabled. Wind calculation
  // can be disabled when the Logger device delivers already wind data.
  connect (_windAnalyser, SIGNAL(newMeasurement(const Vector&, int)),
           _windStore, SLOT(slot_Measurement(const Vector&, int)));

  connect (_windStore, SIGNAL(newWind(Vector&)),
           this, SLOT(slot_Wind(Vector&)));

  connect (this, SIGNAL(newAltitude(const Altitude&)),
           _windStore, SLOT(slot_Altitude(const Altitude&)));
}

Calculator::~Calculator()
{
  if ( _glider )
    {
      delete _glider;
    }

  if ( selectedWp )
    {
      delete selectedWp;
    }

  // save last position as new center position of the map
  GeneralConfig::instance()->setCenterLat(lastPosition.x());
  GeneralConfig::instance()->setCenterLon(lastPosition.y());
}

/** Read property of Altitude for Altimeter display */
const Altitude& Calculator::getAltimeterAltitude()
{
  // qDebug("Calculator::getAltimeterAltitude(): %d",  _altimeter_mode );
  switch ( AltimeterModeDialog::mode() )
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
  // qDebug("slot_Altitude");
}

/** Called if a new heading has been obtained */
void Calculator::slot_Heading( const double& newHeadingValue )
{
  // qDebug("lastSpeed=%f m/s", lastSpeed.getMps());
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
  if (lastBearing < 0)
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

  if( ! manualInFlight )
    {
      lastPosition = lastGPSPosition;
    }

  lastElevation = Altitude( _globalMapContents->findElevation(lastPosition, &lastElevationError) );
  emit newPosition(lastGPSPosition, Calculator::GPS);
  calcDistance();
  calcBearing();
  calcETA();
  calcGlidePath();
  // Calculate List of reachable items
  _reachablelist->calculate(false);
}


/** Called if a new waypoint has been selected. If user action is
    true, change was done by an user interaction.*/
void Calculator::slot_WaypointChange(Waypoint *newWp, bool userAction)
{
  // qDebug( "Calculator::slot_WaypointChange(): NewWp=%x", newWp );

  if ( false ) // newWp
    {
      qDebug( "Calculator::slot_WaypointChange(): NewWpName=%s(%d)",
              newWp->name.toLatin1().data(), newWp->taskPointIndex );
    }

  if ( selectedWp && newWp &&
       selectedWp->taskPointIndex != -1 && newWp->taskPointIndex == -1 )
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
      newWp->projP = _globalMapMatrix->wgsToMap( newWp->origP );
    }

  // save new selected waypoint
  setSelectedWp( newWp );

  if ( newWp == 0 )
    {
      lastBestSpeed.setInvalid();
      emit bestSpeed( lastBestSpeed );

      // reset LD display
      emit newLD( lastRequiredLD=-1, lastCurrentLD=-1 );
      selectedWpInList = -1;
      wpTouched = false;
      wpTouchCounter = 0;
      taskEndReached = false;
      //@JD: reset bearing, and no more calculations
      lastBearing = -1;
      return;
    }

  if ( selectedWp && userAction && selectedWp->taskPointIndex != -1 )
    {
      // this was not an automatic switch, it was made manually by the
      // user in the tasklistview or initiated by pressing accept in the
      // preflight dialog.

      wpTouched = false;
      wpTouchCounter = 0;
      taskEndReached = false;
      selectedWpInList = -1;

      FlightTask *task = _globalMapContents->getCurrentTask();

      if ( task != static_cast<FlightTask *> (0) )
        {
          QList<TaskPoint *> tpList = task->getTpList();

          // Tasks with less 4 entries are incomplete!
          for ( int i=0; i < tpList.count() && tpList.count() > 3; i++ )
            {
              if ( selectedWp->origP == tpList.at(i)->origP &&
                   selectedWp->taskPointIndex == tpList.at(i)->taskPointIndex )
                {
                  selectedWpInList = i;
                  break;
                }
            }
        }
    }

  calcDistance( !userAction );
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
  // @AP: check, if waypoint to be deleted is selected. In this case a
  // deselection must be done
  if ( selectedWp &&
       selectedWp->origP == newWp->origP &&
       selectedWp->taskPointIndex == newWp->taskPointIndex )
    {
      wpTouched = false;
      wpTouchCounter = 0;
      taskEndReached = false;
      setSelectedWp(0);
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
  if ( ! selectedWp )
    {
      return;
    }

  Distance curDistance;

  curDistance.setKilometers(dist(double(lastPosition.x()), double(lastPosition.y()),
                                 selectedWp->origP.lat(), selectedWp->origP.lon()));

  if ( curDistance == lastDistance )
    {
      // no changes in the meantime
      return;
    }

  // get active task
  FlightTask *task = _globalMapContents->getCurrentTask();

  if ( ! task || selectedWp->taskPointIndex == -1 || ! autoWpSwitch )
    {
      // no task active,
      // no selected waypoint from a task
      // no automatic waypoint switch required
      // emit new distance only
      lastDistance = curDistance;
      emit newDistance(lastDistance);
      return;
    }

  // load waypoint list from task
  QList<TaskPoint *> tpList = task->getTpList();

  // Load active task switch scheme. Switch to next TP can be executed
  // by nearest to TP or touched TP sector/cylinder.
  enum GeneralConfig::ActiveNTTaskScheme ntScheme =
    GeneralConfig::instance()->getActiveNTTaskScheme();

  // If we are fast enough (speed > 35Km/h), we do check, if we could
  // inside of an selected task area. This condition is not
  // considered in manual mode to make testing possible.
  bool inside = false;

  if ( moving() || GpsNmea::gps->getGpsStatus() != GpsNmea::validFix )
    {
      inside = task->checkSector( curDistance, lastPosition, selectedWp->taskPointIndex );
    }

  // We set us a flag to remember, that we did arrive the radius of a
  // task point. Must be done because we can have only one touch.
  if ( inside && wpTouched == false && autoWpSwitch )
    {
      wpTouched = true;

      // send a signal to the IGC logger to increase logging interval
      emit taskpointSectorTouched();

      // Display a task end message under following conditions:
      // a) we touched the target radius
      // b) the last task point is selected
      if ( selectedWp->taskPointType == TaskPointTypes::Landing && taskEndReached == false )
        {
          taskEndReached = true;
          emit taskInfo( tr("Task target reached"), true );

          QString text = QString("<html>") +
                         "<table cellpadding=2 cellspacing=0>" +
                         "<tr><th>" +
                         tr("Task Target") +
                         "</th></tr>" +
                         "<tr><td>" +
                         selectedWp->name + " (" + selectedWp->description + ")" +
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
        }
      else if ( taskEndReached == false )
        {
          if ( ntScheme == GeneralConfig::Nearst )
            {
              // Announce task point touch only, if nearest switch scheme is
              // chosen by the user to avoid to much info for him.
              emit taskInfo( tr("Taskpoint area reached"), true );
            }
          else
            {
              // Set touch counter in case of touch switch scheme is used,
              // to ensure that we were really inside of the task point
              // sector/cylinder
              wpTouchCounter = 5; // set touch counter to 5 events, ca. 5s
            }
        }

      lastDistance = curDistance;
      emit newDistance(lastDistance);
      return;
    }

  // We arrived the task point switch radius and after that event the
  // wpTouchedCounter reaches zero or the nearest position to the TP is
  // arrived. In this case we have to execute and announce the task
  // point switch.
  if ( ( curDistance.getMeters() > lastDistance.getMeters() ||
       ( wpTouchCounter > 0 && --wpTouchCounter == 0 ) ) &&
         wpTouched == true )
    {
      wpTouched      = false;
      wpTouchCounter = 0;

      if ( tpList.count() > selectedWpInList + 1 )
        {
          // this loop excludes the last WP
          Waypoint *lastWp = tpList.at(selectedWpInList);
          selectedWpInList++;
          Waypoint *nextWp = tpList.at(selectedWpInList);

          // calculate distance to new waypoint
          Distance dist2Next( dist(double(lastPosition.x()), double(lastPosition.y()),
                                   nextWp->origP.lat(), nextWp->origP.lon()) * 1000);
          lastDistance = dist2Next;

          // announce task point change as none auto switch
          slot_WaypointChange( nextWp, false );

          // Here we send a notice to the user about the task point
          // switch. If end point is reached and landing point is identical
          // to end point, we will suppress the info message
          if ( ! ( lastWp->taskPointType == TaskPointTypes::End &&
                   nextWp->taskPointType == TaskPointTypes::Landing &&
                   lastWp->origP == nextWp->origP ) )
            {

              emit taskInfo( tr("Automatic taskpoint switch"), true );

              TPInfoWidget *tpInfo = new TPInfoWidget( _globalMainWindow );
              tpInfo->prepareSwitchText( lastWp->taskPointIndex, dist2Next.getKilometers() );

              // switch back to map view on close of tp info widget
              connect( tpInfo, SIGNAL( closed() ),
                       _globalMainWindow, SLOT( slotSwitchToMapView() ) );

              // switch off all set accelerators
              _globalMainWindow->setView( MainWindow::tpSwitchView );
              tpInfo->showTP();
            }
        }
    }
  else
    {
      lastDistance = curDistance;
    }

  emit newDistance(lastDistance);
}


/** Calculates the ETA (Estimated Time to Arrival) to the current
    waypoint and emits a signal newETA if the value has changed. */
void Calculator::calcETA()
{
  QTime etaNew(0, 0);

  // qDebug("lastSpeed=%f m/s", lastSpeed.getMps());

  if ( ! _calculateETA || ! selectedWp ||
       lastSpeed.getMps() <= 0.3 || ! GpsNmea::gps->getConnected() )
    {
      if ( ! lastETA.isNull() )
        {
          emit newETA(etaNew);
          lastETA = etaNew;
        }

      return;
    }

  int eta = (int) rint(lastDistance.getMeters() / lastSpeed.getMps());

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
          QTime zero(0, 0);
          emit newETA(zero);
        }
      else
        {
          emit newETA(lastETA);
        }
    }

  // qDebug("New ETA=%s", etaNew.toString().toLatin1().data());
}


/** Calculates the required LD and the current LD about the last 60s,
    if required */
void Calculator::calcLD()
{
  if ( ! selectedWp || _calculateLD == false || samplelist.count() < 2 )
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

  // first calculate current LD
  for ( int i = 1; i < samplelist.count(); i++ )
    {

      timeDiff = (samplelist.at(i).time).msecsTo(end->time);

      if ( timeDiff >= 60*1000 )
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

  // calculate required LD, we consider elevation and security
  // altitude

  GeneralConfig *conf = GeneralConfig::instance();

  double altDiff = lastAltitude.getMeters() - selectedWp->elevation -
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

  if (!_polar)
    {
      arrivalAlt.setInvalid();
      bestSpeed.setInvalid();
      return false;
    }

//  qDebug("Glider=%s", _glider->type().toLatin1().data());

  // we use the method described by Bob Hansen
  // get best speed for zero wind V0
  Speed speed = _polar->bestSpeed(0.0, 0.0, lastMc);
  //qDebug ("rough best speed: %f", speed.getKph());

  // wind has a negative vector!
  //qDebug ("wind: %d/%f", lastWind.getAngleDeg(), lastWind.getSpeed().getKph());

  // assume we are heading for the wp
  Vector groundspeed (aLastBearing, speed);
  //qDebug ("groundspeed: %d/%f", groundspeed.getAngleDeg(), groundspeed.getSpeed().getKph());

  // we add wind because of the negative direction
  Vector airspeed = groundspeed + lastWind;
  //qDebug ("airspeed: %d/%f", airspeed.getAngleDeg(), airspeed.getSpeed().getKph());

  // this is the first iteration of the Bob Hansen method
  Speed headwind = groundspeed.getSpeed() - airspeed.getSpeed() ;
  //qDebug ("headwind: %f", headwind.getKph());

  Altitude minimalArrival( GeneralConfig::instance()->getSafetyAltitude().getMeters() );
  Altitude givenAlt (lastAltitude - Altitude (aElevation) - minimalArrival);

  // improved speed for wind V1
  speed = _polar->bestSpeed(headwind, 0.0, lastMc);
  //qDebug ("improved best speed: %f", speed.getKph());
  bestSpeed = speed;
  // the ld is over ground, so we take groundspeed
  double ld =_polar->bestLD(speed, groundspeed.getSpeed(), 0.0);

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
  if ( ! selectedWp || ! _glider )
    {
      lastRequiredLD = -1;
      return;
    }

  int tpIdx = selectedWp->taskPointIndex;
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
      glidePath( lastBearing, lastDistance, selectedWp->elevation, arrivalAlt, speed );
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

/** Calculates the bearing to the currently selected waypoint, and emits signal newBearing if the bearing has changed. */
void Calculator::calcBearing()
{
  int iresult;
  int lH=lastHeading;
  if (lH == -1)
    lH=0;

  if (selectedWp==0)
    {
      iresult=-1;
      if (iresult!=lastBearing)
        {
          lastBearing=iresult;
          emit newBearing(-1000);
          // if we have no waypoint, let the rel bearing arrow point to north
          emit newRelBearing (-lH);
        }
    }
  else
    {
      double result = getBearing(lastPosition, selectedWp->origP);
      iresult = int (rint(result * 180./M_PI) );
      if (iresult!=lastBearing)
        {
          lastBearing=iresult;
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
      kmPerDeg = dist(lastPosition.x(), lastPosition.y(), lastPosition.x()+600000, lastPosition.y());
      lastPosition = QPoint((int) (lastPosition.x()+(distance/kmPerDeg) * 600), lastPosition.y());
      break;
    case MapMatrix::West:
      kmPerDeg = dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
      lastPosition = QPoint(lastPosition.x(), (int) (lastPosition.y()-(distance/kmPerDeg) * 600));
      break;
    case MapMatrix::East:
      kmPerDeg = dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
      lastPosition = QPoint(lastPosition.x(), (int) (lastPosition.y()+(distance/kmPerDeg) * 600));
      break;
    case MapMatrix::South:
      kmPerDeg = dist(lastPosition.x(), lastPosition.y(), lastPosition.x()+600000, lastPosition.y());
      lastPosition = QPoint((int) (lastPosition.x()-(distance/kmPerDeg) * 600), lastPosition.y());
      break;
    case MapMatrix::Home:
      lastPosition = QPoint(_globalMapMatrix->getHomeCoord());
      break;
    case MapMatrix::Waypoint:
      if (selectedWp)
        {
          lastPosition = selectedWp->origP;
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
  _reachablelist->calculate(true);
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
  if( _glider && lastMc.getMps() <= MAX_MCCREADY - 0.5 )
    {
      lastMc.setMps( lastMc.getMps() + 0.5 );
      calcGlidePath();
      emit newMc( lastMc );
    }
}

/** set McCready value */
void Calculator::slot_Mc(const Speed& mc)
{
  if( _glider && lastMc.getMps() != mc.getMps() )
    {
      lastMc = mc;
      calcGlidePath();
      emit newMc( mc );
    }
}

/** decrement McCready value; don't get negative */
void Calculator::slot_McDown()
{
  if( _glider && lastMc.getMps() >= 0.5 )
    {
      lastMc.setMps( lastMc.getMps() - 0.5 );
      calcGlidePath();
      emit newMc( lastMc );
    }
}

/**
 * set water and bug values used by glider polare.
 */
void Calculator::slot_WaterAndBugs( const int water, const int bugs )
{
  if( _glider &&
      (_glider->polar()->water() != water ||
       _glider->polar()->bugs() != bugs) )
    {
      _glider->polar()->setWater( water, bugs );
      calcGlidePath();
    }
}

/**
 * set TAS value
 */
void Calculator::slot_Tas(const Speed& spd)
{
  lastTas.setMps(spd.getMps());
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
 * GPS variometer lift receiver. The internal variometer
 * calculation can be switched off, if we got values vis this slot.
 */
void Calculator::slot_GpsVariometer(const Speed& lift)
{
  // Hey we got a variometer value directly from the GPS.
  // Therefore internal calculation is not needed and can be
  // switched off.
  _calculateVario = false;

  if (lastVario != lift)
    {
      lastVario = lift;
      emit newVario (lift);
    }
}


/** Sets the current position to point newPos. */
void Calculator::setPosition(const QPoint& newPos)
{
  lastPosition=newPos;
  emit newPosition(lastPosition, Calculator::MAN);

  calcDistance();
  calcBearing();
  calcETA();
  calcGlidePath();
}


/** Resets some internal items to the initial state */
void Calculator::slot_settingsChanged ()
{
  // Send last known wind to mapview for update of speed. User maybe
  // changed the speed unit.
  if ( lastWind.getSpeed().getMps() != 0 )
    {
      emit( newWind(lastWind) );
    }

  // Switch on the internal variometer lift and wind calculation.
  // User could be changed the GPS device.
  _calculateVario = true;
  _calculateWind  = true;

  slot_CheckHomeSiteSelection();
}

/** This slot is called by the NMEA interpreter if a new fix has been received.  */
void Calculator::slot_newFix( const QTime& newFixTime )
{
  // before we start making samples, let's be sure we have all the
  // data we need for that. So, we wait for the second Fix.
  if (!_pastFirstFix)
    {
      _pastFirstFix = true;
      return;
    }

  // create a new sample structure
  FlightSample sample;

  // fill it with the relevant data
  sample.time = newFixTime;
  sample.altitude.setMeters(lastAltitude.getMeters());
  sample.GNSSAltitude.setMeters(lastGNSSAltitude.getMeters());
  sample.position=lastPosition;
  sample.vector.setAngleAndSpeed(lastHeading, lastSpeed);

  //qDebug("Speed in sample: %d kph", int(lastSpeed.getKph()));
  //qDebug("Direction in sample: %d degrees", int(lastHeading));
  Vector groundspeed (lastHeading, lastSpeed);
  // qDebug ("groundspeed: %d/%f", groundspeed.getAngleDeg(), groundspeed.getSpeed().getKph());
  // qDebug ("wind: %d/%f", lastWind.getAngleDeg(), lastWind.getSpeed().getKph());
  Vector airspeed = groundspeed + lastWind;
  // qDebug ("airspeed: %d/%f", airspeed.getAngleDeg(), airspeed.getSpeed().getKph());
  if ( lastWind.getSpeed().getKph() != 0 )
    {
      sample.airspeed = airspeed.getSpeed();
    }

  // add to the samplelist
  samplelist.add(sample);
  lastSample = sample;

  // Call variometer calculation, if required. Can be switched off,
  // when GPS delivers variometer information.
  if ( _calculateVario == true )
    {
      _vario->newAltitude();
    }

  // Call wind analyzer calculation if required. Can be switched off,
  // when GPS delivers wind information.
  if ( _calculateWind == true )
    {
      _windAnalyser->slot_newSample();
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
          ( dist(&samplelist[0].position, &samplelist[1].position) / double(timediff) ) < 0.005) &&
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
      if (abs(angleDiff(lastHead, _cruiseDirection)) <=  MAXCRUISEANGDIFF &&
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
      if (angleDiff(prevHead, lastHead) > (-MINTURNANGDIFF * timediff))
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
      if (angleDiff(prevHead, lastHead) < (MINTURNANGDIFF * timediff))
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
      QTime refTime = samplelist[0].time.addSecs(-TIMEFRAME);
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
              angDiff = (int) rint(angleDiff( samplelist[i].vector.getAngleDeg(), samplelist[i+1].vector.getAngleDeg() ));

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
          _cruiseDirection = samplelist[0].vector.getAngleDeg();
          // qDebug("-->Cruise direction: %d.", _cruiseDirection);
        }
    }

  if (flightMode != lastFlightMode)
    {
      lastFlightMode = flightMode;
      samplelist[0].marker = ++_marker;
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
      samplelist[0].marker = ++_marker;
      // qDebug("new FlightMode: %d",lastFlightMode);
      newFlightMode(flightMode);
    }

  if ( newState == GpsNmea::noFix )
    {
      // Reset LD display
      emit newLD( -1.0, -1.0 );
      // reset first fix passed
      _pastFirstFix = false;
    }
}

/** This function is used internally to emit the flight mode signal with the marker value */
void Calculator::newFlightMode(Calculator::FlightMode fm)
{
  if ( _calculateWind )
    {
      // Wind calculation can be disabled when the Logger device
      // delivers already wind data.
      _windAnalyser->slot_newFlightMode( fm );
    }

  emit flightModeChanged( fm );
}

/** Called if a new wind measurement is delivered by the GPS/Logger device */
void Calculator::slot_GpsWind( const Speed& speed, const short direction )
{
  // Hey we got a wind value directly from the GPS.
  // Therefore internal calculation is not needed and can be
  // switched off.
  _calculateWind = false;

  Vector v;
  v.setAngle( direction );
  v.setSpeed( speed );

  lastWind = v;
  emit newWind(v); // forwards wind info to map
}

/** Called if the wind measurement changes */
void Calculator::slot_Wind(Vector& v)
{
  lastWind = v;
  emit newWind(v); // forwards wind info to map
}

/** Store the property of a new Glider. */
void Calculator::setGlider(Glider* glider)
{
  if (_glider)
    {
      delete _glider;

      // reset glider and polar object to avoid senseless calculations
      _glider = 0;
      _polar  = 0;
    }

  if (glider)
    {
      _glider = glider;
      _polar = _glider->polar();
      lastMc.setKph(0);
      calcETA();
      calcGlidePath();
      emit newGlider( _glider->type() );
    }
  else
    {
      lastMc.setInvalid();
      lastBestSpeed.setInvalid();
      emit bestSpeed( lastBestSpeed );
      emit newGlider( "" );
    }

  // Switches on/off Mc in map view
  emit newMc( lastMc );

  // Recalculate the nearest reachable sites because
  // glider has been modified resp. removed.
  _reachablelist->calculateNewList();
}

bool Calculator::matchesFlightMode(GeneralConfig::UseInMode mode)
{
  if (mode==GeneralConfig::always)
    return true;
  if (mode==GeneralConfig::never)
    return false;
  if (mode==GeneralConfig::standstill)
    return lastFlightMode == standstill;
  if (mode==GeneralConfig::cruising)
    return lastFlightMode == cruising;
  if (mode==GeneralConfig::circling)
    return (lastFlightMode == circlingL || lastFlightMode == circlingR);

  return false;
}

/**
 * Sets a new selected waypoint. The old waypoint instance is
 * deleted and a new one allocated.
 */
void Calculator::setSelectedWp( const Waypoint* newWp )
{
  // delete old waypoint selection
  if ( selectedWp != 0 )
    {
      delete selectedWp;
      selectedWp = 0;
    }

  // make a deep copy of new waypoint to be set
  if ( newWp != 0 )
    {
      selectedWp = new Waypoint( *newWp );
    }

  // inform mapView about the change
  emit newWaypoint(newWp);
}

void Calculator::setManualInFlight(bool switchOn)
{
  manualInFlight = switchOn;
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

  if( selectedWp && selectedWp->name == tr("Home") &&
      selectedWp->origP != conf->getHomeCoord() )
    {
      Waypoint wp;

      wp.name = tr("Home");
      wp.description = tr("Home Site");
      wp.origP.setPos( conf->getHomeCoord() );

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
    if( tpList.at(0)->origP != tpList.at(1)->origP )
      {
        // take first task point
        tp2Taken = tpList.at(0);
      }
    else
      {
        // take second task point
        tp2Taken = tpList.at(1);
      }

    if( selectedWp )
      {
        // Check, if the task point is already selected. In this case this
        // request is ignored.
        if( selectedWp->taskPointIndex == tp2Taken->taskPointIndex )
          {
            return;
          }

        // Check, if another task point is already selected. In this case ask the
        // user if the first point shall be really selected.
        if( selectedWp->taskPointIndex != -1 &&
            selectedWp->taskPointIndex != tp2Taken->taskPointIndex )
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

    slot_WaypointChange( tp2Taken, true );
}

/**
 * @returns true if we are faster in move as or equal 35km/h.
 */
bool Calculator::moving()
{
  const double Limit = 35000.0 / 3600.0; // 35Km/h as m/s

  if( samplelist.count() < 5 )
    {
      // We need to have some samples in order to be able to analyze speed.
      return false;
    }

  double speed = 0.0;

  for( int i = 0; i < 5; i++ )
    {
      // average speed about 5s
      speed += samplelist[i].vector.getSpeed().getMps();
    }

  if( (speed / 5.0) >= Limit )
    {
      return true;
    }

  return false;
}
