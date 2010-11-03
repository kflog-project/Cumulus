/***********************************************************************
 **
 **   gpscon.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2010 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <linux/limits.h>

#include <QtGui>

#include "generalconfig.h"
#include "mapview.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "signalhandler.h"
#include "protocol.h"
#include "ipc.h"
#include "hwinfo.h"

#ifdef BLUEZ
#include "bluetoothdevices.h"
#endif

#ifdef DEBUG
#undef DEBUG
#endif

// define alive check timeout
#define ALIVE_TO 15000

extern MapView *_globalMapView;

/**
 * This module manages the startup and supervision of the GPS client process
 * and the communication between this client and the Cumulus process.
 * All data transfer between the two processes is be done via a
 * socket interface. The path name, used during startup of Cumulus must be
 * passed in the constructor, that the gpsClient resp. gpsMaemoClient binary
 * can be found. It lays in the same directory as the Cumulus binary.
 */
GpsCon::GpsCon(QObject* parent, const char *pathIn) : QObject(parent)
{
  setObjectName( "GpsCon" );

  GeneralConfig *conf = GeneralConfig::instance();

  QString gpsDevice = conf->getGpsDevice();

  // Do check, what kind of connection the user has selected. Under Maemo
  // we have to consider two possibilities.
  if( gpsDevice != MAEMO_LOCATION_SERVICE )
    {
      exe = QString("%1/%2").arg(pathIn).arg("gpsClient");
    }
  else
    {
      // Location Service has an own client.
      exe = QString("%1/%2").arg(pathIn).arg("gpsMaemoClient");
    }

  pid = -1;

  device  = conf->getGpsDevice();
  ioSpeed = conf->getGpsSpeed();

  timer = new QTimer(this);
  timer->connect( timer, SIGNAL(timeout()), this, SLOT(slot_Timeout()) );

  listenNotifier = static_cast<QSocketNotifier *>(0);
  clientNotifier = static_cast<QSocketNotifier *>(0);

  initSignalHandler();

  // Port can be read from the configuration file for debugging purposes. If it
  // is 0, the OS will take the next free available port.
  ushort port = conf->getGpsIpcPort();

  // Initialize IPC instance for gps client connection. A listening
  // end point will be created.
  if( server.init( IPC_IP, port ) == false )
    {
      qWarning() << "IPC Server init failed!";
      return;
    }

  qDebug( "IPC Server listening on %s:%d", IPC_IP, port );

  // Check the start client option. It is introduced for debugging
  // purposes. If set to false, no client process will be started.
  startClient = conf->getGpsStartClientOption();

  // Add a listen socket notifier to the QT main loop, which will be
  // bound to slot_ListenEvent. If a client makes a connection, the
  // slot slot_ListenEvent is called.
  listenNotifier = new QSocketNotifier( server.getListenSock(),
                                        QSocketNotifier::Read, this );

  listenNotifier->connect( listenNotifier, SIGNAL(activated(int)),
                           this, SLOT(slot_ListenEvent(int)) );
}

GpsCon::~GpsCon()
{
  timer->stop();

  if( server.getClientSock(0) != -1 )
    {
      // Sent shutdown to client
      writeClientMessage( 0, MSG_SHD );
    }

  server.closeListenSock();
  server.closeClientSock(0);
  server.closeClientSock(1);

  // send termination signal to client process
  if( getPid() != -1 )
    {
      kill( getPid(), SIGTERM );
    }

  // Wait 10 seconds for termination of GPS client process to prevent a zombie.
  int time = 10;
  bool result = false;

  while( time-- )
    {
      // Ask the system for state of child process. If process has crashed,
      // the zombie will be removed now.

      int stat_loc = 0;
      errno        = 0;

      pid_t pid = waitpid( getPid(), &stat_loc, WNOHANG );

      if(  pid == getPid() )
        {
          // child process has died
          result = true;
          break;
        }

      if(  pid == -1 && errno == ECHILD )
        {
          // child process does not exist any more
          result = true;
          break;
        }

      sleep(1);
    }

  if( result == false )
    {
      qWarning() << "~GpsCon(): TO, Receiver stop failed!";
    }
}

