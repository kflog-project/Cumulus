/***********************************************************************
**
** gpsmaemoclient.cpp
**
** This file is part of Cumulus
**
************************************************************************
**
** Copyright (c): 2010-2011 by Axel Pauli (axel@kflog.org)
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
#include <netdb.h>

#include <QtCore>

#include "gpsmaemoclient.h"
#include "protocol.h"
#include "ipc.h"
#include "hwinfo.h"

// #define DEBUG 1

// Size of internal message queue.
#define QUEUE_SIZE 250

#ifndef MAEMO4
// Defines alive check timeout 10 minutes
#define ALIVE_TO 10*60*1000 // ms

#else

// Defines alive check timeout 60s for MAEMO4
#define ALIVE_TO 60000 // ms

#endif

// Defines a retry timeout in ms which is used after a failed connect to the GPS.
// daemon. The time should not be to short otherwise the OS has problems to
// follow.
#define RETRY_TO 30000

// global object pointer to this class
GpsMaemoClient *GpsMaemoClient::instance = static_cast<GpsMaemoClient *> (0);

/**
 * The Maemo5 Location Service can emit four signals which are handled by the
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
      static void gpsdRunning( LocationGPSDControl* control, gpointer user_data );

      /**
       * Is called from location service when GPSD was stopped.
       */
      static void gpsdStopped( LocationGPSDControl* control, gpointer user_data );

#ifdef MAEMO5
      /**
       * Is called from location service when new GPS data are available.
       */
      static void gpsdLocationchanged( LocationGPSDevice *device,
                                       gpointer user_data );

      /**
       * Is called from location service when GPSD was not startable.
       */
      static void gpsdErrorVerbose( LocationGPSDControl *control,
                                    LocationGPSDControlError error,
                                    gpointer user_data );
#endif

#ifdef MAEMO4
      /**
       * Is called from location service when GPSD was not startable. Should
       * be used only in Maemo4.
       */
      static void gpsdError( LocationGPSDControl * control,
                             gpointer user_data );
#endif

    }
  }
}

/*------Implementation of LibLocation Wrapper Functions-----------------------*/

/**
 * Is called from location service when GPSD is running.
 */
static void LocationCb::gpsdRunning( LocationGPSDControl* control,
                                     gpointer user_data )
{
  Q_UNUSED( control )
  Q_UNUSED( user_data )

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
                                     gpointer user_data )
{
  Q_UNUSED( control )
  Q_UNUSED( user_data )

  qDebug("G-Signal->GPSD Stopped");

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdStopped();
    }
}

#ifdef MAEMO5

/**
 * Is called from location service when GPSD was not startable.
 */
static void LocationCb::gpsdErrorVerbose( LocationGPSDControl *control,
                                          LocationGPSDControlError error,
                                          gpointer user_data )
{
  Q_UNUSED( control )
  Q_UNUSED( user_data )

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

#ifdef MAEMO4
/**
 * Is called from location service when GPSD was not startable. Should
 * be used only in Maemo4.
 */
static void LocationCb::gpsdError( LocationGPSDControl *control,
                                   gpointer user_data )
{
  qDebug("G-Signal->GPSD Error");

  Q_UNUSED(control)
  Q_UNUSED(user_data)

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdError();
    }
}
#endif

#ifdef MAEMO5
/**
 * Is called from location service when new GPS data are available. Under MAEMO4
 * altitude information is not reported continuous by this method. Do not know
 * what is going on under MAEMO5.
 */
static void LocationCb::gpsdLocationchanged( LocationGPSDevice *device,
                                             gpointer user_data )
{
  Q_UNUSED(user_data)

  if( GpsMaemoClient::getInstance() )
    {
      GpsMaemoClient::getInstance()->handleGpsdLocationChanged( device );
    }
}
#endif

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

#ifdef MAEMO4

  readCounter = 0;

  // Default GPSD port
  gpsDaemonPort = 2947;

  // get GPSD port from host service file
  struct servent *gpsEntry = getservbyname("gpsd", "tcp");

  if (gpsEntry)
    {
      gpsDaemonPort = ntohs(gpsEntry->s_port);
    }

#endif

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

#endif

