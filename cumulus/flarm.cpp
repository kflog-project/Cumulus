/***********************************************************************
**
**   flarm.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "flarm.h"
#include "flarmdisplay.h"

// initialize static data items
bool Flarm::collectPflaa = false;

Flarm::FlarmStatus Flarm::flarmStatus;

QHash<QString, Flarm::FlarmAcft> Flarm::pflaaHash;

Flarm::Flarm(QObject* parent) : QObject(parent)
{
  flarmStatus.valid = false;

  // Setup timer for data clearing
  timer = new QTimer( this );
  timer->setSingleShot( true );
  connect( timer, SIGNAL(timeout()), this, SLOT(slotTimeout()) );
}

Flarm::~Flarm()
{
}

/**
 * Extracts all items from $PFLAU sentence from Flarm device.
 */
bool Flarm::extractPflau( const QStringList& stringList )
{
  flarmStatus.valid = false;

  if ( stringList[0] != "$PFLAU" || stringList.size() < 11 )
    {
      qWarning("$PFLAU contains too less parameters!");
      return false;
    }

  /**
    $PFLAU,<RX>,<TX>,<GPS>,<Power>,<AlarmLevel>,<RelativeBearing>,<AlarmType>,
    <RelativeVertical>,<RelativeDistance>,<ID>
  */

  bool ok;
  short value;

  // RX number of received devices
  flarmStatus.RX = 0;
  value = stringList[1].toShort( &ok );

  if( ok )
    {
      flarmStatus.RX = value;
    }

  // TX Transmission status
  flarmStatus.TX = 0;
  value = stringList[2].toShort( &ok );

  if( ok )
    {
      flarmStatus.TX = value;
    }

  // GPS status
  flarmStatus.Gps = NoFix;
  value = stringList[3].toShort( &ok );

  if( ok )
    {
      flarmStatus.Gps = static_cast<enum GpsStatus> (value);
    }

  // Power status
  flarmStatus.Power = 0;
  value = stringList[4].toShort( &ok );

  if( ok )
    {
      flarmStatus.Power = value;
    }

  // AlarmLevel
  value = stringList[5].toShort( &ok );
  flarmStatus.Alarm = No;

  if( ok )
    {
      flarmStatus.Alarm = static_cast<enum AlarmLevel> (value);
    }

  // RelativeBearing
  flarmStatus.RelativeBearing = stringList[6];

  // AlarmType
  value = stringList[7].toShort( &ok );
  flarmStatus.AlarmType = 0;

  if( ok )
    {
      flarmStatus.AlarmType = value;
    }

  // RelativeVertical
  flarmStatus.RelativeVertical = stringList[8];

  // RelativeDistance
  flarmStatus.RelativeDistance = stringList[9];

  // ID 6-digit hex value
  flarmStatus.ID = stringList[10];

  flarmStatus.valid = true;

  return true;
}

/**
 * Extracts all items from the $PFLAA sentence sent by the Flarm device.
 */