/**
 * Device arguments are sent to the client, that it can opens the
 * appropriate device for reading. As devices are usable:
 *
 * a) RS232
 * b) USB
 * c) BT via RFCOMM
 * d) MAEMO Location service
 */
bool GpsCon::startGpsReceiving()
{
#ifdef DEBUG
  static QString method = "GPSCon::startGpsReceiving():";
#endif

  if( server.getClientSock(0) != -1 )
    {
      // send the initialization data to the client process, if a client is
      // connected. Otherwise the initialization will be done in
      // slot_ListenEvent(), if the client makes its connect.
      GeneralConfig *conf = GeneralConfig::instance();
      QString gpsDevice   = conf->getGpsDevice();
      ioSpeed             = conf->getGpsSpeed();
      QString msg;

      // Do check, what kind of connection the user has selected.
      if( gpsDevice == MAEMO_LOCATION_SERVICE )
        {
          // Using Location Service under MAEMO needs no arguments because we
          // have no access to the GPS hardware device.
          msg = QString(MSG_OPEN);
        }

#ifdef BLUEZ

      else if( gpsDevice == BT_ADAPTER )
        {
          // BT Adapter shall be used. Get available BT devices and let the
          // user select one of them. This is done in an extra thread.
          if( BluetoothDevices::getNoOfInstances() == 0 )
            {
              _globalMapView->message( tr("Searching GPS BT devices") );
              // allow only one instance to run.
              BluetoothDevices *btThread = new BluetoothDevices( this );

              // Register a special data type for return results. That must be
              // done to transfer the results between different threads.
              qRegisterMetaType<BtDeviceMap>("BtDeviceMap");

              // Connect the receiver of the results. It is located in this
              // thread and not in the new opened thread.
              connect( btThread,
                       SIGNAL(retrievedBtDevices(bool, QString, BtDeviceMap)),
                       this,
                       SLOT(slot_StartGpsBtReceiving(bool, QString, BtDeviceMap)) );

              btThread->start();
            }

          return true;
        }
#endif

      else
        {
          // Using RS232, USB or Pipe device.
          msg = QString("%1 %2 %3").arg(MSG_OPEN).arg(gpsDevice).arg(QString::number(ioSpeed));
        }

      writeClientMessage(0, msg.toLatin1().data());
      readClientMessage(0, msg);

      if (msg == MSG_NEG)
        {
          _globalMapView->message( tr("GPS device not reachable!") );
          return false;
        }
      else
        {
#ifdef DEBUG
          qDebug() << method << "GPS client initialization succeeded";
#endif
        }

      // we want to get a notification, if data are available on client side.
      writeClientMessage(0, MSG_NTY );
      readClientMessage(0, msg);

      // remember last start time
      lastQuery.start();

      return true;
    }

  return false;
}

#ifdef BLUEZ

void GpsCon::slot_StartGpsBtReceiving( bool ok,
                                       QString error,
                                       BtDeviceMap devices )
{
  // First check for unsuccess
  if( ok == false )
    {
      QMessageBox msgBox( QMessageBox::Critical,
                          QObject::tr("GPS BT Devices?"),
                          QObject::tr("No GPS BT devices are in view!"),
                          QMessageBox::Ok,
                          _globalMapView );

      msgBox.setInformativeText( error );
      msgBox.exec();

      // No BT device available or other error.
      triggerRetry();
      return;
    }

  QString lastBtDevice = GeneralConfig::instance()->getGpsBtDevice();

  QStringList items( devices.keys() );

  // Try to preselect a previous used BT GPS device from the returned results.
  int no = 0;

  if( ! lastBtDevice.isEmpty() )
    {
      for( int i = 0; i < items.size(); i++ )
        {
          if( items[i] == lastBtDevice )
            {
              no = i;
              break;
            }
        }
    }

  bool okay;

  // Ask the user to select a BT device.
  QString item = QInputDialog::getItem( _globalMapView,
                                        QObject::tr( "Select GPS BT Device" ),
                                        QObject::tr( "GPS BT Device:" ),
                                        items, no, false, &okay );
  if( ! okay || item.isEmpty() )
    {
      triggerRetry();
      return;
    }

  // Get BT address from device map.
  QString btAddress = devices.value( item );

  // Save last selected BT device.
  GeneralConfig::instance()->setGpsBtDevice( item );

  // We do send the connection data to the GPS client process now.
  // Note! Device and speed are always expected at GPS client side,
  // never mind are necessary or not.
  QString msg = QString("%1 %2 %3").arg(MSG_OPEN).arg(btAddress).arg(115200);
  writeClientMessage(0, msg.toLatin1().data());
  readClientMessage(0, msg);

  if (msg == MSG_NEG)
    {
      _globalMapView->message(tr("GPS device not reachable!"));
      return;
    }
  else
    {
#ifdef DEBUG
      qDebug() << method << "GPS client initialization succeeded";
#endif
    }

  // we want to get a notification, if data are available on client side.
  writeClientMessage(0, MSG_NTY );
  readClientMessage(0, msg);

  // remember last start time
  lastQuery.start();

  return;
}

