/***********************************************************************
**
**   flarmbincom.cpp
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

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "flarmcrc.h"
#include "flarmbincom.h"

unsigned short FlarmBinCom::m_Seq = 0;

FlarmBinCom::FlarmBinCom( int socket) :
  m_Socket(socket)
{
}

FlarmBinCom::~FlarmBinCom()
{
}

//////////////////////////////////
// commands
//////////////////////////////////

bool FlarmBinCom::ping()
{
  Message m;
  m.hdr.type = FRAME_PING;
  m.hdr.length = HDR_LENGTH;
  m.hdr.version = 0x01;
  sendMsg(&m);

  if(rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type != FRAME_ACK)
    {
      // printf( "Nack \r\n");
      return false;
    }

  return true;
}

bool FlarmBinCom::exit()
{
  Message m;
  m.hdr.type = FRAME_EXIT;
  m.hdr.length = HDR_LENGTH;
  m.hdr.version = 0x01;
  sendMsg(&m);

  if (rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  return true;
}

bool FlarmBinCom::setBaudRate( int nSpeedKey)
{
  Message m;
  m.hdr.type = FRAME_SETBAUDRATE;
  m.hdr.length = HDR_LENGTH + 1;
  m.hdr.version = 0x01;
  m.data[0] = nSpeedKey;
  sendMsg(&m);

  if (rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  return true;
}

bool FlarmBinCom::selectRecord( int nRecord)
{
  Message m;
  m.hdr.type = FRAME_SELECTRECORD;
  m.hdr.length = HDR_LENGTH + 1;
  m.hdr.version = 0x01;
  m.data[0] = nRecord;
  sendMsg(&m);

  if (rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  return true;
}


bool FlarmBinCom::getRecordInfo( char* sData )
{
  Message m;
  m.hdr.type = FRAME_GETRECORDINFO;
  m.hdr.length = HDR_LENGTH;
  m.hdr.version = 0x01;
  sendMsg(&m);

  if (rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  /*
   *
   errno_t strncpy_s(
   char *strDest,
   size_t sizeInBytes,
   const char *strSource,
   size_t count);
   */
  // strncpy_s(sData, MAXSIZE, (const char*) &(m.data[2]), m.hdr.length - 2 - HDR_LENGTH);
  strncpy( sData, (const char*) &(m.data[2]), m.hdr.length - 2 - HDR_LENGTH );

  sData[m.hdr.length - 2 - HDR_LENGTH] = 0;
  // printf( "Length: %i\r\n", m.hdr.length);

  return true;
}

bool FlarmBinCom::getIGDData( char* sData, unsigned int* progress)
{
  Message m;
  m.hdr.type = FRAME_GETIGCDATA;
  m.hdr.length = HDR_LENGTH;
  m.hdr.version = 0x01;
  sendMsg(&m);

  if (rcvMsg(&m) == false)
    {
      return false;
    }

  if (m.hdr.type == FRAME_NACK)
    {
      // finished, closing record
      sData[0] = 0;
      return true;
    }
  else if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  // progress
  *progress = (int) m.data[2];
  char* igcdata = (char*) &(m.data[3]); // skip first two bytes (sequence number) and progress
  int dataSize = m.hdr.length - 3 - HDR_LENGTH;

  /* copy over
  void *_memccpy(
     void *dest,
     const void *src,
     int c,
     size_t count
   ); */
  // _memccpy(sData, igcdata, 0, dataSize);

  memccpy(sData, igcdata, 0, dataSize);
  // ensure null termination
  sData[dataSize] = 0;
  return true;
}

///////////////////////////////
// low level stuff
///////////////////////////////

bool FlarmBinCom::sendMsg( Message* mMsg)
{
  // prepare/copy header
  unsigned char header[HDR_LENGTH];

  // add sequence number
  mMsg->hdr.seq = m_Seq;
  m_Seq++;

  // length
  header[0] = mMsg->hdr.length & 0xff;
  header[1] = (mMsg->hdr.length >> 8);

  // version
  header[2] = mMsg->hdr.version;

  // sequence
  header[3] = mMsg->hdr.seq & 0xff;
  header[4] = (mMsg->hdr.seq >> 8);

  // type
  header[5] = mMsg->hdr.type;

  // crc
  unsigned short crc = computeCRC(mMsg);

  header[6] = crc & 0xff; //mCrc.getCRC() & 0xff;
  header[7] = crc >> 8; // mCrc.getCRC() >> 8;

  // send the stuff
  writeChar(STARTFRAME);

  for (int i = 0; i < HDR_LENGTH; i++)
    {
      send(header[i]);
    }

  for (int i = 0; i < mMsg->hdr.length - HDR_LENGTH; i++)
    {
      send(mMsg->data[i]);
    }

  return false;
}

