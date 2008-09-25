/***********************************************************************
 **
 **   cucalc.cpp
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
 **   This class provides different calculator functionalities
 **
 ***********************************************************************/

#include <stdlib.h>
#include <cmath>
#include <QMessageBox>
#include <QtGlobal>

#include "generalconfig.h"
#include "cucalc.h"
#include "mapcalc.h"
#include "gpsnmea.h"
#include "mapmatrix.h"
#include "windanalyser.h"
#include "reachablelist.h"
#include "altimetermodedialog.h"
#include "tpinfowidget.h"
#include "cumulusapp.h"
#include "whatsthat.h"

#define MAX_MCCREADY 10.0
#define MAX_SAMPLECOUNT 600

CuCalc *calculator = (CuCalc *) 0;
extern CumulusApp  *_globalCumulusApp;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;

CuCalc::CuCalc(QObject* parent) : QObject(parent),
                                  samplelist( LimitedList<flightSample>( MAX_SAMPLECOUNT ) )
{
  GeneralConfig *conf = GeneralConfig::instance();

  manualAltitude.setMeters( conf->getManualNavModeAltitude() );
  lastAltitude = manualAltitude;  // provide support to config altitude in settings
  lastAGLAltitude = manualAltitude;
  lastSTDAltitude = manualAltitude;
  lastGNSSAltitude = manualAltitude;
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
  selectedWp=(wayPoint *) 0;
  lastMc = 0.0;
  _polar = 0;
  _vario = new Vario (this);
  _windAnalyser = new WindAnalyser(this);
  _reachablelist = new ReachableList(this);
  _windStore = new WindStore(this);
  lastFlightMode=unknown;
  _marker=0;
  _glider=0;
  _pastFirstFix=false;
  _altimeter_mode = AltimeterModeDialog::mode();
  selectedWpInList = -1;
  wpTouched = false;
  wpTouchCounter = 0;
  taskEndReached = false;
  manualInFlight = false;

  // hook up the backend components
  connect (_vario, SIGNAL(newVario(const Speed&)),
           this,SLOT(slot_Variometer(const Speed&)));
  connect (this, SIGNAL(newSample()),
           _windAnalyser, SLOT(slot_newSample()));
  connect (this, SIGNAL(flightModeChanged(CuCalc::flightmode, int)),
           _windAnalyser, SLOT(slot_newFlightMode(CuCalc::flightmode, int)));
  connect (_windAnalyser, SIGNAL(newMeasurement(Vector, int)),
           _windStore, SLOT(slot_measurement(Vector, int)));
  connect (_windStore, SIGNAL(newWind(Vector&)),
           this, SLOT(slot_Wind(Vector&)));
  connect (this, SIGNAL(newAltitude(const Altitude&)),
           _windStore, SLOT(slot_Altitude()));

  // make internal connection so the flightModeChanged signal is
  // re-emitted with the marker value
  connect (this, SIGNAL(flightModeChanged(CuCalc::flightmode)),
           this, SLOT(slot_flightModeChanged(CuCalc::flightmode)));
}


CuCalc::~CuCalc()
{
  delete _glider;

  if( selectedWp != 0 ) {
    delete selectedWp;
  }

  // save last position as new center position of the map
  GeneralConfig::instance()->setCenterLat(lastPosition.x());
  GeneralConfig::instance()->setCenterLon(lastPosition.y());
}


/** Read property of Altitude lastAltitude. */
const Altitude& CuCalc::getlastGNDAltitude()
{
  if ( selectedWp ) {
    lastGNDAltitude = lastAltitude - selectedWp->elevation;
    return lastGNDAltitude;
  } else {
    lastGNDAltitude = lastAltitude;
    return lastGNDAltitude;
  }
}


/** Read property of Altitude for Altimeter display */
const Altitude& CuCalc::getAltimeterAltitude()
{
  // qDebug("CuCalc::getAltimeterAltitude(): %d",  _altimeter_mode );
  switch( _altimeter_mode ) {
  case 0:
    return lastAltitude; // MSL
    break;
  case 1:
    return lastAGLAltitude; // GND
    break;
  case 2:
  default:
    return lastSTDAltitude; // STD
    break;
  }
}

const QString CuCalc::getAltimeterAltitudeText()
{
  Altitude alti = getAltimeterAltitude();
  // to be implemented: if heigh>transition heigh switch to FL
  //double d = alti.getFL();
  return alti.getText (false,0);
}


const AltitudeCollection& CuCalc::getAltitudeCollection()
{
  lastAltCollection.gndAltitude=lastAGLAltitude;
  lastAltCollection.stdAltitude=lastSTDAltitude;
  lastAltCollection.gpsAltitude=lastAltitude;
  lastAltCollection.pressureAltitude=lastAltitude;
  lastAltCollection.gpsAltitudeError=0;
  lastAltCollection.gndAltitudeError=lastAGLAltitudeError;

  return lastAltCollection;
}


/** Read property of glider polar  */
Polar *CuCalc::getPolar()
{
  return _polar;
}


