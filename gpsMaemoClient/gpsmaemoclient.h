/***********************************************************************
**
**   gpsmaemoclient.h
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

/**
 * \author Axel Pauli
 *
 * \brief GPS Maemo client wrapper
 *
 * This class handles the interface between Cumulus and the Location Service used
 * by Maemo4/5. Nokia has removed the former GPSD daemon in Maemo5 and replaced by
 * liblocation API library and a set of on-request daemon processes for different
 * location methods. Therefore the Location API must be used to get data from the
 * GPS receiver hardware. The API does not use NMEA sentences and is programmed
 * by using GLib functionality :-(( The received Location data are converted into
 * a string format by this class and send via the socket connection to the
 * Cumulus process. Different methods of this class are called by the running
 * main loop (see source file gpsmaemomain.cpp).
 *
 */

#ifndef _GpsMaemoClient_hh_
#define _GpsMaemoClient_hh_ 1

#include <unistd.h>

/*
 * Maemo location service GPS device control. All functions are pure C-functions.
 * and not enclosed in C tags.
 */
extern "C" {

#include <location/location-gps-device.h>
#include <location/location-gpsd-control.h>

}

#include <qdatetime.h>
#include <QByteArray>
#include <QQueue>

#include "ipc.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

//++++++++++++++++++++++ CLASS GpsMaemoClient +++++++++++++++++++++++++++

class GpsMaemoClient
{

public:

    /**
     * Constructor requires a socket port of the server (listening end
     * point) useable for interprocess communication. As related host is
     * always localhost used.
     */
    GpsMaemoClient( const ushort portIn );

    virtual ~GpsMaemoClient();

    /*
     * Gets the single instance of this class. Can be Null, if no instance
     * was created before.
     */
    static GpsMaemoClient* getInstance()
    {
      return instance;
    };

    /**
     * Processes incoming read events. They can come from the server or
     * from the GPS device.
     */
    void processEvent( fd_set *fdMaskIn );

    /**
     * @return all used read file descriptors as mask, useable by the
     * select call
     */
    fd_set *getReadFdMask();

    int writeGpsData( const char *dataIn );

    /**
    * The Maemo5 location service is called to start the selected device. That
    * can be the internal GPS or a BT GPS mouse.
    */
    bool startGpsReceiving();

    /**
     * The Maemo5 location service is called to stop GPS receiving.
     */
    void stopGpsReceiving();

    /**
     * timeout controller
     */
    void toController();

    /*
     * Wrapper method to handle GLib signal emitted by the
     * location service.
     */
    void handleGpsdRunning();

    /*
     * Wrapper method to handle GLib signal emitted by the
     * location service.
     */
    void handleGpsdStopped();

    /*
     * Wrapper method to handle GLib signal emitted by the location service.
     */
    void handleGpsdError();

    /**
     * Wrapper method to handle GLib signal emitted by the location service.
     */
    void handleGpsdLocationChanged( LocationGPSDevice *device );

    void setShutdownFlag( bool newState )
    {
        shutdown = newState;
    };

    bool getShutdownFlag() const
    {
        return shutdown;
    };

private:

    //----------------------------------------------------------------------
    // Messages from/to the Cumulus will be read/written via the
    // client IPC instance.
    //----------------------------------------------------------------------

    void readServerMsg();

    void writeServerMsg( const char *msg );

    void writeNotifMsg( const char *msg );

    /**
     * put a new message into the process queue and sent a notification
     * to the server, if option notify is true.
     */
    void queueMsg( const char* msg );

    /** Setup timeout controller. */
    void startTimer( uint milliSec );

    //----------------------------------------------------------------------
    // Private data elements of Class
    //----------------------------------------------------------------------

private:

    /** Single instance of this class */
    static GpsMaemoClient* instance;

    /** Maemo GPS location service control instance */
    LocationGPSDControl *control;

    /** Maemo GPS location service device instance */
    LocationGPSDevice *device;

    /** Socket port for IPC to server process */
    ushort ipcPort;

    /** read file descriptor set in use by IPC */
    fd_set fdMask;

    // IPC instance to server process as data channel
    Ipc::Client clientData;

    // IPC instance to server process as notification channel
    Ipc::Client clientNotif;

    // used as timeout control for fix and connection
    QTime last;

    // Defined time span in milli seconds for timeout supervision. If set to
    // zero Timeout handler do nothing.
    long timeSpan;

    // Queue used for intermediate storing
    QQueue<QByteArray> queue;

    // GPS running flag.
    bool gpsIsRunning;

    // If true, a notification is sent to the server, when new data are
    // available in the queue. After that the flag is reset and the server must
    // renew the request.
    bool notify;

    // flag to indicate GPS connection lost
    bool connectionLost;

    // Shutdown flag for main loop. Will be set in case of fatal error
    // or if a shutdown message has been received from the server.
    bool shutdown;
};

#endif
