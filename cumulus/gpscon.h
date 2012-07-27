/***********************************************************************
 **
 **   gpscon.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2004-2012 by Axel Pauli (axel@kflog.org)
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
 * \class GpsCon
 *
 * \author Axel Pauli
 *
 * \brief GPS connection manager.
 *
 * This module manages the startup and supervision of the GPS client process
 * and the communication between this client process and the Cumulus
 * process. All data transfer between the two processes is be done via a
 * socket interface. The path name, used during startup of Cumulus must be
 * passed in the constructor, that the gpsClient resp. gpsMaemoClient binary
 * can be found. It lays in the same directory as Cumulus.
 *
 * \date 2004-2012
 */

#ifndef GPS_CON_H
#define GPS_CON_H

#include <unistd.h>
#include <sys/types.h>

#include <QObject>
#include <QString>
#include <QMap>
#include <QSocketNotifier>
#include <QDateTime>
#include <QTimer>

#include "ipc.h"
#include "datatypes.h"

// Device name for NMEA simulator. This name is also taken for the named pipe.
#define NMEASIM_DEVICE "/tmp/nmeasim"

// GPS device name for TomTom
#define TOMTOM_DEVICE "/var/run/gpspipe"

// Device name for the Maemo location service.
#define MAEMO_LOCATION_SERVICE "GPS Location"

// Bluetooth default adapter of bluez stack.
#define BT_ADAPTER "BT Adapter"

class GpsCon : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY( GpsCon )

 public:

    GpsCon(QObject*, const char *path);

    virtual ~GpsCon();

    /**
     * Starts a new GPS client process via fork/exec or checks, if process is
     * alive. Alive check is triggered by timer routine every 10s. If process
     * is down, a new one will be started.
     */
    bool startClientProcess();

    /**
     * This function returns the currently used baud rate for this connection
     */
    int currentBautrate()const
      {
        return ioSpeed;
      };

    /**
     * This function returns the currently used device for this connection
     */
    QString currentDevice()const
      {
        return device;
      };

    /** This function returns the current PID of the client process or
     * -1 if there isn't any
     */
    int getPid() const
      {
        return pid;
      };

    /**
     * Sends NMEA input sentence to GPS receiver. Checksum is calculated by
     * this routine. Don't add an asterix at the end of the passed sentence!
     * That is part of the check sum.
     *
     * \return true in case of success otherwise false.
     */
    bool sendSentence(const QString& sentence);

    /**
     * Device name and speed are sent to the client, that it
     * can open the connection to the GPS device for data receiving.
     */
    bool startGpsReceiving();

    /**
     * Stops the GPS receiver on the client side.
     */
    bool stopGpsReceiving();

    /**
     * Returns the socket notifier of the daemon connection.
     */
    QSocketNotifier* getDaemonNotifier() const
    {
      return clientNotifier;
    };

    /**
     * Sends the well known GPS message keys to the GPS client process.
     * Only GPS sentences starting with such a key are processed and forwarded
     * to the Cumulus process. That shall avoid unneeded traffic between the
     * two processes.
     */
    void sendGpsKeys();

#ifdef FLARM

    /** Requests a flight list from a Flarm device. */
    bool getFlightListFromFlarm();

    /** Requests the download of the passed flight indexes from the Flarm
     * device.
     */
    bool downloadFlightsFromFlarm( QString& flightIndexes );

#endif

  signals:
    /**
     * This signal is send every time a new sentence has arrived.
     */
    void newSentence(const QString& sentence);

    /**
     * This signal is send, if the GPS connection has been lost.
     */
    void gpsConnectionOff();

    /**
     * This signal is send, if the GPS connection has been established.
     */
    void gpsConnectionOn();

    /**
     * This signal is emitted, when a new Flarm flight list was received.
     */
    void newFlarmFlightList(const QString& list);

    /**
     * This signal is emitted, when a new Flarm flight download info was received.
     */
    void newFlarmFlightDownloadInfo(const QString& info);

    /**
     * This signal is emitted, when a new Flarm flight download progress was received.
     */
    void newFlarmFlightDownloadProgress(const int idx, const int progress);

 private:

    /**
     * Stores process identifier of forked client
     */
    void setPid( pid_t newPid )
    {
      pid = newPid;
    };

    /**
     * Reads a client message from the socket. The protocol consists of two
     * parts. First the message length is read as unsigned integer, after that
     * the actual message as 8 bit character string.
     */
    void readClientMessage( uint index, QString &result );

    /**
     * Writes a client message to the socket. The protocol consists of two
     * parts. First the message length is read as unsigned integer, after that
     * the actual message as 8 bit character string.
     */
    void writeClientMessage( uint index, const char *msg  );

    /**
     * Gets the GPS or status data from the client. The retrieved data will
     * be hand over to the Cumulus process.
     */
    void getDataFromClient();

    /**
     * Triggers a connection retry in case of error.
     */
    void triggerRetry();

  private slots:
    /**
     * This slot is triggered by the QT main loop and is used to handle the
     * notification events from the client.
     */
    void slot_NotificationEvent(int socket);

    /**
     * This slot is triggered by the QT main loop and is used to handle the
     * listen socket events. The GPS client tries to connect to the Cumulus
     * process. There are two connections opened by the client, first as data
     * channel, second as notification channel.
     */
    void slot_ListenEvent(int socket);

    /**
     * This timeout method is used, to call the method startClientProcess(),
     * when the timer is expired. This is the alive check for the forked
     * gpsClient process and ensures the cleaning up of zombies.
     */
    void slot_Timeout();

#ifdef BLUEZ

    /**
     * This slot is called by the Bluetooth device search thread, to
     * return the search results. In dependency of the results the
     * connection to the BT GPS device will be established or not.
     *
     * \param ok        True means BT devices have been found. In case of
     *                  false an error message is contained in parameter
     *                  error.
     * \param error     An error string, if ok is false.
     * \param devices   Found bluetooth devices. Key is the logical name,
     *                  value is the bluetooth address.
     *
     */
    void slot_StartGpsBtReceiving( bool ok,
                                   QString error,
                                   BtDeviceMap devices );
#endif

  private:

    // gpsClient program name with path
    QString exe;

    // start client process flag, used for debugging purposes only
    bool startClient;

    // Pid of GPS client process
    pid_t pid;

    // Notifier for QT main loop
    QSocketNotifier *listenNotifier;
    QSocketNotifier *clientNotifier;

    // used as timeout control for connection supervision
    QTimer *timer;

    // Time of last client query
    QTime lastQuery;

    // Socket port for ipc to server process
    ushort ipcPort;

    // IPC instance to client process
    Ipc::Server server;

    // RX/TX rate of serial device
    uint ioSpeed;

    // GPS device name
    QString device;
 };

#endif