/** Read property of Speed lastSpeed. */
Speed& CuCalc::getLastSpeed()
{
  return lastSpeed;
}


/** Read property of Speed lastBestSpeed. */
Speed& CuCalc::getlastBestSpeed()
{
  return lastBestSpeed;
}


/** Read property of last glide path */
const Altitude& CuCalc::getlastGlidePath()
{
  return lastGlidePath;
}


/** Read property of McCready setting */
const Speed& CuCalc::getlastMc()
{
  return lastMc;
}


/** Read property of QPoint lastPosition. */
const QPoint& CuCalc::getlastPosition()
{
  return lastPosition;
}


/** Read property of int lastHeading. */
int CuCalc::getlastHeading()
{
  return lastHeading;
}


/** Get required LD to target */
const double CuCalc::getLastRequiredLD()
{
  if ( selectedWp ) {
    return lastRequiredLD;
  } else {
    return -1.0;
  }
}


/** Get current LD */
const double CuCalc::getLastCurrentLD()
{
  return lastCurrentLD;
}


/** called on altitude change */
void CuCalc::slot_Altitude()
{
  lastAltitude=gps->getLastAltitude();
  lastSTDAltitude=gps->getLastStdAltitude();
  lastGNSSAltitude=gps->getLastGNSSAltitude();
  lastAGLAltitude = lastAltitude - Altitude( _globalMapContents->findElevation(lastPosition, &lastAGLAltitudeError) );
  emit newAltitude(lastAltitude);
  calcGlidePath();
  // qDebug("slot_Altitude");
}


/** Called if a new heading has been obtained */
void CuCalc::slot_Heading()
{
  // qDebug("lastSpeed=%f m/s", lastSpeed.getMps());
  if( lastSpeed.getMps() <= 0.3 ) {
    // @AP: don't forward values, when the speed is nearly
    // zero. Arrow will stay in last position. Same does make
    // Garmin-Pilot III
    return;
  }
  lastHeading = (int)rint(gps->getLastHeading());
  emit newHeading(lastHeading);
  // if we have no bearing, lastBearing is -1;
  // this is only a small mistake, relBearing points to north
  //@JD: but wrong showing of new rel.-bearing icon, so bail out
  if (lastBearing < 0)
    {
      return;
    }

  emit newRelBearing (lastBearing - lastHeading);
}


/** called if a new speed fix has been received */
void CuCalc::slot_Speed()
{
  lastSpeed=gps->getLastSpeed();
  emit newSpeed(lastSpeed);
}


/** called if a new position-fix has been established. */
void CuCalc::slot_Position()
{
  lastGPSPosition=gps->getLastCoord();
  if(!manualInFlight) lastPosition = lastGPSPosition;
  lastElevation = Altitude( _globalMapContents->findElevation(lastPosition, &lastElevationError) );
  emit newPosition(lastGPSPosition, CuCalc::GPS);
  calcDistance();
  calcBearing();
  calcETA();
  calcGlidePath();
  // Calculate List of reachable items
  _reachablelist->calculate(false);
}


/** Called if a new waypoint has been selected. If user action is
    true, change was done by an user interaction.*/
