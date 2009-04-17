/***********************************************************************
 **
 **   ipc.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2009 by Axel Pauli (axel@kflog.org)
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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include <QString>

#include "ipc.h"

/**
 * This class manages the low layer interfaces for the interprocess
 * communication via sockets. The server part can handle up to two client
 * connections. All io is done in blocking mode.
 */

// set static module name
const char* const Ipc::moduleName = "Ipc";


/**
 * Constructor
 */
Ipc::Ipc()
{
  return;
}


/**
 * Destructor
 */
Ipc::~Ipc()
{
  return;
}


/**
 * Server constructor
 */
Ipc::Server::Server() :
    listenPort(0),
    listenSock(-1)
{
  clientSocks[0] = -1;
  clientSocks[1] = -1;
  return;
}


/**
 * Server Destructor
 */
Ipc::Server::~Server()
{
  if ( listenSock != -1 )
    {
      closeListenSock();
    }

  if ( clientSocks[0] != -1 )
    {
      closeClientSock(0);
    }

  if ( clientSocks[1] != -1 )
    {
      closeClientSock(1);
    }

  return;
}


/**
 * initialize server
 *
 * @param ipAddress: ip address used in bind call, if empty or null
 *                   INADDR_ANY is used.
 * @param port:      port number to be used in bind call, if 0, kernel
 *                   will assign a free number.
 * @returns:         true on success otherwise false
 */
bool Ipc::Server::init( const char *ipAddress,
                              const unsigned short port )
{
  static const char* method = ( "Ipc::Server::init(): " );

  if ( listenSock != -1 )
    {
      cerr << method
           << "A previous opened listen socket will be closed!"
           << " Is this wanted by You?" << endl;

      closeListenSock();
    }

  if ( clientSocks[0] != -1 )
    {
      cerr << method
           << "A previous opened client socket_0 will be closed!"
           << " Is this wanted by You?" << endl;

      closeClientSock(0);
    }

  if ( clientSocks[1] != -1 )
    {
      cerr << method
           << "A previous opened client socket_1 will be closed!"
           << " Is this wanted by You?" << endl;

      closeClientSock(1);
    }

  listenSock = socket( AF_INET, SOCK_STREAM, 0 );

  if ( listenSock == -1 )
    {
      cerr << method
           << "Cannot create socket: errno="
           << errno
           << ", " << strerror(errno) << endl;

      return false;
    }

  int opt = 1;

  setsockopt(listenSock, SOL_SOCKET, SO_KEEPALIVE, (int *)&opt, sizeof(opt));

  setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (int *)&opt, sizeof(opt));

  struct sockaddr_in myAddr;

  memset( &myAddr, 0, sizeof( sockaddr_in ) );

  myAddr.sin_family = AF_INET;

  if ( ipAddress == 0 || strlen( ipAddress ) == 0 )
    {
      myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
  else
    {
      if ( inet_aton( ipAddress, &myAddr.sin_addr ) == 0 )
        {
          cerr << method
               << "inet_aton: invalid IP address format ("
               << ipAddress << ") errno="
               << errno
               << ", " << strerror(errno) << endl;

          closeListenSock();
          return false;
        }
    }

  if ( port > 0 )
    {
      myAddr.sin_port = htons(port);
    }
  else
    {
      // OS assigns a free port number, if 0 is used.
      myAddr.sin_port = 0;
    }

  if ( bind( listenSock, (struct sockaddr *) &myAddr, sizeof(myAddr) ) == -1 )
    {
      cerr << method
           << "Bind() failed"
           <<": errno="
           << errno
           << ", " << strerror(errno) << endl;

      closeListenSock();

      return false;
    }

  // Fetch used or assigned port from listen socket

  memset( &myAddr, 0, sizeof( sockaddr_in ) );

  socklen_t len = sizeof(myAddr);

  if ( getsockname( listenSock, (struct sockaddr *) &myAddr, &len ) == -1 )
    {
      cerr << method
           << "getsockname() call failed"
           << ": errno="
           << errno
           << ", " << strerror(errno) << endl;

      closeListenSock();

      return false;
    }

  listenPort = ntohs( myAddr.sin_port );

  this->ipAddress = inet_ntoa( myAddr.sin_addr );

  cout << method
       << "Server uses IP-Address "
       << inet_ntoa( myAddr.sin_addr )
       << " via port number "
       << listenPort
       << " as listening end point" << endl;

  if ( listen( listenSock, 5 ) == -1 )
    {
      cerr << method
           << "listen() call failed"
           << ": errno="
           << errno
           << ", " << strerror(errno) << endl;

      closeListenSock();

      return false;
    }

  return true;
}


