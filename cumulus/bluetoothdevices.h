/***********************************************************************
**
**   bluetoothdevices.h
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
 * \class BluetoothDevices
 *
 * \author Axel Pauli
 *
 * \brief A class for retrieving of reachable bluetooth devices
 *
 * This class uses the bluetooth library function hci_inquiry to retrieve the
 * reachable bluetooth devices. The found devices or an error description
 * are emitted via a signal to the caller. This class is realized as a thread
 * because scanning of bluetooth devices can take some time and shall not block
 * other GUI activities.
 *
 * \date 2010
 */

#ifndef BLUETOOTH_DEVICES_H
#define BLUETOOTH_DEVICES_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>

#include "datatypes.h"

class BluetoothDevices : public QThread
{
  Q_OBJECT

 public:

  BluetoothDevices( QObject *parent = 0 );

  virtual ~BluetoothDevices();

  /**
   * Number of created instances of this class.
   *
   * \return Current number of class instances.
   */
  static int getNoOfInstances()
  {
    return noOfInstances;
  }

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 private slots:

  /** Called to delete the thread. */
  void slot_Destroy();

  /**
   * Starts the Bluetooth device search.
   */
  void slot_RetrieveBtDevice();

 signals:

  /**
  * This signal emits the results of the bluetooth device scan and the done
  * user selection.
  *
  * \param ok True is a BT device has been found and selected.
  *
  * \param error An error string, if ok is false.
  *
  * \param devices Found bluetooth devices. Key is the logical name,
  *                value is the bluetooth address.
  */
  void retrievedBtDevices( bool ok, QString error, BtDeviceMap devices );

 private:

  static int noOfInstances;

  static QMutex mutex;
};

#endif

