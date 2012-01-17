/***********************************************************************
 **
 **   androidevents.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2010 by Josua Dietze (digidietze@draisberghof.de)
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
	GpsFixEvent() : QEvent((QEvent::Type)QEvent::User){}
	~GpsFixEvent(){}
	double lati;
	double longi;
	double alt;
	float spd;
	float bear;
	float accu;
	long long time;
	double latitude() {
		return lati;
	};
	double longitude() {
		return longi;
	};
	double altitude() {
		return alt;
	};
	float speed() {
		return spd;
	};
	float bearing() {
		return bear;
	};
	float accuracy() {
		return accu;
	};
	long long gpstime() {
		return time;
	};
};

/* Posted by the native method "gpsStatus" which is called from Java object
 * LocationListener, method "onStatusChanged"
 */

class GpsStatusEvent : public QEvent
{
public:
	GpsStatusEvent() : QEvent( (QEvent::Type)(QEvent::User+1) ){}
	~GpsStatusEvent(){}
	int stat;
	int nsats;
	int status() {
		return stat;
	}
	int numsats() {
		return nsats;
	}
};

/* Posted by the native method "gpsNmeaString" which is called from Java object
 * LocationListener, method "onNmeaReceived"
 */

class GpsNmeaEvent : public QEvent
{
public:
	GpsNmeaEvent() : QEvent( (QEvent::Type)(QEvent::User+2) ){}
	~GpsNmeaEvent(){}
	QString nmea_sentence;
	QString sentence() {
		return nmea_sentence;
	}
};

/* Posted by the native method "keyboardAction" which is called from Java
 * if the software keyboard is activated
 */

class KeyboardActionEvent : public QEvent
{
public:
	KeyboardActionEvent() : QEvent( (QEvent::Type)(QEvent::User+2) ){}
	~KeyboardActionEvent(){}
	int keyboardAction;
	int action() {
		return keyboardAction;
	}
};

#endif
