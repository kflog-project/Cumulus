/***********************************************************************
 **
 **   gpsmaemo.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2008-2009 by Axel Pauli (axel@kflog.org)
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
 * This Class is only used by the Cumulus Maemo part to adapt the Cumulus GPS interface
 * to the Maemo requirements.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

#include <QApplication>

#include <gpsmgr.h>

#include "signalhandler.h"
#include "ipc.h"

#include "gpsmaemo.h"

// Defines alive check timeout.
#define ALIVE_TO 30000

// Defines a retry timeout which is used after a failed pairing with the
// BT gps manager. The time should not be to short otherwise cumulus is
// most of the time blocked by the pairing action.
#define RETRY_TO 60000

GpsMaemo::GpsMaemo(QObject* parent) : QObject(parent)
{
  // qDebug("GpsMaemo::GpsMaemo()");

  initSignalHandler();

  timer = new QTimer(this);
  timer->connect(timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()));

  readCounter = 0;
  gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
  ctx = 0;

  // Default GPSD port
  daemonPort = 2947;

  // get gpsd port from host service file
  struct servent *gpsEntry = getservbyname("gpsd", "tcp");

  if (gpsEntry)
    {
      daemonPort = ntohs(gpsEntry->s_port);
    }
}

GpsMaemo::~GpsMaemo()
{
  timer->stop();

  if (client.getSock() != -1)
    {
      client.closeSock();
    }

  if (ctx)
    {
      gpsbt_stop(ctx);
      delete ctx;
    }

  if (gpsDaemonNotifier)
    {
      delete gpsDaemonNotifier;
    }
}

/**
 * The Maemon BT manager is called to start the pairing to available BT devices.
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

  if (gpsDaemonNotifier)
    {
      delete gpsDaemonNotifier;
      gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
    }

  if (ctx)
    {
      // delete old gpsd context
      delete ctx;
      ctx = 0;
    }

  ctx = new gpsbt_t();
  memset(ctx, 0, sizeof(gpsbt_t));
  errno = 0;
  char buf[256];
  buf[0] = '\0';

  if (gpsbt_start(NULL, 0, 0, daemonPort, &buf[0], sizeof(buf), 0, ctx) < 0)
    {
      qWarning("Starting GPSD failed: errno=%d, %s", errno, strerror(errno));
      qWarning("GPSBT Error: %s", buf);
      // restart alive check timer with a retry time
      timer->start(RETRY_TO);
      return false;
    }

  // print out returned GPS devices
  for( int i = 0; ctx->rfcomms[i] != static_cast<char *> (0); i++ )
    {
      qDebug( "Found GPS Device %s", ctx->rfcomms[i] );
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
      return false;
    }

  // ask for protocol number, gpsd version , list of accepted letters
  strcpy(buf, "l\n");

  // Write message to gpsd to initialize in raw and watcher mode
  int res = client.writeMsg(buf, strlen(buf));

  if (res == -1)
    {
      qWarning("Write to GPSD failed");
      client.closeSock();
      return false;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return false;
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
      return false;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return false;
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
      return false;
    }

  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res == -1)
    {
      qWarning("Read from GPSD failed");
      client.closeSock();
      return false;
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

  return true;
}

/**
 * Closes the connection to the GPS Daemon and stops the daemon too.
 */
bool GpsMaemo::stopGpsReceiving()
{
  timer->stop();

  if (client.getSock() == -1)
    {
      // No connection to the server established.
      return false;
    }

  char buf[256];

  // request clear watcher mode
  strcpy(buf, "w-\n");

  // Write message to gpsd
  int res = client.writeMsg(buf, strlen(buf));

  // read answer
  res = client.readMsg(buf, sizeof(buf) - 1);

  if (res)
    {
      buf[res] = '\0';
      // qDebug("W-Answer: %s", buf);
    }

  // close socket
  client.closeSock();

  // shutdown gpsd
  if (ctx)
    {
      gpsbt_stop(ctx);
      delete ctx;
      ctx = 0;
    }

  // reset QT notifier
  if (gpsDaemonNotifier)
    {
      delete gpsDaemonNotifier;
      gpsDaemonNotifier = static_cast<QSocketNotifier *>(0);
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
  extern bool shutdownState;

  if (shutdownState)
    {
      // Shutdown is requested via signal. Therefore we can close all sockets.
      stopGpsReceiving();

      QApplication::exit(0);
      return;
    }

  if ( readCounter == 0 )
    {
      // check GPSD state, because no data have been read in the meantime
      int res = gpsmgr_is_gpsd_running(0, 0, GPSMGR_MODE_JUST_CHECK);

      if (res == GPSMGR_RUNNING )
        {
          qWarning("WD-TO: GPSD is running but not sending data - stopping it");
          // GPSD is alive but will not provide any data. Maybe his BT connection
          // is broken. Therefore we stop it for a restart.
          stopGpsReceiving();
          // make a break before a new restart to give the OS time for
          // internal clearing
          sleep( 2 );
        }

        // GPSD is not running now, try to restart it
        qWarning("WD-TO: GPSD is not running - trying restart");

        emit gpsConnectionLost();

        startGpsReceiving();
        return;
    }

  // check socket connection
  if (client.getSock() == -1)
    {
      // no connection to GPSD, start reconnecting
      qWarning("WD-TO: GPSD connection is broken - trying restart");

      emit gpsConnectionLost();
      startGpsReceiving();
      return;
    }

  // reset read counter
  readCounter = 0;
}

/**
 * This slot is triggered by the QT main loop and is used to take over
 * the provided data from the GPS daemon.
 */
void
GpsMaemo::slot_NotificationEvent(int /* socket */)
{
  readGpsData();
}

/**
 * This method reads the data provided by the GPS daemon. It is checked, if
 * the function is called only once.
 * @return true=success / false=unsuccess
 */
bool
GpsMaemo::readGpsData()
{
  static short caller = 0;

  if (client.getSock() == -1) // no connection is active
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
      return false;
    }

  if (bytes == -1)
    {
      qWarning("GpsMaemo: Read error, errno=%d, %s", errno, strerror(errno));
      caller--;
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

void
GpsMaemo::readSentenceFromBuffer()
{
  char *start = databuffer;
  char *end = 0;

  // Deactivate socket notifier, during receive buffer processing to avoid
  // a recalling when signal newSentence is emitted.
  if (gpsDaemonNotifier)
    {
      gpsDaemonNotifier->setEnabled(false);
    }

  while (strlen(start))
    {
      // Search for a newline in the receiver buffer
      if (!(end = strchr(start, '\n')))
        {
          // No newline in the receiver buffer, wait for more
          // characters
          if (gpsDaemonNotifier)
            {
              gpsDaemonNotifier->setEnabled(true);
            }
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

  if (gpsDaemonNotifier)
    {
      gpsDaemonNotifier->setEnabled(true);
    }
}

