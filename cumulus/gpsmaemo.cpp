/***********************************************************************
 **
 **   gpsmaemo.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2010 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * This module manages the start/stop of the Maemo GPS daemon and the connection to
 * it. The Maemo daemon is requested to pass all GPS data in raw and watcher mode.
 *
 * This Class is only used by the Cumulus Maemo part to adapt the Cumulus GPS
 * interface to the Maemo requirements. Cumulus uses the Nokia location library for
 * that purpose.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

#include <QApplication>

#include "signalhandler.h"
#include "ipc.h"

#include "gpsmaemo.h"

// Defines alive check timeout.
#define ALIVE_TO 30000

// Defines a retry timeout which is used after a failed connect to the GPS.
// daemon. The time should not be to short otherwise Cumulus is
// most of the time blocked by the connect action.
#define RETRY_TO 60000

// global object pointer to this class
GpsMaemo *GpsMaemo::instance = static_cast<GpsMaemo *> (0);

/*
 * The Maemo location service can emit three signal, which are handled by the
 * next three static pure C-functions.
 *
 * See Maemo5: http://maemo.org/api_refs/5.0/beta/liblocation
 * See Maemo4: http://maemo.org/maemo_release_documentation/maemo4.1.x/node10.html#SECTION001027000000000000000
 *
 * for more information.
 */

/**
 * Is called from location service when GPSD is running. We try then
 * to connect the GPSD Daemon.
 */
static void ::gpsdRunning( LocationGPSDControl * /*control*/, gpointer /*userData*/ )
{
  qDebug("G-Signal->GPSD Running");
  GpsMaemo::getInstance()->handleGpsdRunning();
}

/**
 * Is called from location service when GPSD was stopped.
 */
static void ::gpsdStopped( LocationGPSDControl * /*control*/, gpointer /*userData*/ )
{
  qDebug("G-Signal->GPSD Stopped");
  GpsMaemo::getInstance()->handleGpsdStopped();
}

/**
 * Is called from location service when GPSD was not startable.
 */
static void ::gpsdError( LocationGPSDControl * /*control*/, gpointer /*userData*/ )
{
  qDebug("G-Signal->GPSD Error");
  GpsMaemo::getInstance()->handleGpsdError();
}

//--------------------------------------------------------------------------
// Methods of GpsMaemo class
//--------------------------------------------------------------------------

GpsMaemo::GpsMaemo(QObject* parent) : QObject(parent)
{
  if ( instance  )
    {
      qWarning("GpsMaemo::GpsMaemo(): You can create only one class instance!");
      return;
    }

  instance = this;

  initSignalHandler();

  timer = new QTimer(this);
  timer->connect(timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()));

  readCounter = 0;
  gpsDaemonNotifier = static_cast<QSocketNotifier *> (0);
  control = static_cast<LocationGPSDControl *> (0);

  // Default GPSD port
  daemonPort = 2947;

  // get gpsd port from host service file
  struct servent *gpsEntry = getservbyname("gpsd", "tcp");

  if (gpsEntry)
    {
      daemonPort = ntohs(gpsEntry->s_port);
    }

  // Get the GPSD control object from location service.
  control = location_gpsd_control_get_default();

  if( ! control )
    {
      qWarning( "GpsMaemo::GpsMaemo(): No GPSD control object returned" );
    }
  else
    {
      // Subscribe to location service signals. That must be done only once!
      g_signal_connect( control, "error",        G_CALLBACK(gpsdError),   NULL );
      g_signal_connect( control, "gpsd_stopped", G_CALLBACK(gpsdStopped), NULL );
      g_signal_connect( control, "gpsd_running", G_CALLBACK(gpsdRunning), NULL );
    }
}

GpsMaemo::~GpsMaemo()
{
  timer->stop();

  if (control)
    {
      // Here we have to stop the GPSD.
      LocationGPSDControl *ctrl = control;
      control = static_cast<LocationGPSDControl *> (0);
      location_gpsd_control_stop( ctrl );

      // Give location service time for stopping of GPSD otherwise
      // we will get problems during the next connection try.
      sleep(2);
    }

  if (client.getSock() != -1)
    {
      client.closeSock();
    }

  if (gpsDaemonNotifier)
    {
      delete gpsDaemonNotifier;
    }

  instance = static_cast<GpsMaemo *> (0);
}

