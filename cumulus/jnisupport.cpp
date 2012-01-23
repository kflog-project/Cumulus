/***********************************************************************
 **
 **   jnisupport.cpp
 **
 **   This file is part of Cumulus
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

#include <QtGui>
#include <QtAndroidBridge/qtandroidbridge.h>

#include "jnisupport.h"
#include "androidevents.h"
#include "gpsnmea.h"
#include "mainwindow.h"


JavaVM *jvm = NULL;
JNIEnv *jniEnvironment = NULL;
jobject jniProxyObject;
jclass jniProxyClass;
jmethodID javaMethodID;

extern MainWindow *_globalMainWindow;

// ---- The native methods ---

/** Called from the Java code on every location update by LocationListener ll
 */

static void nativeGpsFix(JNIEnv * /*jniEnvironment*/, jobject /*myproxyobject*/, jdouble lati, jdouble longi,
				jdouble alt, jfloat speed, jfloat bear, jfloat accu, jlong time)
{
//	qDebug("*** nativeGpsFix: lat %f, lon %f", lati, longi);
	GpsFixEvent *ge = new GpsFixEvent();
	ge->lati = lati;
	ge->longi = longi;
	ge->alt = alt;
	ge->spd = speed;
	ge->bear = bear;
	ge->accu = accu;
	ge->time = time;
	QCoreApplication::postEvent(GpsNmea::gps, ge);
}

/** Called from the Java code on every status change by LocationListener ll
 */

static void nativeGpsStatus(JNIEnv * /*jniEnvironment*/, jobject /*myproxyobject*/, jint status, jint numsats)
{
//	qDebug("*** nativeGpsStatus: st %d , ns %d", status, numsats);
	GpsStatusEvent *ge = new GpsStatusEvent();
	ge->stat = status;
	ge->nsats = numsats;
	QCoreApplication::postEvent(GpsNmea::gps, ge);
}

static void nativeNmeaString(JNIEnv *env /*jniEnvironment*/, jobject /*myproxyobject*/, jstring jnmea)
{
	const char * nativeString = env->GetStringUTFChars(jnmea, 0);
	QString qnmea(nativeString);
	env->ReleaseStringUTFChars(jnmea, nativeString);
	GpsNmeaEvent *ne = new GpsNmeaEvent();
	ne->nmea_sentence = qnmea;
	QCoreApplication::postEvent(GpsNmea::gps, ne);
}

static void nativeKeypress(JNIEnv *env /*jniEnvironment*/, jobject /*myproxyobject*/, jchar code)
{
	unsigned int qtCode;
	qDebug("nativeKeypress: code is %d", (unsigned int)code);
	switch ((unsigned int)code) {
		case 25:
			qtCode = Qt::Key_F11; break;
		case 26:
			qtCode = Qt::Key_F12; break;
		case 27:
			qtCode = Qt::Key_F13; break;
		default:
			qDebug("nativeKeypress: code is %d",code);
			return;
	}
	QKeyEvent *ke = new QKeyEvent( QEvent::KeyPress, qtCode, 0 );
	QCoreApplication::postEvent(_globalMainWindow, ke);
}

#if 0
static void nativePreflight()
{
	QKeyEvent *ke = new QKeyEvent( QEvent::KeyPress, Qt::Key_F12, 0 );
	QCoreApplication::postEvent(_globalMainWindow, ke);
}
#endif

static bool isRootWindow()
{
	return MainWindow::isRootWindow;
}

static void keyboardAction(JNIEnv * /*jniEnvironment*/, jobject /*myproxyobject*/, jint action)
{
//qDebug("Native keyboardAction: started");
	KeyboardActionEvent* ke = new KeyboardActionEvent();
	ke->keyboardAction = action;
	QCoreApplication::postEvent(_globalMainWindow, ke);
}

/* The array of native methods to register.
 * The name string must match the "native" declaration in Java.
 * The parameter string must match the types in the "native" declaration
 * (I = integer, J = long, F = float, D = double, V = void etc. )
 */

static JNINativeMethod methods[] = {
	{"nativeGpsFix", "(DDDFFFJ)V", (void *)nativeGpsFix},
	{"nativeGpsStatus", "(II)V", (void *)nativeGpsStatus},
	{"nativeNmeaString","(Ljava/lang/String;)V", (void *)nativeNmeaString},
	{"nativeKeypress", "(C)V", (void *)nativeKeypress},
//	{"nativeSetup", "()V", (void *)nativeSetup},
//	{"nativePreflight", "()V", (void *)nativePreflight},
	{"isRootWindow", "()Z", (bool *)isRootWindow},
	{"keyboardAction", "(I)V", (void *)keyboardAction}
};