bool FlarmBinCom::rcvMsg( Message* mMsg)
{
  // wait for start frame
  unsigned char ch = 0;
  /*
   SYSTEMTIME  start, end;
   GetSystemTime( & start);
   */
  do
    {
      // printf( "Waiting for startframe\n");
      if( readChar(&ch) <= 0 )
        {
          // printf( "No StartFrame\n");
          return false;
        }
    }

  while (STARTFRAME != ch);
  /*
   GetSystemTime( & end);
   printf( "r: %li  ", -start.wSecond*1000 - start.wMilliseconds + end.wSecond*1000 + end.wMilliseconds);
   printf( "startframe found\n");
   */

  unsigned char hdr[HDR_LENGTH];

  // receive header
  for (int i = 0; i < HDR_LENGTH; i++)
    {
      if (rcv(&hdr[i]) == false)
        {
          // printf( "Unable to receive header\n");
          return false;
        }
    }

  mMsg->hdr.length = hdr[0] + (hdr[1] << 8);
  mMsg->hdr.version = hdr[2];
  mMsg->hdr.seq = hdr[3] + (hdr[4] << 8);
  mMsg->hdr.type = hdr[5];
  mMsg->hdr.crc = hdr[6] + (hdr[7] << 8);

  // receive payload
  for (int i = 0; i < mMsg->hdr.length - HDR_LENGTH; i++)
    {
      if (rcv(&mMsg->data[i]) == false)
        {
          // printf( "Receiving payload failed\n");
          return false;
        }
    }

  // check crc
  unsigned short crc = computeCRC(mMsg);

  if (crc != mMsg->hdr.crc)
    {
      // printf( "CRC wrong!\n");
      return false;
    }

  return true;
}


bool FlarmBinCom::rcv( unsigned char* b)
{
  *b = 0xff;

  if( readChar(b) <= 0 )
    {
      // printf( "received %i bytesm char 0x %x\r\n", nb, *b);
      return false;
    }

  // printf( "r:0x%02x ", *b);
  if (*b == ESCAPE)
    {
      if( readChar(b) <= 0 )
        {
          // printf( "Read failed\n");
          return false;
        }

      switch (*b)
        {
          // case STARTFRAME : return true; break;
          case ESC_ESC:
            *b = ESCAPE;
            break;
          case ESC_START:
            *b = STARTFRAME;
            break;
          default:
            *b = 0;
            return false;
        }

      return true;
    }

  return true;
}


void  FlarmBinCom::send( unsigned char c)
{
  switch (c)
    {
      // DWORD nb;
      case STARTFRAME:
        writeChar(ESCAPE);
        writeChar(ESC_START);
        break;
      case ESCAPE:
        writeChar(ESCAPE);
        writeChar(ESC_ESC);
        break;
      default:
        writeChar(c);
        break;
     }
}


int FlarmBinCom::writeChar(const unsigned char c)
{
  int done = -1;

  while(true)
    {
      done = write( m_Socket, &c, sizeof(c) );

      if( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }
        }

      break;
    }

  return done;
}

int FlarmBinCom::readChar(unsigned char* b)
{
  // Check, if something available to read
  int done = ioctl( m_Socket, FIONREAD, &done );

  if( done < 0 )
    {
      // Error
      return done;
    }

  if( done > 0 )
    {
      // we can read bytes and return
      return read( m_Socket, b, sizeof(unsigned char) );
    }

  // No data available, wait for it until timeout
  int maxFds = getdtablesize();

  fd_set readFds;
  FD_ZERO( &readFds );
  FD_SET( m_Socket, &readFds );

  struct timeval timerInterval;
  timerInterval.tv_sec  =  3;
  timerInterval.tv_usec =  0;

  done = select( maxFds, &readFds, (fd_set *) 0, (fd_set *) 0, &timerInterval );

  if( done <= 0 )
    {
      // done = -1 -> Error
      // done = 0  -> Timeout
      return done;
    }

  return read( m_Socket, b, sizeof(unsigned char) );
}

/**
 * CRC computation. Length information in header must be correct!
 */
unsigned short FlarmBinCom::computeCRC( Message * mMsg)
{
  FlarmCrc mCrc;

  // header
  mCrc.update(mMsg->hdr.length & 0xff);
  mCrc.update(mMsg->hdr.length >> 8);
  mCrc.update((mMsg->hdr.version));
  mCrc.update(mMsg->hdr.seq & 0xff);
  mCrc.update(mMsg->hdr.seq >> 8);
  mCrc.update(mMsg->hdr.type & 0xff);

  // payload crc
  for (int i = 0; i < (mMsg->hdr.length - HDR_LENGTH); i++)
    {
      mCrc.update(mMsg->data[i]);
    }

  return mCrc.getCRC();
}
