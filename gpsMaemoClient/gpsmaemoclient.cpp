/***********************************************************************
**
**   gpsmaemoclient.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2010 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

using namespace std;

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <QtCore>

#include "gpsmaemoclient.h"
#include "protocol.h"
#include "ipc.h"

#ifdef DEBUG
#undef DEBUG
#endif

// Switch this on for permanent error logging. That will display
// all open and reconnect failures. Because open and reconnect
// is done periodically you will get a lot of such message in error
// case.
#ifdef ERROR_LOG
#undef ERROR_LOG
#endif

// Size of internal message queue.
#define QUEUE_SIZE 500

// Defines alive check timeout in ms.
#define ALIVE_TO 30000

// Defines a retry timeout in ms which is used after a failed connect to the GPS.
// daemon. The time should not be to short otherwise Cumulus is
// most of the time blocked by the connect action.
#define RETRY_TO 60000

// global object pointer to this class
GpsMaemoClient *GpsMaemoClient::instance = static_cast<GpsMaemoClient *> (0);

/*
 * The Maemo5 location service can emit four signal, which are handled by the
 * next four static pure C-functions.
 *
 * See Maemo5: http://wiki.maemo.org/Documentation/Maemo_5_Developer_Guide/Using_Connectivity_Components/Using_Location_API
 *
 * for more information.
 */

/**
 * Is called from location service when GPSD is running.
 */
static void LocationCb::gpsdRunning( LocationGPSDControl* control,
                                     gpointer userData )
{
  qDebug("G-Signal->GPSD Running");

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdRunning();
    }
}

/**
 * Is called from location service when GPSD was stopped.
 */
static void LocationCb::gpsdStopped( LocationGPSDControl* control,
                                     gpointer userData )
{
  qDebug("G-Signal->GPSD Stopped");

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdStopped();
    }
}

/**
 * Is called from location service when GPSD was not startable.
 */
static void LocationCb::gpsdErrorVerbose( LocationGPSDControl *control,
                                          LocationGPSDControlError error,
                                          gpointer user_data )
{
  switch( error )
    {
    case LOCATION_ERROR_USER_REJECTED_DIALOG:
      qWarning( "GpsdError: User didn't enable requested methods" );
      break;
    case LOCATION_ERROR_USER_REJECTED_SETTINGS:
      qWarning( "GpsdError: User changed settings, which disabled location" );
      break;
    case LOCATION_ERROR_BT_GPS_NOT_AVAILABLE:
      qWarning( "GpsdError: Problems with BT GPS" );
      break;
    case LOCATION_ERROR_METHOD_NOT_ALLOWED_IN_OFFLINE_MODE:
      qWarning( "GpsdError: Requested method is not allowed in offline mode" );
      break;
    case LOCATION_ERROR_SYSTEM:
      qWarning( "GpsdError: System error" );
      break;
    }

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdError();
    }
}

/**
 * Is called from location service when new gps data are available.
 */
static void LocationCb::gpsdLocationchanged( LocationGPSDevice *device,
                                             gpointer user_data )
{
  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdLocationchanged( device );
    }
}


// Constructor requires a socket port of the server (listening end point)
// useable for interprocess communication. As related host is always localhost
// used. It will be opened two sockets to the server, one for data transfer,
// the other only as notification channel.
GpsMaemoClient::GpsMaemoClient( const ushort portIn )
{
  if ( instance  )
    {
      qWarning("GpsMaemo::GpsMaemo(): You can create only one class instance!");
      return;
    }

  ipcPort         = portIn;
  gpsIsRunning    = false;
  notify          = false;
  connectionLost  = true;
  shutdown        = false;
  timeSpan        = 0;
  instance        = this;

  // establish a connection to the server
  if( ipcPort )
    {
      if( clientData.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          cerr << "Command channel Connection to Cumulus Server failed!"
               << " Fatal error, terminate process!" << endl;

          setShutdownFlag(true);
          return;
        }

      if( clientNotif.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          cerr << "Notification channel Connection to Cumulus Server failed!"
               << " Fatal error, terminate process!" << endl;

          setShutdownFlag(true);
          return;
        }
    }

  // Get the GPSD control object from the location service.
  control = location_gpsd_control_get_default();

  if( ! control )
    {
      qWarning( "GpsMaemoClient(): No GPSD control object returned" );
    }
  else
    {
      // Set preferred-method in control
      g_object_set( G_OBJECT(control), "preferred-method",
                    LOCATION_METHOD_GNSS, NULL );

      // Set preferred-interval in control to one second
      g_object_set(G_OBJECT(control), "preferred-interval",
                   LOCATION_INTERVAL_1S, NULL);

      // Subscribe to location service signals. That must be done only once!
      g_signal_connect( control, "error-verbose", G_CALLBACK(LocationCb::gpsdErrorVerbose), NULL );
      g_signal_connect( control, "gpsd_stopped",  G_CALLBACK(LocationCb::gpsdStopped), NULL );
      g_signal_connect( control, "gpsd_running",  G_CALLBACK(LocationCb::gpsdRunning), NULL );
    }

  // Setup device instance
  device = (LocationGPSDevice *) g_object_new(LOCATION_TYPE_GPS_DEVICE, NULL);

  // Subscribe to location service signal.
  g_signal_connect( device, "changed", G_CALLBACK(LocationCb::gpsdLocationchanged), NULL );

  // Set last time to current time.
  last.start();
}

