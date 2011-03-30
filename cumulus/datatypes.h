/***********************************************************************
**
**   datatypes.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2011 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
 *  \date 2010-2011
 *
 *  \version $Id$
 */

#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

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

#ifdef WELT2000_THREAD

#include "airfield.h"

/**
 * Special data type to return the loaded airfield data lists of Welt2000 to
 * another thread.
 */
typedef QList<Airfield> AirfieldList;

Q_DECLARE_METATYPE(AirfieldList)

typedef QList<Airfield>* AirfieldListPtr;

Q_DECLARE_METATYPE(AirfieldListPtr)

#endif

//------------------------------------------------------------------------------

#endif // DATA_TYPES_H_
