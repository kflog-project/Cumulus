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

#include <cerrno>
#include <cstring>

#include <QtGui>

#include "flarmcrc.h"
#include "flarmbincom.h"

unsigned short FlarmBinCom::m_Seq = 0;

// Enable DEBUG_SR to dump out the messages on the interface in hex format
#define DEBUG_SR 1

FlarmBinCom::FlarmBinCom()
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
      return false;
    }

  if( m.data[0] != (m_Seq & 0xff) && m.data[1] != (m_Seq >> 8) )
    {
      qWarning() << "Ping answer SeqNo wrong!";
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

bool FlarmBinCom::setBaudRate( const int nSpeedKey )
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

bool FlarmBinCom::selectRecord( const int nRecord )
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

  // strncpy_s(sData, MAXSIZE, (const char*) &(m.data[2]), m.hdr.length - 2 - HDR_LENGTH);
  strncpy( sData, (const char*) &(m.data[2]), m.hdr.length - 2 - HDR_LENGTH );

  sData[m.hdr.length - 2 - HDR_LENGTH] = 0;
  return true;
}

bool FlarmBinCom::getIGCData( char* sData, unsigned int* progress)
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
      // @AP Note: The right end of the transmission is reached, if the last
      // character of a record is EOF (0x1A).
      sData[0] = 0;
      return true;
    }
  else if (m.hdr.type != FRAME_ACK)
    {
      return false;
    }

  // progress
  *progress = (int) m.data[2];

  // skip first two bytes (sequence number) and progress
  char* igcdata = (char*) &(m.data[3]);
  int dataSize = m.hdr.length - 3 - HDR_LENGTH;

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

  // add the next sequence number
  mMsg->hdr.seq = ++m_Seq;

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

  header[6] = crc & 0xff;
  header[7] = crc >> 8;

#ifdef DEBUG_SR
  QString dump = dumpHex( (const uchar*) "s", 1) +
                 dumpHex( header, HDR_LENGTH) +
                 dumpHex( mMsg->data, mMsg->hdr.length - HDR_LENGTH);

  qDebug() << "S:" << dump;
#endif

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

  do
    {
      // printf( "Waiting for start frame\n");
      if( readChar(&ch) <= 0 )
        {
          // printf( "No start frame\n");
          return false;
        }
    }

  while (STARTFRAME != ch);

  unsigned char hdr[HDR_LENGTH];

  // receive header
  for (int i = 0; i < HDR_LENGTH; i++)
    {
      if (rcv(&hdr[i]) == false)
        {
          return false;
        }
    }

  mMsg->hdr.length = hdr[0] + (hdr[1] << 8);
  mMsg->hdr.version = hdr[2];
  mMsg->hdr.seq = hdr[3] + (hdr[4] << 8);
  mMsg->hdr.type = hdr[5];
  mMsg->hdr.crc = hdr[6] + (hdr[7] << 8);

  if( (mMsg->hdr.length - HDR_LENGTH) > MAXSIZE )
    {
      qWarning() << "FlarmBinCom::rcvMsg() buffer overflow! bs="
                  << MAXSIZE << "ds=" << (mMsg->hdr.length - HDR_LENGTH);
    }

  // receive payload
  for (int i = 0; i < mMsg->hdr.length - HDR_LENGTH; i++)
    {
      if (rcv(&mMsg->data[i]) == false)
        {
          // printf( "Receiving payload failed\n");
          return false;
        }
    }

#ifdef DEBUG_SR
  QString dump = dumpHex( (const uchar*) "s", 1) +
                 dumpHex( hdr, HDR_LENGTH) +
                 dumpHex( mMsg->data, mMsg->hdr.length - HDR_LENGTH);

  qDebug() << "R:" << dump;
#endif

  // Check sequence numbers.
  if( mMsg->data[0] != (m_Seq & 0xff) && mMsg->data[1] != (m_Seq >> 8) )
    {
      qWarning( "RcvMsg: SeqNo mismatch! RMT=0x%02X, Sent=%04x, Rev=%04x",
                  mMsg->hdr.type, m_Seq, mMsg->data[0] + (mMsg->data[1] << 8) );
    }

  // check crc
  unsigned short crc = computeCRC(mMsg);

  if (crc != mMsg->hdr.crc)
    {
      return false;
    }

  return true;
}

bool FlarmBinCom::rcv( unsigned char* b)
{
  *b = 0xff;

  if( readChar(b) <= 0 )
    {
      return false;
    }

  if (*b == ESCAPE)
    {
      if( readChar(b) <= 0 )
        {
          return false;
        }

      switch (*b)
        {
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

void  FlarmBinCom::send( const unsigned char c)
{
  switch( c )
    {
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

/**
 * CRC computation. Length information in header must be correct!
 */
unsigned short FlarmBinCom::computeCRC( Message* mMsg)
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

QString FlarmBinCom::dumpHex( const uchar* data, int length )
{
  QString out;

  for( int i = 0; i < length; i++ )
    {
      ushort byte = data[i];
      out += QString("%1 ").arg( byte, 2, 16, QChar('0'));
    }

  return out;
}