/**
* The Maemo location service is called to start the selected devices. That
* can be the internal GPS or a BT GPS mouse.
* After that it is tried to contact the Maemo GPS daemon on its standard port.
* The Maemo GPS daemon is based on freeware to find here http://gpsd.berlios.de.
* The daemon is requested to hand over GPS data in raw and watcher mode. A socket
* notifier is setup in the QT main loop for data receiving.
*/
bool GpsMaemo::startGpsReceiving()
{
  // qDebug("GpsMaemo::startGpsReceiving()");
  readCounter = 0;

  // reset buffer pointer
  datapointer = databuffer;
  dbsize = 0;
  memset(databuffer, 0, sizeof(databuffer));

  // setup alive check, that guarantees a restart after unsuccessful start
  timer->start(ALIVE_TO);

  if (client.getSock() != -1)
    {
      // close a previous connection
      client.closeSock();
    }

  if ( gpsDaemonNotifier )
    {
      delete gpsDaemonNotifier;
      gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
    }

  // Get the GPSD control object from location service.
  control = location_gpsd_control_get_default();

  if( ! control )
    {
      qWarning( "No GPSD control object returned" );
      // setup timer for restart
      timer->start(RETRY_TO);
      return false;
    }

  // Start GPSD, results are emitted by signals.
  location_gpsd_control_start( control );
  return true;
}

/**
 * Closes the connection to the GPS Daemon and stops the daemon too.
 */
bool GpsMaemo::stopGpsReceiving()
{
  // qDebug( "GpsMaemo::stopGpsReceiving: control=%X", control );
  timer->stop();

  if (client.getSock() != -1)
    {
      // Close connection to the GPSD server.
      char buf[256];

      // request clear watcher mode
      strcpy(buf, "w-\n");

      // Write message to gpsd
      int res = client.writeMsg(buf, strlen(buf));

      // read answer
      res = client.readMsg(buf, sizeof(buf) - 1);

      if (res)
        {
          // buf[res] = '\0';
          // qDebug("W-Answer: %s", buf);
        }

      // close socket
      client.closeSock();
    }

  // reset QT notifier
  if (gpsDaemonNotifier)
    {
      delete gpsDaemonNotifier;
      gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
    }

  // Shutdown GPSD, if we have started it.
  if (control)
    {
      control = static_cast<LocationGPSDControl *> (0);
      location_gpsd_control_stop( location_gpsd_control_get_default() );
    }

  return true;
}

/**
 * This timeout method is used, to check, if the GPSD is alive.
 * If not, a new startup is executed. The alive check expects,
 * that some data have been read in the last timer period.
 */
void GpsMaemo::slot_Timeout()
{
  // qDebug( "GpsMaemo::slot_Timeout(): readCounter=%d", readCounter );
  extern bool shutdownState;

  if (shutdownState)
    {
      // Shutdown is requested via signal. Therefore we can close all sockets.
      stopGpsReceiving();
      QApplication::exit(0);
      return;
    }

  if( ! control )
    {
      // There is no control object available, try a restart of location service.
      startGpsReceiving();
      return;
    }

  if ( readCounter == 0 )
    {
      // No data have been read in the meantime
      qWarning("WD-TO: GPSD is running but not sending data - trying restart");
      // Maybe the GPS connection is broken. Therefore we stop it for a restart.
      emit gpsConnectionLost();

      // Stop GPSD
      stopGpsReceiving();

      // setup timer for restart
      timer->start(RETRY_TO);
      return;
    }

  // check socket connection
  if (client.getSock() == -1)
    {
      // no connection to GPSD, start reconnecting
      qWarning("WD-TO: GPSD connection is broken - trying restart");

      emit gpsConnectionLost();

      // Stop GPSD
      stopGpsReceiving();

      // setup timer for restart
      timer->start(RETRY_TO);
      return;
    }

  // reset read counter
  readCounter = 0;
}

/**
 * This slot is triggered by the QT main loop and is used to take over
 * the provided data from the GPS daemon.
 */
void GpsMaemo::slot_NotificationEvent(int /* socket */)
{
  checkAndReadGpsData();
}

/**
 * Checks, if GPS data are available in the socket receiver buffer and if yes
 * all data will be read.
 */
void GpsMaemo::checkAndReadGpsData()
{
  // qDebug("GpsMaemo::checkAndReadGpsData() is called");

  // Deactivate socket notifier during receive buffer processing to avoid
  // a recalling when signals like newSentence are emitted.
  if (gpsDaemonNotifier)
    {
      gpsDaemonNotifier->setEnabled(false);
    }

  int maxLoop = 25; // limit looping

  while( maxLoop > 0 && client.numberOfReadableBytes() > 0 )
    {
      maxLoop--;
      readGpsData();
    }

  if (gpsDaemonNotifier)
    {
      gpsDaemonNotifier->setEnabled(true);
    }
}

/**
 * This method reads the data provided by the GPS daemon. It is checked, if
 * the function is called only once.
 * @return true=success / false=unsuccess
 */