GpsMaemoClient::~GpsMaemoClient()
{
  if( gpsIsRunning == true )
    {
      // Here we have to stop the GPSD.
      location_gpsd_control_stop( control );

      // Give location service time for stopping of GPSD otherwise
      // we will get problems during the next connection try.
      sleep(2);
    }

  g_object_unref( device );
  g_object_unref( control );

  instance = static_cast<GpsMaemoClient *> (0);
}

/*
 * Wrapper method to handle GLib signal emitted by the
 * location service.
 */
void GpsMaemoClient::handleGpsdRunning()
{
  gpsIsRunning   = true;
  connectionLost = false;

  // restart timer with alive check supervision
  startTimer(ALIVE_TO);
}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdStopped()
{
  gpsIsRunning   = false;
  connectionLost = true;
}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdError()
{
  qWarning( "GpsMaemo::handleGpsdError(): Location service said GPSD Error, trying restart" );

  gpsIsRunning   = false;
  connectionLost = true;

  // stop GPSD in error case
  location_gpsd_control_stop( control );

  // setup timer for restart
  startTimer(RETRY_TO);
}

/**
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdLocationchanged( LocationGPSDevice *device )
{
  // New GPS data available.




  // update supervision timer/variables
  startTimer(ALIVE_TO);
  connectionLost = false;
}

/**
 * Return all currently used read file descriptors as mask, useable by the
 * select call
 */
fd_set *GpsMaemoClient::getReadFdMask()
{
  // Reset file descriptor mask bits
  FD_ZERO( &fdMask );

  if( ipcPort ) // data channel to server
    {
      int sfd = clientData.getSock();

      if( sfd != -1 )
        {
          FD_SET( sfd, &fdMask );
        }
    }

  return &fdMask;
}

// Processes incoming read events. They can come from the server.
void GpsMaemoClient::processEvent( fd_set *fdMask )
{
  if( ipcPort )
    {
      int sfd = clientData.getSock();

      if( sfd != -1 && FD_ISSET( sfd, fdMask ) )
        {
          readServerMsg();
        }
    }
}

/**
* The Maemo5 location service is called to start the selected devices. That
* can be the internal GPS or a BT GPS mouse.
*/
bool GpsMaemoClient::startGpsReceiving()
{
  if( gpsIsRunning == true )
    {
      qWarning( "GpsMaemoClient::startGpsReceiving(): GPSD is running, ignore request!" );
      return true;
    }

  // remove all old queued messages
  queue.clear();

  // setup alive check, that guarantees a restart after unsuccessful start
  startTimer(ALIVE_TO);

  // Start GPSD, results are emitted by signals.
  location_gpsd_control_start( control );

  return true;
}

/**
 * The Maemo5 location service is called to stop GPS receiving.
 */
void GpsMaemoClient::stopGpsReceiving()
{
  // Stop timer
  timeSpan = 0;

  // Shutdown GPSD, if we have started it.
  if( gpsIsRunning == true )
    {
       location_gpsd_control_stop( control );
    }
}

// timeout controller
void GpsMaemoClient::toController()
{
  if( timeSpan == 0 )
    {
      // Do nothing if time span is set to zero.
      return;
    }

  if( last.elapsed() > timeSpan )
    {
      // Reset timer
      timeSpan = 0;

      if( connectionLost == false )
        {
          // connection is lost, send only one message to the server
          connectionLost = true;
          queueMsg( MSG_CONLOST );
        }

      if( gpsIsRunning == false )
        {
          qWarning() << "GpsMaemoClient::toController():"
                     << "Connection to GPS seems to be dead, trying restart.";

          // GPS is not running, try a restart of location service.
          startGpsReceiving();
          return;
        }

      // Stop GPSD due to timeout and initiate a restart.
      stopGpsReceiving();

      // setup timer for restart
      startTimer(RETRY_TO);
    }
}

// Reads a server message from the socket. The protocol consists of
// two parts. First the message length is read as unsigned
// integer, after that the actual message as 8 bit character string.

