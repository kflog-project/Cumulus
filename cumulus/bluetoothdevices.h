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
 * \author Axel Pauli
 *
 * \brief A class for retrieving of reachable bluetooth devices
 *
 * This class uses the bluetooth hcitool command line tool to retrieve the
 * reachable bluetooth devices. If devices are found, the user has to select one
 * device from a combo box dialog. The results are emitted via a signal. This
 * class is realized as a thread because scanning of bluetooth devices can take
 * some time and shall not block other GUI activities.
 */

#ifndef BLUETOOTH_DEVICES_H
#define BLUETOOTH_DEVICES_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QString>

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
  void slot_destroy();

  /**
   * Helper method, which is called by a timer signal. The timer is setup in the
   * run method of this thread, to call this method after timeout has expired.
   * So the run method can step into the event loop, which is needed for
   * event distributing and processing.
   */
  void slot_RetrieveBtDevice();

 signals:

  /**
  * This signal emits the results of the bluetooth device scan and the done
  * user selection.
  *
  * \param ok True is a BT device has been found and selected.
  *
  * \param btAddress Address of select BT device if ok is true, otherwise
  *                  it contains an error text.
  */
  void retrievedBtDevice( bool ok, QString& btAddress );

 private:

  static int noOfInstances;

  static QMutex mutex;
};

#endif

