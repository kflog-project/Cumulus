/***********************************************************************
 **
 **   jnisupport.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <jni.h>

#include <QtGui>

#include "jnisupport.h"
#include "androidevents.h"
#include "gpsnmea.h"
#include "mainwindow.h"

/**
 * Adapt this define to the full Java CumulusActivity class name including
 * the package name.
 */
#define CumulusActivityClassName "org/kflog/cumulus8/CumulusActivity"

static JavaVM*   m_jvm            = 0;
static JNIEnv*   m_jniEnv         = 0;
static jobject   m_jniProxyObject = 0; // Java instance of CumulusActivity
static jmethodID m_AddDataDirID   = 0;
static jmethodID m_AppDataDirID   = 0;
static jmethodID m_playSoundID    = 0;

extern MainWindow *_globalMainWindow;

// Function declarations
bool jniEnv();
bool isJavaExceptionOccured();
bool jniGetDir( const char* method, jmethodID mId, QString& directory );

// ---- The native methods ---

/**
 * Called from the Java code on every location update by LocationListener ll
 */

static void nativeGpsFix( JNIEnv * /*jniEnvironment*/,
                          jobject /*myproxyobject*/,
                          jdouble lati,
                          jdouble longi,
                          jdouble alt,
                          jfloat speed,
                          jfloat bear,
                          jfloat accu,
                          jlong time )
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

/**
 * Called from the Java code on every status change by LocationListener ll
 */

static void nativeGpsStatus( JNIEnv * /*jniEnvironment*/,
                             jobject /*myproxyobject*/,
                             jint status,
                             jint numsats )
{
//	qDebug("*** nativeGpsStatus: st %d , ns %d", status, numsats);
	GpsStatusEvent *ge = new GpsStatusEvent();
	ge->stat = status;
	ge->nsats = numsats;
	QCoreApplication::postEvent(GpsNmea::gps, ge);
}

static void nativeNmeaString(JNIEnv* env, jobject /*myobject*/, jstring jnmea)
{
	const char * nativeString = env->GetStringUTFChars(jnmea, 0);
	QString qnmea(nativeString);
	env->ReleaseStringUTFChars(jnmea, nativeString);
	GpsNmeaEvent *ne = new GpsNmeaEvent();
	ne->nmea_sentence = qnmea;
	QCoreApplication::postEvent(GpsNmea::gps, ne);
}

static void nativeKeypress(JNIEnv* /*env*/, jobject /*myobject*/, jchar code)
{
  unsigned int qtCode;
  qDebug("nativeKeypress: code is %d", (unsigned int) code);

  switch ((unsigned int) code)
    {
  case 25:
    qtCode = Qt::Key_F11;
    break;
  case 26:
    qtCode = Qt::Key_F12;
    break;
  case 27:
    qtCode = Qt::Key_F13;
    break;
  default:
    qDebug("nativeKeypress: code is %d", code);
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
  qDebug() << "JNI isRootWindow()" << MainWindow::isRootWindow();
  return MainWindow::isRootWindow();
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

/**
  * Called by qtmain_android.cpp to initialize the Java native interface.
  */

bool initJni( JavaVM* vm, JNIEnv* env )
{
  qDebug() << "initJni() is called: vm=" << vm << "env=" << env;

  m_jvm    = vm;
  m_jniEnv = env;

  // @AP: Note! The call FindClass do work only in this
  // method, which is called by the JNI_OnLoad method from:
  // necessitas/Android/Qt/480/armeabi/src/android/cpp/qtmain_android.cpp
  // The normal C++ program is started in an own thread, which has no
  // knowlegde about the java classes. See here for more info:
  // http://developer.android.com/guide/practices/design/jni.html
  // under FAQ: Why didn't FindClass find my class?
  jclass clazz = m_jniEnv->FindClass(CumulusActivityClassName);

  if( clazz == 0 )
    {
      qWarning( "initJni: FindClass 'org/kflog/cumulus7/CumulusActivity' failed!" );
      return false;
    }

  if( m_jniEnv->RegisterNatives( clazz,
                                 methods,
                                 sizeof(methods) / sizeof(methods[0])) < 0 )
    {
      qWarning( "initJni: Registering of Java custom natives failed!" );
      return false;
    }

  qDebug("initJni: Native methods registered successfully");

  jmethodID jId = m_jniEnv->GetStaticMethodID( clazz,
                                               "getObjectRef",
                                               "()Ljava/lang/Object;");

  if (isJavaExceptionOccured())
    {
      return false;
    }

  jobject result = (jobject) m_jniEnv->CallStaticObjectMethod( clazz, jId );

  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: Exception when calling CallStaticObjectMethod";
      return false;
    }

  if( ! result )
    {
      qWarning() << "initJni: Null activity object returned";
      return false;
    }

  // Save result as global reference. Otherwise it becomes invalid after return.
  // http://developer.android.com/guide/practices/design/jni.html

  // TODO: free this reference at the end of main
  m_jniProxyObject = m_jniEnv->NewGlobalRef(result);

  m_AddDataDirID = m_jniEnv->GetMethodID( clazz,
                                          "getAddDataDir",
                                          "()Ljava/lang/String;");
  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of getAddDataDir";
      return false;
    }

  m_AppDataDirID = m_jniEnv->GetMethodID( clazz,
                                          "getAppDataDir",
                                          "()Ljava/lang/String;");
  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of getAppDataDir";
      return false;
    }

  m_playSoundID = m_jniEnv->GetMethodID( clazz,
                                         "playSound",
                                         "(ILjava/lang/String;)V");

  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of playSound";
      return false;
    }

  return true;
}

