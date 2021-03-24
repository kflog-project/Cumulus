/***********************************************************************
**
**   flarmbase.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtCore>

#include "flarmbase.h"

// initialize static data items
bool FlarmBase::m_collectPflaa = false;

FlarmBase::FlarmStatus  FlarmBase::m_flarmStatus;
FlarmBase::FlarmData    FlarmBase::m_flarmData;
FlarmBase::FlarmError   FlarmBase::m_flarmError;
FlarmBase::ProtocolMode FlarmBase::m_protocolMode = text;

QHash<QString, FlarmBase::FlarmAcft>      FlarmBase::m_pflaaHash;

QMutex FlarmBase::m_mutex;

FlarmBase::FlarmBase()
{
  m_flarmStatus.valid = false;
}

FlarmBase::~FlarmBase()
{
}

QByteArray FlarmBase::replaceUmlauts( QByteArray string )
{
  QByteArray array( string );

  array = array.replace( Qt::Key_Adiaeresis, "Ae" );
  array = array.replace( Qt::Key_Odiaeresis, "Oe" );
  array = array.replace( Qt::Key_Udiaeresis, "Ue" );
  array = array.replace( Qt::Key_Adiaeresis + 0x20, "ae" );
  array = array.replace( Qt::Key_Odiaeresis + 0x20, "oe" );
  array = array.replace( Qt::Key_Udiaeresis + 0x20, "ue" );
  array = array.replace( 0xdf, "ss" );

  // An asterisk in the Flarm command payload is not accepted by Flarm.
  array = array.replace( "*", "+" );

  return array;
}

QString FlarmBase::translateAlarmType( const short hexType )
{
  QString alarmType;

  switch( hexType )
  {
    case 0:
      alarmType = QObject::tr("No Traffic");
      break;
    case 2:
      alarmType = QObject::tr("Aircraft Alarm");
      break;
    case 3:
      alarmType = QObject::tr("Obstacle Alarm");
      break;
    case 4:
      alarmType = QObject::tr("Traffic Advisory");
      break;
    case 0x41:
      alarmType = QObject::tr("Skydiver drop zone");
      break;
    case 0x42:
      alarmType = QObject::tr("Aerodrom traffic zone");
      break;
    case 0x43:
      alarmType = QObject::tr("Military firing area");
      break;
    case 0x44:
      alarmType = QObject::tr("Kite flying zone");
      break;
    case 0x45:
      alarmType = QObject::tr("Winch launching area");
      break;
    case 0x46:
      alarmType = QObject::tr("RC flying area");
      break;
    case 0x47:
      alarmType = QObject::tr("UAS flying area");
      break;
    case 0x48:
      alarmType = QObject::tr("Acrobatic box");
      break;
    case 0x7e:
      alarmType = QObject::tr("Generic danger area");
      break;
    case 0x7f:
      alarmType = QObject::tr("Generic prohibited area");
      break;
    default:
      alarmType = QObject::tr("Other alert zone");
  }

  return alarmType;
}

QString FlarmBase::translateAcftType( const short acftTypeIn )
{
  QString acftType;

  switch( acftTypeIn )
   {
     case 1:
       acftType = QObject::tr("Glider/TMG");
       break;
     case 2:
       acftType = QObject::tr("Tow plane");
       break;
     case 3:
       acftType = QObject::tr("Helicopter");
       break;
     case 4:
       acftType = QObject::tr("Skydiver");
       break;
     case 5:
       acftType = QObject::tr("Drop plane");
       break;
     case 6:
       acftType = QObject::tr("Hangglider");
       break;
     case 7:
       acftType = QObject::tr("Paraglider");
       break;
     case 8:
       acftType = QObject::tr("Motor Aircraft");
       break;
     case 9:
       acftType = QObject::tr("Jet Aircraft");
       break;
     case 0xA:
       acftType = QObject::tr("unknown");
       break;
     case 0xB:
       acftType = QObject::tr("Balloon");
       break;
     case 0xC:
       acftType = QObject::tr("Airship");
       break;
     case 0xD:
       acftType = QObject::tr("Drone");
       break;
     case 0xF:
       acftType = QObject::tr("Obstacle");
       break;
     default:
       break;
   }

  return acftType;
}
