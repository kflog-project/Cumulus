/***********************************************************************
**
**   ipc.h
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

#ifndef _Ipc_hh_
#define _Ipc_hh_ 1

#include <QByteArray>

#define IPC_IP "127.0.0.1"

/**
 * \class Ipc
 *
 * \author Axel pauli
 *
 * \brief Interprocess communication class via sockets.
 *
 * This class manages the low layer interfaces for the interprocess
 * communication via sockets. The server part can handle up to two client
 * connections. All io is done in blocking mode.
 *
 * \date 2004-2010
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

    /**
     * \class Server
     *
     * \author Axel Pauli
     *
     * \brief Server part of interprocess communication class.
     */
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
         * Initialize server connection.
         *
         * @param ipAddress IP address used in bind call. If it is empty or null
         *                  INADDR_ANY is used.
         * @param port      Port number to be used in bind call. If 0, kernel
         *                  will assign a free number.
         * @return          true on success otherwise false
         */
        bool init( const char *ipAddress=0,
                   const unsigned short port=0 );

        /**
         * @return the socket descriptor of the next available client connection
         * or -1 in error case.
         */
        int connect2Client(uint index);

        /**
         * Closes the client connection.
         * @return -1 in error case otherwise 0
         */
        int closeClientSock(uint index);

        /**
         * Closes the listen socket.
         * @return -1 in error case otherwise 0
         */
        int closeListenSock();

        /**
         * Reads once data from the connected client socket and returns.
         * @return -1 in error case or number of read bytes
         */
        int readMsg( uint index, void *data, int length );

        /**
         * Writes the passed data to the connected client socket
         * @return -1 in error case or number of written bytes
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

    /**
     * \class Client
     *
     * \author Axel Pauli
     *
     * \brief Client part of interprocess communication class.
     */
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
         * Establishes a connection to the server.
         *
         * @param ipAddress IP address for call connection. If is empty or null
         *                  address of local host (IP_IPC) is used.
         *
         * @param port      port number to be used in connect call
         *
         * @return          -1 in error case otherwise 0
         */
        int connect2Server( const char *ipAddress,
                            const unsigned short port );

        /**
         * Reads one time data from the connected client socket and returns.
         * @return -1 in error case or number of read bytes
         */
        int readMsg( void *data, int length );

        /**
         * Writes the passed data to the connected client socket
         * @return -1 in error case or number of written bytes
         */
        int writeMsg( void *data, int length );

        /**
         * Closes the socket of the server connection
         * @return -1 in error case otherwise 0
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

        /**
         * Returns the number of the readable bytes in the read queue.
         * @return 0 if nothing is to read or in error case.
         */
        int numberOfReadableBytes();
    };

};

#endif
