/***********************************************************************
**
** gpsmaemoclient.cpp
**
** This file is part of Cumulus
**
************************************************************************
**
** Copyright (c): 2010 by Axel Pauli (axel@kflog.org)
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** $Id$
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

// #define DEBUG 1

// Size of internal message queue.
#define QUEUE_SIZE 250

// Defines alive check timeout in ms.
#define ALIVE_TO 10*60*1000

// Defines a retry timeout in ms which is used after a failed connect to the GPS.
// daemon. The time should not be to short otherwise the OS has problems to
// follow.
#define RETRY_TO 30000

// global object pointer to this class
GpsMaemoClient *GpsMaemoClient::instance = static_cast<GpsMaemoClient *> (0);

/**
 * The Maemo5 Location Service can emit four signal, which are handled by the
 * next four static pure C-functions.
 *
 * See Maemo5: http://wiki.maemo.org/Documentation/Maemo_5_Developer_Guide/Using_Connectivity_Components/Using_Location_API
 *
 * for more information.
 */


/*------Declaration of LibLocation Wrapper Functions--------------------------*/

namespace
{ // we put the GLib wrapper functions in anonymous name space
  // so they are not exported at link time
namespace LocationCb
  {
    extern "C"
    {
      /**
       * Is called from location service when GPSD is running.
       */
      static void gpsdRunning( LocationGPSDControl* control, gpointer userData );

      /**
       * Is called from location service when GPSD was stopped.
       */
      static void gpsdStopped( LocationGPSDControl* control, gpointer userData );

#ifndef MAEMO4

      /**
       * Is called from location service when GPSD was not startable.
       */
      static void gpsdErrorVerbose( LocationGPSDControl *control,
                                    LocationGPSDControlError error,
                                    gpointer user_data );
#endif

      /**
       * Is called from location service when GPSD was not startable. Should
       * be used only in Maemo4.
       */
      static void gpsdError( LocationGPSDControl * /*control*/,
                             gpointer /*userData*/ );

      /**
       * Is called from location service when new GPS data are available.
       */
      static void gpsdLocationchanged( LocationGPSDevice *device,
                                       gpointer user_data );
    }
  }
}

/*------Implementation of LibLocation Wrapper Functions-----------------------*/

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

#ifndef MAEMO4

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

#endif

/**
 * Is called from location service when GPSD was not startable. Should
 * be used only in Maemo4.
 */
static void LocationCb::gpsdError( LocationGPSDControl * /*control*/,
                                   gpointer /*userData*/ )
{
  qDebug("G-Signal->GPSD Error");

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdError();
    }
}

/**
 * Is called from location service when new GPS data are available.
 */
static void LocationCb::gpsdLocationchanged( LocationGPSDevice *device,
                                             gpointer user_data )
{
  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdLocationChanged( device );
    }
}

/**
 * Constructor requires a socket port of the server (listening end point)
 * useable for interprocess communication. As related host is always localhost
 * used. It will be opened two sockets to the server, one for data transfer,
 * the other only as notification channel.
 */