void GpsMaemoClient::readServerMsg()
{
  uint msgLen = 0;

  uint done = clientData.readMsg( &msgLen, sizeof(msgLen) );

  if( done <= 0 )
    {
      clientData.closeSock();
      return; // Error occurred
    }

  if( msgLen > 256 )
    {
      // such messages length are not defined. we will ignore that.
      qWarning() << "GpsMaemoClient::readServerMsg():"
                 << "message" << msgLen << "too large, ignoring it!";
      return;
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = clientData.readMsg( buf, msgLen );

  if( done <= 0 )
    {
      clientData.closeSock();
      delete [] buf;
      buf = 0;
      return; // Error occurred
    }

#ifdef DEBUG
   cout << "GpsMaemoClient::readServerMsg(): Received Message: " << buf << endl;
#endif

  // Split the received message into its single parts. Space is used
  // as separator.

  QString qbuf( buf );
  QStringList args = qbuf.split(" ");
  delete [] buf;
  buf = 0;

  // look, what server is requesting

  if( MSG_GM == args[0] )
    {
      // Get message is requested. We take the oldest element out of the
      // queue, if there is any.
      if( queue.count() == 0 ) // queue empty
        {
          writeServerMsg( MSG_NEG );
          return;
        }

      QByteArray msg = queue.dequeue();
      QByteArray res = QByteArray(MSG_RM) + " " + msg;

      writeServerMsg( res.data() );
      msg=0;
    }
  else if( MSG_MAGIC == args[0] )
    {
      // check protocol versions, reply with pos or neg
      if( MSG_PROTOCOL != args[1] )
        {
          cerr << "GpsMaemoClient::readServerMsg(): "
               << "Message protocol versions are incompatible, "
               << "closes connection and shutdown client" << endl;

          writeServerMsg( MSG_NEG );
          setShutdownFlag(true);
          return;
        }

      writeServerMsg( MSG_POS );
    }
  else if( MSG_OPEN == args[0] )
    {
      // Initialization of GPSD location device is requested.
      bool res = startGpsReceiving();

      if( res )
        {
          writeServerMsg( MSG_POS );
        }
      else
        {
          writeServerMsg( MSG_NEG );
        }
    }
  else if( MSG_CLOSE == args[0] )
    {
      // Close Gps device is requested
      stopGpsReceiving();
      writeServerMsg( MSG_POS );
    }
  else if( MSG_NTY == args[0] )
    {
      // A notification shall be sent, if new data are available. This is only
      // valid for one notification. After notification is sent, the server
      // must renew its request.
      notify = true;
      writeServerMsg( MSG_POS );
    }
  else if( MSG_SM == args[0] && args.count() == 2 )
    {
      // Sent message to the GPS device is not supported.
      writeServerMsg( MSG_NEG );
    }
  else if( MSG_SHD == args[0] )
    {
      // Shutdown is requested by the server. This message will not be
      // acked!
      clientData.closeSock();
      clientNotif.closeSock();
      setShutdownFlag(true);
    }
  else
    {
      cerr << "GpsMaemoClient::readServerMsg(): "
           << "Unknown message received: " << buf << endl;
      writeServerMsg( MSG_NEG );
    }

  delete [] buf;
  return;
}

// Send a message via data socket to the server. The protocol consists of two
// parts. First the message length is transmitted as unsigned integer, after
// that the actual message as 8 bit character string.

void GpsMaemoClient::writeServerMsg( const char *msg )
{
  uint msgLen = strlen( msg );

  int done = clientData.writeMsg( (char *) &msgLen, sizeof(msgLen) );

  done = clientData.writeMsg( (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, close socket
      clientData.closeSock();
    }

  return;
}

// Sent a message via notification socket to the server. The protocol consists
// of two parts. First the message length is transmitted as unsigned integer,
// after that the actual message as 8 bit character string.

void GpsMaemoClient::writeNotifMsg( const char *msg )
{
  static QString method = "GpsMaemoClient::writeNotifMsg():";

  uint msgLen = strlen( msg );

  int done = clientNotif.writeMsg( (char *) &msgLen, sizeof(msgLen) );

  done = clientNotif.writeMsg( (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, close socket
      clientNotif.closeSock();
    }

#ifdef DEBUG
  cout << method.toLatin1().data() << " " << msg << endl;
#endif

  return;
}

// put a new message into the process queue and sent a notification to
// the server, if option notify is true. The notification is sent
// only once to avoid a flood of them, if server is busy.

void GpsMaemoClient::queueMsg( const char* msg )
{
  queue.enqueue( msg );

  if( queue.count() > QUEUE_SIZE )
    {
      // start dequeuing, to avoid memory overflows
      queue.dequeue ();

      qWarning() << "GpsMaemoClient::queueMsg: Max. queue size of" << QUEUE_SIZE
                 << "reached, remove oldest element!" << endl;
    }

  if( notify )
    {
      // inform server about new messages available, if not already
      // done.
      writeNotifMsg( MSG_DA );
      notify = false;
    }
}

/** Setup timeout controller. */
void GpsMaemoClient::startTimer( uint milliSec )
{
  last.start();
  timeSpan = milliSec;
}
