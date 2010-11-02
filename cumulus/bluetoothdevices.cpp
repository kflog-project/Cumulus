/***********************************************************************
**
**   bluetoothdevices.cpp
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <QtGui>

#include "mapview.h"
#include "bluetoothdevices.h"
#include "generalconfig.h"

extern MapView *_globalMapView;

// set static member variable
QMutex BluetoothDevices::mutex;

// set static member variable
int BluetoothDevices::noOfInstances = 0;

BluetoothDevices::BluetoothDevices( QObject *parent ) : QThread( parent )
{
  noOfInstances++;

  setObjectName( "BluetoothDevices" );

  // Activate self destroy after finish signal has been caught.
  connect( this, SIGNAL(finished()), this, SLOT(slot_Destroy()) );
}

BluetoothDevices::~BluetoothDevices()
{
  noOfInstances--;
}

void BluetoothDevices::run()
{
  qDebug() << "BT run() entry, Tid=" << QThread::currentThreadId();

  QTimer timer;
  timer.setSingleShot( true );

  // @AP Note! The flag Qt::DirectConnection is the most important one,
  //  otherwise the wrong thread is connected.
  connect( &timer, SIGNAL(timeout()),
           this, SLOT(slot_RetrieveBtDevice()),
           Qt::DirectConnection );

  timer.start(0);

  // Starts the event loop of this thread.
  exec();

  qDebug() << "BT run() exit, Tid=" << QThread::currentThreadId();
}

void BluetoothDevices::slot_RetrieveBtDevice()
{
  qDebug() << "slot_RetrieveBtDevice() in"
           << objectName()
           << QThread::currentThreadId();

  // Device map where the key is the BT name and the value is the BT address.
  BtDeviceMap btDevices;

  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &mutex );

  // The following code is taken from:
  // http://people.csail.mit.edu/albert/bluez-intro
  // A special thank you to the author!

  // Get the device identifier of the local default adapter
  int devId = hci_get_route(0);

  QString error;

  if( devId < 0 )
    {
      error = QObject::tr("Bluetooth Service is offline!");

      qWarning() << error << strerror(errno);

      emit retrievedBtDevices( false, error, btDevices );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  struct hci_dev_info di;

  if( hci_devinfo(devId, &di) < 0 )
    {
       qDebug() << "Error" << errno << "hci_devinfo:" << strerror(errno);
    }

  int len = 8;
  int max_rsp = 0;
  long flags = IREQ_CACHE_FLUSH;
  uint8_t lap[3] = { 0x33, 0x8b, 0x9e };

  inquiry_info *info = static_cast<inquiry_info *> (0);

  int num_rsp = 0;

  num_rsp = hci_inquiry( devId, len, max_rsp, lap, &info, flags );

  if( num_rsp < 0 )
    {
      error = QObject::tr("Bluetooth Scan failed!");

      qWarning() << error << strerror(errno);

      emit retrievedBtDevices( false, error, btDevices );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  int btSocket = hci_open_dev( devId );

  if( btSocket < 0 )
    {
      error = QObject::tr("Bluetooth Service is offline!");

      qWarning() << error << strerror(errno);

      emit retrievedBtDevices( false, error, btDevices );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  char addr[18];
  char name[249];

  for( int i = 0; i < num_rsp; i++ )
    {
      memset( addr, 0, sizeof(addr) );
      memset( name, 0, sizeof(name) );

      ba2str( &(info+i)->bdaddr, addr );

      // Translate bt address into an human name.
      if( hci_read_remote_name( btSocket, &(info+i)->bdaddr, sizeof(name), name, 25000 ) < 0 )
        {
          // No name available for BT address.
          btDevices.insert( QString(addr), QString(addr) );
        }
      else
        {
          btDevices.insert( QString(name), QString(addr) );
        }

      qDebug() << "BT-Name:"  << name << "BT-Address:" << addr;
  }

  if( info ) free( info );
  hci_close_dev( btSocket );

  if( btDevices.isEmpty() )
    {
      error = QObject::tr("Please switch on your BT GPS!");
      emit retrievedBtDevices( false, error, btDevices );
    }
  else
    {
      emit retrievedBtDevices( true, error, btDevices );
    }

  // Stop the event loop and destroy this thread.
  quit();
  return;
}

void BluetoothDevices::slot_Destroy()
{
  // deletes the thread object after execution is finished
  delete this;
}
