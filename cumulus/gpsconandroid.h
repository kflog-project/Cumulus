/***********************************************************************
 **
 **   gpsconandroid.h
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
 ***********************************************************************/

/**
 * \class GpsConAndroid
 *
 * \author Axel Pauli
 *
 * \brief GPS connection interface from and to Android Java part.
 *
 * This module manages the GPS data transfer to and from the Android Java
 * part.
 *
 * \date 2012
 */

#ifndef GPS_CON_ANDROID_H
#define GPS_CON_ANDROID_H

#include <QObject>

class QByteArray;
class QMutex;
class QString;

class GpsConAndroid : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY( GpsConAndroid )

 public:

  GpsConAndroid( QObject parent=0 );

  virtual ~GpsConAndroid();

  static void rcvByte( const char byte );

  static void forwardNmea( QString& qnmea );

  static bool verifyCheckSum( const char *sentence );

 private:

  /** Receive buffer for GPS data from java part. */
  static QByteArray rcvBuffer;

  /** Thread synchronizer. */
  static QMutex mutex;
};

#endif
