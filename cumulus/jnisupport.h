/***********************************************************************
**
**   jnisupport.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010 by Josua Dietze
**                   2012-2013 by Axel Pauli
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

#include <jni.h>

#include <QHash>
#include <QString>

/**
 * Returns true if the java native methods could be registered successfully
 * otherwise false.
 */
bool initJni(JavaVM* vm, JNIEnv* env);

/**
 * Called to detach the current thread from the VM.
 */
bool jniDetachCurrentThread();

/**
 * Called, when the GUI is going to shutdown to stop JNI transfer.
 */
void jniShutdown( bool option=true);

/**
 * Make Android OS play a sound. stream=0 will set STREAM_NOTIFICATION as
 * audio stream, stream=1 will set STREAM_ALARM.
 * Java code (playSound) is in QtMain.java
 */
bool jniPlaySound(int stream, QString soundName);

/**
 * Forward a GPS command sentence to the Android OS. Android pass it to the
 * connected BT GPS device.
 */
bool jniGpsCmd(QString& cmd);

/**
 * Forward a byte to the Android OS. Android pass it to the
 * connected BT GPS device.
 */
bool jniByte2Gps(const char byte);

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
 * Gets the install state of the AddData.
 *
 * @return true AddData are installed, otherwise false.
 */
bool jniAddDataInstalled();

/**
 * Gets the user's default language from Android.
 *
 *  @return The language used by Android.
 */
QString jniGetLanguage();

/**
 * Gets the display metrics elements used by Android as a hash in key value
 * notation. The key is a string, the value a floating number.
 *
 * @return The display metrics used by Android
 */
QHash<QString, float> jniGetDisplayMetrics();


/**
 * Returns the Android build data as a hash in key value
 * notation.
 *
 * @return The Android build data as hash.
 */
QHash<QString, QString> jniGetBuildData();

/**
 * Tells the android activity to switch on/off the dimming of the screen.
 *
 * @snewState true activates the screen dimm, false deactivates it.
 */
void jniDimmScreen( bool newState );

/**
 * Calls the retriever per SMS e.g. after an outlanding.
 *
 * \param smsText SMS text content.
 */
bool jniCallRetriever( QString& smsText );

/**
 * Opens the Android hardware menu.
 */
bool jniOpenHardwareMenu();
