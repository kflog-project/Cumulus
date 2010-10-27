/***********************************************************************
**
**   gpsclient.cpp
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

#include <QtCore>

#include "gpsclient.h"
#include "gpscon.h"
#include "protocol.h"
#include "ipc.h"

#ifdef BLUEZ
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#endif

#ifdef DEBUG
#undef DEBUG
#endif

// Switch this on for permanent error logging. That will display
// all open and reconnect failures. Because open and reconnect
// is done periodically you will get a lot of such message in error
// case.
#ifdef ERROR_LOG
#undef ERROR_LOG
#endif

// Size of internal message queue.
#define QUEUE_SIZE 500

// define connection lost timeout in milli seconds
#define TO_CONLOST  10000


GpsClient::GpsClient( const ushort portIn )
{
  device           = "";
  ioSpeedTerminal  = B0;
  ioSpeedDevice    = 0;
  ipcPort          = portIn;
  fd               = -1;
  notify           = false;
  connectionLost   = true;
  shutdown         = false;
  datapointer      = databuffer;
  dbsize           = 0;
  badSentences     = 0;

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
 * Return all currently used read file descriptors as mask, usable by the
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
          // problem occurred, likely buffer overrun. we do restart the GPS
          // receiving.
          closeGps();
          sleep(1);
          // reopen connection
          openGps( device.data(), ioSpeedDevice );
        }
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

  // First check, if enough space is available in the receiver buffer.
  // If we read only trash for a while we can run in a dead lock.
  int freeSpace = sizeof(databuffer) - dbsize;

  if( freeSpace < 10 )
    {
      // Reset buffer pointer because the minimal free buffer space is
      // reached. That will discard all already read data but we never
      // read a end of line. That is our emergency break.
      datapointer = databuffer;
      dbsize = 0;
    }

  // all available GPS data lines are read successive
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
      last.start();
      connectionLost = false;
    }

  return true;
}

// Sends a NMEA sentence to the GPS. Check sum will be calculated by
// this routine. Don't add an asterix at the end of the sentence. It
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
  int result = write (fd, cmd.toLatin1().data(), cmd.length());

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
  qDebug() << "GpsClient::openGps:" << deviceIn << "," << ioSpeedIn;

  device          = deviceIn;
  ioSpeedDevice   = ioSpeedIn;
  ioSpeedTerminal = getBaudrate(ioSpeedIn);
  badSentences    = 0;

  if( deviceIn == (const char *) 0 || strlen(deviceIn) == 0 )
    {
      // no valid device has been passed
      return false;
    }

  // remove all old queued messages
  queue.clear();

  // reset buffer pointer
  datapointer = databuffer;
  dbsize = 0;
  memset( databuffer, 0, sizeof(databuffer) );

  if( fd != -1 )
    {
      // closes an existing connection before opening a new one
      closeGps();
      sleep(2);
    }

#ifdef BLUEZ

  // Define a reg. expression for a bluetooth address like "XX:XX:XX:XX:XX:XX"
  QRegExp regExp("([0-9A-Fa-f]{2,2}:){5,5}[0-9A-Fa-f]{2,2}");

  if( QString(deviceIn).contains(QRegExp( regExp )) )
    {
      fd = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

      struct sockaddr_rc addr;

      memset( &addr, 0, sizeof (addr) );

      addr.rc_family = AF_BLUETOOTH;

      // 1 is the default channel for a connection to the BT daemon
      addr.rc_channel = (uint8_t) 1;
      str2ba( deviceIn, &addr.rc_bdaddr );

      if( connect( fd, (struct sockaddr *) &addr, sizeof (addr)) == -1 &&
          errno != EINPROGRESS )
        {
          cerr << "BT connect error, errno="
          << errno << ", " << strerror(errno) << endl;

          close( fd );
          fd = -1;

          last = QTime();
          setShutdownFlag(true);
          return false;
        }

      fcntl( fd, F_SETFL, O_NONBLOCK ); // NON blocking io is requested

      // Stop supervision control to give the BT daemon time for connection.
      // The first data read will activate it again.
      last = QTime();
      return true;
    }

#endif

  // create a fifo for the nmea simulator, if device starts not with /dev/
  if( strncmp( "/dev/", deviceIn, strlen("/dev/") ) != 0 )
    {
      int ret = mkfifo( device.data(), S_IRUSR | S_IWUSR );

      if( ret && errno != EEXIST )
        {
          perror("mkfifo");
        }
    }

  fd = open( device.data(), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK );

  if( fd == -1 )
    {
      perror( "Open GPS device:" );

      // Could not open the serial device.
#ifdef ERROR_LOG
      cerr << "openGps: Unable to open GPS device "
           << device.data() << " at transfer rate "
           << ioSpeedIn << endl;
#endif

      last.start(); // store time point for restart control
      return false;
    }

  if( ! isatty(fd) )
    {
      // Fifo needs no serial initialization.
      // Write a notice for the user about that fact
      if( device.startsWith("/dev/") )
        {
          qDebug() << "GpsClient::openGps: Device '"
                   << deviceIn
                   << "' is not a TTY!";
        }
    }
  else
    {
      tcgetattr(fd, &oldtio); // get current options from port

      // copy current values into new structure for changes
      memcpy( &newtio, &oldtio, sizeof(newtio) );

      // prepare new port settings for:
      // - canonical input (line oriented input)
      // - 8 data bits
      // - no parity
      // - no cr
      // - blocking mode

      newtio.c_cflag &= ~PARENB;
      newtio.c_cflag &= ~CSTOPB;
      newtio.c_cflag &= ~CSIZE;

      newtio.c_cflag |= CS8 | CLOCAL | CREAD;

      newtio.c_iflag = IGNCR; // no cr

      newtio.c_oflag = ONLCR; // map nl to cr-nl

      // raw input without any echo
      newtio.c_lflag = ~(ICANON | ECHO | ECHOE | ISIG );

      newtio.c_cc[VMIN] = 1;

      newtio.c_cc[VTIME] = 0;

      // AP: Note, the setting of the speed must be done at last
      // because the manipulation of the c_iflag and c_oflag can
      // destroy the already assigned values! Needed me several hours
      // to find out that. Setting the baud rate under c_cflag seems
      // also to work.
      cfsetispeed( &newtio, ioSpeedTerminal ); // set baud rate for input
      cfsetospeed( &newtio, ioSpeedTerminal ); // set baud rate for output

      tcflush(fd, TCIOFLUSH);
      tcsetattr(fd, TCSANOW, &newtio);

      fcntl(fd, F_SETFL, FNDELAY); // NON blocking io is requested
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
      // Search for a newline in the receiver buffer.
      // That is the normal end of a GPS sentence.
      if( ! (end = strchr( start, '\n' )) )
        {
          // No newline in the receiver buffer, wait for more characters
          return;
        }

      if( start == end )
        {
          // skip newline and start at next position with a new search
          start++;
          continue;
        }

      // found a complete record in the buffer, it will be extracted now
      char *record = (char *) malloc( end-start + 2 );

      memset( record, 0, end-start + 2 );

      strncpy( record, start, end-start + 1);

      if( verifyCheckSum( record ) == true )
        {
          // store sentence in the receiver queue, if checksum is ok
          queueMsg( record );
        }

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
 * closes the connection to the GPS device
 */