#endif

void GpsCon::triggerRetry()
{
  // No GPS BT devices are available or other error. We do shutdown
  // the GPS client process. That will initiate a restart
  // by the Cumulus process supervision.
  clientNotifier->setEnabled( false );
  delete clientNotifier;
  clientNotifier = static_cast<QSocketNotifier *>(0);

  writeClientMessage( 0, MSG_SHD );

  server.closeClientSock(0);
  server.closeClientSock(1);

  // Try next daemon restart after 30s
  timer->start( 30000 );
}

/**
 * Stops the GPS receiver on the client side.
 */
bool GpsCon::stopGpsReceiving()
{
  static QString method = "GPSCon::stopGpsReceiving():";

  if( server.getClientSock(0) == -1 )
    {
      // No connection to the client established.
      return false;
    }

  QString msg;

  // send close message to the client
  writeClientMessage( 0, MSG_CLOSE );
  readClientMessage( 0, msg );

  if( msg != MSG_POS )
    {
      qWarning() << method << "Nack, Receiver stop failed!";
      return false;
    }

  return true;
}

/**
 * Starts a new GPS client process via fork/exec or checks, if process is
 * alive. Alive check is triggered by timer routine every 15s. If process is
 * down, a new one will be started.
 */
bool GpsCon::startClientProcess()
{
  static QString method = "GPSCon::startClientProcess():";

  extern bool childDeadState;
  extern bool shutdownState;

  if( shutdownState )
    {
      // don't start a new process in shutdown phase
      return false;
    }

  timer->start( ALIVE_TO ); // setup alive check every 15s

  // check, if client startup is desired. Can be disabled via config
  // option for debugging purposes.
  if( ! startClient )
    {
      return false;
    }

  // At first check, if the child process is running
  if( getPid() != -1 )
    {
      // Ask the system for state of child process. If process has crashed,
      // the zombie will be removed now.

      int stat_loc = 0;
      errno        = 0;

      pid_t pid = waitpid( getPid(), &stat_loc, WNOHANG );

      if( pid == 0 )
        {
          childDeadState = false;

          // process is alive, do nothing

#ifdef DEBUG

          qDebug() << method
                   << "gpsClient process"
                   << getPid()
                   << "is alive!";
#endif

          return true;
        }

      if(  pid == getPid() )
        {
          // child process has died

          qWarning( "%s gpsClient(%d) process has died!",
                    method.toLatin1().data(), getPid() );

          _globalMapView->message( tr("GPS daemon crashed!") );
        }
      else if(  pid == -1 && errno == ECHILD )
        {
          // child process does not exist any more

          qWarning( "%s gpsClient(%d) process does not exist!",
                    method.toLatin1().data(), getPid() );
        }
    }

  setPid(-1);
  childDeadState = false;

  // Closes previous IPC client sockets. Client has died.
  if( server.getClientSock(0) != -1 )
    {
      server.closeClientSock(0);
    }

  if( server.getClientSock(1) != -1 )
    {
      server.closeClientSock(1);
    }

  // not more relevant after a crash, remove it
  if( clientNotifier )
    {
      delete clientNotifier;
      clientNotifier = static_cast<QSocketNotifier *>(0);
    }

  QStringList pathes;

  char *pathVar = getenv( "PATH" );

  QString pathOSVar = GeneralConfig::instance()->getGpsClientPath();

  if( pathVar )
    {
      pathOSVar += QString(":") + QString(pathVar);

      pathes = pathOSVar.split( QChar(':'), QString::SkipEmptyParts );
    }

  bool found = false;

  // look, if gpsClient is to find via paths of PATH variable and the added ones
  QString fileName = QFileInfo(exe).fileName();

  for( int i=0; i < pathes.count(); i++ )
    {
      QString testExe = QString("%1/%2").arg(pathes[i]).arg(fileName);

      if( access(testExe.toLatin1().data(), X_OK) == 0 )
        {
          found = true;
          exe = testExe;
          break;
        }
    }

  // Check, if passed GPS client binary is accessible
  if( found == false && access(exe.toLatin1().data(), X_OK) != 0 )
    {
      qWarning() << method
                 << "GPS client binary"
                 << exe
                 << "is not accessible! Cannot start GPS client.";

      return false;
    }

#ifdef DEBUG
  qDebug() << method
           << "Path to gpsClient is:"
           << exe;
#endif

  //---------------------------------------------------------------
  // Fork a new process
  //---------------------------------------------------------------

  pid_t pid = vfork();

  if( pid == -1 ) // fork error
    {

      qWarning() << method
                 << "vfork() ERROR:"
                 << errno
                 << strerror(errno);

      return false;
    }

  //---------------------------------------------------------------
  // new child process
  //---------------------------------------------------------------

  if( pid == 0 )
    {

#if 0
      // Duplicate file descriptors 0, 1, 2 that the new process has
      // its own set.
      int i = open( "/dev/null", O_RDWR );

      dup2( i, fileno(stdin) );
      dup2( i, fileno(stdout) );
      dup2( i, fileno(stderr) );

      close(i);
#endif

      // Set close on exit bit for all usable file descriptors. Don't
      // do that for the standard devices (stdin, stdout, stderr)
      // otherwise the new process has not such devices. That can
      // cause trouble, if a module uses printf with stdout or stderr
      // respectively.
      struct rlimit rlim;

      memset( &rlim, 0, sizeof(rlimit) );

      // ask for the current maximum value, can be changed
      int maxOpenFds = getrlimit( RLIMIT_NOFILE, &rlim );

      if( maxOpenFds == -1 ) // call failed
        {
          maxOpenFds = NR_OPEN; // normal default from linux/limits.h
        }
      else
        {
          maxOpenFds = rlim.rlim_cur;
        }

      // Start closing beginning at file descriptor 3
      for( int fd=3; fd <= maxOpenFds; fd++ )
        {
          fcntl( fd, F_SETFD, FD_CLOEXEC );
        }

      // Start a new GPS client. The binary is expected at the same
      // directory as Cumulus.
      //
      // arguments are:
      // 1) -port portNumber
      // 2) -slave
      int res = execl( exe.toLatin1().data(),
                       exe.toLatin1().data(),
                       "-port",
                       QString::number(server.getListenPort()).toLatin1().data(),
                       "-slave",
                       (char *) 0 );

      if( res == -1 )
        {
          qWarning() << method
                     << "Startup gpsClient process failed!";

          _globalMapView->message( tr("GPS daemon start failed!") );
          return false;
        }

      QApplication::exit(0);
    }

  //---------------------------------------------------------------
  // parent process goes on here
  //---------------------------------------------------------------

  setPid( pid ); // store child's pid

  qDebug() << method
           << "Startup gpsClient process"
           << getPid()
           << "succeeded!";

  return true;
}

