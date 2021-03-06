/***********************************************************************
**
**   flarmbase.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2017 Axel Pauli
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

  // An additional asterisk in the NMEA sentence is not accepted by Flarm.
  array = array.replace( "*", "." );

  return array;
}

QString FlarmBase::translateAlarmType( const short hexType )
{
  QString alarmType;

  switch( hexType )
  {
    case 0:
    case 1:
    case 2:
      alarmType = QObject::tr("Traffic");
      break;
    case 3:
      alarmType = QObject::tr("Obstacle");
      break;
    case 4:
      alarmType = QObject::tr("Info Alert");
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
