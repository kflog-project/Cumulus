/***********************************************************************
**
**   gpsclient.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

using namespace std;

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <QStringList>

#include "gpsclient.h"
#include "gpscon.h"
#include "protocol.h"
#include "ipc.h"

#ifdef DEBUG
#undef DEBUG
#endif


// Size of internal message queue.
#define QUEUE_SIZE 500

// define connection lost timeout in milli seconds
#define TO_CONLOST  10000

// Constructor requires a socket port of the server (listening end point)
// useable for interprocess communication. As related host is always localhost
// used. It will be opened two sockets to the server, one for data transfer,
// the other only as notification channel.

GpsClient::GpsClient( const ushort portIn )
{
  device           = "";
  ioSpeed          = B0;
  ipcPort          = portIn;
  fd               = -1;
  notify           = false;
  connectionLost   = true;
  shutdown         = false;
  datapointer      = databuffer;
  dbsize           = 0;
  lastGpsErrorTime = 0;

  // establish a connection to the server

  if( ipcPort )
    {
      if( clientData.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          cerr << "Command channel Connection to Cumulus Server failed!"
          << " Fatal error, terminate process!" << endl;

          setShutdownFlag(true);
          return;
        }

      if( clientNotif.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          cerr << "Notification channel Connection to Cumulus Server failed!"
          << " Fatal error, terminate process!" << endl;

          setShutdownFlag(true);
          return;
        }
    }
}

GpsClient::~GpsClient()
{
  closeGps();
}

/**
 * Return all currently used read file descriptors as mask, useable by the
 * select call
 */

fd_set *GpsClient::getReadFdMask()
{
  // Reset file descriptor mask bits
  FD_ZERO( &fdMask );

  if( fd != -1 ) // serial device
    {
      FD_SET( fd, &fdMask );
    }

  if( ipcPort ) // data channel to server
    {
      int sfd = clientData.getSock();

      if( sfd != -1 )
        {
          FD_SET( sfd, &fdMask );
        }
    }

  return &fdMask;
}


// Processes incoming read events. They can come from the server or
// from the Gps device.

void GpsClient::processEvent( fd_set *fdMask )
{
  if( fd != -1 && FD_ISSET( fd, fdMask ) )
    {
      if( readGpsData() == false )
        {
          // problem occured, likely buffer overrun. we restart the gps
          // receiving.
          closeGps();
          openGps( device, ioSpeed );
        }
    }

  // This function is called every 20 seconds from cumulus main. If
  // the bluetooth host is down (GPS device off) but the bluetooth
  // dongle is connected, the open takes 20 seconds and cumulus is
  // blocking that time. (This 20 seconds come from bluetooth
  // setup and are not related to the 20 seconds from cumulus,
  // checking if gpsclient is alive.)  But immediately if the
  // bluetooth dongle is removed (or not present), the open returns.

#define GPS_RETRY_TIME 10

  if( fd == -1 && time(NULL) - lastGpsErrorTime > GPS_RETRY_TIME &&
      device.contains("rfcomm") )
    {
      openGps( device, ioSpeed );
    }

  if( ipcPort )
    {
      int sfd = clientData.getSock();

      if( sfd != -1 && FD_ISSET( sfd, fdMask ) )
        {
          readServerMsg();
        }
    }
}


// returns true=success / false=unsuccess

bool GpsClient::readGpsData()
{
  if( fd == -1 ) // no connection is active
    {
      return false;
    }

  // all available gps data lines are read successive

  int bytes = 0;

  bytes = read(fd, datapointer, sizeof(databuffer) - dbsize -1);

  if( bytes == 0 ) // Nothing read, should normally not happen
    {
      cerr << "Read has read 0 bytes!" << endl;
      return false;
    }

  if( bytes == -1 )
    {
      cerr << "Read error, errno="
      << errno << ", " << strerror(errno) << endl;
      return false;
    }

  if( bytes > 0 )
    {
      dbsize      += bytes;
      datapointer += bytes;

      databuffer[dbsize] = '\0'; // terminate buffer with a null

      readSentenceFromBuffer();

      // update supervision timer/variables
      last.restart();
      connectionLost = false;
    }

  return true;
}


