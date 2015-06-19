/***********************************************************************
**
**   flarmbincom.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012-2015 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   Thanks to Flarm Technology GmbH, who supported us.
**
***********************************************************************/

#ifndef FLARM_BIN_COM_H_
#define FLARM_BIN_COM_H_

/**
 * \class FlarmBinCom
 *
 * \author Flarm Technology GmbH, Axel Pauli
 *
 * \date 2012-2015
 *
 * \brief Flarm binary communication interface.
 *
 * \version 1.1
 *
 */

// protocol version
#define MYVERSION           0x01

#ifdef PAGESIZE
#undef PAGESIZE
// FLASH Seitengrösse aus dataflash.h
#define PAGESIZE 528
#endif

// Maximale frame size
#define MAXSIZE     600

// Spezialzeichen
#define STARTFRAME   's'
#define ESCAPE       'x'
#define ESC_ESC      'U'
#define ESC_START    '1'

// Telegram IDs
#define FRAME_PING          0x01
#define FRAME_SETBAUDRATE   0x02
#define FRAME_SETLEDS       0x03
#define FRAME_FLASHUPLOAD   0x10
#define FRAME_EXIT          0x12

// IGC readout telegrams
#define FRAME_SELECTRECORD  0x20
#define FRAME_GETRECORDINFO 0x21
#define FRAME_GETIGCDATA    0x22

#define FRAME_ACK           0xA0
#define FRAME_NACK          0xB7

// Speed keys for baud rate
#define SPEED_4800          0x00
#define SPEED_9600          0x01
#define SPEED_19200         0x02
#define SPEED_38400         0x04
#define SPEED_57600         0x05

typedef struct {
  unsigned short      length;      // 16 bit
  unsigned char       version;     // 8 bit
  unsigned short      seq;         // 16 bit
  unsigned char       type;        // 8 bit
  unsigned short      crc;         // 16 bit
} Header;

// Header length is _not_ sizeof( Header)!
#define HDR_LENGTH  8

typedef struct {
  Header hdr;
  unsigned char data[MAXSIZE];
} Message;

class QString;

class FlarmBinCom
{
 public:

  FlarmBinCom();

  virtual ~FlarmBinCom();

  /**
   * Checks the connection.
   */
  bool ping();

  /**
   * Resets the Flarm device back to the text-based protocol.
   */
  bool exit();

  /**
   * Set a new baud rate on the serial interface.
   * 0: 4800 bps
   * ...
   * 5: 57600 bps
   */
  bool setBaudRate( const int nSpeedKey );

  /**
   * Selects the flight record to be read as next.
   */
  bool selectRecord(const int nRecord);

  /**
   * Copies the info string of the currently selected record
   * to sData (max 127 char.). Null terminated.
   */
  bool getRecordInfo(char* sData);

  /**
   * Returns a chunk of the IGC file of the selected flight record.
   * String is null terminated, sData must at least hold 600 bytes.
   * Transfer is finished, if EOF 0x1A is sent as last character.
   *
   * \param sData character array for IGC chunk.
   *
   * \param progress Download progress in percent.
   *
   * \return true if data available in error case false.
   */
  bool getIGCData(char* sData, int* progress);

 protected:

  /** Low level write character port method. Must be implemented by the user. */
  virtual int writeChar(const unsigned char c) = 0;

  /** Low level read character port method. Must be implemented by the user. */
  virtual int readChar(unsigned char* b, const int timeout) = 0;

 private:

  /** Sends a message to the Flarm. */
  bool sendMsg(Message* mMsg);

  /** Receives a message from the Flarm. */
  bool rcvMsg(Message* mMsg, const int timeout);

  /** Sends a character in escape mode.*/
  void send(const unsigned char c);

  /** Gets a character in escape mode.*/
  bool rcv(unsigned char* b, const int timeout);

  /** Calculates the CRC checksum according too the XMODEM algorithm. */
  unsigned short computeCRC(Message* mMsg);

  /** Dumps the passed data array as hex string. */
  QString dumpHex( const uchar* data, int length );

  /** Message sequence number. */
  static unsigned short m_Seq;

  /** Default timeout in ms for reading from serial port. */
  static const int TimeoutNormal = 10000;

  /** Default timeout in ms for reading from serial port. */
  static const int TimeoutExit = 3000;

  /** Default timeout in ms for reading from serial port. */
  static const int TimeoutPing = 2000;

};

#endif /* FLARM_BIN_COM_H_ */
