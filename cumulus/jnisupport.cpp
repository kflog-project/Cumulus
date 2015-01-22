/***********************************************************************
 **
 **   jnisupport.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2010-2012 by Josua Dietze
 **                   2012-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/*
 * Look here for more info about jni usage under Android:
 *
 * http://developer.android.com/guide/practices/jni.html
 */

#include <unistd.h>

#include <QtGui>
//#include <QWindowSystemInterface>

#include "jnisupport.h"
#include "androidevents.h"
#include "calculator.h"
#include "gpsconandroid.h"
#include "gpsnmea.h"
#include "generalconfig.h"
#include "mainwindow.h"

/**
 * Adapt this define to the full Java CumulusActivity class name including
 * the package name.
 */
#define CumulusActivityClassName "org/kflog/cumulus/CumulusActivity"

static JavaVM*   m_jvm                = 0;
static jclass    m_CumActClass        = 0;
static jobject   m_cumActObject       = 0; // Java instance of CumulusActivity
static jmethodID m_AddDataDirID       = 0;
static jmethodID m_AddDataInstalledID = 0;
static jmethodID m_AppDataDirID       = 0;
static jmethodID m_languageID         = 0;
static jmethodID m_DisplayMetricsID   = 0;
static jmethodID m_BuildDataID        = 0;
static jmethodID m_playSoundID        = 0;
static jmethodID m_dimmScreenID       = 0;
static jmethodID m_gpsCmdID           = 0;
static jmethodID m_callRetrieverID    = 0;
static jmethodID m_byte2Gps           = 0;
static jmethodID m_nativeShutdownID   = 0;
static jmethodID m_openHardwareMenu   = 0;
static jmethodID m_downloadFile       = 0;

// Shutdown flag to disable message transfer to the GUI. It is reset by the
// MainWindow class.
static bool shutdown = true;

// Function declarations
bool jniCallStringMethod( const char* method, jmethodID mId, QString& strResult );

bool jniShutdownFlag()
{
  return shutdown;
}

// Necessitas has moved the JNI interface to another file in SDK alpha 4.
// Therefore we can provide an own JNI_ONLOAD function.
// http://developer.android.com/guide/practices/jni.html
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
  JNIEnv* env;

  if( vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK )
    {
      return -1;
    }

  m_jvm    = vm;

  bool ok = initJni( vm, env );

  if( ok )
    {
      return JNI_VERSION_1_6;
    }

  return -1;
}

void jniShutdown( bool option )
{
  // Sets the shutdown flag to true or false. This function is called by the
  // MainWindow class, to signal a shutdown or startup complete of the
  // application. In shutdown case all message forwarding has to be stopped to
  // the GUI part. Otherwise the App can crash in the shutdown phase.
  shutdown = option;

  JNIEnv* env = 0;

  bool ok = jniEnv( &env );

  if( ok )
    {
      // Tells the Java side, the state of the native part. Is option is true,
      // the native side is going down. The java side removes then all handlers,
      // which can call the native side to avoid unexpected behavior.
      env->CallVoidMethod( m_cumActObject,
                           m_nativeShutdownID,
                           (jboolean) option );

      if ( isJavaExceptionOccured(env) )
	{
	  qWarning("jniShutdown: exception when calling Java method \"nativeShutdown\"");
	}
    }

  jniDetachCurrentThread();
}

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
  if( ! shutdown )
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
}

/**
 * Called from the Java code on every status change by LocationListener ll
 */
static void nativeGpsStatus( JNIEnv * /*jniEnvironment*/,
                             jobject /*myproxyobject*/,
                             jint status )
{
  if( ! shutdown )
    {
      GpsStatusEvent *ge = new GpsStatusEvent( status );
      QCoreApplication::postEvent( GpsNmea::gps, ge );
    }
}