// Sends a NMEA sentence to the GPS. Check sum will be calculated by
// this routine. Don't add an asterix at the end of the sentance. It
// will be part of the check sum.

int GpsClient::writeGpsData( const char *sentence )
{
  // don't try to send anything if there is no valid connection
  // available
  if( fd < 0 )
    {
      return -1;
    }

  uint csum = calcCheckSum( sentence );
  QString check;
  check.sprintf ("*%02X\r\n", csum);
  QString cmd (sentence + check);

  // write sentence to gps device
  int result = write (fd, cmd.toLatin1(), cmd.length());

  if (result != (int) cmd.length())
    {
      cerr << "Only " << result
           << " characters were written: "
           << cmd.toLatin1().data() << endl;
    }

  return result;
}


// Opens the connection to the Gps. All old messages in the queue will
// be removed.

bool GpsClient::openGps( const char *deviceIn, const uint ioSpeedIn )
{
  device  = deviceIn;
  ioSpeed = getBaudrate(ioSpeedIn);
  bool fifo = false;

  // remove all old queued messages
  queue.clear();

  // reset buffer pointer
  datapointer = databuffer;
  dbsize = 0;
  memset( databuffer, 0, sizeof(databuffer) );

  // create a fifo for the nmea simulator, if device starts not with /dev/
  if( strncmp( "/dev/", deviceIn, strlen("/dev/") ) != 0 )
    {
      int ret = mkfifo(device, S_IRUSR | S_IWUSR);

      if(ret && errno != EEXIST) perror("mkfifo"); 
      else fifo = true;
    }

  if( fd != -1 )
    {
      // closes an existing connection before opening a new one
      closeGps();
    }

  fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

  if( fd == -1 )
    {
      perror( "Open GPS device:" );

      // Could not open the serial device.
      if( lastGpsErrorTime == 0 )
        {
          // print this message only the first time
          cerr << "openGps: Unable to open serial device "
               << device.data() << " at transfer rate "
          << ioSpeedIn << endl;
        }

      lastGpsErrorTime = time(NULL);
      return false;
    }

  lastGpsErrorTime = 0;

  if(!fifo) {
    // fifo needs no serial initialization.
    if( ! isatty(fd) )
      {
        // Not a tty file descriptor.
        cerr << "openGps: Serial device '" << device.data()
             << "' is not connected to a TTY!" << endl;
        return false;
      }
    else
      {
        tcgetattr(fd, &oldtio); // get current options from port
        
        fcntl(fd, F_SETFL, FNDELAY); // NON blocking io is requested
        
        // copy current values into new structure for changes
        memcpy( &newtio, &oldtio, sizeof(newtio) );
        
        // prepare new port settings for:
        // - canonical input (line oriented input)
        // - 8 data bits
        // - no parity
        // - no cr
        // - blocking mode
        
        newtio.c_cflag = (CSIZE & CS8) | CLOCAL | CREAD;
        
        newtio.c_iflag = IGNPAR | IGNCR; // no parity and no cr
        
        newtio.c_oflag = ONLCR; // map nl to cr-nl
        
        newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG );
        
        newtio.c_cc[VMIN] = 1;
        
        newtio.c_cc[VTIME] = 0;
        
        // AP: Note, the setting of the speed must be done at last
        // because the manipulation of the c_iflag and c_oflag can
        // destroy the already assigned values! Needed me several hours
        // to find out that. Setting the baud rate under c_cflag seems
        // also to work.
        
        cfsetispeed( &newtio, ioSpeed ); // set baud rate for input
        cfsetospeed( &newtio, ioSpeed ); // set baud rate for output
        
        tcflush(fd, TCIOFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);
      }
  }
  last.start(); // store time point for supervision control
  return true;
}


/**
 * This method tries to read all lines contained in the receive buffer. A line
 * is always terminated by a newline. If it finds any, it sends them as
 * QStrings via the newsentence signal to whoever is listening (that will be
 * GPSNMEA) and removes the sentences from the buffer.
 */