/**
 * This timeout method is used, to call the method startClientProcess(), when
 * the timer is expired. This is the alive check for the forked gpsClient
 * process and ensures the cleaning up of zombies.
 */
void GpsCon::slot_Timeout()
{
  extern bool shutdownState;

  if( shutdownState )
    {
      // Shutdown is requested via signal and client got the signal
      // too. Therefore we can close all sockets.
      server.closeListenSock();
      server.closeClientSock(0);
      server.closeClientSock(1);
      timer->stop();
      QApplication::exit(0);
      return;
    }

  if( ! startClientProcess() )
    {
      // client not alive or start not desired
      return;
    }

  if( GeneralConfig::instance()->getGpsDevice() != NMEASIM_DEVICE &&
      lastQuery.elapsed () > ALIVE_TO )
    {
      // more as 15s no new data notification came in. We send a query to the
      // client to be sure, that the notification has not gone lost.
      queryClient();
    }
}

/**
 * This slot is triggered by the QT main loop and is used to handle the listen
 * socket events. The GPS client tries to connect to the Cumulus
 * process. There are two connections opened by the client, first as data
 * channel, second as notification channel.
 */
void GpsCon::slot_ListenEvent( int socket )
{
  Q_UNUSED( socket )

  static QString method = "GPSCon::slot_ListenEvent():";

  // Client tries to connect. Normally we accept only one connection. That
  // means if the file descriptor is occupied the next one is taken.
  if( server.getClientSock(0) == -1 ) // data channel
    {
      // open cmd/data channel to client
      server.connect2Client(0);
      return;
    }

  if( server.getClientSock(1) == -1 ) // notification channel
    {
      // open notification channel to client
      if( server.connect2Client(1) == -1 )
        {
          return; // accept failed
        }

      // activate a socket notifier for the new client. The client can be
      // programmed to send a notification, if there are new data
      // available. That makes polling superfluous.

      // Delete an old existing notifier, it remains after a crash.
      if( clientNotifier )
        {
          delete clientNotifier;
        }

      clientNotifier = new QSocketNotifier( server.getClientSock(1),
                                            QSocketNotifier::Read, this );

      clientNotifier->connect( clientNotifier, SIGNAL(activated(int)), this,
                               SLOT(slot_NotificationEvent(int)) );

      // After the second client connect we send the initialization to the
      // client. That must be done at this point and not earlier, to avoid a
      // deadlock in the communication

      // now we check the protocol version
      QString msg = QString("%1 %2").arg(MSG_MAGIC).arg(MSG_PROTOCOL);

      writeClientMessage( 0, msg.toAscii().data() );
      readClientMessage( 0, msg );

      if( msg == MSG_NEG )
        {
          qWarning() << method << "Client-Server protocol mismatch!";
          return;
        }

      // Tells the client, what GPS sentences are to be processed.
      sendGpsKeys();

      // Start the GPS receiver after a new connect to get it running.
      startGpsReceiving();
      return;
    }

  qWarning() << method << "All available socket descriptors are occupied!";
}