GpsMaemoClient::GpsMaemoClient( const ushort portIn )
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::GpsMaemoClient()";
#endif

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
  device          = 0;
  control         = 0;

  // Establish a connection to the server. The server is the Cumulus process
  // which has forked this process.
  if( ipcPort )
    {
      if( clientData.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          qWarning() << "Command channel Connection to Cumulus Server failed!"
                     << "Fatal error, terminate process!";

          setShutdownFlag(true);
          return;
        }

      if( clientNotif.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          qWarning() << "Notification channel Connection to Cumulus Server failed!"
                     << "Fatal error, terminate process!";

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

#ifdef MAEMO4

      // Maemo4 knows only this signal.
      // Subscribe to location service signals. That must be done only once!
      g_signal_connect( control, "error", G_CALLBACK(LocationCb::gpsdError), NULL );

#else
      // Set preferred-method in control
      g_object_set( G_OBJECT(control), "preferred-method",
                    LOCATION_METHOD_GNSS, NULL );

      // Set preferred-interval in control to one second
      g_object_set(G_OBJECT(control), "preferred-interval",
                   LOCATION_INTERVAL_1S, NULL);

      // Subscribe to location service signals. That must be done only once!
      g_signal_connect( control, "error-verbose", G_CALLBACK(LocationCb::gpsdErrorVerbose), NULL );

#endif

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
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::~GpsMaemoClient()";
#endif

  if( gpsIsRunning == true )
    {
      // Here we have to stop the GPSD.
      location_gpsd_control_stop( control );

      // Give location service time for stopping of GPSD otherwise
      // we will get problems during the next connection try.
      sleep(2);
    }

  if( device )
    {
      g_object_unref( device );
    }

  if( control )
    {
      g_object_unref( control );
    }

  instance = static_cast<GpsMaemoClient *> (0);
}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdRunning()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::handleGpsdRunning()";
#endif

  gpsIsRunning   = true;
  connectionLost = false;

  // restart timer with alive check supervision
  startTimer(ALIVE_TO);

  // Sends a message to Cumulus to signal that the GPS connection is established.
  // LibLocation reports not before a GPS fix any data.
  queueMsg( MSG_CON_ON );

#ifdef MAEMO4

  if ( control && ! control->can_control )
    {
      qWarning( "GPSD can not be controlled by Cumulus!" );
    }

  // print out returned GPS devices
  if ( control && control->ctx )
    {
      for( int i = 0; control->ctx->rfcomms[i] != static_cast<char *> (0); i++ )
        {
          qDebug( "Found GPS Device %s", control->ctx->rfcomms[i] );
        }
    }

#endif

}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdStopped()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::handleGpsdStopped()";
#endif

  gpsIsRunning   = false;
  connectionLost = true;
}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemoClient::handleGpsdError()
{
  qWarning( "GpsMaemoClient: Location service said GPSD Error, trying restart" );

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
void GpsMaemoClient::handleGpsdLocationChanged( LocationGPSDevice *device )
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::handleGpsdLocationChanged()";
#endif

  if( ! device )
    {
      queueMsg( MSG_CON_OFF );
      // Device is lost, set retry timeout to make a restart.
      startTimer(RETRY_TO);
      return;
    }

  if( ! device->online )
    {
      startTimer(RETRY_TO);
      // There is no connection to the GPS hardware. We do ignore this call.
      return;
    }

  // update supervision timer/variables
  startTimer(ALIVE_TO);

  connectionLost = false;

  /**
   * Definition of two proprietary sentences to transmit MAEMO GPS data.
   *
   *  0) $MAEMO
   *  1) Mode
   *  2) Time stamp as unsigned integer
   *  3) Ept
   *  4) Latitude in KFLog degrees
   *  5) Longitude in KFLog degrees
   *  6) Eph in m
   *  7) Speed in km/h
   *  8) Eps
   *  9) Track in degree 0...359
   * 10) Epd
   * 11) Altitude in meters
   * 12) Epv
   * 13) Climb
   * 14) Epc
   *
   *  0) $MAEMO1
   *  1) Status
   *  2) Satellites in view
   *  3) Satellites in use
   *  4) Satellite number
   *  5) Elevation
   *  6) Azimuth
   *  7) Signal strength
   *  8) Satellite in use
   *  9) Repetition of 4-8 according to Satellites in view
   */

   if (device->fix && device->fix->mode != LOCATION_GPS_DEVICE_MODE_NOT_SEEN )
    {
      QString mode = QString("%1").arg(device->fix->mode);
      QString altitude = "";
      QString epv = "";
      QString speed = "";
      QString eps = "";
      QString track = "";
      QString epd = "";
      QString climb = "";
      QString epc = "";
      QString latitude = "";
      QString longitude = "";
      QString eph = "";
      QString time = "";
      QString ept = "";

      // @AP: Note all items must be checked, if the are numbers!
      // Otherwise you can get nan (not a number) as string.
      if (device->fix->fields & LOCATION_GPS_DEVICE_ALTITUDE_SET)
        {
          if( ! isnan(device->fix->altitude) )
            {
              altitude = QString("%1").arg(device->fix->altitude, 0, 'f');
            }

          if( ! isnan(device->fix->epv) )
            {
              epv = QString("%1").arg(device->fix->epv, 0, 'f');
            }
        }

      if (device->fix->fields & LOCATION_GPS_DEVICE_SPEED_SET)
        {
          if( ! isnan(device->fix->speed) )
            {
              speed = QString("%1").arg(device->fix->speed, 0, 'f');
            }

          if( ! isnan(device->fix->eps) )
            {
              eps = QString("%1").arg(device->fix->eps, 0, 'f');
            }
        }

      if (device->fix->fields & LOCATION_GPS_DEVICE_TRACK_SET)
        {
          if( ! isnan(device->fix->track) )
            {
              track = QString("%1").arg(device->fix->track, 0, 'f');
            }

          if( ! isnan(device->fix->epd) )
            {
              epd   = QString("%1").arg(device->fix->epd, 0, 'f');
            }
        }

      if (device->fix->fields & LOCATION_GPS_DEVICE_CLIMB_SET)
        {
          if( ! isnan(device->fix->climb) )
            {
              climb = QString("%1").arg(device->fix->climb, 0, 'f');
            }

          if( ! isnan(device->fix->epc) )
            {
              epc = QString("%1").arg(device->fix->epc, 0, 'f');
            }
        }

      if (device->fix->fields & LOCATION_GPS_DEVICE_LATLONG_SET)
        {
          if( (! isnan(device->fix->latitude)) && (! isnan(device->fix->longitude)) )
            {
              // conversion to KFLog degree
              latitude  = QString("%1").arg(static_cast<int> (rint( device->fix->latitude * 600000.0)));
              longitude = QString("%1").arg(static_cast<int> (rint( device->fix->longitude * 600000.0)));
            }

          if( ! isnan(device->fix->eph) )
            {
              eph = QString("%1").arg(device->fix->eph / 100., 0, 'f'); // conversion to meter
            }
        }

      if (device->fix->fields & LOCATION_GPS_DEVICE_TIME_SET)
        {
          if( ! isnan(device->fix->time) && device->fix->time > 0.0 )
            {
              uint uiTime = static_cast<uint> (rint( device->fix->time));
              time = QString("%1").arg(uiTime);
            }

          if( ! isnan(device->fix->ept) )
            {
              ept  = QString("%1").arg(device->fix->ept, 0, 'f');
            }
        }

      QStringList list0;
      list0 << "$MAEMO0"
            << mode
            << time
            << ept
            << latitude
            << longitude
            << eph
            << speed
            << eps
            << track
            << epd
            << altitude
            << epv
            << climb
            << epc;

      QString sentence0 = list0.join( ",") + "\r\n";

      // store the new sentence in the queue
      queueMsg( sentence0.toAscii().data() );
   }

   QString satellitesInView = "0";
   QString satellitesInUse  = "0";
   QString status           = QString("%1").arg( device->status );

   if( device->satellites_in_view >= 0 )
     {
       // @AP: Sometimes -1 has been seen by me.
       satellitesInView = QString("%1").arg( device->satellites_in_view );
     }

   if( device->satellites_in_use >= 0 )
     {
       // @AP: Sometimes -1 has been seen by me.
       satellitesInUse = QString("%1").arg( device->satellites_in_use );
     }

   QStringList list1;
   list1 << "$MAEMO1"
         << status
         << satellitesInView
         << satellitesInUse;

   if( device->satellites != static_cast<GPtrArray *>(0) )
     {
       GPtrArray* sats = device->satellites;

       // There are satellite information available. Append them to the list
       for( uint i = 0; i < sats->len; i++ )
         {
           LocationGPSDeviceSatellite* sat =
               static_cast<LocationGPSDeviceSatellite *>(g_ptr_array_index( sats, i ));

           list1 << QString("%1").arg( sat->prn )
                 << QString("%1").arg( sat->elevation )
                 << QString("%1").arg( sat->azimuth )
                 << QString("%1").arg( sat->signal_strength )
                 << QString("%1").arg( sat->in_use );
         }
     }

   QString sentence1 = list1.join( ",") + "\r\n";

   // store the new sentence in the queue
   queueMsg( sentence1.toAscii().data() );
}