void GpsClient::readSentenceFromBuffer()
{
  char *start = databuffer;
  char *end   = 0;

  while(strlen(start))
    {
      // Search for a newline in the receiver buffer
      if( ! (end = strchr( start, '\n' )) )
        {
          // No newline in the receiver buffer, wait for more
          // characters
          return;
        }

      if( start == end )
        {
          // skip newline and start at next position with a new search
          start++;
          continue;
        }

      // found a complete record in the buffer, it will be extracted
      // now
      char *record = (char *) malloc( end-start + 2 );

      memset( record, 0, end-start + 2 );

      strncpy( record, start, end-start + 1);

      // store sentence in the receiver queue
      queueMsg( record );

#ifdef DEBUG

      cout << "GpsClient(): Extracted NMEA Record: " << record;
#endif

      free(record);
      record = 0;

      // remove queued record from receive buffer
      memmove( databuffer, end+1,
               databuffer + sizeof(databuffer)-1 - end );

      datapointer -= (end+1 - databuffer);

      dbsize -= ( end+1 - databuffer);

      start = databuffer;

      end = 0;
    }
}


/**
 * closes the connection to the Gps
 */

void GpsClient::closeGps()
{
  if( fd != -1 )
    {
      if( isatty(fd) )
        {
          tcflush(fd, TCIOFLUSH);

          newtio.c_ispeed = B0;
          newtio.c_ospeed = B0;

          cfsetispeed( &newtio, B0 ); // set baud rate for input
          cfsetospeed( &newtio, B0 ); // set baud rate for output

          tcsetattr( fd, TCSANOW, &newtio ); // stop tty

          oldtio.c_cflag |= HUPCL;  // make sure DTR goes down

          tcsetattr( fd, TCSANOW, &oldtio );
        }
      
      fcntl( fd, F_SETFL, 0 ); // reset channel to blocking mode
      close(fd);
      fd = -1;
    }

  // remove all queued messages
  queue.clear();
  connectionLost = true;
}


// calculate check sum over nmea record

uchar GpsClient::calcCheckSum( const char *sentence )
{
  uchar sum = 0;

  for( uint i=1; i<strlen(sentence); i++ )
    {
      uchar c = (uchar) sentence[i];

      if( c == '$' ) // Start sign will not be considered
        continue;

      if( c == '*' ) // End of sentence reached
        break;

      sum ^= c;
    }

  return sum;
}


// timeout controler
void GpsClient::toController()
{
  if( last.elapsed() > TO_CONLOST && connectionLost == false )
    {
      // connection is lost, send only one message to the server
      connectionLost = true;
      queueMsg( MSG_CONLOST );
    }
}


// Reads a server message from the socket. The protocol consists of
// two parts. First the message length is read as unsigned
// integer, after that the actual message as 8 bit character string.

