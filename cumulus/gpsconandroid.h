/***********************************************************************
 **
 **   gpsconandroid.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2012 by Axel Pauli (kflog.cumulus@gmail.com)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
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
 * \date 2012
 *
 * \version $Id$
 */

#ifndef GPS_CON_ANDROID_H
#define GPS_CON_ANDROID_H

#include <QObject>
#include <QTime>

class QByteArray;
class QMutex;
class QString;

class GpsConAndroid : public QObject
{
  Q_OBJECT

 private:

  /**
   * Constructor is private because this is a singleton class.
   */
  GpsConAndroid( QObject* parent=0 );

  Q_DISABLE_COPY( GpsConAndroid )

 public:

  virtual ~GpsConAndroid();

  /**
   * @return The single instance of the class.
   */
  static GpsConAndroid* instance()
  {
    static GpsConAndroid instance;

    return &instance;
  };

  static void rcvByte( const char byte );

  static bool sndByte( const char byte );

  static bool sndBytes( QByteArray& bytes );

  static bool getByte( unsigned char* b );

  static bool verifyCheckSum( const char *sentence );

  /**
   * Forwards a NMEA message as event to the related method running in the
   * GUI thread.
   */
  static void forwardNmea( QString& qnmea );

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

  /** Receive buffer for GPS data from java part. */
  static QByteArray rcvBuffer;

  /** Thread synchronizer for read action. */
  static QMutex mutexRead;

  /** Thread synchronizer for write action. */
  static QMutex mutexWrite;

  /** Thread synchronizer for actions. */
  static QMutex mutexAction;

  /** Timeout control for Flarm IGC download. */
  QTime downloadTimeControl;
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
* \version $Id$
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
* \version $Id$
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

#endif
