/***********************************************************************
**
**   flarmbase.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2015 Axel Pauli
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
FlarmBase::FlarmVersion FlarmBase::m_flarmVersion;
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

  return array;
}

QString FlarmBase::FlarmAlertZone::translateAlertZoneType( const short hexType )
{
  QString azt;

  switch( hexType )
  {
    case 0x41:
      azt = QObject::tr("Skydiver drop zone");
      break;
    case 0x42:
      azt = QObject::tr("Aerodrom traffic zone");
      break;
    case 0x43:
      azt = QObject::tr("Military firing area");
      break;
    case 0x44:
      azt = QObject::tr("Kite flying zone");
      break;
    case 0x45:
      azt = QObject::tr("Winch launching area");
      break;
    case 0x46:
      azt = QObject::tr("RC flying area");
      break;
    case 0x47:
      azt = QObject::tr("UAS flying area");
      break;
    case 0x48:
      azt = QObject::tr("Acrobatic box");
      break;
    case 0x7e:
      azt = QObject::tr("Generic danger area");
      break;
    case 0x7f:
      azt = QObject::tr("Generic prohibited area");
      break;
    default:
      azt = QObject::tr("Other alert zone");
  }

  return azt;
}