void GpsClient::closeGps()
{
  if( fd != -1 )
    {
      if( isatty(fd) )
        {
          tcflush(fd, TCIOFLUSH);

          //cfsetispeed( &newtio, B0 ); // set baud rate for input
          //cfsetospeed( &newtio, B0 ); // set baud rate for output

          //tcsetattr( fd, TCSANOW, &newtio ); // stop tty
          //tcsetattr( fd, TCSANOW, &oldtio );
        }

      close(fd);
      fd = -1;
    }

  // remove all queued messages
  queue.clear();
  connectionLost = true;
  badSentences   = 0;
  last = QTime();
}

/**
 * Verify the checksum of the passed sentences.
 *
 * @returns true (success) or false (error occurred)
 */
bool GpsClient::verifyCheckSum( const char *sentence )
{
  // Filter out wrong data messages read in from the GPS port. Known messages
  // do start with a dollar sign or an exclamation mark.
  if( sentence[0] != '$' && sentence[0] != '!' )
    {
      qDebug() << "GpsClient::verifyCheckSum: ignore sentence" << sentence;
      badSentences++;

      if( badSentences >= 3 )
        {
          // Close the receiver after 3 bad sentences back-to-back.
          closeGps();
          last.start(); // activate restart control
        }

      return false;
    }

  badSentences = 0;

  for( int i = strlen(sentence) - 1; i >= 0; i-- )
    {
      if( sentence[i] == '*' )
        {
          if( (strlen(sentence) - 1 - i) < 2 )
            {
              // too less characters
              return false;
            }

          char checkBytes[3];
          checkBytes[0] = sentence[i+1];
          checkBytes[1] = sentence[i+2];
          checkBytes[2] = '\0';

          uchar checkSum = (uchar) QString( checkBytes ).toUShort( 0, 16 );

          if( checkSum == calcCheckSum( sentence ) )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  return false;
}

/** Calculate check sum over NMEA record. */
uchar GpsClient::calcCheckSum( const char *sentence )
{
  uchar sum = 0;

  for( uint i = 1; i < strlen( sentence ); i++ )
    {
      uchar c = (uchar) sentence[i];

      if( c == '$' ) // Start sign will not be considered
        {
          continue;
        }

      if( c == '*' ) // End of sentence reached
        {
          break;
        }

      sum ^= c;
    }

  return sum;
}

// Timeout controller
void GpsClient::toController()
{
  // Null time is used to switch off the timeout control.
  if( last.isNull() == false && last.elapsed() > TO_CONLOST )
    {
      if( connectionLost == false )
        {
          // connection is lost, send only one message to the server
          connectionLost = true;
          queueMsg( MSG_CON_OFF );
        }

#ifdef ERROR_LOG
      cerr << "GpsClient::toController(): "
           << "Connection to GPS seems to be dead, trying restart."
           << endl;
#endif

      // A timeout occurs from GPS side, when the receiver is
      // switched off or the adapter cable is disconnected.
      // In such a case we close the device, that the OS can
      // release the temporary allocated resource. Otherwise
      // we will block the resource and a reconnect is never
      // possible.
      if( fd != -1 )
        {
          // closes an existing connection before opening a new one
          closeGps();
          sleep(3); // wait before a restart is tried
        }

      // try to reconnect to the GPS receiver
      if( openGps( device.data(), ioSpeedDevice ) == false )
        {
          last.start(); // set next retry time point
        }
    }
}

// Reads a server message from the socket. The protocol consists of
// two parts. First the message length is read as unsigned
// integer, after that the actual message as 8 bit character string.

void GpsClient::readServerMsg()
{
  uint msgLen = 0;

  uint done = clientData.readMsg( &msgLen, sizeof(msgLen) );

  if( done < sizeof(msgLen) )
    {
      qWarning() << "GpsClient::readServerMsg(): MSG length" << done << "too short";
      clientData.closeSock();
      exit(-1); // Error occurred
    }

  if( msgLen > 256 )
    {
      // such messages length are not defined. we will ignore that.
      cerr << "GpsClient::readServerMsg(): "
           << "message " << msgLen << " too large, ignoring it!"
           << endl;
      exit(-1); // Error occurred
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = clientData.readMsg( buf, msgLen );

  if( done <= 0 )
    {
      qWarning() << "GpsClient::readServerMsg(): MSG data" << done << "too short";
      clientData.closeSock();
      delete [] buf;
      buf = 0;
      exit(-1); // Error occurred
    }

#ifdef DEBUG
  cout << "GpsClient::readServerMsg(): Received Message: " << buf << endl;
#endif

  // Split the received message into its single parts. Space is used
  // as separator.

  QString qbuf( buf );
  QStringList args = qbuf.split(" ");
  delete [] buf;
  buf = 0;

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

      bool res = openGps( args[1].toLatin1().data(), args[2].toUInt() );

      if( res )
        {
          writeServerMsg( MSG_POS );
        }
      else
        {
          writeServerMsg( MSG_NEG );
        }
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
    case 230400:
      return B230400;
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