/**
 * This slot is triggered by the QT main loop and is used to handle the
 * notification events from the client.
 */
void GpsCon::slot_NotificationEvent( int socket )
{
  static QString method = QString("GPSCon::slot_NotificationEvent(%1):").arg(socket);

  // Disable client notifier if socket shall be read. Advised by Qt.
  clientNotifier->setEnabled( false );

  QString msg = "";

  // reads the notification message from the socket.
  readClientMessage( 1, msg );

#ifdef DEBUG
  qDebug("%s %s, got notification", method.toLatin1().data(), msg.toLatin1().data());
#endif

  if( msg != MSG_DA )
    {
      qWarning("%s unexpected notification message code %s",
               method.toLatin1().data(), msg.toLatin1().data() );
    }
  else
    {
      queryClient();
    }

  // Enable client notifier after read.
  clientNotifier->setEnabled( true );
}

/**
 * Query the client, if NMEA records are available. If true, the data will
 * be hand over to the Cumulus process.
 */
void GpsCon::queryClient()
{
  if (server.getClientSock(0) == -1)
    {
      // No connection to the client established.
      return;
    }

  QString msg;
  int loops = 250; // limit loops to avoid a dead lock

  // Now get all messages from the GPS client
  while( loops-- )
    {
      writeClientMessage(0, MSG_GM );
      readClientMessage(0, msg);

      if (server.getClientSock(0) == -1)
        {
          // socket will be closed in case of any problems, e.g. client
          // crash. we check that to avoid a dead lock here
          return;
        }

      if (msg.indexOf(MSG_RM) == 0)
        {
          // reply message received, 3 kinds are possible. remove message key
          // and space separator before further processing
          msg = msg.right(msg.length() - strlen(MSG_RM) - 1);

          if (msg == MSG_CON_OFF) // GPS connection has gone off
            {
              emit gpsConnectionOff();
              qDebug(MSG_CON_OFF);
            }
          else if ( msg == MSG_CON_ON ) // GPS connection has gone on
            {
              emit gpsConnectionOn();
              qDebug(MSG_CON_ON);
            }
          else // GPS NMEA record
            {
              emit newSentence(msg);
            }

          continue;
        }

      // no more messages are available on client
      if (msg.indexOf(MSG_NEG) == 0)
        {
          // no more data available
          break;
        }
    }

  // renew notification subscription
  writeClientMessage(0, MSG_NTY );
  readClientMessage(0, msg);

  // remember last start time
  lastQuery.start();
}