static void nativeNmeaString(JNIEnv* env, jobject /*myobject*/, jstring jnmea)
{
  if( ! shutdown )
    {
      const char* nativeString = env->GetStringUTFChars(jnmea, 0);
      QString qnmea(nativeString);
      env->ReleaseStringUTFChars(jnmea, nativeString);

      GpsConAndroid::forwardNmea( qnmea );
    }
}

static void nativeKeypress(JNIEnv* /*env*/, jobject /*myobject*/, jchar code)
{
  qDebug("JNI nativeKeypress: code is %d", (unsigned int) code);

  if( shutdown == true || MainWindow::isRootWindow() == false )
    {
      // Forward keys only if the root window is active and shutdown is false.
      return;
    }

  extern Calculator* calculator;

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

      case 28: // Close is requested via the back key. That is not enabled,
               // if Cumulus is in move to avoid an undesired press.
        if( calculator != 0 && calculator->moving() )
          {
            return;
          }

        qtCode = Qt::Key_Close;
        qDebug() << "JNI nativeKeypress: Close is sent";
        break;

      case 30: // Close is requested via the Android menu
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

  if( ! receiver )
    {
      return;
    }

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
  usleep( 10 * 1000 );

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
  // A byte was read from the Java part via the BT port. It is forwarded to the
  // GPS data handler class for Android.
  if( ! shutdown )
    {
      GpsConAndroid::rcvByte( (const char) byte );
    }
}

static void nativeBaroAltitude( JNIEnv* /*env*/,
                                jobject /*myobject*/,
                                jdouble altitude )
{
  if( ! shutdown )
    {
      AltitudeEvent *ae = new AltitudeEvent( altitude );
      QCoreApplication::postEvent( GpsNmea::gps, ae );
    }
}

/**
 * Called from the Java side, if the app if recreated after a kill to restore
 * the last target in the calculator.
 */
static void nativeRestoreTarget( JNIEnv* /*env*/, jobject /*myobject*/ )
{
  if( shutdown == true )
    {
      return;
    }

  extern Calculator* calculator;

  CalculatorRestoreTargetEvent *crte = new CalculatorRestoreTargetEvent();
  QCoreApplication::postEvent( calculator, crte );
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
	{"nativeByteFromGps", "(B)V", (void *)nativeByteFromGps},
	{"nativeBaroAltitude", "(D)V", (void *)nativeBaroAltitude},
	{"nativeRestoreTarget", "()V", (void *)nativeRestoreTarget}
};

/**
  * Called by qtmain_android.cpp to initialize the Java native interface.
  */
