/***********************************************************************
**
**   LiveTrack24Logger.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2018 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "calculator.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "LiveTrack24Logger.h"
#include "LiveTrack24.h"
#include "skylines/SkyLinesTracker.h"

LiveTrack24Logger::LiveTrack24Logger( QObject *parent ) :
  QObject(parent),
  m_ltGateway(0),
  m_isFlying(false)
{
  m_lastTrackReporting.start();
  m_lastMoveTimePoint.start();

  // supervision timer for new fix arrival
  m_closeSessionTimer = new QTimer( this );
  m_closeSessionTimer->setSingleShot( true );
  m_closeSessionTimer->setInterval( 30000 );

  connect( m_closeSessionTimer, SIGNAL(timeout ()), SLOT(slotFinishLogging()));

  slotConfigChanged();
}

void LiveTrack24Logger::slotConfigChanged()
{
  // Initiate the selected LiveTrack gateway
  GeneralConfig* conf = GeneralConfig::instance();

  if( m_ltGateway != 0 )
    {
      // Stop current tracking, configuration has been changed.
      m_ltGateway->endTracking();
      m_ltGateway->deleteLater();
      m_ltGateway = 0;
    }

  // Setup a new LiveTrack gateway after a configuration change.
  if( conf->getLiveTrackServer() != SkyLinesTracker::getServerName() )
    {
      m_ltGateway = new LiveTrack24( this );
    }
  else
    {
      m_ltGateway = new SkyLinesTracker( this );
    }
}

LiveTrack24Logger::~LiveTrack24Logger()
{
  m_closeSessionTimer->stop();
}

void LiveTrack24Logger::slotNewFixEntry()
{
  GeneralConfig* conf = GeneralConfig::instance();

  // Check if LiveTracking is switched on
  if( conf->isLiveTrackOnOff() == false )
    {
      m_isFlying = false;
      return;
    }

  if( calculator->moving() )
    {
      m_lastMoveTimePoint.start();

      if( m_isFlying == false )
        {
          // We have to report a start of moving
          m_isFlying = true;
          m_ltGateway->startTracking();
        }
    }

  if( m_isFlying )
    {
      int reportInterval = conf->getLiveTrackInterval() * 1000; // ms

      if( m_lastTrackReporting.elapsed() >= reportInterval )
        {
          // We have to report a new track point
          m_lastTrackReporting.start();
          reportRoutePoint();
        }

      // No moving for 60 seconds and calculator reports a stand still,
      // we assume a landing and stop tracking.
      if( m_lastMoveTimePoint.elapsed() >= 60000 &&
          calculator->currentFlightMode() == Calculator::standstill )
        {
          // We have to report an end of moving
          m_lastTrackReporting.start();
          reportRoutePoint();
          m_ltGateway->endTracking();
          m_isFlying = false;
        }

      // start/restart close session supervision timer
      m_closeSessionTimer->start();
    }
}

void LiveTrack24Logger::slotNewSwitchState( bool state )
{
  if( state == false )
    {
      // LiveTracking has been switched off
      if( m_isFlying == true )
        {
          // We are flying, finish logging.
          m_closeSessionTimer->stop();
          slotFinishLogging();
        }
    }
}

void LiveTrack24Logger::reportRoutePoint()
{
  m_ltGateway->routeTracking( calculator->getlastPosition(),
                              rint(calculator->getlastAltitude().getMeters()),
                              rint(calculator->getLastSpeed().getKph()),
                              calculator->getlastHeading() == -1 ? 0 : calculator->getlastHeading() % 360,
                              calculator->getlastVario().getMps(),
                              GpsNmea::gps->getLastUtc().currentMSecsSinceEpoch() / 1000 );
}

void LiveTrack24Logger::slotFinishLogging()
{
  if( m_isFlying == false )
    {
      // We are not in fly mode therefore an end tracking is not necessary.
      return;
    }

  m_ltGateway->endTracking();
  m_isFlying = false;
  m_lastTrackReporting.start();
  m_lastMoveTimePoint.start();
}
