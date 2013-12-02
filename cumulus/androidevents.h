/***********************************************************************
 **
 **   androidevents.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2010 by Josua Dietze (digidietze@draisberghof.de)
 **                   2012-2013 by Axel Pauli
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
 *
 * \author Josua Dietze, Axel Pauli
 *
 * \brief Custom events for using Android Location service
 *        and getting soft keyboard state via JNI native methods.
 *
 * \date 2012-2013
 *
 * \version $Id$
 *
 */

#ifndef ANDROID_EVENTS_H
#define ANDROID_EVENTS_H

#include <QtCore>
#include <QEvent>

/* Posted by the native method "gpsFix" which is called from Java object
 * "LocationListener", method "onLocationChanged"
 */

class GpsFixEvent : public QEvent
{
  public:

    GpsFixEvent( double latitude,
                 double longitude,
                 double altitude,
                 float speed,
                 float heading,
                 float accuracy,
                 long long time ) :
      QEvent((QEvent::Type) QEvent::User),
      m_latitude(latitude),
      m_longitude(longitude),
      m_altitude(altitude),
      m_speed(speed),
      m_heading(heading),
      m_accuracy(accuracy),
      m_time(time)
        {
        };

    virtual ~GpsFixEvent(){};

    double latitude() {
      return m_latitude;
    };
    double longitude() {
      return m_longitude;
    };
    double altitude() {
      return m_altitude;
    };
    float speed() {
      return m_speed;
    };
    float heading() {
      return m_heading;
    };
    float accuracy() {
      return m_accuracy;
    };
    long long time() {
      return m_time;
    };

  private:

    double m_latitude;
    double m_longitude;
    double m_altitude;
    float m_speed;
    float m_heading;
    float m_accuracy;
    long long m_time;
};

/* Posted by the native method "gpsStatus" which is called from Java object
 * LocationListener, method "onStatusChanged"
 */
class GpsStatusEvent : public QEvent
{
  public:

    GpsStatusEvent( const int status ) :
      QEvent( (QEvent::Type) (QEvent::User + 1) ),
      m_status(status)
    {};

    virtual ~GpsStatusEvent() {};

    int status() const
    {
      return m_status;
    };

  private:

    int m_status;
};

/* Posted by the native method "gpsNmeaString" which is called from Java object
 * LocationListener, method "onNmeaReceived"
 */
class GpsNmeaEvent : public QEvent
{
  public:

    GpsNmeaEvent( const QString& nmeaSentence ) :
      QEvent( (QEvent::Type)(QEvent::User+2) ),
      m_nmeaSentence(nmeaSentence)
    {};

    virtual ~GpsNmeaEvent() {};

    QString& sentence()
      {
         return m_nmeaSentence;
      };

  private:

    QString m_nmeaSentence;
};

class FlarmFlightListEvent : public QEvent
{
  public:

  FlarmFlightListEvent( const QString& list ) :
    QEvent( (QEvent::Type) (QEvent::User+3) ),
    m_list(list)
  {};

  virtual ~FlarmFlightListEvent() {};

  QString& flightList()
    {
       return m_list;
    };

 private:

  QString m_list;
};

class FlarmFlightDownloadInfoEvent : public QEvent
{
  public:

  FlarmFlightDownloadInfoEvent( const QString& info ) :
    QEvent( (QEvent::Type) (QEvent::User+4) ),
    m_info(info)
  {};

  virtual ~FlarmFlightDownloadInfoEvent() {};

  QString& flightDownloadInfo()
    {
       return m_info;
    };

 private:

  QString m_info;
};

class FlarmFlightDownloadProgressEvent : public QEvent
{
  public:

  FlarmFlightDownloadProgressEvent( const int idx, const int progress ) :
    QEvent( (QEvent::Type) (QEvent::User+5) ),
    m_idx(idx),
    m_progress(progress)
  {};

  virtual ~FlarmFlightDownloadProgressEvent() {};

  void flightDownloadInfo( int& idx, int& progress )
    {
       idx = m_idx;
       progress = m_progress;
       return;
    };

 private:

  int m_idx;
  int m_progress;
};

/* Posted by the native method "nativeBaroAltitude" which is called from Java
 * object BaroSensorListener, method "onSensorChanged".
 */
class AltitudeEvent : public QEvent
{
  public:

	AltitudeEvent( const double altitude ) :
      QEvent( (QEvent::Type) (QEvent::User + 6) ),
      m_altitude(altitude)
    {};

    virtual ~AltitudeEvent() {};

    double altitude() const
    {
      return m_altitude;
    };

  private:

    double m_altitude;
};

#endif