/**
 * Returns the currently used read file descriptors as mask, useable by the
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

/**
 * Processes incoming read events. They can come from the server.
 */
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
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::startGpsReceiving()";
#endif

  if( gpsIsRunning == true )
    {
      qWarning( "GpsMaemoClient::startGpsReceiving(): GPSD is running, ignore request!" );
      return true;
    }

  // remove all old queued messages
  queue.clear();

  // setup alive check, that guarantees a restart after an unsuccessful start
  startTimer(RETRY_TO);

  // There is no active connection, set flag.
  connectionLost = true;

  // Start GPSD, results are emitted by signals.
  location_gpsd_control_start( control );

  return true;
}

/**
 * The Maemo5 location service is called to stop GPS receiving.
 */
void GpsMaemoClient::stopGpsReceiving()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::stopGpsReceiving()";
#endif

  // Stop timer
  timeSpan = 0;

  // Shutdown GPSD, if we have started it.
  if( gpsIsRunning == true )
    {
       location_gpsd_control_stop( control );
    }
}

/**
 * Timeout controller for connection supervision.
 */
void GpsMaemoClient::toController()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::toController()";
#endif

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
          queueMsg( MSG_CON_OFF );
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

/**
 * Reads a server message from the socket. The protocol consists of
 * two parts. First the message length is read as unsigned
 * integer, after that the actual message as 8 bit character string.
 */
void GpsMaemoClient::readServerMsg()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::readServerMsg()";
#endif

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
   qDebug() << "GpsMaemoClient::readServerMsg(): Received Message:" << buf;
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
          qWarning() << "GpsMaemoClient::readServerMsg():"
                     << "Message protocol versions are incompatible,"
                     << "closes connection and shutdown client";

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
      // Shutdown is requested by the server. This message will not be acked!
      clientData.closeSock();
      clientNotif.closeSock();
      setShutdownFlag(true);
    }
  else
    {
      qWarning() << "GpsMaemoClient::readServerMsg():"
                 << "Unknown message received:" << buf;
      writeServerMsg( MSG_NEG );
    }

  delete [] buf;
  return;
}

/**
 * Send a message via data socket to the server. The protocol consists of two
 * parts. First the message length is transmitted as unsigned integer, after
 * that the actual message as 8 bit character string.
 */
void GpsMaemoClient::writeServerMsg( const char *msg )
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::writeServerMsg():" << msg;
#endif

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

/**
 * Sent a message via notification socket to the server. The protocol consists
 * of two parts. First the message length is transmitted as unsigned integer,
 * after that the actual message as 8 bit character string.
 */
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
  qDebug() << method << msg;
#endif

  return;
}

/**
 *  put a new message into the process queue and sent a notification to
 *  the server, if option notify is true. The notification is sent
 *  only once to avoid a flood of them, if server is busy.
 */
void GpsMaemoClient::queueMsg( const char* msg )
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::queueMsg():" << msg;
#endif

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
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::startTimer(): TS=" << milliSec << "ms";
#endif

  last.start();
  timeSpan = milliSec;
}
