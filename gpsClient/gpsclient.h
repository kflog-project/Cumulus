/***********************************************************************
**
**   gpsclient.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2010 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#ifndef _GpsClient_hh_
#define _GpsClient_hh_ 1

#include <termios.h>
#include <unistd.h>

#include <QDateTime>
#include <QByteArray>
#include <QQueue>

#include "ipc.h"

#define _POSIX_SOURCE 1 /* POSIX complaint source */

//++++++++++++++++++++++ CLASS GpsClient +++++++++++++++++++++++++++

class GpsClient
{

public:

    /**
     * Constructor requires a socket port of the server (listening end
     * point) usable for interprocess communication. As related host is
     * always localhost used.  Additionally serial device and transfer
     * speed can be passed.
     */
    GpsClient( const ushort portIn );

    virtual ~GpsClient();

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

    /**
     * @return true=success / false=unsuccess
     */
    bool readGpsData();

    int writeGpsData( const char *dataIn );

    /**
     * opens connection to the Gps
     */
    bool openGps( const char *deviceIn, const uint ioSpeedIn );

    /**
     * closes connection to the Gps
     */
    void closeGps();

    /**
     * timeout controller
     */
    void toController();

    /**
     * calculate check sum over NMEA record
     */
    uchar calcCheckSum( const char *sentence );

    /**
     * Verify the checksum of the passed sentences.
     *
     * @returns true (success) or false (error occurred)
     */
    bool verifyCheckSum( const char *sentence );

    void setShutdownFlag( bool newState )
    {
        shutdown = newState;
    };

    bool getShutdownFlag() const
    {
        return shutdown;
    };

    QByteArray getDevice() const { return device; }

private:

    //----------------------------------------------------------------------
    // Messages from/to the cumulus will be read/written via the
    // client IPC instance.
    //----------------------------------------------------------------------

    void readServerMsg();

    void writeServerMsg( const char *msg );

    void writeNotifMsg( const char *msg );

    uint getBaudrate(int rate);

    /**
     * put a new message into the process queue and sent a notification
     * to the server, if option notify is true.
     */
    void queueMsg( const char* msg );

    void readSentenceFromBuffer();

    //----------------------------------------------------------------------
    // Private data elements of Class
    //----------------------------------------------------------------------

private:

    // Serial device
    QByteArray device;

    // RX/TX rate of serial device
    uint ioSpeedTerminal, ioSpeedDevice;

    // data buffers and pointers
    char* datapointer;

    char  databuffer[1024];

    int   dbsize;

    // file descriptor to serial device
    int fd;

    // terminal info data
    struct termios oldtio, newtio;

    // Socket port for ipc to server process
    ushort ipcPort;

    // read file descriptor set in use by gps and ipc
    fd_set fdMask;

    // IPC instance to server process as data channel
    Ipc::Client clientData;

    // IPC instance to server process as notification channel
    Ipc::Client clientNotif;

    // used as timeout control for fix and connection
    QTime last;

    // Queue used for intermediate storing
    QQueue<QByteArray> queue;

    // If true, a notification is sent to the server, when new data are
    // available in the queue. After that the flag is reset and the server must
    // renew the request.
    bool notify;

    // flag to indicate gps connection lost
    bool connectionLost;

    // Shutdown flag for main loop. Will be set in case of fatal error
    // or if a shutdown message has been received from the server.
    bool shutdown;

    // Quality sentence counter
    int badSentences;
};

#endif  // #ifndef _GpsClient_hh_
