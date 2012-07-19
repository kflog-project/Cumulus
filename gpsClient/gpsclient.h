/***********************************************************************
**
**   gpsclient.h
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
 * \class GpsClient
 *
 * \author Axel Pauli
 *
 * \date 2004-2012
 *
 * \brief GPS client manager
 *
 * This class manages the connection and supervision to a GPS device via
 * different interfaces. Those can be:
 *
 * a) RS232
 * b) USB
 * c) a named pipe
 * d) Bluetooth via RFCOMM
 *
 * The communication between this client class and the Cumulus main
 * process is realized via two sockets. One socket for NMEA data message
 * transfer and a second socket for command exchange.
 */

#ifndef _GpsClient_hh_
#define _GpsClient_hh_ 1

#include <termios.h>
#include <unistd.h>

#include <QDateTime>
#include <QByteArray>
#include <QQueue>
#include <QSet>

#include "ipc.h"

//++++++++++++++++++++++ CLASS GpsClient +++++++++++++++++++++++++++

class GpsClient
{

public:

  /**
  * The constructor requires a socket port of the server (listening end point)
  * usable for interprocess communication. As host is always localhost
  * used. It will be opened two sockets to the server, one for data transfer,
  * the other only as command and notification channel.
  *
  * \param portIn The listening socket port of the server to be connected to.
  */
  GpsClient( const ushort portIn );

  virtual ~GpsClient();

  /**
   * Processes incoming read events. They can come from the server or
   * from the GPS device.
   *
   * \param fdMaskIn A mask of file descriptors ready for read.
   */
  void processEvent( fd_set *fdMaskIn );

  /**
   * Returns all currently used read file descriptors as mask, usable by the
   * select call.
   *
   * \return A file descriptor set to be checked for read events.
   */
  fd_set *getReadFdMask();

  /**
   * Reads data from the connected GPS device.
   *
   * @return true=success / false=unsuccess
   */
  bool readGpsData();

  int writeGpsData( const char *dataIn );

  /**
   * Opens a connection to the GPS device.
   *
   * \param deviceIn Name of the device.
   * \param ioSpeedIn Speed of the device.
   * \return True on success otherwise false.
   */
  bool openGps( const char *deviceIn, const uint ioSpeedIn );

  /**
   * Closes the connection to the GPS device.
   */
  void closeGps();

  /**
   * GPS device data timeout controller.
   */
  void toController();

  /**
   * Calculates the check sum over a NMEA record.
   *
   * \param sentence NMEA sentence to be checked.
   * \return The calculated check sum of sentence.
   */
  uchar calcCheckSum( const char *sentence );

  /**
   * Verify the checksum of the passed sentences.
   *
   * @returns true (success) or false (error occurred)
   */
  bool verifyCheckSum( const char *sentence );

  /**
   * Check GPS message key, if it shall be processed or filtered out.
   *
   * @returns true if processing desired otherwise false
   */
  bool checkGpsMessageFilter( const char *sentence );

  /**
   * \param newState The new value for the shutdown state.
   */
  void setShutdownFlag( bool newState )
  {
    shutdown = newState;
  };

  /**
   * \return The current value of the shutdown flag.
   */
  bool getShutdownFlag() const
  {
    return shutdown;
  };

  /**
   * \return The current used device.
   */
  QByteArray getDevice() const { return device; }

  private:

  //----------------------------------------------------------------------
  // Messages from/to Cumulus will be read/written via the
  // client IPC instance.
  //----------------------------------------------------------------------

  void readServerMsg();

  void writeServerMsg( const char *msg );

  void writeForwardMsg( const char *msg );

  uint getBaudrate( int rate );

  void readSentenceFromBuffer();

  //----------------------------------------------------------------------
  // Private data elements of Class
  //----------------------------------------------------------------------

  private:

  // Used GPS device. Can be a /dev/... device name, a named pipe or a
  // Bluetooth address.
  QByteArray device;

  // RX/TX rate of serial device
  uint ioSpeedTerminal, ioSpeedDevice;

  // data buffers and pointers
  char* datapointer;

  char  databuffer[1024];

  int   dbsize;

  // file descriptor to GPS device
  int fd;

  // terminal info data
  struct termios oldtio, newtio;

  // Socket port for IPC to server process
  ushort ipcPort;

  // read file descriptor set in use by GPS and IPC
  fd_set fdMask;

  // IPC instance to server process as data channel
  Ipc::Client clientData;

  // IPC instance to server process as message forward channel
  Ipc::Client clientForward;

  // used as timeout control supervision for the GPS device connection
  QTime last;

  // Flag to indicate forwarding of GPS data to the server process.
  bool forwardGpsData;

  // Flag to indicate GPS connection lost
  bool connectionLost;

  // Shutdown flag for main loop. Will be set in case of fatal error
  // or if a shutdown message has been received from the server.
  bool shutdown;

  // Quality sentence counter
  int badSentences;

  /**
   * Filter set with well known GPS message keys. Only GPS messages starting
   * with such a key are processed and forwarded.
   */
  QSet<QString> gpsMessageFilter;

  /**
   * Set containing reported unknown GPS message keys to avoid an endless error
   * reporting.
   */
  QSet<QString> unknownsReported;
};

#endif
