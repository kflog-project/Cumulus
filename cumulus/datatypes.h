/***********************************************************************
**
**   datatypes.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief A class defining QT user data types.
 *
 *  A class defining QT user data types usable in queued connections of
 *  threads.
 *
 *  \date 2010-2014
 *
 *  \version $Id$
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

//------------------------------------------------------------------------------

#ifdef BLUEZ

#include <QMetaType>
#include <QString>
#include <QMap>

/**
 * Special data type to return the found Bluetooth devices to the GUI thread
 * via a signal from another running thread.
 * The map key is the logical Bluetooth device name, the map value is the
 * Bluetooth address belonging to key.
 */
typedef QMap<QString, QString> BtDeviceMap;

Q_DECLARE_METATYPE(BtDeviceMap)

#endif

//------------------------------------------------------------------------------

#include "airfield.h"
#include "airspace.h"
#include "radiopoint.h"

/**
 * Special data type to return the loaded airfield data list to the GUI thread.
 */
typedef QList<Airfield> AirfieldList;

Q_DECLARE_METATYPE(AirfieldList)

typedef QList<Airfield>* AirfieldListPtr;

Q_DECLARE_METATYPE(AirfieldListPtr)

typedef QList<RadioPoint>* RadioListPtr;

Q_DECLARE_METATYPE(RadioListPtr)

/**
 * Special data type to return the loaded airspace data list to the GUI thread.
 */
typedef QList<Airspace*>* AirspaceListPtr;

Q_DECLARE_METATYPE(AirspaceListPtr)

//------------------------------------------------------------------------------

#endif // DATA_TYPES_H