bool GpsMaemo::readGpsData()
{
  static short caller = 0;

  if( client.getSock() == -1 ) // no connection is active
    {
      return false;
    }

  if( caller )
    {
      qWarning("GpsMaemo::readGpsData() is called recursive");
      return false;
    }

  caller++;

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

  bytes = read(client.getSock(), datapointer, sizeof(databuffer) - dbsize - 1);

  if (bytes == 0) // Nothing read, should normally not happen
    {
      qWarning("GpsMaemo: Read has read 0 bytes!");
      caller--;

      if (client.getSock() != -1)
        {
          client.closeSock();
        }

      if (gpsDaemonNotifier)
        {
          delete gpsDaemonNotifier;
          gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
        }

      return false;
    }

  if (bytes == -1)
    {
      qWarning("GpsMaemo: Read error, errno=%d, %s", errno, strerror(errno));
      caller--;

      if (client.getSock() != -1)
        {
          client.closeSock();
        }

      if (gpsDaemonNotifier)
        {
          delete gpsDaemonNotifier;
          gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
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

/**
 * This method tries to read all lines contained in the receive buffer. A line
 * is always terminated by a newline. If it finds any, it sends them as
 * QStrings via the newSentence signal to whoever is listening (that will be
 * GPSNMEA) and removes the sentences from the buffer.
 */

void GpsMaemo::readSentenceFromBuffer()
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
      else
        {
          // forward GPS record to subscribers
          // qDebug( "GpsMaemo: Extracted NMEA Record: %s", record );
          emit newSentence(qRecord);
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

/*
 * Wrapper method to handle GLib signal emitted by the
 * location service.
 */
void GpsMaemo::handleGpsdRunning()
{
  if( ! control )
    {
      // Get the GPSD control object from location service.
      control = location_gpsd_control_get_default();
    }

  if ( ! control->can_control )
    {
      qWarning("GPSD can not be controlled by Cumulus!");
    }

  // print out returned GPS devices
  if ( control->ctx )
    {
      for( int i = 0; control->ctx->rfcomms[i] != static_cast<char *> (0); i++ )
        {
          qDebug( "Found GPS Device %s", control->ctx->rfcomms[i] );
        }
    }

  // Restart alive check timer with a retry time which is used in case of error
  // return.
  timer->start(RETRY_TO);

  // wait that daemon can make its initialization
  sleep(3);

  // try to connect the gpsd on its listen port
  if (client.connect2Server("", daemonPort) == 0)
    {
      qDebug("GPSD successfully connected on port %d", daemonPort);
    }
  else
    {
      // Connection failed
      qWarning("GPSD could not be connected on port %d", daemonPort);
      return;
    }

  // ask for protocol number, gpsd version , list of accepted letters
  char buf[256];
  strcpy(buf, "l\n");

  // Write message to gpsd to initialize in raw and watcher mode
  int res = client.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      client.closeSock();
      return;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return;
    }

  buf[res] = '\0';

  qDebug("GPSD-l (ProtocolVersion-GPSDVersion-RequestLetters): %s", buf);

  // ask for GPS identification string
  strcpy(buf, "i\n");

  // Write message to gpsd to get the GPS id string
  res = client.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      client.closeSock();
      return;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return;
    }

  buf[res] = '\0';

  qDebug("GPSD-i (GPS-ID): %s", buf);

  // request raw and watcher mode
  strcpy(buf, "r+\nw+\n");

  // Write message to gpsd to initialize it in raw and watcher mode.
  // - Raw mode means, NMEA data records will be sent
  // - Watcher mode means that new data will sent without polling
  res = client.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      client.closeSock();
      return;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return;
    }

  buf[res] = '\0';

  // qDebug("GPSD-r+w+: %s", buf );

  // Add a socket notifier to the QT main loop, which will be
  // bound to slot_NotificationEvent. Qt will trigger this method, if
  // the gpsd has sent new data.
  gpsDaemonNotifier = new QSocketNotifier(client.getSock(), QSocketNotifier::Read, this);

  gpsDaemonNotifier->connect( gpsDaemonNotifier, SIGNAL(activated(int)),
                              this, SLOT(slot_NotificationEvent(int)) );

  // restart alive check timer with alive timeout
  timer->start(ALIVE_TO);
}

/*
 * Wrapper method to handle GLib signal emitted by the
 * location service.
 */
void GpsMaemo::handleGpsdStopped()
{
  control = static_cast<LocationGPSDControl *> (0);
}

/*
 * Wrapper method to handle GLib signal emitted by the location service.
 */
void GpsMaemo::handleGpsdError()
{
  qWarning( "GpsMaemo::handleGpsdError(): Location service said GPSD Error, trying restart" );

  // Reset control object
  control = static_cast<LocationGPSDControl *> (0);

  // stop GPSD in error case
  location_gpsd_control_stop( location_gpsd_control_get_default() );

  // setup timer for restart
  timer->start(RETRY_TO);
}