bool Flarm::extractPflaa( const QStringList& stringList, FlarmAcft& aircraft )
{
  if ( stringList[0] != "$PFLAA" || stringList.size() < 12 )
    {
      qWarning("$PFLAA contains too less parameters!");
      return false;
    }

  /**
    00: $PFLAA,
    01: <AlarmLevel>,
    02: <RelativeNorth>,
    03: <RelativeEast>,
    04: <RelativeVertical>,
    05: <IDType>,
    06: <ID>,
    07: <Track>,
    08: <TurnRate>,
    09: <GroundSpeed>,
    10: <ClimbRate>,
    11: <AcftType>
  */

  bool ok;

  aircraft.TimeStamp = QTime::currentTime();

  // AlarmLevel
  aircraft.Alarm = static_cast<enum AlarmLevel> (stringList[1].toInt( &ok ));

  if( ! ok )
    {
      aircraft.Alarm = No;
    }

  aircraft.RelativeNorth = stringList[2].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeNorth = INT_MIN;
    }

  aircraft.RelativeEast = stringList[3].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeEast = INT_MIN;
    }

  aircraft.RelativeVertical = stringList[4].toInt( &ok );

  if( ! ok )
    {
      aircraft.RelativeVertical = INT_MIN;
    }

  aircraft.IdType = stringList[5].toInt( &ok );

  if( ! ok )
    {
      aircraft.IdType = 0;
    }

  aircraft.ID = stringList[6];

  // 0-359 or INT_MIN in stealth mode
  aircraft.Track = stringList[7].toInt( &ok );

  if( ! ok )
    {
      aircraft.Track = INT_MIN;
    }

  // degrees per second or INT_MIN in stealth mode
  aircraft.TurnRate = stringList[8].toDouble( &ok );

  if( ! ok )
    {
      aircraft.TurnRate = INT_MIN;
    }

  // meters per second or INT_MIN in stealth mode
  aircraft.GroundSpeed = stringList[9].toDouble( &ok );

  if( ! ok )
    {
      aircraft.GroundSpeed = INT_MIN;
    }

  // meters per second or INT_MIN in stealth mode
  aircraft.ClimbRate = stringList[10].toDouble( &ok );

  if( ! ok )
    {
      aircraft.ClimbRate = INT_MIN;
    }

  aircraft.AcftType = stringList[11].toShort( &ok );

  if( ! ok )
    {
      aircraft.AcftType = 0; // unknown
    }

  // Check, if parsed data should be collected. In this case the data record
  // is put or updated in the pflaaHash hash dictionary.
  QString key = createHashKey( aircraft.IdType, aircraft.ID );

  if( collectPflaa == true || key == FlarmDisplay::getSelectedObject() )
    {
      // first check, if record is already contained in the hash.
      if( pflaaHash.contains( key ) == true )
        {
          // update entry
          FlarmAcft& aircraftEntry = pflaaHash[key];
          aircraftEntry = aircraft;
        }
      else
        {
          // insert new entry
          pflaaHash.insert( key, aircraft );
        }
    }

  return true;
}

bool Flarm::getFlarmRelativeBearing( int &relativeBearing )
{
  if( ! flarmStatus.valid && flarmStatus.RelativeBearing.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeBearing = flarmStatus.RelativeBearing.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool Flarm::getFlarmRelativeVertical( int &relativeVertical )
{
  if( ! flarmStatus.valid && flarmStatus.RelativeVertical.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeVertical = flarmStatus.RelativeVertical.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool Flarm::getFlarmRelativeDistance( int &relativeDistance )
{
  if( ! flarmStatus.valid && flarmStatus.RelativeDistance.isEmpty() )
    {
      return false;
    }

  bool ok;

  relativeDistance = flarmStatus.RelativeDistance.toInt( &ok );

  if( ok )
    {
      return true;
    }
  else
    {
      return false;
    }
}

/**
 * PFLAA data collection is finished.
 */
void Flarm::collectPflaaFinished()
{
  // Check the hash dictionary for expired data. This old data items have to
  // be removed. Seems to be the best place, to do it after the end trigger as
  // to trust that following methods will do that.
  QMutableHashIterator<QString, Flarm::FlarmAcft> it(pflaaHash);

  while( it.hasNext() )
    {
      it.next();

      // Get next aircraft
      Flarm::FlarmAcft& acft = it.value();

      // Make time expire check, check time unit is in milli seconds.
      if( acft.TimeStamp.elapsed() > 3000 )
        {
          // Object was longer time not updated, so we do remove it from the
          // hash. No other way available as the time expire check.
          it.remove();
        }
    }

  // Start Flarm data clearing supervision.
  if( pflaaHash.size() > 0 )
    {
      timer->start( 6000 );
    }

  // Emit signal, if further processing in radar view is required.
  if( Flarm::getCollectPflaa() )
    {
      emit newFlarmPflaaData();
    }
}

/** Called if timer has expired. Used for Flarm data clearing. */
void Flarm::slotTimeout()
{
  pflaaHash.clear();
  emit newFlarmPflaaData();
}
