/***********************************************************************
**
**   gpsmaemoclient.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2010 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

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
     * from the Gps device.
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
    void handleGpsdLocationchanged( LocationGPSDevice *device );

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

    // Single instance of this class
    static GpsMaemoClient* instance;

    // Maemo5 GPS location service control instance
    LocationGPSDControl *control;

    // Maemo5 GPS location service device instance
    LocationGPSDevice *device;

    // Socket port for ipc to server process
    ushort ipcPort;

    // read file descriptor set in use by ipc
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

    // flag to indicate gps connection lost
    bool connectionLost;

    // Shutdown flag for main loop. Will be set in case of fatal error
    // or if a shutdown message has been received from the server.
    bool shutdown;
};

//---------------LibLocation Wrapper Functions----------------------------------

namespace
{ // we put the glib wrapper functions in anonymous name space
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

      /**
       * Is called from location service when GPSD was not startable.
       */
      static void gpsdErrorVerbose( LocationGPSDControl *control,
                                    LocationGPSDControlError error,
                                    gpointer user_data );
      /**
       * Is called from location service when new gps data are available.
       */
      static void gpsdLocationchanged( LocationGPSDevice *device,
                                       gpointer user_data );
    }
  }
}

#endif
