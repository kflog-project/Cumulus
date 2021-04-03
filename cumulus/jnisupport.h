/***********************************************************************
**
**   jnisupport.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2010 by Josua Dietze
**                   2012-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * Android Java native interface between Java part and C++ part.
 */

#include <jni.h>

#include <QHash>
#include <QString>


enum NetworkError {
    NoError = 0,

    // network layer errors [relating to the destination server] (1-99):
    ConnectionRefusedError = 1,
    RemoteHostClosedError,
    HostNotFoundError,
    TimeoutError,
    OperationCanceledError,
    SslHandshakeFailedError,
    TemporaryNetworkFailureError,
    UnknownNetworkError = 99,

    // proxy errors (101-199):
    ProxyConnectionRefusedError = 101,
    ProxyConnectionClosedError,
    ProxyNotFoundError,
    ProxyTimeoutError,
    ProxyAuthenticationRequiredError,
    UnknownProxyError = 199,

    // content errors (201-299):
    ContentAccessDenied = 201,
    ContentOperationNotPermittedError,
    ContentNotFoundError,
    AuthenticationRequiredError,
    ContentReSendError,
    UnknownContentError = 299,

    // protocol errors
    ProtocolUnknownError = 301,
    ProtocolInvalidOperationError,
    ProtocolFailure = 399
};

/**
 * Returns true if the java native methods could be registered successfully
 * otherwise false.
 */
bool initJni(JavaVM* vm, JNIEnv* env);

/**
 * Called to setup a jni environment for a local thread to be attached to the vm.
 *
 * \param[out] JNIEnv** The jni environment of the attaching thread
 *
 * \return True means success otherwise false is returned.
 */
bool jniEnv( JNIEnv** env);

/**
 * Called to check, if a java exception has occurred.
 *
 * \param[in] JNIEnv* The jni environment
 *
 * \return True means all is ok.
 */
bool isJavaExceptionOccured( JNIEnv* env );

/**
 * Called to detach the current thread from the VM.
 */
bool jniDetachCurrentThread();

/**
 * Called, when the GUI is going to shutdown to stop JNI transfer.
 */
void jniShutdown( bool option=true);

/**
 * Make Android OS play a sound. soundId=0 will play a NOTIFICATION
 * soundId=1 will an ALARM.
 * Java code (playSound) is in CumulusActivity.java
 */
bool jniPlaySound(int soundId);

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

/**
 * \return JNI shutdown flag state
 */
bool jniShutdownFlag();

/**
 * Sent a file download request to the Android activity.
 *
 * \param url The file url to be downloaded.
 *
 * \param destination The storage destination of the file.
 *
 * \return Result of download. 0 means all is ok.
 */
int jniDownloadFile( QString& url, QString& destination );

/**
 * Gets the restart flag from the Android activity.
 *
 * \return True if system was restarted otherwise false.
 */
bool jniIsRestarted();

/**
 * Gets the Android API level from the Android activity.
 *
 * \return Android API level
 */
int jniGetApiLevel();
