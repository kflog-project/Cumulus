/***********************************************************************
**
**   flarmbincom.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
**   Thanks to Flarm Technology GmbH, who supported us.
**
***********************************************************************/

#ifndef FLARM_BIN_COM_H_
#define FLARM_BIN_COM_H_

/**
 * \class FlarmBinCom
 *
 * \author Flarm, Axel Pauli
 *
 * \date 2012
 *
 * \brief Flarm binary communication interface.
 *
 *  \version $Id$
 *
 */

// protocol version
#define MYVERSION           0x01

// FLASH Seitengrösse aus dataflash.h
#define PAGESIZE 528

// maximale frame size
#define MAXSIZE     600

// spezialzeichen
#define STARTFRAME   's'
#define ESCAPE       'x'
#define ESC_ESC      'U'
#define ESC_START    '1'

// Telegram IDs
#define FRAME_PING          0x01
#define FRAME_SETBAUDRATE   0x02
#define FRAME_SETLEDS           0x03
#define FRAME_FLASHUPLOAD   0x10
#define FRAME_EXIT          0x12

// IGC readout telegrams
#define FRAME_SELECTRECORD  0x20
#define FRAME_GETRECORDINFO 0x21
#define FRAME_GETIGCDATA    0x22

#define FRAME_ACK           0xA0
#define FRAME_NACK          0xB7

// speed keys for baud rate
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


class FlarmBinCom
{
 public:

  FlarmBinCom(int s);

  ~FlarmBinCom();

  bool ping();

  bool exit();

  bool setBaudRate(int nSpeedKey);

  bool selectRecord(int nRecord);

  /**
   * Copies the info string of the currently selected record
   * to sData (max 127 char.). Null terminated.
   */
  bool getRecordInfo(char* sData);

  /**
   * Returns a chunk of the IGC file.
   * String is null terminated, sData must at least hold 600 bytes.
   */
  bool getIGDData(char* sData, unsigned int* progress);

  /** Sends a message to the Flarm. */
  bool sendMsg(Message* mMsg);

  /** Receives a message from the Flarm. */
  bool rcvMsg(Message* mMsg);

 private:

  void send(unsigned char c);
  bool rcv(unsigned char* b);

  // Low level port methods.
  int writeChar(const unsigned char c);
  int readChar(unsigned char* b);

  unsigned short computeCRC(Message* mMsg);

  int m_Socket;
  unsigned short m_Seq;

};

#endif /* FLARM_BIN_COM_H_ */
