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

/*
 * Look here for more info about jni usage under Android:
 *
 * http://developer.android.com/guide/practices/jni.html
 */

#include <unistd.h>
#include <jni.h>

#include <QtGui>
#include <QWindowSystemInterface>

#include "jnisupport.h"
#include "androidevents.h"
#include "gpsnmea.h"
#include "generalconfig.h"
#include "mainwindow.h"

/**
 * Adapt this define to the full Java CumulusActivity class name including
 * the package name.
 */
#define CumulusActivityClassName "org/kflog/cumulus/CumulusActivity"

static JavaVM*   m_jvm            = 0;
static JNIEnv*   m_jniEnv         = 0;
static jobject   m_jniProxyObject = 0; // Java instance of CumulusActivity
static jmethodID m_AddDataDirID   = 0;
static jmethodID m_AppDataDirID   = 0;
static jmethodID m_languageID     = 0;
static jmethodID m_playSoundID    = 0;
static jmethodID m_dimmScreenID   = 0;
static jmethodID m_gpsCmdID       = 0;
static jmethodID m_byte2Gps       = 0;

// Function declarations
bool jniEnv();
bool isJavaExceptionOccured();
bool jniCallStringMethod( const char* method, jmethodID mId, QString& strResult );
void forwardNmea( QString& nmea );

// ---- The native methods ---

/**
 * Called from the Java code on every location update by LocationListener ll
 */
static void nativeGpsFix( JNIEnv * /*jniEnvironment*/,
                             jobject /*myproxyobject*/,
                             jdouble latitude,
                             jdouble longitude,
                             jdouble altitude,
                             jfloat speed,
                             jfloat heading,
                             jfloat accuracy,
                             jlong time )
{
  // qDebug("*** nativeGpsFix: lat %f, lon %f", lati, longi);
  GpsFixEvent *ge = new GpsFixEvent( latitude,
                                     longitude,
                                     altitude,
                                     speed,
                                     heading,
                                     accuracy,
                                     time );

  QCoreApplication::postEvent( GpsNmea::gps, ge, Qt::HighEventPriority );
}

/**
 * Called from the Java code on every status change by LocationListener ll
 */

static void nativeGpsStatus( JNIEnv * /*jniEnvironment*/,
                                jobject /*myproxyobject*/,
                                jint status )
{
  GpsStatusEvent *ge = new GpsStatusEvent( status );
  QCoreApplication::postEvent( GpsNmea::gps, ge );
}

static void nativeNmeaString(JNIEnv* env, jobject /*myobject*/, jstring jnmea)
{
  const char * nativeString = env->GetStringUTFChars(jnmea, 0);
  QString qnmea(nativeString);
  env->ReleaseStringUTFChars(jnmea, nativeString);
  forwardNmea( qnmea );
}

static void forwardNmea( QString& qnmea )
{
  static QHash<QString, short> gpsKeys;
  static GeneralConfig* gci = 0;
  static bool init = false;

  if( init == false )
    {
      GpsNmea::getGpsMessageKeys( gpsKeys );
      gci = GeneralConfig::instance();
      init = true;
    }

  if( gci->getGpsNmeaLogState() == false )
    {
      // Check, if sentence is of interest for us.
      QString item = qnmea.mid( 0, qnmea.indexOf( QChar(',') ) );

      if( gpsKeys.contains(item) == false )
        {
          // Ignore undesired sentences for performance reasons. They are
          // only forwarded, if data file logging is switched on.
          return;
        }
    }

  GpsNmeaEvent *ne = new GpsNmeaEvent(qnmea);
  QCoreApplication::postEvent( GpsNmea::gps, ne, Qt::HighEventPriority );
}

static void nativeKeypress(JNIEnv* /*env*/, jobject /*myobject*/, jchar code)
{
  // qDebug("JNI nativeKeypress: code is %d", (unsigned int) code);

  if( MainWindow::isRootWindow() == false )
    {
      // Forward keys only if the root window is active.
      return;
    }

  unsigned int qtCode;

  switch( code )
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

      case 28:
        // The close key at the Android device is pressed. It is forwarded as
        // close key to close the main window.
        qtCode = Qt::Key_Close;
        qDebug() << "JNI nativeKeypress: Close is sent";
        break;

      case 29:
        // toggle visibility of map info boxes
        qtCode = Qt::Key_D;
        break;

      default:
        qWarning("JNI nativeKeypress: code %d is unknown!", code);
        return;
    }

  // Only the main window shall receive key events. Especially the close key has
  // made a lot of trouble, if it was sent to other widgets as the main window.
  // Therefore I decided to forward the close key only once to the main window.
  // All other widgets have a close button, which should be used instead.
  QObject *receiver = MainWindow::mainWindow();

#if 0
  if( QApplication::activeModalWidget() )
    {
      receiver = QApplication::activeModalWidget();
    }
  else if( QApplication::focusWidget() )
    {
      receiver = QApplication::focusWidget();
    }
  else if( QApplication::activeWindow() )
    {
      // Forward key event to the current active window.
      receiver = QApplication::activeWindow();
    }

  qDebug() << "KeyReceiver:" << receiver
            << "ActiveWindow:" << QApplication::activeWindow()
            << "MainWindow:" << MainWindow::mainWindow()
            << "FocusWindow:" << QApplication::focusWidget()
            << "ActiveModalWidget:" << QApplication::activeModalWidget();
#endif

  QKeyEvent *kpe = new QKeyEvent( QEvent::KeyPress, qtCode, Qt::NoModifier );
  QCoreApplication::postEvent( receiver, kpe, Qt::NormalEventPriority );

  // Make a short break to simulate a key press
  usleep( 100 * 1000 );

  QKeyEvent *kre = new QKeyEvent( QEvent::KeyRelease, qtCode, Qt::NoModifier );
  QCoreApplication::postEvent( receiver, kre, Qt::NormalEventPriority );

