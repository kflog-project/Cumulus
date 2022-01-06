/***********************************************************************
**
**   BluetoothDeviceDiscovery.h
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

#include <QtCore>

#include "BluetoothDeviceDiscovery.h"

// set static member variable
int BluetoothDeviceDiscovery::noOfInstances = 0;

BluetoothDeviceDiscovery::BluetoothDeviceDiscovery( QObject *parent ) :
  QObject( parent )
{
  if( noOfInstances > 0 )
    {
      return;
    }

  noOfInstances++;

  setObjectName( "BluetoothDeviceDiscovery" );

  // Create a discovery agent and connect to its signals
  discoveryAgent = new QBluetoothDeviceDiscoveryAgent( parent );

  connect( discoveryAgent, SIGNAL( deviceDiscovered(const QBluetoothDeviceInfo &)),
           this, SLOT( slotDeviceDiscovered(const QBluetoothDeviceInfo &)) );

  // catch finish signal
  connect( discoveryAgent, SIGNAL(finished()), this, SLOT(slotFinished()) );

  // catch cancel signal
  connect( discoveryAgent, SIGNAL(canceled()), this, SLOT(slotCanceled()) );

  // catch error signal
  connect( discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)) ,
           this, SLOT(slotError(QBluetoothDeviceDiscoveryAgent::Error)) );
}

BluetoothDeviceDiscovery::~BluetoothDeviceDiscovery()
{
  noOfInstances--;
}

/**
 * That is the start method of the BT discovery agent.
 */
void BluetoothDeviceDiscovery::start( QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods )
{
  if( discoveryAgent->isActive() == true )
    {
      return;
    }

  discoveryAgent->start( methods );
}

/**
 * That stops the BT discovery agent.
 */
void BluetoothDeviceDiscovery::stop()
{
  if( discoveryAgent->isActive() == true )
    {
      discoveryAgent->stop();
    }
}

QList<QBluetoothDeviceInfo> BluetoothDeviceDiscovery::getBtDevices() const
{
  return discoveryAgent->discoveredDevices();
}

void BluetoothDeviceDiscovery::slotError( QBluetoothDeviceDiscoveryAgent::Error error )
{
  qDebug() << "BluetoothDeviceDiscovery::slotError()" << error
           << discoveryAgent->errorString();
}

void BluetoothDeviceDiscovery::slotCanceled()
{
  qDebug() << "BluetoothDeviceDiscovery::slotCanceled()";
}

void BluetoothDeviceDiscovery::slotFinished()
{
  qDebug() << "BluetoothDeviceDiscovery::slotFinished()";

  bool result = true;
  QString error = discoveryAgent->errorString();

  // Check for error
  if( discoveryAgent->error() != QBluetoothDeviceDiscoveryAgent::NoError )
    {
      result = false;

      qDebug() << "BluetoothDeviceDiscovery::slotFinished()"
               << discoveryAgent->error()
               << error;
    }

  QList<QBluetoothDeviceInfo> btdd = discoveryAgent->discoveredDevices();

  // Publish the found BT devices.
  emit foundBtDevices( result, error, btdd );
}

void BluetoothDeviceDiscovery::slotDeviceDiscovered( const QBluetoothDeviceInfo &device )
{
  qDebug() << "Found new BT device:"
           << "Device Name:" << device.name()
           << "Address:" << '(' << device.address().toString() << ')';
}