void CuCalc::slot_WaypointChange(wayPoint *newWp, bool userAction)
{
  // qDebug( "CuCalc::slot_WaypointChange(): NewWp=%x", newWp );

  if( false ) // newWp
    {
      qDebug( "CuCalc::slot_WaypointChange(): NewWpName=%s(%d)",
              newWp->name.toLatin1().data(), newWp->taskPointIndex );
    }

  if( selectedWp && newWp &&
      selectedWp->taskPointIndex != -1 && newWp->taskPointIndex == -1 ) {
    // A user action will overwrite a task point. That will stop the
    // automatic taskpoint switch. We will notice the user about
    // that fact.

    int answer=
      QMessageBox::warning( 0, tr("Replace current taskpoint?"),
                            tr("<html>"
                               "A flight task is activated!<br>"
                               "This selection will stop the automatic taskpoint switch."
                               "To avoid that make a selection from task menu."
                               "<br>Do You really want to replace?"
                               "</html>"),
                            QMessageBox::Ok | QMessageBox::Default,
                            QMessageBox::Cancel | QMessageBox::Escape );

    if( answer != QMessageBox::Ok )
      {
        // do nothing change
        return;
      }
  }

  // Map new waypoint instance to current projection to be sure, that
  // all is correct before distribution.
  if( newWp ) {
    newWp->projP = _globalMapMatrix->wgsToMap( newWp->origP );
  }

  // save new selected waypoint
  setSelectedWp( newWp );

  // inform mapView about the change
  emit newWaypoint(newWp);

  if ( newWp == 0 ) {
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

  if( selectedWp && userAction && selectedWp->taskPointIndex != -1 ) {
    // this was not an automatic switch, it was made manually by the
    // user in the tasklistview or initiated by pressing accept in the
    // preflight dialog.

    wpTouched = false;
    wpTouchCounter = 0;
    taskEndReached = false;
    selectedWpInList = -1;

    FlightTask *task = _globalMapContents->getCurrentTask();

    if( task != 0 ) {
      QList<wayPoint*> wpList = task->getWPList();

      // Tasks with less 4 entries are incomplete! The selection
      // of the start point is also senseless. Therefore we start with one.
      for( int i=1; i < wpList.count() && wpList.count() > 3; i++ ) {
        if( selectedWp->origP == wpList.at(i)->origP &&
            selectedWp->taskPointIndex == wpList.at(i)->taskPointIndex ) {
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
void CuCalc::slot_WaypointDelete(wayPoint * newWp)
{
  // @AP: check, if waypoint to be deleted is selected. In this case a
  // deselection must be done
  if( selectedWp && selectedWp->origP == newWp->origP ) {
    wpTouched = false;
    wpTouchCounter = 0;
    taskEndReached = false;
    setSelectedWp(0);
    calcDistance();
    calcBearing();
    calcETA();
    calcGlidePath();
    emit newLD( lastRequiredLD=-1, lastCurrentLD=-1 );
  }
}


/** Calculates the distance to the currently selected waypoint and
    emits newDistance if the distance has been changed. If a flight
    task has been activated, the automatic switch from one task point
    to the next is also controlled and handled here.
*/
void CuCalc::calcDistance( bool autoWpSwitch )
{
  if( ! selectedWp ) {
    return;
  }

  Distance curDistance;

  curDistance.setKilometers(dist(double(lastPosition.x()), double(lastPosition.y()),
                                 selectedWp->origP.lat(), selectedWp->origP.lon()));

  if( curDistance == lastDistance ) {
    // no changes in the meantime
    return;
  }

  // get active task
  FlightTask *task = _globalMapContents->getCurrentTask();

  if( ! task || selectedWp->taskPointIndex == -1 || ! autoWpSwitch ) {
    // no task active,
    // no selected waypoint from a task
    // no automatic waypoint switch required
    // emit new distance only
    lastDistance = curDistance;
    emit newDistance(lastDistance);
    return;
  }

  // load waypoint list from task
  QList<wayPoint*> wpList = task->getWPList();

  // Load active task switch scheme. Switch to next TP can be executed
  // by nearst to TP or touched TP sector/cylinder.
  enum GeneralConfig::ActiveNTTaskScheme ntScheme =
    GeneralConfig::instance()->getActiveNTTaskScheme();

  // If we are fast enough (speed > 35Km/h), we do check, if we could
  // inside of an selected task sector. This condition is not
  // considered in manual mode to make testing possible.
  bool inside = false;

  if( lastSpeed.getKph() > 35 || ! gps->getConnected() ) {
    inside = task->checkSector( curDistance, lastPosition, selectedWp->taskPointIndex );
  }

  // We set us a flag to remember, that we did arrive the radius of a
  // task point. Must be done because we can have only one touch.
  if( inside && wpTouched == false && autoWpSwitch ) {
    wpTouched = true;

    // send a signal to the igc logger to increase logging interval
    emit taskpointSectorTouched();

    // Display an task end message under following conditions:
    // a) we touched the target radius
    // b) the last task point is selected
    if( selectedWp->taskPointType == wayPoint::Landing && taskEndReached == false ) {
      taskEndReached = true;
      emit taskInfo( tr("Task target reached"), true );

      QString text = "<html><hr><b><nobr>" +
        tr("Task Target reached") +
        "</nobr></b><hr>" +
        "<p><center><b>" +
        tr("Congratulations!") +
        "</b></center></p><br><br><b>" +
        tr("You have reached the <br>task target sector:") +
        "</b><p align=\"left\"><b>" +
        selectedWp->name + " (" + selectedWp->description + ")</b></p><br></html>";

      // fetch info show time from config and compute it as milli seconds
      int showTime = GeneralConfig::instance()->getInfoDisplayTime() * 1000;

      WhatsThat *box = new WhatsThat( _globalCumulusApp, text,  showTime );
      box->show();
    }
    else if( taskEndReached == false ) {
      if( ntScheme == GeneralConfig::Nearst ) {
      // Announce task point touch only, if nearst switch scheme is
      // choosen by the user to avoid to much info for him.
      emit taskInfo( tr("Taskpoint sector reached"), true );
      }
      else {
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

  // We arrived the taskpoint switch radius and after that event the
  // wpTouchedCounter reaches zero or the nearst position to the TP is
  // arrived. In this case we have to execute and announce the task
  // point switch.
  if( ( curDistance.getMeters() > lastDistance.getMeters() ||
      ( wpTouchCounter > 0 && --wpTouchCounter == 0 ) ) &&
      wpTouched == true ) {

    wpTouched      = false;
    wpTouchCounter = 0;

    if( wpList.count() > selectedWpInList + 1 ) {
      // this loop excludes the last WP
      wayPoint *lastWp = wpList.at(selectedWpInList);
      selectedWpInList++;
      wayPoint *nextWp = wpList.at(selectedWpInList);

      // calculate distance to new waypoint
      Distance dist2Next( dist(double(lastPosition.x()), double(lastPosition.y()),
                               nextWp->origP.lat(), nextWp->origP.lon()) * 1000);
      lastDistance = dist2Next;

      // announce taskpoint change as none auto switch
      slot_WaypointChange( nextWp, false );

      // Here we send a notice to the user about the taskpoint
      // switch. If end point reached and landing point is identical
      // to end point, we will suppress the info message
      if( ! ( lastWp->taskPointType == wayPoint::End &&
              nextWp->taskPointType == wayPoint::Landing &&
              lastWp->origP == nextWp->origP ) ) {

	emit taskInfo( tr("Automatic taskpoint switch"), true );

        // switch off all set accelerators
        _globalCumulusApp->setView( CumulusApp::tpSwitchView );

        TPInfoWidget *tpInfo = new TPInfoWidget( _globalCumulusApp );
        tpInfo->prepareSwitchText( lastWp->taskPointIndex, dist2Next.getKilometers() );

        // switch back to map view on close of tp info widget
        connect( tpInfo, SIGNAL( close() ),
                 _globalCumulusApp, SLOT( slotSwitchToMapView() ) );

        tpInfo->showTP();
      }
    }
  }
  else {
    lastDistance = curDistance;
  }

  emit newDistance(lastDistance);
}


/** Calculates the ETA (Estimated Time to Arrival) to the current
    waypoint and emits a signal newETA if the value has changed. */
void CuCalc::calcETA()
{
  QTime etaNew(0, 0);

  // qDebug("lastSpeed=%f m/s", lastSpeed.getMps());

  if( ! _calculateETA || ! selectedWp ||
      lastSpeed.getMps() <= 0.3 || ! gps->getConnected() ) {
    if( ! lastETA.isNull() ) {
      emit newETA(etaNew);
      lastETA = etaNew;
    }

    return;
  }

  int eta = (int) rint(lastDistance.getMeters() / lastSpeed.getMps());

  etaNew = etaNew.addSecs(eta);

  if( lastETA.hour() == etaNew.hour() &&
      lastETA.minute() == etaNew.minute() ) {
    // changes in seconds only will be ignored
    return;
  }

  if( etaNew != lastETA ) {
    lastETA = etaNew;

    if( eta > 99*3600 ) {
      // Don't emit times greater as 99 hours. ETA will be set to
      // undefined in such a case to avoid a problem in the map
      // display with to large values.
      QTime zero(0, 0);
      emit newETA(zero);
    }
    else {
      emit newETA(lastETA);
    }
  }

  // qDebug("New ETA=%s", etaNew.toString().toLatin1().data());
}


/** Calculates the required LD and the current LD about the last 60s,
    if required */
void CuCalc::calcLD()
{
  if( ! selectedWp || _calculateLD == false || samplelist.count() < 2 ) {
    return;
  }

  const flightSample *start = 0;
  const flightSample *end = &samplelist.at(0);
  int timeDiff = 0;
  double distance = 0.0;
  double newCurrentLD = -1.0;
  double newRequiredLD = -1.0;
  bool notify = false;

  // first calculate current LD
  for( int i = 1; i < samplelist.count(); i++ ) {

    timeDiff = (samplelist.at(i).time).msecsTo(end->time);

    if( timeDiff >= 60*1000 ) {
      break;
    }

    // summarize single distances from positions
    // distance += dist( &samplelist->at(i-1)->position, &samplelist->at(i)->position ) * 1000;

    // summarize single distances from speed
    distance += samplelist[i].vector.getSpeed().getMps();

    // qDebug( "i=%d, dist=%f", i, distance );
    // store start record
    start = &samplelist[i];
  }

  if( ! start ) {
    // time distance too short
    lastCurrentLD = -1.0;
  } else {

    // calculate altitude difference
    double altDiff = start->altitude.getMeters() - end->altitude.getMeters();

    if( altDiff <= 0.2 ) {
      // we climbed in the last time, therefore the result will become huge
      newCurrentLD = 999.0;
    } else {

      newCurrentLD = distance / altDiff ;

      if( newCurrentLD  > 999.0 ) {
        newCurrentLD = 999.0;
      }
    }
  }

  // calculate required LD, we consider elevation and security
  // altitude

  GeneralConfig *conf = GeneralConfig::instance();

  double altDiff = lastAltitude.getMeters() - selectedWp->elevation -
    conf->getSafetyAltitude().getMeters();

  if( altDiff <= 10.0 ) {
    newRequiredLD = -1.0;
  } else {
    newRequiredLD = lastDistance / altDiff;
  }

  if( newRequiredLD != lastRequiredLD ) {
    lastRequiredLD = newRequiredLD;
    notify = true;
  }

  if( newCurrentLD != lastCurrentLD ) {
    lastCurrentLD = newCurrentLD;
    notify = true;
  }

  if( notify ) {
    emit newLD( lastRequiredLD, lastCurrentLD );
    // qDebug("SIGNAL: rLD=%f, cLD=%f", lastRequiredLD, lastCurrentLD);
  }
}


/** Calculates the glide path to the current waypoint and the needed height */
bool CuCalc::glidePath(int aLastBearing, Distance aDistance,
                       Altitude aElevation, Altitude &arrivalAlt,
                       Speed &bestSpeed )
{

  if (!_polar) {
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


void CuCalc::calcGlidePath()
{
//  qDebug ("CuCalc::calcGlidePath");
  Speed speed;
  Altitude above;

  // Calculate new glide path, if a waypoint is selected and
  // a glider is defined.
  if ( ! selectedWp || ! _glider ) {
    lastRequiredLD = -1;
    return;
  }

  glidePath(lastBearing, lastDistance, selectedWp->elevation, above, speed );

  if (speed != lastBestSpeed) {
    lastBestSpeed = speed;
    emit bestSpeed (speed);
  }
  if (above != lastGlidePath) {
    lastGlidePath = above;
    emit glidePath (above);
  }
}


/** Calculates the bearing to the currently selected waypoint, and emits signal newBearing if the bearing has changed. */
void CuCalc::calcBearing()
{
  int iresult;
  int lH=lastHeading;
  if (lH == -1)
    lH=0;

  if (selectedWp==0) {
    iresult=-1;
    if (iresult!=lastBearing) {
      lastBearing=iresult;
      emit newBearing(-1000);
      // if we have no waypoint, let the rel bearing arrow point to north
      emit newRelBearing (-lH);
    }
  } else {
    double result = getBearing(lastPosition, selectedWp->origP);
    iresult = int (rint(result * 180./M_PI) );
    if (iresult!=lastBearing) {
      lastBearing=iresult;
      emit newBearing(lastBearing);
      emit newRelBearing (lastBearing - lH);
    }
  }
}


/** Called if the position is changed manually. */
void CuCalc::slot_changePosition(int direction)
{
  extern MapMatrix * _globalMapMatrix;

  double distance=_globalMapMatrix->getScale(MapMatrix::CurrentScale)*10;
  double kmPerDeg;

  switch(direction) {
  case MapMatrix::North:
    kmPerDeg=dist(lastPosition.x(), lastPosition.y(), lastPosition.x()+600000, lastPosition.y());
    lastPosition=QPoint((int) rint(lastPosition.x()+(distance/kmPerDeg) * 600), lastPosition.y());
    break;
  case MapMatrix::West:
    kmPerDeg=dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
    lastPosition=QPoint(lastPosition.x(), (int) rint(lastPosition.y()-(distance/kmPerDeg) * 600));
    break;
  case MapMatrix::East:
    kmPerDeg=dist(lastPosition.x(), lastPosition.y(), lastPosition.x(), lastPosition.y()+600000);
    lastPosition=QPoint(lastPosition.x(), (int) rint(lastPosition.y()+(distance/kmPerDeg) * 600));
    break;
  case MapMatrix::South:
    kmPerDeg=dist(lastPosition.x(),lastPosition.y(), lastPosition.x()+600000,lastPosition.y());
    lastPosition=QPoint((int) rint(lastPosition.x()-(distance/kmPerDeg) * 600), lastPosition.y());
    break;
  case MapMatrix::Home:
    lastPosition=QPoint(_globalMapMatrix->getHomeCoord());
    break;
  case MapMatrix::Waypoint:
    if (selectedWp)
      lastPosition=selectedWp->origP;
    break;
  }

  lastElevation = Altitude( _globalMapContents->findElevation(lastPosition, &lastElevationError) );
  emit newPosition(lastPosition, CuCalc::MAN);

  lastAltitude = manualAltitude;  // provide support to config altitude in settings
  lastAGLAltitude = lastAltitude - Altitude( _globalMapContents->findElevation(lastPosition) );
  lastSTDAltitude = manualAltitude;

  GeneralConfig *conf = GeneralConfig::instance();
  int qnhDiff = 1013 - conf->getQNH();

  if( qnhDiff != 0 ) {
    // calculate altitude correction in meters from pressure difference
    int delta = (int) rint( qnhDiff * 8.6 );

    // qDebug("CuCalc::slot_changePosition(): QHN=%d, Delta=%d", conf->getQNH(), delta);

    lastSTDAltitude.setMeters(manualAltitude.getMeters() + delta );
  }

  emit newAltitude(lastAltitude);

  calcDistance();
  calcBearing();
  calcGlidePath();
  // calculate always when moving manually (big delta !)
  _reachablelist->calculate(true);
  //qDebug("Elevation: %d m",_globalMapContents->findElevation(lastPosition) );
}


/** No descriptions */
void CuCalc::slot_changePositionN()
{
  slot_changePosition(MapMatrix::North);
}


/** No descriptions */
void CuCalc::slot_changePositionS()
{
  slot_changePosition(MapMatrix::South);
}


/** No descriptions */
void CuCalc::slot_changePositionE()
{
  slot_changePosition(MapMatrix::East);
}


/** No descriptions */
void CuCalc::slot_changePositionW()
{
  slot_changePosition(MapMatrix::West);
}


/** No descriptions */
void CuCalc::slot_changePositionHome()
{
  slot_changePosition(MapMatrix::Home);
}


/** No descriptions */
void CuCalc::slot_changePositionWp()
{
  slot_changePosition(MapMatrix::Waypoint);
}


/** increment McCready value */
void CuCalc::slot_McUp()
{
  if (lastMc.getMps() <= MAX_MCCREADY - 0.5) {
    lastMc.setMps(lastMc.getMps()+0.5);
    calcGlidePath();
    emit newMc (lastMc);
  }
}


/** set McCready value */
void CuCalc::slot_Mc(const Speed& spd)
{
  lastMc.setMps(spd.getMps());
  calcGlidePath();
  emit newMc (lastMc);
}


/** decrement McCready value; don't get negative */
void CuCalc::slot_McDown()
{
  if (lastMc.getMps() >= 0.5) {
    lastMc.setMps(lastMc.getMps()-0.5);
    calcGlidePath();
    emit newMc (lastMc);
  }
}


void CuCalc::slot_Variometer (const Speed& lift)
{
  if (lastVario != lift) {
    lastVario = lift;
    emit newVario (lift);
  }
}


/** Sets the current position to point newPos. */
void CuCalc::setPosition(const QPoint& newPos)
{
  lastPosition=newPos;
  emit newPosition(lastPosition, CuCalc::MAN);

  calcDistance();
  calcBearing();
  calcETA();
  calcGlidePath();
}


/** Sets the glider type to allow calculation of polar data */
void CuCalc::slot_settingschanged ()
{
  _altimeter_mode = AltimeterModeDialog::mode();
  emit newAltitude(lastAltitude);  // show initial altitude for manual mode
  // qDebug("Settings changed %d",_altimeter_mode );

  // Send last known wind to mapview for update of speed. User maybe
  // changed the speed unit.
  if( lastWind.getSpeed().getMps() != 0 )
    {
      emit( newWind(lastWind) );
    }
}


/** This slot is called by the NMEA interpreter if a new fix has been received.  */
void CuCalc::slot_newFix()
{
  // before we start making samples, let's be sure we have all the
  // data we need for that. So, we wait for the second Fix.
  if (!_pastFirstFix) {
    _pastFirstFix=true;
    return;
  }

  // create a new sample struct
  flightSample sample;

  // fill it with the relevant data
  sample.time=gps->getLastTime();
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
  if( lastWind.getSpeed().getKph() != 0 )
    {
      sample.airspeed = airspeed.getSpeed();
    }

  // add to the samplelist
  samplelist.add(sample);
  lastSample = sample;

  // call vario calculation, if required
  if( _calculateVario == true) {
    _vario->newAltitude();
  }

  // cal LD calculation
  calcLD();

  // start analysing...
  // determine if we are standing still, cruising, circling or doing something else
  determineFlightStatus();

  // let the world know we have added a new sample to our samplelist
  emit newSample();
}


/** Determines the status of the flight: unkown, standstill, cruising, circlingL, circlingR */
void CuCalc::determineFlightStatus()
{
  /*
    WARNING: THIS CODE IS HIGHLY EXPERIMENTAL AND UNTESTED IN FLIGHT!

    In principle, we are determining the flightstatus from the last 20
    seconds of flight. We may need to adjust this, as this is highly
    experimental code. However, if we allready have a flightstatus, we
    only need to check if it has changed. If that is the case, we set
    the status to "Unkown" so the next fix will trigger a normal
    analysation again.

    Further optimisations could include using the airspeed to
    determine the flightmode. This however requires a good working
    windestimation, wich requires a good working flightstatus.... etc.
    Another approch would be to keep a counter that could allow for,
    say, one or two 'errors' in the data before a flightmode is
    changed to 'unknown'.
  */

  /*
    We define a maximum turnrate of 10 degrees per second. This should
    allow for course-corrections.  This can be a bit more advanced to
    allow for temporary changes in circling speed in order to better
    center a thermal.
  */
#define MAXCRUISEANGDIFF 8 //see above

  /*
    We define a minimum turnrate of 4 degrees per second (that is,
    one turn takes at most one and a half minute).

    This can be a bit more advanced to allow for temporary changes in
    circling speed in order to better center a thermal.
  */
#define MINTURNANGDIFF 4 //see above

  /*
    We define a maximum altitude drift to refine the detection of a
    standstill. Because GPS altitudes tend to drift, we can't rely
    on the exact same altitude over any length of time. We need to
    allow for some drift.
  */
#define MAXALTDRIFT 10 //see above

  /*
    We define an analysis timeframe. This timeframe determines how
    far back we're going in history (in seconds) for our
    analysis. Note that the actual timedifference between the first
    and the last sample used may differ from this timeframe.
  */
#define TIMEFRAME 10

  if (samplelist.count() < 5)
    return; //we need to have some samples in order to be able to analyse anything.

  flightmode newFlightMode=unknown;

  //get headings from the last two samples
  int lastHead=samplelist[0].vector.getAngleDeg();
  int prevHead=samplelist[1].vector.getAngleDeg();

  //get the timedifference between these samples
  int timediff=samplelist[1].time.secsTo(samplelist[0].time);

  if (timediff==0)
    return; //if the time difference is 0, return (just to be sure). This will only cause problems...


  //we are not doing a full analysis if we allready have a flightmode. It suffices to check some
  //basic criteria.
  //lastFlightMode=unknown; //force complete evaluation for now.
  //qDebug("Anglediff: %d",angleDiff(lastHead, prevHead));

  switch (lastFlightMode) {
  case standstill: //we are not moving at all!
    if (samplelist[0].position == samplelist[1].position) {  //may be too ridgid, GPS errors could cause problems here
      return; //no change in flightmode
    } else {
      newFlightMode=unknown;
    }
    break;

  case wave: //we are not moving at all, except vertically!  Needs lots of tweaking and testing...
    if (samplelist[0].position == samplelist[1].position) {  //may be too ridgid, GPS errors could cause problems here
      return; //no change in flightmode
    } else {
      newFlightMode=unknown;
    }
    break;

  case cruising: //we are flying from point A to point B
    if (abs(angleDiff(lastHead, _cruiseDirection)) <  MAXCRUISEANGDIFF &&
        samplelist[0].vector.getSpeed().getMps()>5) {
      return; //no change in flightmode
    } else {
      newFlightMode=unknown;
      break;
    }

  case circlingL:
    //turning left means: the heading is decreasing
    if (angleDiff(prevHead, lastHead) > (-MINTURNANGDIFF * timediff)) {
      newFlightMode=unknown;
      break;
    } else {
      return; //no change in flightmode
    }


  case circlingR:
    //turning right means: the heading is increasing
    if (angleDiff(prevHead, lastHead) < (MINTURNANGDIFF * timediff)) {
      newFlightMode=unknown;
      break;
    } else {
      return; //no change in flightmode
    }

  default:
    newFlightMode=unknown;
  }

  if (newFlightMode==unknown) {
    //we need some real analysis
    QTime tmp= samplelist[0].time.addSecs(-TIMEFRAME); //reference time
    int ls=1;

    while((samplelist[ls].time > tmp) && ( ls < samplelist.count()-1) )
      ls++;
    //ls now contains the index of the oldest sample we will use for this analysis.
    //Newer samples have lower indices!

    //initialize some values we will be needing...
    bool mayBeL=true;     //this may be a left turn (that is, no big dirchange to the right)
    bool mayBeR=true;     //this may be a right turn (that is, no big dirchange to the left)
    int totalDirChange=0;  //total headingchange. If cruising, this will be low, if turning, it will be high
    int maxSpeed=0;        //maximum speed obtained in this set of samples
    int aDiff=0;                 //difference in heading between two samples
    int totalAltChange=0;  //total change in altitude (absolute)
    int maxAltChange=0;  //maximum change of altitude between samples.
    int altChange=0;
    bool break_analysis=false; //flag to indicate we can stop further analyisis.


    //loop through the samples to get some basic data we can use to distinguish flightmodes
    for (int i=ls-1;i>=0;i--) {
      aDiff = angleDiff( samplelist[i+1].vector.getAngleDeg(), samplelist[i].vector.getAngleDeg() );
      //qDebug("analysis: angle1=%d, angle2=%d, diff=%d",int(samplelist->at(i+1)->vector.getAngleDeg()),int(samplelist->at(i)->vector.getAngleDeg()), aDiff);
      //qDebug("analysis: position=(%d, %d)", samplelist->at(i)->position.x(),samplelist->at(i)->position.y() );
      totalDirChange += abs(aDiff);
      maxSpeed = qMax( maxSpeed, (int) rint(samplelist[i].vector.getSpeed().getKph()));
      altChange = int(samplelist[i].altitude.getMeters() - samplelist[i+1].altitude.getMeters());
      totalAltChange += altChange;
      maxAltChange = int(qMax(abs(altChange), maxAltChange));

      if (aDiff >  MINTURNANGDIFF)
        mayBeL=false;
      if (aDiff < -MINTURNANGDIFF)
        mayBeR=false;
      //qDebug("  analysis: sample: %d, speed: %f",i,samplelist->at(i)->vector.getKph());
    }
    //qDebug ("analysis: aDiff=%d, totalDirChange=%d, maxSpeed=%d, totalAltChange=%d, maxAltChange=%d, maybeLeft=%d, maybeRight=%d",aDiff, totalDirChange, maxSpeed, totalAltChange, maxAltChange, int(mayBeL), int(mayBeR));

    //try standstill. We are using a value > 0 because of possible GPS errors.
    /*
      The detection of standstills may be extended further by checking if the altitude matches the terrain altitude. If not
      (or no where near), we can not assume a standstill. This is probably wave flying.
    */
    if (maxSpeed<10) { // if we get under 10 kph for maximum speed, we may be standing still
      if (abs(totalAltChange) * 2 <= MAXALTDRIFT && maxAltChange <= MAXALTDRIFT) {  //check if we had any significant altitude changes
        newFlightMode=standstill;
        break_analysis=true;
      } else {
        newFlightMode=wave;
        break_analysis=true;
      }
    }

    if (!break_analysis) {
      //get the timedifference between the first and the last sample. This might not be the 20 secs we were planning to use at all!
      timediff= samplelist[ls-1].time.secsTo(samplelist[0].time);

      //see if we might be cruising...
      if (mayBeL && mayBeR) { //basicly, we have been going (almost) strait it seems...
        if (totalDirChange < 2 * timediff) {
          //qDebug("analysis: distance=%f m, timedifference=%d s",dist(&samplelist->at(ls-1)->position,&samplelist[0].position)*1000,timediff);
          if (dist(&samplelist[ls-1].position, &samplelist[0].position)*1000 > 5*timediff) //our average speed should be at least 5 m/s to qualify for cruising
            newFlightMode=cruising;
          _cruiseDirection=samplelist[0].vector.getAngleDeg();
          // qDebug("Cruise direction: %d.",_cruiseDirection);
        }
        break_analysis=true;
      }
    }

    if (!break_analysis) {
      //So, we are not standing still, nor are we cruising. Circling then maybe?
      if ((MINTURNANGDIFF * timediff ) < totalDirChange) { //if circling, we should have a minimum average turnrate of MINTURNANGDIFF degrees per second
        if (mayBeL && !mayBeR)
          newFlightMode=circlingL;
        if (mayBeR && !mayBeL)
          newFlightMode=circlingR;
        //we have a new mode: we are circling! If neither of the above are true, we are doing something
        //funny, and the flightmode 'unkown' still applies, so we may also quit analysing...
      }
    }
  }

  if (newFlightMode!=lastFlightMode) {
    lastFlightMode=newFlightMode;
    samplelist[0].marker=++_marker;
    // qDebug("new flightmode: %d",lastFlightMode);
    emit flightModeChanged(newFlightMode);
  }
}


/** Called if the status of the GPS changes. */
void CuCalc::slot_GpsStatus(GPSNMEA::connectedStatus newState)
{
  // qDebug("connection status changed...");
  flightmode newFlightMode=unknown;
  // qDebug("new flightmode=%d, last flightmode=%d",newFlightMode, lastFlightMode);

  if (newFlightMode!=lastFlightMode) {
    lastFlightMode=newFlightMode;
    samplelist[0].marker=++_marker;
    // qDebug("new flightmode: %d",lastFlightMode);
    emit flightModeChanged(newFlightMode);
  }

  if( newState == GPSNMEA::noFix ) {
    // Reset LD display
    emit newLD( -1.0, -1.0 );
    // reset first fix passed
    _pastFirstFix = false;
  }
}


CuCalc::flightmode CuCalc::currentFlightMode()
{
  return lastFlightMode;
}


/** This slot is used internally to re-emit the flightmode signal with the marker value */
void CuCalc::slot_flightModeChanged(CuCalc::flightmode fm)
{
  emit flightModeChanged(fm, _marker);
}


/** Called if the wind measurement changes */
void CuCalc::slot_Wind(Vector& v)
{
  lastWind = v;
  emit newWind(v);
}


/** Read property of Glider glider. */
Glider *CuCalc::glider() const
{
  return _glider;
}


QString CuCalc::gliderType() const
{
  if (_glider)
    return _glider->type();
  else
    return QString::null;
}


/** Store property of new Glider. */
void CuCalc::setGlider(Glider * _newVal)
{
  if (_glider)
    {
      delete _glider;

      // reset glider and polar object to avoid senseless calculations
      _glider = 0;
      _polar  = 0;
    }

  if (_newVal)
    {
      _glider = _newVal;
      _polar = _glider->polar();
      calcETA();
      calcGlidePath();
      emit newGlider( _glider->type() );
    }
  else
    {
      emit newGlider( QString::null );
    }
}


void CuCalc::newSites(void)
{
  _reachablelist->calculateFullList();
}


bool CuCalc::matchesFlightMode(GeneralConfig::UseInMode mode)
{
  if (mode==GeneralConfig::always)
    return true;
  if (mode==GeneralConfig::never)
    return false;
  if (mode==GeneralConfig::standstill)
    return lastFlightMode == standstill;
  if (mode==GeneralConfig::cruising)
    return lastFlightMode == cruising;
  if (mode==GeneralConfig::wave)
    return lastFlightMode == wave;
  if (mode==GeneralConfig::circling)
    return (lastFlightMode == circlingL || lastFlightMode == circlingR);

  return false;
}

/**
 * Sets a new selected waypoint. The old waypoint instance is
 * deleted and a new one allocated.
 */
void CuCalc::setSelectedWp( const wayPoint* newWp )
{
  // delete old waypoint selection
  if( selectedWp != 0 ) {
    delete selectedWp;
    selectedWp = 0;
  }

  // make a deep copy of new waypoint to be set
  if( newWp != 0 ) {
    selectedWp = new wayPoint( *newWp );
  }
}

void CuCalc::setManualInFlight(bool switchOn) {
  manualInFlight = switchOn;
  // immediately put glider into center if outside
  // we can only switch off if GPS data coming in
  emit switchManualInFlight();
}

bool CuCalc::isManualInFlight() {
  return manualInFlight;
}
