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

QHash<QString, FlarmBase::FlarmAcft> FlarmBase::m_pflaaHash;
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