#ifdef MAEMO5
      // Set preferred-method in control
      /* temporary switched off because user reported long fix times, if that is set here.
      g_object_set( G_OBJECT(control), "preferred-method",
                    LOCATION_METHOD_GNSS, NULL ); */

      // Set preferred-interval in control to one second
      g_object_set(G_OBJECT(control), "preferred-interval",
                   LOCATION_INTERVAL_1S, NULL);

      // Subscribe to location service signals. That must be done only once!
      g_signal_connect( control, "error-verbose", G_CALLBACK(LocationCb::gpsdErrorVerbose), NULL );

#endif

      g_signal_connect( control, "gpsd_stopped",  G_CALLBACK(LocationCb::gpsdStopped), NULL );
      g_signal_connect( control, "gpsd_running",  G_CALLBACK(LocationCb::gpsdRunning), NULL );
    }

#ifdef MAEMO5
  // Setup device instance
  device = (LocationGPSDevice *) g_object_new(LOCATION_TYPE_GPS_DEVICE, NULL);

  // Subscribe to location service signal.
  g_signal_connect( device, "changed", G_CALLBACK(LocationCb::gpsdLocationchanged), NULL );
#endif

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

#ifdef MAEMO4

  if( gpsDaemon.getSock() != -1 )
    {
      gpsDaemon.closeSock();
    }

#endif
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
  // LibLocation reports not any data before a GPS fix.
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

  // wait that daemon can make its initialization
  sleep(3);

  // try to connect the GPSD on its listen port
  if (gpsDaemon.connect2Server("", gpsDaemonPort) == 0)
    {
      qDebug("GPSD successfully connected on port %d", gpsDaemonPort);
    }
  else
    {
      // Connection failed
      qWarning("GPSD could not be connected on port %d", gpsDaemonPort);
      return;
    }

  // ask for protocol number, gpsd version , list of accepted letters
  char buf[256];
  strcpy(buf, "l\n");

  // Write message to gpsd to initialize in raw and watcher mode
  int res = gpsDaemon.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  res = gpsDaemon.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  buf[res] = '\0';

  qDebug("GPSD-l (ProtocolVersion-GPSDVersion-RequestLetters): %s", buf);

  // ask for GPS identification string
  strcpy(buf, "i\n");

  // Write message to gpsd to get the GPS id string
  res = gpsDaemon.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  res = gpsDaemon.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  buf[res] = '\0';

  qDebug("GPSD-i (GPS-ID): %s", buf);

  // request raw and watcher mode
  strcpy(buf, "r+\nw+\n");

  // Write message to gpsd to initialize it in raw and watcher mode.
  // - Raw mode means, NMEA data records will be sent
  // - Watcher mode means that new data will sent without polling
  res = gpsDaemon.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  res = gpsDaemon.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      gpsDaemon.closeSock();
      return;
    }

  buf[res] = '\0';

  // qDebug("GPSD-r+w+: %s", buf );
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

#ifdef MAEMO5
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
#endif

/**
 * Returns the currently used read file descriptors as mask, usable by the
 * select call
 */
fd_set *GpsMaemoClient::getReadFdMask()
{
  // Reset file descriptor mask bits
  FD_ZERO( &fdMask );

  if( ipcPort ) // data channel to Cumulus
    {
      int sfd = clientData.getSock();

      if( sfd != -1 )
        {
          FD_SET( sfd, &fdMask );
        }
    }

#ifdef MAEMO4

  // Channel to GPSD
  if( gpsDaemon.getSock() != -1 )
    {
      FD_SET( gpsDaemon.getSock(), &fdMask );
    }

#endif

  return &fdMask;
}

/**
 * Processes incoming read events. They can come from Cumulus or the GPSD.
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

#ifdef MAEMO4

  int gpsSock = gpsDaemon.getSock();

  if( gpsSock != -1 && FD_ISSET( gpsSock, fdMask ) )
    {
      readGpsData();
    }

#endif

}

/**
* The Maemo location service is called to start the selected devices. That
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

#ifdef MAEMO4

  readCounter = 0;

  // reset buffer pointer
  datapointer = databuffer;
  dbsize = 0;
  memset(databuffer, 0, sizeof(databuffer));

  if ( gpsDaemon.getSock() != -1 )
    {
      // close a previous connection
      gpsDaemon.closeSock();
    }

#endif

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
 * The Maemo location service is called to stop GPS receiving.
 */
void GpsMaemoClient::stopGpsReceiving()
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::stopGpsReceiving()";
#endif

