/***********************************************************************
**
**   BluetoothServiceDiscovery.h
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

#include <BluetoothServiceDiscovery.h>
#include <QtCore>

// set static member variable
int BluetoothServiceDiscovery::noOfInstances = 0;

BluetoothServiceDiscovery::BluetoothServiceDiscovery( QObject *parent ) : QObject( parent )
{
  if( noOfInstances > 0 )
    {
      return;
    }

  noOfInstances++;

  setObjectName( "BluetoothServiceDiscovery" );

  // Create a discovery agent and connect to its signals
  discoveryAgent = new QBluetoothServiceDiscoveryAgent( parent );

  connect( discoveryAgent, SIGNAL( serviceDiscovered(const QBluetoothServiceInfo &) ),
           this, SLOT( slotServiceDiscovered(const QBluetoothServiceInfo &)) );

  // catch finish signal
  connect( discoveryAgent, SIGNAL(finished()), this, SLOT(slotFinished()) );

  // catch cancel signal
  connect( discoveryAgent, SIGNAL(canceled()), this, SLOT(slotCanceled()) );

  // catch error signal
  connect( discoveryAgent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error )),
           this, SLOT(slotError(QBluetoothServiceDiscoveryAgent::Error )) );
}

BluetoothServiceDiscovery::~BluetoothServiceDiscovery()
{
  noOfInstances--;
}

/**
 * That is the start method of the BT discovery agent.
 */
void BluetoothServiceDiscovery::start( QBluetoothServiceDiscoveryAgent::DiscoveryMode mode )
{
  if( discoveryAgent->isActive() == true )
    {
      return;
    }

  discoveryAgent->clear();
  discoveryAgent->start( mode );
}

/**
 * That stops the BT discovery agent.
 */
void BluetoothServiceDiscovery::stop()
{
  if( discoveryAgent->isActive() == true )
    {
      discoveryAgent->stop();
    }
}

QList<QBluetoothServiceInfo> BluetoothServiceDiscovery::getBtServices() const
{
  return discoveryAgent->discoveredServices();
}

void BluetoothServiceDiscovery::slotError( QBluetoothServiceDiscoveryAgent::Error error )
{
  qDebug() << "BluetoothServiceDiscovery::slotError()" << error
           << discoveryAgent->errorString();
}

void BluetoothServiceDiscovery::slotCanceled()
{
  qDebug() << "BluetoothServiceDiscovery::slotCanceled()";
}

void BluetoothServiceDiscovery::slotFinished()
{
  qDebug() << "BluetoothServiceDiscovery::slotFinished()";

  bool result = true;
  QString error = discoveryAgent->errorString();

  // Check for error
  if( discoveryAgent->error() != QBluetoothServiceDiscoveryAgent::NoError )
    {
      result = false;

      qDebug() << "BluetoothServiceDiscovery::slotFinished()"
               << discoveryAgent->error()
               << error;
    }

  QList<QBluetoothServiceInfo> btsi = discoveryAgent->discoveredServices();

  // Publish the found BT devices.
  emit foundBtDevices( result, error, btsi );
}

void BluetoothServiceDiscovery::slotServiceDiscovered( const QBluetoothServiceInfo &service )
{
  qDebug() << "Found new service:"
           << "Service:" << service.serviceName()
           << "Device Name:" << service.device().name()
           << "Address:" << '(' << service.device().address().toString() << ')';
}