/* The following two functions are borrowed from Denis Kormalev,
 * out of his post on the qt-android mailing list:
 * http://groups.google.com/group/android-qt/browse_thread/thread/ ..
 * .. d60d28796f61177d/4bf2e434c9cddf77#4bf2e434c9cddf77
 */

// Exception checking
bool isJavaExceptionOccured()
{
  if (!m_jniEnv)
    {
      return false;
    }

  if (m_jniEnv->ExceptionOccurred())
    {
      m_jniEnv->ExceptionDescribe();
      m_jniEnv->ExceptionClear();
      return true;
    }

  return false;
}

// JNI initialization
bool jniEnv()
{
  // qDebug("C++ jniEnv: started");

  m_jniEnv = 0;

  if (m_jvm == 0)
    {
      qWarning() << "jniEnv: No Java VM available!";
      return false;
    }

  JavaVMAttachArgs args;
  args.name = 0;
  args.group = 0;
  args.version = JNI_VERSION_1_4;

  m_jvm->AttachCurrentThread(&m_jniEnv, &args);

  if (isJavaExceptionOccured())
    {
      qWarning("jniEnv: exception during AttachCurrentThread");
      return false;
    }

  if (!m_jniEnv)
    {
      qWarning("jniEnv: could not get Java environment");
      return false;
    }

  return true;
}

bool jniPlaySound(int stream, QString soundName)
{
  if (!jniEnv())
    {
      return false;
    }

  jstring jSoundName = m_jniEnv->NewString((jchar*) soundName.constData(),
                                           (jsize) soundName.length());

  m_jniEnv->CallObjectMethod( m_jniProxyObject,
                              m_playSoundID,
                              (jint) stream,
                              jSoundName );

  if (isJavaExceptionOccured())
    {
      qWarning("jniPlaySound: exception when calling Java method \"playSound\"");
      return false;
    }

  return true;
}

QString jniGetAppDataDir()
{
  QString dir ="";

  bool ok = jniGetDir( "getAppDataDir", m_AppDataDirID, dir );

  Q_UNUSED(ok)

  return dir;
}

QString jniGetAddDataDir()
{
  QString dir ="";

  bool ok = jniGetDir( "getAddDataDir", m_AddDataDirID, dir );

  Q_UNUSED(ok)

  return dir;
}

bool jniGetDir( const char* method, jmethodID mId, QString& directory )
{
  directory = "";

  if (!jniEnv())
    {
      qDebug() << "jniGetDir: jniEnv failed, can't call Java method" << method;
      return false;
    }

  jstring result = (jstring) m_jniEnv->CallObjectMethod( m_jniProxyObject, mId);

  if (isJavaExceptionOccured())
    {
      qDebug() << "jniGetDir: Exception when calling Java method" << method;
      return false;
    }

  const char *resultChars = m_jniEnv->GetStringUTFChars(result, 0);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  directory = QString(resultChars);

  m_jniEnv->ReleaseStringUTFChars(result, resultChars);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  return true;
}