/**
 * @returns the socket descriptor of the next available client connection
 * or -1 in error case.
 */
int Ipc::Server::connect2Client(uint index)
{
  QString method = QString( "Ipc::Server::connect2Client(%1): ").arg(index);

  // close an previous open connection. Only one session is possible.

  if ( getClientSock(index) != -1 )
    {
      cerr << method.toLatin1().data()
           << "A previous opened client socket("
           << index << ") will be closed!"
           << " Is this wanted by You?" << endl;

      closeClientSock(index);
    }

  struct sockaddr_in peer;
  socklen_t len = sizeof(peer);

  clientSocks[index] = accept( listenSock,
                               (struct sockaddr *) &peer,
                               &len );

  if ( clientSocks[index] == -1 )
    {
      cerr << method.toLatin1().data()
           << "accept() call failed"
           << ": errno="
           << errno
           << ", " << strerror(errno) << endl;
    }
  else
    {
      cout << method.toLatin1().data()
           << "Connection to Client "
           << inet_ntoa( peer.sin_addr )
           << ":" << ntohs( peer.sin_port )
           << " established." << endl;
    }

  return clientSocks[index];
}


/**
 * Reads once data from the connected client socket and returns.
 * @returns -1 in error case or number of read bytes
 */
int Ipc::Server::readMsg(  uint index, void *data, int length )
{
  QString method = QString( "Ipc::Server::readMsg(%1): ").arg(index );

  if ( getClientSock(index) == -1 )
    {
      cerr << method.toLatin1().data()
           << "No client connection is established!" << endl;

      errno = ENOTCONN;
      return -1;
    }

  int done = 0;

  memset( data, 0, length ); // clear data buffer

  while (1)
    {
      done = read( clientSocks[index], data, length );

      if ( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          cerr << method.toLatin1().data()
               << "read() returns with ERROR: errno="
               << errno
               << ", " << strerror(errno) << endl;
          return -1;
        }

      break;
    }

  return done;
}


/**
 * Writes the passed data to the connected client socket
 * @returns -1 in error case or number of written bytes
 */
int Ipc::Server::writeMsg( uint index, void *data, int length )
{
  QString method = QString( "Ipc::Server::writeMsg(%1): ").arg(index);

  if ( getClientSock(index) == -1 )
    {
      cerr << method.toLatin1().data()
           << "No client connection is established!" << endl;

      errno = ENOTCONN;
      return -1;
    }

  char *ptr = (char *) data;

  int writtenBytes = 0;

  while (1)
    {
      int done = write( clientSocks[index], ptr, length );

      if ( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          cerr << method.toLatin1().data()
               << "read() returns with ERROR: errno="
               << errno
               << ", " << strerror(errno) << endl;
          return -1;
        }

      writtenBytes += done;

      if ( done < length ) // Not all has been written, write again
        {
          ptr += done;
          length -= done;
          continue;
        }

      break;
    }

  return writtenBytes;
}


/**
 * Closes the client connection.
 * @returns -1 in error case otherwise 0
 */
int Ipc::Server::closeClientSock(uint index)
{
  QString method = QString( "Ipc::Server::closeClientSock(%1): ").arg(index);

  if ( clientSocks[index] == -1 )
    {
      return 0;
    }

  int res = close( clientSocks[index] );

  clientSocks[index] = -1;

  if ( res == -1 )
    {
      cerr << method.toLatin1().data()
           << "close returns with ERROR: errno="
           << errno
           << ", " << strerror(errno) << endl;
    }

  return res;
}


/**
 * Closes the listen socket.
 * @returns -1 in error case otherwise 0
 */
int Ipc::Server::closeListenSock()
{
  static const char* method = ( "Ipc::Server::closeListenSock(): " );

  if ( listenSock == -1 )
    {
      return 0;
    }

  int res = close( listenSock );

  listenSock = -1;
  listenPort = 0;

  if ( res == -1 )
    {
      cerr << method
           << "close() returns with ERROR: errno="
           << errno
           << ", " << strerror(errno) << endl;
    }

  return res;
}

/**
 * client constructor
 */
Ipc::Client::Client() :
    port(0),
    sock(-1)
{
  // static const char* method = ( "Ipc::Client::Client(): " );

  return;
}

/**
 * client destructor
 */
Ipc::Client::~Client()
{
  // static const char* method = ( "Ipc::Client::~Client(): " );

  if ( sock != -1 )
    {
      closeSock();
    }

  return;
}


/**
 * establishes a connection to the server
 *
 * @param ipAddress  ip address used in connect call, if empty or null
 *                   address of local host (IP_IPC) is used.
 * @param port       port number to be used in connect call
 *
 * @returns         -1 in error case otherwise 0
 */
