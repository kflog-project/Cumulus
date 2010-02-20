/***********************************************************************
 **
 **   gpsmaemo.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2008-2010 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef GPSMAEMO_H
#define GPSMAEMO_H

#include <unistd.h>
#include <sys/types.h>

/*
 * Maemo location service GPS device control. All functions are pure C-functions.
 * and not enclosed in C tags.
 */
extern "C" {

#include <location/location-gps-device.h>
#include <location/location-gpsd-control.h>

}

#include <QObject>
#include <QString>
#include <QSocketNotifier>
#include <QDateTime>
#include <QTimer>

#include "ipc.h"

/**
 * This module manages the start/stop of the Maemo GPS daemon and the connection to
 * it. The Maemo daemon is requested to pass all GPS data in raw and watcher mode.
 *
 * This Class is only used by the Cumulus Maemo part to adapt the Cumulus GPS
 * interface to the Maemo requirements.
 */
class GpsMaemo : public QObject
  {
    Q_OBJECT

private:

  Q_DISABLE_COPY ( GpsMaemo )

  public:

    GpsMaemo( QObject* parent );

    virtual ~GpsMaemo();

    /*
     * Gets the single instance of this class. Can be Null, if no instance
     * was created before.
     */
    static GpsMaemo* getInstance()
    {
      return instance;
    };

    /**
    * The Maemo location service is called to start the selected devices. That
    * can be the internal GPS or a BT GPS mouse.
    * After that it is tried to contact the Maemo GPS daemon on its standard port.
    * The Maemo GPS daemon is based on freeware to find here http://gpsd.berlios.de.
    * The daemon is requested to hand over GPS data in raw and watcher mode. A socket
    * notifier is setup in the QT main loop for data receiving.
    */
    bool startGpsReceiving();

    /**
     * Closes the connection to the GPS Daemon and stops the daemon via the
     * location service.
     */
    bool stopGpsReceiving();

    /**
     * Checks, if data are available in the socket receiver buffer and if yes
     * all data will be read.
     */
    void checkAndReadGpsData();

    /**
     * Returns the socket notifier of the daemon connection.
     */
    QSocketNotifier* getDaemonNotifier() const
    {
      return gpsDaemonNotifier;
    }

    /*
     * Wrapper methods to handle GLib signals emitted by the location service.
     * The wrapper method are introduced because the location service functions
     * are pure C functions.
     */
    void handleGpsdRunning();
    void handleGpsdStopped();
    void handleGpsdError();

  signals:
    /**
     * This signal is used every time a new sentence has arrived.
     */
    void newSentence(const QString& sentence);

    /**
     * This signal is used, if the gps connection has been lost
     */
    void gpsConnectionLost();

  private slots:
    /**
     * This slot is triggered by the QT main loop and is used to handle the
     * notification events from the GPS daemon.
     */
    void slot_NotificationEvent(int socket);

    /**
     * This timeout method is used, to call the method startClientProcess(),
     * when the timer is expired. This is the alive check for the forked
     * gpsClient process and ensures the cleaning up of zombies.
     */
    void slot_Timeout();

  private:

    /**
    * This method tries to read all lines contained in the receive buffer. A line
    * is always terminated by a newline. If it finds any, it sends them as
    * QStrings via the newSentence signal to whoever is listening (that will be
    * GPSNMEA) and removes the sentences from the buffer.
    */
    void readSentenceFromBuffer();

    /**
     * This method reads the data provided by the GPS daemon.
     * @return true=success / false=unsuccess
     */
    bool readGpsData();

    //------------------------------------------------------------------------------
    // Data members
    //------------------------------------------------------------------------------

    // Notifier for QT main loop
    QSocketNotifier *gpsDaemonNotifier;

    // used as timeout control for connection supervision
    QTimer *timer;

    // Socket port for ipc to daemon process
    ushort daemonPort;

    // IPC instance to GPS daemon server process
    Ipc::Client client;

    // Maemo GPS location service control instance
    LocationGPSDControl *control;

    // data buffers and pointers
    char* datapointer;
    char  databuffer[2048];
    int   dbsize;
    // counter used for read data check
    int   readCounter;

    // Single instance of this class
    static GpsMaemo* instance;
  };

#endif