void GpsClient::readServerMsg()
{
  uint msgLen = 0;

  uint done = clientData.readMsg( &msgLen, sizeof(msgLen) );

  if( done <= 0 )
    {
      clientData.closeSock();
      return; // Error occurred
    }

  if( msgLen > 256 )
    {
      // such messages length are not defined. we will ignore that.
      cerr << "GpsClient::readServerMsg(): "
      << "message " << msgLen << " too large, ignoring it!"
      << endl;
      return;
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = clientData.readMsg( buf, msgLen );

  if( done <= 0 )
    {
      clientData.closeSock();
      delete [] buf;
      buf=NULL;
      return; // Error occurred
    }

#ifdef DEBUG
  cout << "GpsClient::readServerMsg(): Received Message: " << buf << endl;
#endif

  // Split the received message into its single parts. Space is used
  // as separator.

  QString qbuf( buf );
  QStringList args = qbuf.split(" ");
  delete [] buf;
  buf=NULL;

  // look, what server is requesting

  if( MSG_GM == args[0] )
    {
      // Get message is requested. We take the oldest element out of the
      // queue, if there is any.

      if( queue.count() == 0 ) // queue empty
        {
          writeServerMsg( MSG_NEG );
          return;
        }

      QByteArray msg = queue.dequeue();
      QByteArray res = QByteArray(MSG_RM) + " " + msg;

      writeServerMsg( res.data() );
      msg=0;
    }
  else if( MSG_MAGIC == args[0] )
    {
      // check protocol versions, reply with pos or neg
      if( MSG_PROTOCOL != args[1] )
        {
          cerr << "GpsClient::readServerMsg(): "
          << "Message protocol versions are incompatible, "
          << "closes connection and shutdown client" << endl;

          writeServerMsg( MSG_NEG );
          setShutdownFlag(true);
          return;
        }

      writeServerMsg( MSG_POS );
    }
  else if( MSG_OPEN == args[0] && args.count() == 3 )
    {
      // Initialization of gps device is requested. The message
      // consists of two parts separated by spaces.
      // 1) serial device
      // 2) io speed

      bool res = openGps( args[1].toLatin1(), args[2].toUInt() );

      if( res )
        writeServerMsg( MSG_POS );
      else
        writeServerMsg( MSG_NEG );
    }
  else if( MSG_CLOSE == args[0] )
    {
      // Close Gps device is requested
      closeGps();
      writeServerMsg( MSG_POS );
    }
  else if( MSG_NTY == args[0] )
    {
      // A notification shall be sent, if new data are available. This is only
      // valid for one notification. After notification is sent, the server
      // must renew its request.
      notify = true;
      writeServerMsg( MSG_POS );
    }
  else if( MSG_SM == args[0] && args.count() == 2 )
    {
      // Sent message to the GPS device
      int res = writeGpsData( args[1].toLatin1() );

      if( res == -1 )
        {
          writeServerMsg( MSG_NEG );
        }
      else
        {
          writeServerMsg( MSG_POS );
        }
    }
  else if( MSG_SHD == args[0] )
    {
      // Shutdown is requested by the server. This message will not be
      // acked!
      clientData.closeSock();
      clientNotif.closeSock();
      setShutdownFlag(true);
    }
  else
    {
      cerr << "GpsClient::readServerMsg(): "
      << "Unknown message received: " << buf << endl;
      writeServerMsg( MSG_NEG );
    }

  delete [] buf;
  return;
}


// Send a message via data socket to the server. The protocol consists of two
// parts. First the message length is transmitted as unsigned integer, after
// that the actual message as 8 bit character string.

void GpsClient::writeServerMsg( const char *msg )
{
  uint msgLen = strlen( msg );

  int done = clientData.writeMsg( (char *) &msgLen, sizeof(msgLen) );

  done = clientData.writeMsg( (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, close socket
      clientData.closeSock();
    }

  return;
}


// Sent a message via notification socket to the server. The protocol consists
// of two parts. First the message length is transmitted as unsigned integer,
// after that the actual message as 8 bit character string.

void GpsClient::writeNotifMsg( const char *msg )
{
  static QString method = "GpsClient::writeNotifMsg():";

  uint msgLen = strlen( msg );

  int done = clientNotif.writeMsg( (char *) &msgLen, sizeof(msgLen) );

  done = clientNotif.writeMsg( (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, close socket
      clientNotif.closeSock();
    }

#ifdef DEBUG
  cout << method.toLatin1().data() << " " << msg << endl;
#endif

  return;
}


// Translate baud rate to terminal speed definition.

uint GpsClient::getBaudrate(int rate)
{
  switch (rate)
    {
    case 600:
      return B600;
    case 1200:
      return B1200;
    case 2400:
      return B2400;
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    default:
      return B4800;
    }
}


// put a new message into the process queue and sent a notification to
// the server, if option notify is true. The notification is sent
// only once to avoid a flood of them, if server is busy.

void GpsClient::queueMsg( const char* msg )
{
  queue.enqueue( msg );

  if( queue.count() > QUEUE_SIZE )
    {
      // start dequeuing, to avoid memory overflows
      queue.dequeue ();

      cerr << "queueMsg: Max.queue size of " << QUEUE_SIZE
      << " reached, remove oldest element!" << endl;
    }

  if( notify )
    {
      // inform server about new messages available, if not already
      // done.
      writeNotifMsg( MSG_DA );
      notify = false;
    }
}
