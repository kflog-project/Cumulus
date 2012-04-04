/***********************************************************************
 **
 **   androidevents.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2010 by Josua Dietze (digidietze@draisberghof.de)
 **                   2012 by Axel Pauli
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
 * \author Josua Dietze
 *
 * \brief Custom events for using Android Location service
 *        and getting soft keyboard state via JNI native methods.
 *
 * \date 2012
 *
 * \version $Id$
 *
 */

#ifndef ANDROID_EVENTS_H
#define ANDROID_EVENTS_H

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

/* Posted by the native method "keyboardAction" which is called from Java
 * if the software keyboard is activated
 */
class KeyboardActionEvent : public QEvent
{
  public:

    KeyboardActionEvent( const int keyboardAction ) :
      QEvent( (QEvent::Type)(QEvent::User+2) ),
      m_keyboardAction(keyboardAction)
    {};

    virtual ~KeyboardActionEvent() {};

    int action() const
      {
        return m_keyboardAction;
      };

  private:

    int m_keyboardAction;
};

#endif
