/***********************************************************************
**
**   ipc.h
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

#ifndef _Ipc_hh_
#define _Ipc_hh_ 1

#include <QByteArray>

#define IPC_IP "127.0.0.1"

/**
 * This class manages the low layer interfaces for the interprocess
 * cummunication via sockets. The server part can handle up to two client
 * connections. All io is done in blocking mode.
 */
class Ipc
{
    //----------------------------------------------------------------------
    // Public data elements of Class
    //----------------------------------------------------------------------

public:

    static const char * const moduleName;

    //----------------------------------------------------------------------
    // Private data elements of Class
    //----------------------------------------------------------------------

public:

    /**
     * Constructor
     */
    Ipc();

    /**
     * Destructor
     */
    ~Ipc();

    //----------------------------------------------------------------------
    // Server class definition
    //----------------------------------------------------------------------

    class Server
    {
    private:

        QByteArray host;
        QByteArray ipAddress;
        ushort  listenPort;

        int listenSock;
        int clientSocks[2];

    public:

        /**
         * Server constructor
         */
        Server();

        /**
         * Server Destructor
         */
        ~Server();

        /**
         * initialize server
         *
         * @param ipAddress: ip address used in bind call, if empty or null
         *                   INADDR_ANY is used.
         * @param port:      port number to be used in bind call, if 0, kernel
         *                   will assign a free number.
         * @returns:         true on success otherwise false
         */
        bool init( const char *ipAddress=0,
                         const unsigned short port=0 );

        /**
         * @returns the socket descriptor of the next available client connection
         * or -1 in error case.
         */
        int connect2Client(uint index);

        /**
         * Closes the client connection.
         * @returns -1 in error case otherwise 0
         */
        int closeClientSock(uint index);

        /**
         * Closes the listen socket.
         * @returns -1 in error case otherwise 0
         */
        int closeListenSock();

        /**
         * Reads once data from the connected client socket and returns.
         * @returns -1 in error case or number of read bytes
         */
        int readMsg( uint index, void *data, int length );

        /**
         * Writes the passed data to the connected client socket
         * @returns -1 in error case or number of written bytes
         */
        int writeMsg( uint index, void *data, int length );

        int getListenSock() const
        {
            return listenSock;
        };

        int getClientSock(uint index) const
        {
            if( index == 0 )
                return clientSocks[0];
            else
                return clientSocks[1];
        };

        unsigned short getListenPort() const
        {
            return listenPort;
        };

        const char *getIpAddress()
        {
          return ipAddress.data();
        };

    };

    //----------------------------------------------------------------------
    // Client class definition
    //----------------------------------------------------------------------

public:

    class Client
    {
    private:

        QByteArray host;
        QByteArray ipAddress;
        unsigned short port;
        int sock;

    public:

        /**
         * client constructor
         */
        Client();

        /**
         * client destructor
         */
        ~Client();

        /**
         * establishes a connection to the server
         *
         * @param ipAddress  ip address used in connect call, if empty or null
         *                   address of local host (IP_IPC) is used.
         * @param port       port number to be used in connect call
         *
         * @returns         -1 in error case otherwise 0
         */
        int connect2Server( const char *ipAddressIn,
                                  const unsigned short portIn );

        /**
         * Reads one time data from the connected client socket and returns.
         * @returns -1 in error case or number of read bytes
         */
        int readMsg( void *data, int length );

        /**
         * Writes the passed data to the connected client socket
         * @returns -1 in error case or number of written bytes
         */
        int writeMsg( void *data, int length );

        /**
         * Closes the socket of the server connection
         * @returns -1 in error case otherwise 0
         */
        int closeSock();

        int getSock() const
        {
            return sock;
        };

        unsigned short getPort() const
        {
            return port;
        };

        const char *getIpAddress()
        {
            return ipAddress.data();
        };

    };

};

#endif  // #ifndef _Ipc_hh_
