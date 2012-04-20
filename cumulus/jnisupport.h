/***********************************************************************
**
**   jnisupport.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010 by Josua Dietze
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * Android interface between Java part and Qt C++ part.
 */

#include <QString>

/**
 * Returns true if the java native methods could be registered successfully
 * otherwise false.
 */
bool initJni();

/**
 * Make Android OS play a sound. stream=0 will set STREAM_NOTIFICATION as
 * audio stream, stream=1 will set STREAM_ALARM.
 * Java code (playSound) is in QtMain.java
 */
bool jniPlaySound(int stream, QString soundName);

/**
 * Get path to the internal data directory from Android.
 */
QString jniGetAppDataDir();

/**
 * Get path to the additional data directory from Android. That is normally
 * the path to the sdcard.
 */
QString jniGetAddDataDir();


/**
 * Tells the android activity to switch on/off the dimming of the screen.
 *
 * @snewState true activates the screen dimm, false deactivates it.
 */
void jniDimmScreen( bool newState );