bool initJni( JavaVM* vm, JNIEnv* env )
{
  qDebug() << "initJni() is called: vm=" << vm << "env=" << env;

  // @AP: Note! The call FindClass do work only in this
  // method, which is called by the JNI_OnLoad method from:
  // necessitas/Android/Qt/480/armeabi/src/android/cpp/qtmain_android.cpp
  // The normal C++ program is started in an own thread, which has no
  // knowlegde about the java classes. See here for more info:
  // http://developer.android.com/guide/practices/design/jni.html
  // under FAQ: Why didn't FindClass find my class?
  jclass clazz = env->FindClass(CumulusActivityClassName);

  if( clazz == 0 )
    {
      qWarning( "initJni: FindClass 'org/kflog/cumulus/CumulusActivity' failed!" );
      return false;
    }

  if( env->RegisterNatives( clazz,
                            methods,
                            sizeof(methods) / sizeof(methods[0])) < 0 )
    {
      qWarning( "initJni: Registering of Java custom natives failed!" );
      return false;
    }

  qDebug("initJni: Native methods registered successfully");

  jmethodID jId = env->GetStaticMethodID( clazz,
                                          "getObjectRef",
                                          "()Ljava/lang/Object;");

  if ( isJavaExceptionOccured(env) )
    {
      return false;
    }

  jobject object = (jobject) env->CallStaticObjectMethod( clazz, jId );

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: Exception when calling CallStaticObjectMethod";
      return false;
    }

  if( ! object )
    {
      qWarning() << "initJni: Null activity object returned";
      return false;
    }

  // Save object as global reference. Otherwise it becomes invalid after return.
  // http://developer.android.com/guide/practices/design/jni.html

  // TODO: free this reference at the end of main
  m_CumActClass = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));

  // TODO: free this reference at the end of main
  m_cumActObject = env->NewGlobalRef(object);

  m_AddDataDirID = env->GetMethodID( clazz,
                                     "getAddDataDir",
                                     "()Ljava/lang/String;");
  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of getAddDataDir";
      return false;
    }

  m_AppDataDirID = env->GetMethodID( clazz,
                                     "getAppDataDir",
                                      "()Ljava/lang/String;");
  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of getAppDataDir";
      return false;
    }

  m_languageID = env->GetMethodID( clazz,
                                   "getLanguage",
                                   "()Ljava/lang/String;");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of getLanguage";
      return false;
    }

  m_DisplayMetricsID = env->GetMethodID( clazz,
                                         "getDisplayMetrics",
                                         "()Ljava/lang/String;");
  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of getDisplayMetrics";
      return false;
    }


  m_BuildDataID = env->GetMethodID( clazz,
                                    "getBuildData",
                                    "()Ljava/lang/String;");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of getBuildData";
      return false;
    }

  m_playSoundID = env->GetMethodID( clazz,
                                    "playSound",
                                    "(ILjava/lang/String;)V");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of playSound";
      return false;
    }

  m_nativeShutdownID = env->GetMethodID( clazz,
                                         "nativeShutdown",
                                         "(Z)V");

  if (isJavaExceptionOccured(env) )
  {
    qWarning() << "initJni: could not get ID of nativeShutdown";
    return false;
  }

  m_dimmScreenID = env->GetMethodID( clazz,
                                     "dimmScreen",
                                     "(Z)V");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of playSound";
      return false;
    }

  m_gpsCmdID = env->GetMethodID( clazz,
                                 "gpsCmd",
                                 "(Ljava/lang/String;)Z");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of gpsCmd";
      return false;
    }

  m_callRetrieverID = env->GetMethodID( clazz,
                                        "callRetriever",
                                        "(Ljava/lang/String;)V");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of callRetriever";
      return false;
    }

  m_byte2Gps= env->GetMethodID( clazz,
                                "byte2Gps",
                                "(B)Z");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of byte2Gps";
      return false;
    }

  m_AddDataInstalledID = env->GetStaticMethodID( clazz,
                                                 "addDataInstalled",
                                                 "()Z");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of addDataInstalled";
      return false;
    }

  m_openHardwareMenu = env->GetMethodID( clazz,
                                         "openHardwareMenu",
                                         "()V");

  if (isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of openHardwareMenu";
      return false;
    }

  m_downloadFile = env->GetMethodID( clazz,
                                     "downloadFile",
                                     "(Ljava/lang/String;Ljava/lang/String;)I");

  if ( isJavaExceptionOccured(env) )
    {
      qWarning() << "initJni: could not get ID of downloadFile";
      return false;
    }

  return true;
}

/* The following two functions are borrowed from Denis Kormalev,
 * out of his post on the qt-android mailing list:
 * http://groups.google.com/group/android-qt/browse_thread/thread/ ..
 * .. d60d28796f61177d/4bf2e434c9cddf77#4bf2e434c9cddf77
 */
bool isJavaExceptionOccured( JNIEnv* env )
{
  if ( env == 0 )
    {
      return false;
    }

  if (env->ExceptionOccurred())
    {
      env->ExceptionDescribe();
      env->ExceptionClear();
      return true;
    }

  return false;
}

bool jniEnv( JNIEnv** env )
{
  *env = 0;

  if (m_jvm == 0)
    {
      qWarning() << "jniEnv: No Java VM available!";
      return false;
    }

  JavaVMAttachArgs args;
  args.name = 0;
  args.group = 0;
  args.version = JNI_VERSION_1_6;

  m_jvm->AttachCurrentThread((void **) env, &args);

  if (! *env)
    {
      qWarning("jniEnv: could not get Java environment");
      return false;
    }

  return true;
}

