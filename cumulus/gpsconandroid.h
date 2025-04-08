/***********************************************************************
 **
 **   gpsconandroid.h
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
 ***********************************************************************/

/**
 * \class GpsConAndroid
 *
 * \author Axel Pauli
 *
 * \brief GPS connection interface from and to Android's Java part as singleton
 *        class.
 *
 * This class manages the GPS data transfer to and from the Android Java part.
 *
 * \date 2012-2025
 *
 * \version 1.2
 */

#pragma once

#include <QObject>
#include <QTime>

#include "MainWindow.h"
#include "TcpSocket.h"

class QByteArray;
class QMutex;
class QString;
class QEvent;
class KRT2;

class GpsConAndroid : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY( GpsConAndroid )

 public:

  /**
   * This is a singleton class.
   */
  GpsConAndroid( QObject* parent=0 );

  virtual ~GpsConAndroid();

  /**
   * @return The single instance of the class.
   */
  static GpsConAndroid* instance()
  {
    return instance1;
  };

  static void rcvByte( const char byte );

  static bool sndByte( const char byte );

  static bool sndBytes( QByteArray& bytes );

  /**
   * Low level read character port method.
   *
   * \param[out] b Character to be returned,
   *
   * \param[in] timeout Time to be wait for a character in milli seconds.
   *
   * \return 0 means timeout, -1 means error, 1 means ok
   */
  static bool getByte( unsigned char* b, const int timeout );

  static bool verifyCheckSum( const char *sentence );

  /**
   * Forwards a NMEA message as event to the related method running in the
   * GUI thread.
   */
  static void forwardNmea( QString& qnmea );

  /**
   * Handle WiFi request from the Andoid side.
   *
   * @param request 1=WiFi on, 0=WiFi off
   */
  void handleWiFiRequest( int request );

  /**
   * Returns the WiFi request state from Android.
   *
   * @return true=WiFi on, false=Wifi off
   */
  static bool getWiFiRequest()
  {
    return m_wiFiRequest;
  }

  /**
   * @return Returns the TCP GPS instance.
   */
  static TcpSocket* gpsDriver()
  {
    return m_xcgps;
  }

  /**
   * @return Returns the TCP KRT2 instance.
   */
  static KRT2* krt2Driver()
  {
    return m_krt2;
  }

 public slots:

 /**
  * Extract a NMEA sentence from the data stream. Two rxBuffers (0, 1) are
  * exist, to handle 2 streams separately.
  */
 void slot_extractNmea( QByteArray stream, const short rxBufferIndex );

 /**
  * This slot is called, to handle configuration changes.
  */
 void slot_configChanged();

  /**
   * This method is called to activate/deactivate the KRT-2 TCP connection interface.
   */
  void slot_krt2();

  /**
   * This method is called to activate/deactivate the XCVario TCP connection interface.
   */
  void slot_xcvario();

  /**
   * This method is called to activate/deactivate the XCVario GPS TCP connection interface.
   */
  void slot_xcgps();

 protected:

   /** Add an event receiver, used by Android only. */
   bool event(QEvent *event);

 public:

#ifdef FLARM

  /** Gets the flight list from the Flarm device. */
  void getFlarmFlightList();

  /**
   * Downloads the requested IGC flights. The args string contains the destination
   * directory and one or more flight numbers. The single elements are separated
   * by vertical tabs.
   */
  void getFlarmIgcFiles( QString& args );

  /**
   * Starts a thread which gets the Flarm flight list.
   */
  void startGetFlarmFlightList();

  /**
   * Starts a thread which executes the Flarm flight IGC downloads.
   */
  void startGetFlarmIgcFiles( QString& flightData );

  /**
   * Resets the Farm device. Should be called only if Flarm is in binary mode.
   */
  bool flarmReset();

  /**
   * Switches the Flarm device into the binary mode.
   *
   * \return True on success otherwise false.
   */
  bool flarmBinMode();

 private:

  /** Reports an error to the calling application. */
  void flarmFlightListError();

  /** Reports an info to the calling application. */
  void flarmFlightDowloadInfo( QString info );

  /** Reports the download progress to the calling application. */
  void flarmFlightDowloadProgress( const int idx, const int progress );

#endif

 private:

  /** Pointer to single instance. */
  static GpsConAndroid* instance1;

  /** Receive buffer for GPS data from java part. */
  static QByteArray rcvBuffer;

  /** Thread synchronizer for read action. */
  static QMutex mutexRead;

  /** Thread synchronizer for write action. */
  static QMutex mutexWrite;

  /** Thread synchronizer for read action. */
  static QMutex mutexNmeaForward;

  /** Thread synchronizer for actions. */
  static QMutex mutexAction;

  /** Timeout control for Flarm IGC download. */
  QTime downloadTimeControl;

  // XCVario TCP interface objects
  static TcpSocket* m_xcvario;
  static QString m_xcvarioIp;
  static QString m_xcvarioPort;
  static bool m_xcvarioActive;

  // XCVario GPS TCP interface objects
  static TcpSocket* m_xcgps;
  static QString m_xcgpsIp;
  static QString m_xcgpsPort;
  static bool m_xcgpsActive;

  // KRT-2 TCP interface objects
  static KRT2* m_krt2;
  static QString m_krt2Ip;
  static QString m_krt2Port;
  static bool m_krt2Active;

  // store request of last Android WiFi command
  static bool m_wiFiRequest;
};

/******************************************************************************/

/**
* \class FlarmFlightListThread
*
* \author Axel Pauli
*
* \brief Class to execute a read of a Flarm flight list in an extra thread.
*
* \date 2012
*
* \version 1.0
*/

#include <QThread>

class FlarmFlightListThread : public QThread
{
  Q_OBJECT

 public:

  FlarmFlightListThread( QObject *parent );

  virtual ~FlarmFlightListThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();
};

/******************************************************************************/


/**
* \class FlarmIgcFilesThread
*
* \author Axel Pauli
*
* \brief Class to execute a download of Flarm IGC files in an extra thread.
*
* \date 2012
*
* \version 1.0
*/

#include <QThread>

class FlarmIgcFilesThread : public QThread
{
  Q_OBJECT

 public:

  FlarmIgcFilesThread( QObject *parent, QString& flightData );

  virtual ~FlarmIgcFilesThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 private:

  QString m_flightData;
};

