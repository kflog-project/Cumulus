/***********************************************************************
**
**   BluetoothDiscovery.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2022 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class BluetoothDiscovery
 *
 * \author Axel Pauli
 *
 * \brief A class for retrieving of reachable bluetooth devices
 *
 * This class uses the bluetooth QT services to retrieve the paired and possible
 * reachable bluetooth devices. The found devices or an error description
 * are emitted via a signal to the caller. This class is realized as a thread
 * because scanning of bluetooth devices can take some time and shall not block
 * other GUI activities.
 *
 * \date 2022
 */

#pragma once

#include <QObject>
#include <QString>
#include <QBluetoothServiceDiscoveryAgent>

#include "datatypes.h"

class BluetoothDiscovery : public QObject
{
  Q_OBJECT

 public:

  BluetoothDiscovery( QObject *parent = nullptr );

  virtual ~BluetoothDiscovery();

  /**
   * Number of created instances of this class.
   *
   * \return Current number of class instances.
   */
  static int getNoOfInstances()
  {
    return noOfInstances;
  }

  /**
   * That is the start method of the BT discovery agent.
   */
  void start( QBluetoothServiceDiscoveryAgent::DiscoveryMode mode =
                      QBluetoothServiceDiscoveryAgent::MinimalDiscovery );

  /**
   * That stops the BT discovery agent.
   */
  void stop();

  /**
   * @return Returns a list with the stored BT services from the last scan.
   */
  QList<QBluetoothServiceInfo> getBtServices() const;

 private slots:

  void slotCanceled();

  void slotError( QBluetoothServiceDiscoveryAgent::Error error );

  void slotFinished();

  void slotServiceDiscovered( const QBluetoothServiceInfo &service );

 signals:

  /**
  * This signal emits the results of the bluetooth device scan and the done
  * user selection.
  *
  * \param ok True is a BT device has been found and selected.
  *
  * \param error An error string, if ok is false.
  *
  * \param devices List of found bluetooth devices.
  */
  void foundBtDevices( bool ok, QString& error, QList<QBluetoothServiceInfo>& btsi );

 private:

  static int noOfInstances;

  // BT agent
  QBluetoothServiceDiscoveryAgent *discoveryAgent;
};
