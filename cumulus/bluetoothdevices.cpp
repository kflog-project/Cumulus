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
  connect( this, SIGNAL(finished()), this, SLOT(slot_destroy()) );
}

BluetoothDevices::~BluetoothDevices()
{
  noOfInstances--;
}

void BluetoothDevices::run()
{
  // Start this timer to continue the processing after stepping into the
  // thread's event loop.
  QTimer::singleShot( 1000, this, SLOT(slot_RetrieveBtDevice()) );

  // Starts the event loop of this thread.
  exec();
}

void BluetoothDevices::slot_RetrieveBtDevice()
{
  qDebug() << "slot_RetrieveBtDevice() in" << objectName();

  // Set a global lock during execution to avoid calls in parallel.
  QMutexLocker locker( &mutex );

  // The following code is taken from:
  // http://people.csail.mit.edu/albert/bluez-intro
  // A special thank you to the author!

  // Get the device identifier of the local default adapter
  int devId = hci_get_route(0);

  int btSocket = hci_open_dev( devId );

  QString result;

  if( devId < 0 || btSocket < 0 )
    {
      result = QObject::tr("Bluetooth Service is offline!");

      qWarning() << result << strerror(errno);

      emit retrievedBtDevice( false, result );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  int len = 8;
  int max_rsp = 255;
  long flags = IREQ_CACHE_FLUSH;

  inquiry_info *ii = (inquiry_info *) malloc( max_rsp * sizeof(inquiry_info) );

  int num_rsp = hci_inquiry( devId, len, max_rsp, 0, &ii, flags );

  if( num_rsp < 0 )
    {
      result = QObject::tr("Bluetooth Scan failed!");

      qWarning() << result << strerror(errno);

      emit retrievedBtDevice( false, result );

      // Stop the event loop and destroy this thread.
      free( ii );
      quit();
      return;
    }

  // Device map where the key is the BT name and the value is the BT address.
  QMap<QString, QString> knownDevices;

  char addr[19];
  char name[248];

  for( int i = 0; i < num_rsp; i++ )
    {
      memset( addr, 0, sizeof(addr) );
      memset( name, 0, sizeof(name) );

      ba2str( &(ii+i)->bdaddr, addr );

      if( hci_read_remote_name( btSocket, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0 )
        {
          // No name available for BT address.
          knownDevices.insert( QString(addr), QString(addr) );
        }
      else
        {
          knownDevices.insert( QString(name), QString(addr) );
        }

      qDebug() << "BT-Name:"  << name << "BT-Address:" << addr;
  }

  free( ii );
  hci_close_dev( btSocket );

  if( knownDevices.isEmpty() )
    {
      result = QObject::tr("Please switch on your BT GPS!");
      emit retrievedBtDevice( false, result );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  QString lastBtDevice = GeneralConfig::instance()->getGpsBtDevice();

  QStringList items( knownDevices.keys() );

  // Try to preselect a previous used BT GPS device
  int no = 0;

  if( ! lastBtDevice.isEmpty() )
    {
      for( int i = 0; i < items.size(); i++ )
        {
          if( items[i] == lastBtDevice )
            {
              no = i;
              break;
            }
        }
    }

  QString item = QInputDialog::getItem( _globalMapView,
                                        QObject::tr( "Select BT Adapter" ),
                                        QObject::tr( "BT Adapter:" ),
                                        items, no, false );
  if( item.isEmpty() )
    {
      result = "Empty line error!";
      emit retrievedBtDevice( false, result );

      // Stop the event loop and destroy this thread.
      quit();
      return;
    }

  // Get BT address from device map.
  result = knownDevices.value( item );

  // Save last selected BT device.
  GeneralConfig::instance()->setGpsBtDevice( item );

  emit retrievedBtDevice( true, result );

  // Stop the event loop and destroy this thread.
  quit();
  return;
}

void BluetoothDevices::slot_destroy()
{
  // deletes the thread object after execution is finished
  delete this;
}