bool jniDetachCurrentThread()
{
  if (m_jvm == 0)
    {
      qWarning() << "jniDCT: No Java VM available!";
      return false;
    }

  jint res = m_jvm->DetachCurrentThread();

  if( res != 0 )
    {
      qWarning("jniDCT: failure during DetachCurrentThread");
      return false;
    }

  return true;
}

bool jniPlaySound(int stream, QString soundName)
{
  JNIEnv* env = 0;

  if (!jniEnv(&env) || shutdown )
    {
      return false;
    }

  jstring jSoundName = env->NewStringUTF(soundName.toUtf8());

  env->CallVoidMethod( m_cumActObject,
                       m_playSoundID,
                       (jint) stream,
                       jSoundName );

  bool result = true;

  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniPlaySound: exception when calling Java method \"playSound\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

bool jniGpsCmd(QString& cmd)
{
  JNIEnv* env = 0;

  if ( !jniEnv( &env ) || shutdown )
    {
      return false;
    }

  jstring jgpsCmd = env->NewStringUTF(cmd.toUtf8());

  jboolean result = (jboolean) env->CallBooleanMethod( m_cumActObject,
                                                       m_gpsCmdID,
                                                       jgpsCmd );
  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniGpsCmd: exception when calling Java method \"gpsCmd\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

bool jniCallRetriever( QString& smsText )
{
  JNIEnv* env = 0;

  if (!jniEnv( &env ) || shutdown )
    {
      return false;
    }

  jstring jsmsText = env->NewStringUTF(smsText.toUtf8());

  env->CallVoidMethod( m_cumActObject,
                       m_callRetrieverID,
                       jsmsText );

  bool result = true;

  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniGpsCmd: exception when calling Java method \"jniCallRetriever\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

bool jniByte2Gps(const char byte)
{
  JNIEnv* env = 0;

  if (!jniEnv( &env ) || shutdown )
    {
      return false;
    }

  jboolean result = (jboolean) env->CallBooleanMethod( m_cumActObject,
                                                       m_byte2Gps,
                                                       static_cast<jbyte> (byte) );
  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniByte2Gps: exception when calling Java method \"byte2Gps\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

void jniDimmScreen( bool newState )
{
  JNIEnv* env = 0;

  if ( !jniEnv( &env ) || shutdown )
    {
      return;
    }

  env->CallVoidMethod( m_cumActObject,
                       m_dimmScreenID,
                       (jboolean) newState );

  if( isJavaExceptionOccured(env) )
    {
      qWarning("jniDimmScreen: exception when calling Java method \"dimmScreen\"");
    }

  jniDetachCurrentThread();
  return;
}

QString jniGetAppDataDir()
{
  QString dir;

  bool ok = jniCallStringMethod( "getAppDataDir", m_AppDataDirID, dir );

  Q_UNUSED(ok)
  return dir;
}

QString jniGetAddDataDir()
{
  QString dir;

  bool ok = jniCallStringMethod( "getAddDataDir", m_AddDataDirID, dir );

  Q_UNUSED(ok)
  return dir;
}

bool jniAddDataInstalled()
{
  JNIEnv* env = 0;

  if( !jniEnv( &env ) )
    {
      return false;
    }

  jboolean result = (jboolean) env->CallStaticBooleanMethod( m_CumActClass,
                                                             m_AddDataInstalledID );

  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniAddDataInstalled: exception when calling Java method \"addDataInstalled\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

QString jniGetLanguage()
{
  QString lang;

  bool ok = jniCallStringMethod( "getLanguage", m_languageID, lang );

  Q_UNUSED(ok)
  return lang;
}

bool jniOpenHardwareMenu()
{
  JNIEnv* env = 0;

  if (!jniEnv( &env ) || shutdown )
    {
      return false;
    }

  env->CallVoidMethod( m_cumActObject, m_openHardwareMenu );

  bool result = true;

  if ( isJavaExceptionOccured( env ) )
    {
      qWarning("jniOpenHardwareMenu: exception when calling Java method \"openHardwareMenu\"");
      result = false;
    }

  jniDetachCurrentThread();
  return result;
}

QHash<QString, float> jniGetDisplayMetrics()
{
  QString displayMetrics;
  QHash<QString, float> dmh;

  bool ok = jniCallStringMethod( "getDisplayMetrics", m_DisplayMetricsID, displayMetrics );

  if( ok == false || displayMetrics.size() == 0 )
    {
      // No metrics are returned
      return dmh;
    }

  // The returned string is a key value string separated with semicolons with
  // the following content:
  // density=<float>;
  // densityDpi=<float>;
  // heightPixels=<int>;
  // scaledDensity=<float>;
  // widthPixels=<int>;
  // xdpi=<float>;
  // ydpi=<float>;

  QStringList kvList = displayMetrics.split(";", QString::SkipEmptyParts );

  if( kvList.size() == 0 )
    {
      // No metrics are returned
      return dmh;
    }

  for( int i = 0; i < kvList.size(); i++ )
    {
      // Split the elements into key and value
      QStringList kvPair = kvList.at(i).split("=");

      if( kvPair.size() == 2 )
        {
          dmh.insert( kvPair.at(0), kvPair.at(1).toFloat() );
        }
    }

  return dmh;
}

QHash<QString, QString> jniGetBuildData()
{
  QString buildData;
  QHash<QString, QString> bdh;

  bool ok = jniCallStringMethod( "getBuildData", m_BuildDataID, buildData );

  if( ok == false )
    {
      return bdh;
    }

  if( buildData.size() == 0 )
    {
      // No data are returned
      return bdh;
    }

  // The returned string is a key value string separated with newline signs
  // containing the following keys:
  // CPU_ABI
  // BRAND
  // PRODUCT
  // MANUFACTURE
  // HARDWARE
  // MODEL
  // DEVICE
  // DISPLAY
  // FINGERPRINT
  // ID
  // SERIAL

  QStringList kvList = buildData.split("\n", QString::SkipEmptyParts );

  if( kvList.size() == 0 || kvList.size() % 2 )
    {
      // No or not enough build data are returned.
      return bdh;
    }

  for( int i = 0; i < kvList.size() - 1; i += 2 )
    {
      bdh.insert( kvList.at(i), kvList.at( i+1 ) );
    }

  return bdh;
}

bool jniCallStringMethod( const char* method, jmethodID mId, QString& strResult )
{
  JNIEnv* env = 0;
  strResult.clear();

  if ( ! jniEnv(&env) )
    {
      qWarning() << "jniCallStringMethod: jniEnv failed, can't call Java method" << method;
      return false;
    }

  jstring result = (jstring) env->CallObjectMethod( m_cumActObject, mId);

  if (isJavaExceptionOccured( env ))
    {
      qWarning() << "jniCallStringMethod: Exception when calling Java method" << method;
      jniDetachCurrentThread();
      return false;
    }

  const char *resultChars = env->GetStringUTFChars(result, 0);

  if( resultChars != 0 )
    {
      strResult = QString(resultChars);
    }

  env->ReleaseStringUTFChars(result, resultChars);
  jniDetachCurrentThread();
  return true;
}

int jniDownloadFile( QString& url, QString& destination )
{
  JNIEnv* env = 0;

  if( !jniEnv( &env ) || shutdown )
    {
      return OperationCanceledError;
    }

  jstring jurl = env->NewStringUTF( url.toUtf8() );

  jstring jdest = env->NewStringUTF( destination.toUtf8() );

  jint result = env->CallIntMethod( m_cumActObject,
                                    m_downloadFile,
                                    jurl,
                                    jdest );

  if ( isJavaExceptionOccured(env) )
    {
      qWarning("jniGpsCmd: exception when calling Java method \"downloadFile\"");
      result = OperationCanceledError;
    }

  jniDetachCurrentThread();
  return result;
}
