/***********************************************************************
**
**   datatypes.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 by Axel Pauli (axel@kflog.org)
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
 */

#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

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

#endif // DATA_TYPES_H_