/**
 * Reads a client message from the socket. The protocol consists of two
 * parts. First the message length is read as unsigned integer, after that the
 * actual message as 8 bit character string.
 */
void GpsCon::readClientMessage( uint index, QString &result )
{
  result = "";

  uint msgLen = 0;

  uint done = server.readMsg( index, &msgLen, sizeof(msgLen) );

  if( done <= 0 )
    {
      server.closeClientSock(index);
      qWarning() << "GpsCon::readClientMessage ERROR"
                 << errno
                 << strerror(errno);
      return; // Error occurred
    }

  if( msgLen > 256 )
    {
      // such messages length are not defined. we will ignore that.
      qWarning( "GPSCon::readClientMessage(%d): "
                "message %d too large, ignoring it!", index, msgLen );
      return;
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = server.readMsg( index, buf, msgLen );

  if( done <= 0 )
    {
      server.closeClientSock(index);
      delete [] buf;
      buf = 0;
      return; // Error occurred
    }

  result = buf;
  delete [] buf;
}

/**
 * Writes a client message to the socket. The protocol consists of two
 * parts. First the message length is read as unsigned integer, after that the
 * actual message as 8 bit character string.
 */
void GpsCon::writeClientMessage( uint index, const char *msg  )
{
  uint msgLen = strlen( msg );

  int done = server.writeMsg( index, (char *) &msgLen, sizeof(msgLen) );

  done = server.writeMsg( index, (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, close socket
      server.closeClientSock(index);
    }

  return;
}

/**
 * Sends NMEA input sentence to the GPS receiver. Checksum is calculated by this
 * routine. Don't add an asterix at the end of the passed sentence! That is
 * part of the check sum.
 */
void GpsCon::sendSentence(const QString& sentence)
{
  QString method = "GPSCon::sendSentence():";

  // don't try to send anything if there is no valid file
  if( server.getClientSock(0) == -1 )
    {
      return;
    }

  QString msg = QString("%1 %2").arg(MSG_SM).arg(sentence);

  writeClientMessage( 0, msg.toLatin1().data() );
  readClientMessage( 0, msg );

  if( msg == MSG_NEG )
    {
      qWarning() << method << msg << "failed!";
    }
  else
    {
#ifdef DEBUG
      qDebug() << method << msg << "succeeded!";
#endif
    }
}

void GpsCon::sendGpsKeys()
{
  QString method = "GPSCon::sendGpsKeys():";

  const QHash<QString, short>& gpsHash = GpsNmea::getGpsMessageKeys();

  if( gpsHash.isEmpty() )
    {
      return;
    }

  // Retrieve all GPS message keys from the GPS hash dictionary.
  QStringList items( gpsHash.keys() );

  QString msg = QString("%1 %2").arg(MSG_GPS_KEYS).arg(items.join(","));

  writeClientMessage( 0, msg.toLatin1().data() );
  readClientMessage( 0, msg );

  if( msg == MSG_NEG )
    {
      qWarning() << method << msg << "failed!";
    }
  else
    {
#ifdef DEBUG
      qDebug() << method << msg << "succeeded!";
#endif
    }
}
