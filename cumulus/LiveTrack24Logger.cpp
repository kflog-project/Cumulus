/***********************************************************************
**
**   LiveTrack24Logger.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "calculator.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "LiveTrack24Logger.h"

LiveTrack24Logger::LiveTrack24Logger( QObject *parent ) :
  QObject(parent),
  m_isFlying(false)
{
  m_lastTrackReporting.start();
  m_lastMoveTimePoint.start();
}

LiveTrack24Logger::~LiveTrack24Logger()
{
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
          m_lt24Gateway.startTracking();
        }
    }

  if( m_isFlying )
    {
      int reportInterval = conf->getLiveTrackInterval() * 1000;

      if( m_lastTrackReporting.elapsed() >= reportInterval )
        {
          // We have to report a new track point
          m_lastTrackReporting.start();
          reportRoutePoint();
        }

      // No moving for 30 seconds and calculator reports a stand still,
      // we assume a landing and stop tracking.
      if( m_lastMoveTimePoint.elapsed() >= 30000 &&
          calculator->currentFlightMode() == Calculator::standstill )
        {
          // We have to report an end of moving
          m_lastTrackReporting.start();
          reportRoutePoint();
          m_lt24Gateway.endTracking();
          m_isFlying = false;
        }
    }
}

void LiveTrack24Logger::slotNewSwitchState( bool state )
{
  qDebug() << "LiveTrack24Logger::slotNewSwitchState" << state;

  if( state == false )
    {
      // LiveTracking has been switched off
      if( m_isFlying == true )
        {
          // We are flying, finish logging.
          finishLogging();
        }
    }
}

void LiveTrack24Logger::reportRoutePoint()
{
  m_lt24Gateway.routeTracking( calculator->getlastPosition(),
                               rint(calculator->getlastAltitude().getMeters()),
                               rint(calculator->getLastSpeed().getKph()),
                               calculator->getlastHeading(),
                               GpsNmea::gps->getLastUtc().currentMSecsSinceEpoch() / 1000 );
}

void LiveTrack24Logger::finishLogging()
{
  qDebug() << "LiveTrack24Logger::finishLogging()";

  if( m_isFlying == false )
    {
      // We are not in fly mode therefore end tracking is not necessary.
      return;
    }

  m_lt24Gateway.endTracking();
  m_isFlying = false;
  m_lastTrackReporting.start();
  m_lastMoveTimePoint.start();
}