#if 0
  // Bogdan wrote, that handleKeyEvent do not work properly
  // Callback functions for plugins found in gui/kernel/qwindowsysteminterface_qpa.cpp
  // Necessitas use that in plugins/platforms/android/src/androidjnimain.cpp
  QWidget *w = QApplication::activeWindow();
  QWindowSystemInterface::handleKeyEvent(w, QEvent::KeyPress,   qtCode, Qt::NoModifier);
  QWindowSystemInterface::handleKeyEvent(w, QEvent::KeyRelease, qtCode, Qt::NoModifier);
#endif
}

static bool isRootWindow()
{
  return MainWindow::isRootWindow();
}

static void nativeByteFromGps(JNIEnv* /*env*/, jobject /*myobject*/, jbyte byte)
{
  // A byte was read from the Java part from the BT port.

}

/* The array of native methods to register.
 * The name string must match the "native" declaration in Java.
 * The parameter string must match the types in the "native" declaration
 * ( B = byte, I = integer, J = long, F = float, D = double, V = void etc. )
 * see: http://java.sun.com/docs/books/jni/html/types.html#65751
 */
static JNINativeMethod methods[] = {
	{"nativeGpsFix", "(DDDFFFJ)V", (void *)nativeGpsFix},
	{"nativeGpsStatus", "(I)V", (void *)nativeGpsStatus},
	{"nativeNmeaString","(Ljava/lang/String;)V", (void *)nativeNmeaString},
	{"nativeKeypress", "(C)V", (void *)nativeKeypress},
	{"isRootWindow", "()Z", (bool *)isRootWindow},
        {"nativeByteFromGps", "(B)V", (void *)nativeByteFromGps}
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

  m_languageID = m_jniEnv->GetMethodID( clazz,
                                        "getLanguage",
                                        "()Ljava/lang/String;");
  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of getLanguage";
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

  m_dimmScreenID = m_jniEnv->GetMethodID( clazz,
                                          "dimmScreen",
                                          "(Z)V");

  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of playSound";
      return false;
    }

  m_gpsCmdID = m_jniEnv->GetMethodID( clazz,
                                      "gpsCmd",
                                      "(Ljava/lang/String;)Z");

  if (isJavaExceptionOccured())
    {
      qDebug() << "initJni: could not get ID of gpsCmd";
      return false;
    }

  m_byte2Gps= m_jniEnv->GetMethodID( clazz,
                                     "byte2Gps",
                                     "(B)Z");

  if (isJavaExceptionOccured())
  {
    qDebug() << "initJni: could not get ID of byte2Gps";
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

  m_jniEnv->CallVoidMethod( m_jniProxyObject,
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

bool jniGpsCmd(QString& cmd)
{
  if (!jniEnv())
    {
      return false;
    }

  jstring jgpsCmd = m_jniEnv->NewString((jchar*) cmd.constData(),
                                        (jsize) cmd.length());

  jboolean result = (jboolean) m_jniEnv->CallBooleanMethod( m_jniProxyObject,
                                                            m_gpsCmdID,
                                                            jgpsCmd );
  if (isJavaExceptionOccured())
    {
      qWarning("jniGpsCmd: exception when calling Java method \"gpsCmd\"");
      return false;
    }

  return result;
}

bool jniByte2Gps(const char byte)
{
  if (!jniEnv())
    {
      return false;
    }

  jboolean result = (jboolean) m_jniEnv->CallBooleanMethod( m_jniProxyObject,
                                                            m_byte2Gps,
                                                            static_cast<jbyte> (byte) );
  if (isJavaExceptionOccured())
    {
      qWarning("jniByte2Gps: exception when calling Java method \"byte2Gps\"");
      return false;
    }

  return result;
}


void jniDimmScreen( bool newState )
{
  if (!jniEnv())
    {
      return;
    }

  m_jniEnv->CallVoidMethod( m_jniProxyObject,
                            m_dimmScreenID,
                            (jboolean) newState );

  if (isJavaExceptionOccured())
    {
      qWarning("jniDimmScreen: exception when calling Java method \"dimmScreen\"");
      return;
    }

  return;
}

QString jniGetAppDataDir()
{
  QString dir ="";

  bool ok = jniCallStringMethod( "getAppDataDir", m_AppDataDirID, dir );

  Q_UNUSED(ok)
  return dir;
}

QString jniGetAddDataDir()
{
  QString dir ="";

  bool ok = jniCallStringMethod( "getAddDataDir", m_AddDataDirID, dir );

  Q_UNUSED(ok)
  return dir;
}

QString jniGetLanguage()
{
  QString lang ="";

  bool ok = jniCallStringMethod( "getLanguage", m_languageID, lang );

  Q_UNUSED(ok)
  return lang;
}

bool jniCallStringMethod( const char* method, jmethodID mId, QString& strResult )
{
  strResult = "";

  if (!jniEnv())
    {
      qDebug() << "jniCallStringMethod: jniEnv failed, can't call Java method" << method;
      return false;
    }

  jstring result = (jstring) m_jniEnv->CallObjectMethod( m_jniProxyObject, mId);

  if (isJavaExceptionOccured())
    {
      qDebug() << "jniCallStringMethod: Exception when calling Java method" << method;
      return false;
    }

  const char *resultChars = m_jniEnv->GetStringUTFChars(result, 0);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  strResult = QString(resultChars);

  m_jniEnv->ReleaseStringUTFChars(result, resultChars);

  if (isJavaExceptionOccured())
    {
      return false;
    }

  return true;
}
