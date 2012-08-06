/***********************************************************************
**
**   flarmbase.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