#ifdef MAEMO4

  if (gpsDaemon.getSock() != -1)
    {
      // Close connection to the GPSD server.
      char buf[256];

      // request clear watcher mode
      strcpy(buf, "w-\n");

      // Write message to gpsd
      int res = gpsDaemon.writeMsg(buf, strlen(buf));

      // read answer
      res = gpsDaemon.readMsg(buf, sizeof(buf) - 1);

      if (res)
        {
          // buf[res] = '\0';
          // qDebug("W-Answer: %s", buf);
        }

      // close socket
      gpsDaemon.closeSock();
    }

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

  if( done < sizeof(msgLen) )
    {
      qWarning() << "GpsMaemoClient::readServerMsg(): MSG length" << done << "too short";
      clientData.closeSock();
      exit(-1); // Error occurred
    }

  if( msgLen > 256 )
    {
      // such messages length are not defined. we will ignore that.
      qWarning() << "GpsMaemoClient::readServerMsg():"
                 << "message" << msgLen << "too large, ignoring it!";
      exit(-1); // Error occurred
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = clientData.readMsg( buf, msgLen );

  if( done <= 0 )
    {
      clientData.closeSock();
      delete [] buf;
      buf = 0;
      exit(-1); // Error occurred
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
      // Get message is requested. We take all messages out of the
      // queue, if there are any.
      if( queue.count() == 0 )
        {
          writeServerMsg( MSG_NEG ); // queue is empty
          return;
        }

      // At first we sent the number of available messages in the queue
      QByteArray res = QByteArray(MSG_RMC) + " " +
                       QByteArray::number(queue.count());

      writeServerMsg( res.data() );

      // It follow all messages from the queue in order. That is done to improve
      // the transfer performance. The former single handshake method was to slow,
      // if the message number was greater than 5.
      while( queue.count() )
        {
          QByteArray msg = queue.dequeue();
          QByteArray res = QByteArray(MSG_RM) + " " + msg;

          writeServerMsg( res.data() );
        }
    }
  else if( MSG_MAGIC == args[0] )
    {
      // check protocol versions, reply with pos or neg
      if( MSG_PROTOCOL != args[1] )
        {
          qCritical() << "GpsMaemoClient::readServerMsg():"
                      << "Client-Server protocol mismatch!"
                      << "Client:" << MSG_PROTOCOL
                      << "Server:" << args[1];

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
  else if( MSG_GPS_KEYS == args[0] && args.count() == 2 )
    {
      // Well known GPS message keys are received. We do ignore that here.
      writeServerMsg( MSG_POS );
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

#ifdef MAEMO4
/**
 * Verify the checksum of the passed sentences.
 *
 * @returns true (success) or false (error occurred)
 */
bool GpsMaemoClient::verifyCheckSum( const char *sentence )
{
  // Filter out wrong data messages read in from the GPS port. Known messages
  // do start with a dollar sign or an exclamation mark.
  if( sentence[0] != '$' && sentence[0] != '!' )
    {
      return false;
    }

  for( int i = strlen(sentence) - 1; i >= 0; i-- )
    {
      if( sentence[i] == '*' )
        {
          if( (strlen(sentence) - 1 - i) < 2 )
            {
              // too less characters
              return false;
            }

          char checkBytes[3];
          checkBytes[0] = sentence[i+1];
          checkBytes[1] = sentence[i+2];
          checkBytes[2] = '\0';

          uchar checkSum = (uchar) QString( checkBytes ).toUShort( 0, 16 );

          if( checkSum == calcCheckSum( sentence ) )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  return false;
}

/** Calculate check sum over NMEA record. */
uchar GpsMaemoClient::calcCheckSum( const char *sentence )
{
  uchar sum = 0;

  for( uint i = 1; i < strlen( sentence ); i++ )
    {
      uchar c = (uchar) sentence[i];

      if( c == '$' ) // Start sign will not be considered
        {
          continue;
        }

      if( c == '*' ) // End of sentence reached
        {
          break;
        }

      sum ^= c;
    }

  return sum;
}

#endif

/** Setup timeout controller. */
void GpsMaemoClient::startTimer( uint milliSec )
{
#ifdef DEBUG
  qDebug() << "GpsMaemoClient::startTimer(): TS=" << milliSec << "ms";
#endif

  last.start();
  timeSpan = milliSec;
}

#ifdef MAEMO4
/**
 * This method reads the data provided by the GPS daemon. It is checked, if
 * the function is called only once.
 * @return true=success / false=unsuccess
 */
bool GpsMaemoClient::readGpsData()
{
  static short caller = 0;

  if( gpsDaemon.getSock() == -1 ) // no connection is active
    {
      return false;
    }

  if( caller )
    {
      qWarning("GpsMaemo::readGpsData() is called recursive");
      return false;
    }

  caller++;

  // update supervision timer/variables
  startTimer(ALIVE_TO);

  connectionLost = false;

  // Check for end of buffer. If this happens we will discard all
  // data to avoid a dead lock. Should normally not happen, if valid
  // data records are passed containing a terminating new line.
  if ( (sizeof(databuffer) - dbsize - 1) < 100 )
    {
      qWarning( "GpsMaemo::readGpsData reached end of buffer, discarding all received data!" );

      // reset buffer pointer
      datapointer = databuffer;
      dbsize = 0;
      databuffer[0] = '\0';

      caller--;
      return false;
    }

  // all available GPS data lines are read successive
  int bytes = 0;

  // qDebug("-> datapointer=%x, databuffer=%x, dbsize=%d", datapointer, databuffer, dbsize );

  bytes = read( gpsDaemon.getSock(), datapointer, sizeof(databuffer) - dbsize - 1 );

  if (bytes == 0) // Nothing read, should normally not happen
    {
      qWarning("GpsMaemo: Read has read 0 bytes!");
      caller--;

      if (gpsDaemon.getSock() != -1)
        {
          gpsDaemon.closeSock();
        }

      return false;
    }

  if (bytes == -1)
    {
      qWarning("GpsMaemo: Read error, errno=%d, %s", errno, strerror(errno));
      caller--;

      if( gpsDaemon.getSock() != -1 )
        {
          gpsDaemon.closeSock();
        }

      return false;
    }

  if (bytes > 0)
    {
      readCounter++;
      dbsize += bytes;
      datapointer += bytes;
      databuffer[dbsize] = '\0'; // terminate buffer with a null

      // qDebug("<- bytes=%d, datapointer=%x, databuffer=%x, dbsize=%d", bytes, datapointer, databuffer, dbsize );

      // extract NMEA sentences from buffer and forward it via signal
      readSentenceFromBuffer();
    }

  caller--;
  return true;
}
#endif

#ifdef MAEMO4
/**
 * This method tries to read all lines contained in the receive buffer. A line
 * is always terminated by a newline. If it finds any, it sends them as
 * QStrings via the newSentence signal to whoever is listening (that will be
 * GPSNMEA) and removes the sentences from the buffer.
 */
void GpsMaemoClient::readSentenceFromBuffer()
{
  char *start = databuffer;
  char *end = 0;

  while( strlen(start) )
    {
      // Search for a newline in the receiver buffer
      if ( !(end = strchr(start, '\n')) )
        {
          // No newline in the receiver buffer, wait for more
          // characters
          return;
        }

      if (start == end)
        {
          // skip newline and start at next position with a new search
          start++;
          continue;
        }

      // found a complete record in the buffer, it will be extracted now
      char *record = (char *) malloc(end - start + 2);

      memset(record, 0, end - start + 2);

      strncpy(record, start, end - start + 1);

      QString qRecord(record);

      if ( qRecord.startsWith( "GPSD,") )
        {
          // Filter out and display GPSD messages
          qWarning( "GPSD Message: %s", record );
        }
      else if( ! qRecord.startsWith( "$PNOKU,") &&
               ! qRecord.startsWith( "$GPVTG,") &&
               ! qRecord.startsWith( "$GPGLL,") &&
               ! qRecord.startsWith( "$GPGST,") )
        {
          if( verifyCheckSum( record ) == true )
            {
              // Store sentence in the receiver queue, if checksum is ok.
              // qDebug( "GpsMaemo: Extracted NMEA Record: %s", record );
              queueMsg( record );
            }
        }

      free(record);
      record = 0;

      // remove queued record from receive buffer
      memmove(databuffer, end + 1, databuffer + sizeof(databuffer) - 1 - end);

      datapointer -= (end + 1 - databuffer);

      dbsize -= (end + 1 - databuffer);

      start = databuffer;

      end = 0;

      // qDebug("After Read: datapointer=%x, dbsize=%d", datapointer, dbsize );
    }
}
#endif