/* The following two functions are borrowed from Denis Kormalev,
 * out of his post on the qt-android mailing list:
 * http://groups.google.com/group/android-qt/browse_thread/thread/ ..
 * .. d60d28796f61177d/4bf2e434c9cddf77#4bf2e434c9cddf77
 */

//Exception checking
inline bool isJavaExceptionOccured()
{
	if (!jniEnvironment)
		return false;
	if (jniEnvironment->ExceptionOccurred()) {
		jniEnvironment->ExceptionDescribe();
		jniEnvironment->ExceptionClear();
		return true;
	}
	return false;
}

//JNI initialization
bool jniEnv()
{
qDebug("C++ jniEnv: started");
	jniEnvironment = NULL;
	if (jvm == NULL)
		jvm = QtAndroidBridge::javaVM();
	if (jvm != NULL) {
		JavaVMAttachArgs args;
		args.name = NULL;
		args.group = NULL;
		args.version = JNI_VERSION_1_4;
		jvm->AttachCurrentThread(&jniEnvironment, &args);
		if (isJavaExceptionOccured()) {
			qDebug("jniEnv: exception when attaching Java thread");
			return false;
		}
		if (!jniEnvironment) {
			qDebug("jniEnv: could not get Java environment");
			return false;
		}
	} else {
		qDebug("jniEnv: could not get Java VM");
		return false;
	}
	jniProxyObject = QtAndroidBridge::jniProxyObject();
	jniProxyClass = jniEnvironment->GetObjectClass(jniProxyObject);
	if (!jniProxyClass) {
		qDebug("jniEnv: retrieving the class failed");
		return false;
	}
	return true; 
}

// The public functions

bool jniRegister()
{
qDebug("C++ jniRegister: started");
	if (! jniEnv()) {
		qDebug("jniRegister: jniEnv failed, can't register native methods");
		return false;
	}
	if ( jniEnvironment->RegisterNatives (jniProxyClass, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
		qDebug("jniRegister: registering of custom natives failed");
		return false;
	}

	qDebug("jniRegister: native methods registered successfully");
	return true;
}

bool jniPlaySound(int stream, QString soundName)
{
	if (! jniEnv()) {
		qDebug("jniPlaySound: jniEnv failed, can't call Java method jniPlaySound");
		return false;
	}
	javaMethodID = jniEnvironment->GetMethodID(jniProxyClass,"playSound","(ILjava/lang/String;)V"); 
	if (isJavaExceptionOccured()) {
		qDebug("jniPlaySound: could not get ID of \"playSound\"");
		return false;
	}
	jstring jSoundName = jniEnvironment->NewString((jchar*)soundName.constData(), (jsize)soundName.length() );
	jniEnvironment->CallObjectMethod( jniProxyObject, javaMethodID, (jint)stream, jSoundName );
	if (isJavaExceptionOccured()) {
		qDebug("jniPlaySound: exception when calling Java method \"playSound\"");
		return false;
	}
	return true;
}

QString jniGetAppDataDir()
{
  QString dir ="";

  bool ok = jniGetDir( "getAppDataDir", dir );

  return dir;
}

QString jniGetAddDataDir()
{
  QString dir ="";

  bool ok = jniGetDir( "getAddDataDir", dir );

  return dir;
}

bool jniGetDir( const char* method, QString& directory )
{
  directory = "";

  if (!jniEnv())
    {
      qDebug() << "jniGetDir: jniEnv failed, can't call Java method" << method;
      return false;
    }

  javaMethodID = jniEnvironment->GetMethodID( jniProxyClass,
                                              method,
                                              "()Ljava/lang/String;");

  if (isJavaExceptionOccured())
    {
      qDebug() << "jniGetDir: could not get ID of" << method;
      return false;
    }

  jstring result = (jstring) jniEnvironment->CallObjectMethod(jniProxyObject,
                                                              javaMethodID);

  if (isJavaExceptionOccured())
    {
      qDebug() << "jniGetDir: Exception when calling Java method" << method;
      return false;
    }

  const char *resultChars = jniEnvironment->GetStringUTFChars(result, 0);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  directory = QString(resultChars);

  jniEnvironment->ReleaseStringUTFChars(result, resultChars);
  jniEnvironment->DeleteLocalRef(result);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  return true;
}
