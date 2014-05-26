/***********************************************************************
**
**   gpsmaemoclient.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c): 2010-2012 by Axel Pauli (kflog.cumulus@gmail.com)
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
 * Because LibLocation in MAEMO4 does not report continuous altitude information
 * the GPS daemon is directly connected by this class.
 *
 * \date 2010-2012
 *
 * \version $Id$
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

#include "ipc.h"

class QTime;

//++++++++++++++++++++++ CLASS GpsMaemoClient +++++++++++++++++++++++++++

class GpsMaemoClient
{

public:

  /**
   * Constructor requires a socket port of the server (listening end
   * point) usable for interprocess communication. As related host is
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
   * @return all used read file descriptors as mask, usable by the
   * select call
   */
  fd_set *getReadFdMask();

  int writeGpsData( const char *dataIn );

  /**
  * The Maemo location service is called to start the selected device. That
  * can be the internal GPS or a BT GPS mouse.
  */
  bool startGpsReceiving();

  /**
   * The Maemo location service is called to stop GPS receiving.
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

#ifdef MAEMO4

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

#endif

private:

  //----------------------------------------------------------------------
  // Messages from/to the Cumulus will be read/written via the
  // client IPC instance.
  //----------------------------------------------------------------------

  void readServerMsg();

  void writeServerMsg( const char *msg );

  void writeForwardMsg( const char *msg );

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

  // IPC instance to server process as message forward channel
  Ipc::Client clientForward;

  // used as timeout control for fix and connection
  QTime last;

  // Defined time span in milli seconds for timeout supervision. If set to
  // zero Timeout handler do nothing.
  long timeSpan;

  // GPS running flag.
  bool gpsIsRunning;

  // Flag to indicate forwarding of GPS data to the server process.
  bool forwardGpsData;

  // flag to indicate GPS connection lost
  bool connectionLost;

  // Shutdown flag for main loop. Will be set in case of fatal error
  // or if a shutdown message has been received from the server.
  bool shutdown;

#ifdef MAEMO4

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

  // Socket port for IPC to GPS Daemon process
  ushort gpsDaemonPort;

  // IPC instance to GPS Daemon, only used by MAEMO4
  Ipc::Client gpsDaemon;

  // data buffers and pointers
  char* datapointer;
  char  databuffer[2048];
  int   dbsize;

  // counter used for read data check
  int   readCounter;

#endif

};

#endif