int Ipc::Client::connect2Server( const char *ipAddressIn,
                                       const unsigned short portIn )
{
  static const char* method = ( "Ipc::Client::connect2Server(): " );

  if ( sock != -1 )
    {
      cerr << method
           << "A previous opened socket will be closed!"
           << " Is this wanted by You?" << endl;

      closeSock(); // closes the previous used socket
    }

  sock = socket( AF_INET, SOCK_STREAM, 0 );

  if ( sock == -1 )
    {
      cerr << method
           << "Cannot create socket: errno="
           << errno
           << ", " << strerror(errno) << endl;

      return -1;
    }

  struct sockaddr_in peer;

  memset( &peer, 0, sizeof( peer ) );

  peer.sin_family = AF_INET;

  if ( ipAddressIn == 0 || strlen(ipAddressIn) == 0 )
    {
      // use local ip address for connect
      ipAddress = IPC_IP;
    }
  else
    {
      ipAddress = ipAddressIn;
    }

  int res = inet_aton( ipAddress, &peer.sin_addr );

  if ( res == 0 )
    {
      cerr << method
           << "inet_aton: invalid IP address format ("
           << ipAddress.data() <<  ") errno="
           << errno
           << ", " << strerror(errno) << endl;

      closeSock();
      return -1;
    }

  port = portIn;
  peer.sin_port = htons( port );

  while (1)
    {
      cerr << "Connecting socket: "
           << sock << " ipAddress: "
           << ipAddress.data() << " port: "
           << port << endl;

      res = connect( sock, (struct sockaddr *) &peer, sizeof(peer) );

      if ( res == 0 )
        {
          break;
        }

      if ( errno == EINTR )
        {
          continue; // ignore interrupts
        }

      cerr << method
           << "connect() returns with " << res << "ERROR: errno="
           << errno
           << ", " << strerror(errno) << endl;

      return -1;
    }

  uint rBufSize = 0;
  uint sBufSize = 0;

  uint rBufLen = sizeof(rBufSize);
  uint sBufLen = sizeof(sBufSize);

  // read out read and write buffer sizes
  getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (int *)&rBufSize, &rBufLen);
  getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (int *)&sBufSize, &sBufLen);

  // report buffer sizes
  cerr << method
       << "ReadBufferSize="  << rBufSize/1024 << "kB, "
       << "WriteBufferSize=" << sBufSize/1024 << "kB"
       << endl;

  return 0;
}

/**
 * Reads one time data from the connected client socket and returns.
 * @returns -1 in error case or number of read bytes
 */
int Ipc::Client::readMsg( void *data, int length )
{
  static const char* method = ( "Ipc::Client::readMsg(): " );

  if ( getSock() == -1 )
    {
      cerr << method
           << "No server connection is established!" << endl;

      errno = ENOTCONN;
      return -1;
    }

  int done = 0;

  memset( data, 0, length ); // clear data buffer

  while (1)
    {
      done = read( sock, data, length );

      if ( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          cerr << method
               << "read() returns with ERROR: errno="
               << errno
               << ", " << strerror(errno) << endl;
          return -1;
        }

      break;
    }

  return done;
}

/**
 * Writes the passed data to the connected client socket
 * @returns -1 in error case or number of written bytes
 */
int Ipc::Client::writeMsg( void *data, int length )
{
  static const char* method = ( "Ipc::Client::writeMsg(): " );

  if ( getSock() == -1 )
    {
      cerr << method
           << "No server connection is established, "
           << "Message will be discarded!" << endl;

      errno = ENOTCONN;
      return -1;
    }

  char *ptr = (char *) data;

  int writtenBytes = 0;

  while (1)
    {
      int done = write( sock, ptr, length );

      if ( done < 0 )
        {
          if ( errno == EINTR )
            {
              continue; // Ignore interrupts
            }

          cerr << method
               << "read() returns with ERROR: errno="
               << errno
               << ", " << strerror(errno) << endl;
          return -1;
        }

      writtenBytes += done;

      if ( done < length ) // Not all has been written, write again
        {
          ptr += done;
          length -= done;
          continue;
        }

      break;
    }

  return writtenBytes;
}


/**
 * Closes the socket of the server connection
 * @returns -1 in error case otherwise 0
 */
int Ipc::Client::closeSock()
{
  static const char* method = ( "Ipc::Client::closeSock(): " );

  int res = close( sock );

  sock = -1;

  if ( res == -1 )
    {
      cerr << method
           << "close() returns with ERROR: errno="
           << errno
           << ", " << strerror(errno) << endl;
    }

  return res;
}
