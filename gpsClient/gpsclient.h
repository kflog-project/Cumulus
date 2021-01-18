/***********************************************************************
**
**   gpsclient.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2021 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***********************************************************************/

/**
 * \class GpsClient
 *
 * \author Axel Pauli
 *
 * \date 2004-2021
 *
 * \brief GPS client manager
 *
 * This class manages the connection and supervision to a GPS or other device
 * via different interfaces. Those can be:
 *
 * a) RS232
 * b) USB
 * c) a named pipe
 * d) Bluetooth via RFCOMM
 * c) 1-2 TCP sockets
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
#include <QString>
#include <QStringList>
#include <QTcpSocket>
#include <QTime>

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
   * Reads NMEA data from the connected GPS device via tty, BT or pipe.
   *
   * @return true=success / false=unsuccess
   */
  bool readGpsData();

  /**
   * Reads NMEA data from the connected socket 1
   *
   * @return true=success / false=unsuccess
   */
  bool readSocket1Data();

  /**
   * Reads NMEA data from the connected socket 2
   *
   * @return true=success / false=unsuccess
   */
  bool readSocket2Data();

  /**
   * Writes data to the connected GPS device.
   */
  int writeGpsData( const char *dataIn );

  /**
   * Opens a connection to the GPS device via tty, BT or pipe.
   *
   * \param deviceIn Name of the device.
   * \param ioSpeedIn Speed of the device.
   * \return True on success otherwise false.
   */
  bool openGps( const char *deviceIn, const uint ioSpeedIn );

  /**
   * Opens a connection to one or two NMEA sockets. E.g. XC-Vario. The socket
   * data are contained in the class members so1Data and so2Data.
   *
   * \return True on success otherwise false.
   */
  bool openNmeaSockets();

  /**
   * CCloses all connections to the external GPS devices.
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
   * \return The current used non socket device.
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

#ifdef FLARM

  /** Gets the flight list from the Flarm device. */
  void getFlarmFlightList();

  /** Reports an error to the calling application. */
  void flarmFlightListError();

  /**
   * Downloads the requested IGC flights. The args string contains the destination
   * directory and one or more flight numbers. The single elements are separated
   * by vertical tabs.
   */
  void getFlarmIgcFiles( QString& args );

  /** Reports an info to the calling application. */
  void flarmFlightDowloadInfo( QString info );

  /** Reports the download progress to the calling application. */
  void flarmFlightDowloadProgress( const int idx, const int progress );

  /**
   * Switches the Flarm device into the binary mode.
   *
   * \return True on success otherwise false.
   */
  bool flarmBinMode();

  /**
   * Resets the Farm device. Should be called only if Flarm is in binary mode.
   */
  bool flarmReset();

#endif

  //----------------------------------------------------------------------
  // Private data elements of Class
  //----------------------------------------------------------------------

  private:

  // Used GPS device. Can be a tty device name, a named pipe or a
  // Bluetooth address.
  QByteArray device;

  // RX/TX rate of serial device
  uint ioSpeedTerminal, ioSpeedDevice;

  // data buffers and pointers
  char* datapointer;

  char  databuffer[1024];

  int   dbsize;

  // file descriptor to TTY GPS device
  int fd;

  // TCP socket 1 to e.g.XC-Vario data channel
  QTcpSocket* so1;

  // IP and port data of socket 1
  QStringList so1Data;

  // TCP socket 2 to e.g.XC-Vario Flarm data channel
  QTcpSocket* so2;

  // IP and port data of socket 2
  QStringList so2Data;

  // Pointer to Flarm socket
  QTcpSocket* soFlarm;

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

  /** activate flag for timeout after Flarm reset. */
  bool activateTimeout;

  /** Timeout control for Flarm IGC download. */
  QTime downloadTimeControl;
};

#endif
