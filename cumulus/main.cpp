/***********************************************************************
 **
 **   main.cpp
 **
 **   This file is part of Cumulus. It contains the start procedure of
 **   the GUI. Currently we use the release QT/X11 4.7.x for the build
 **   process.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008-2014 by Axel Pauli
 **
 **   Email of maintainer: <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Main of Cumulus
 *
 * This file contains the start procedure of the Cumulus GUI. Cumulus is a C++
 * Application built with the Qt/X11 SDK. Qt is a cross-platform
 * application and UI framework. See here for more information:
 *
 * http://qt-project.org/
 *
 * Cumulus can be built with the Qt release 4.8.x. and 5.0.x
 *
 */

#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include <QtGui>

#include "target.h"
#include "mainwindow.h"
#include "generalconfig.h"
#include "messagehandler.h"
#include "hwinfo.h"

#ifdef ANDROID
#include "jnisupport.h"
#endif

/////////////////////
int main(int argc, char *argv[])
{
  // Workaround to start browser from QTextView
  qputenv( "BROWSER", "browser --url" );

  // @AP: Reset the locale that is used for number formatting to "C" locale.
  setlocale(LC_NUMERIC, "C");

  GeneralConfig *conf = GeneralConfig::instance();

#ifdef ANDROID

  // Gets the additional data directory from our app. That is normally the
  // storage path to the SD-Card as /sdcard/Cumulus.
  QString addDir = jniGetAddDataDir();

  while( addDir.isEmpty() )
    {
      qDebug() << " Waiting for Cumulus addDir ...";
      usleep(250000);
      addDir = jniGetAddDataDir();
    }

  // Nice trick to overwrite the HOME directory under Android by us ;-)
  // That must be done before the QApplication constructor is called.
  // Otherwise another HOME is used by QApplication.
  qputenv ( "HOME", addDir.toLatin1().data() );

  // Set data directory after every startup because different Android APIs
  // uses different locations. Otherwise that can cause problems during Andoid
  // updates to a newer release.
  conf->setUserDataDirectory( addDir );

#endif

#ifndef ANDROID
#if QT_VERSION < 0x050000
  // Note this must be called before QApplication constructor
  QApplication::setGraphicsSystem( "raster" );
#endif
#endif

  QApplication app(argc, argv, true);

  QCoreApplication::setApplicationName( "Cumulus" );
  QCoreApplication::setApplicationVersion( CU_VERSION );
  QCoreApplication::setOrganizationName( "KFLog" );
  QCoreApplication::setOrganizationDomain( "www.kflog.org" );

  // Make sure the application uses utf8 encoding for translated widgets
#if QT_VERSION < 0x050000
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );
#endif

  // Note, that first $HOME must be overwritten under Android otherwise the
  // setting file is created/searched in the internal data area under:
  // /data/data/org.kflog.cumulus/files. That is the $HOME, set by Necessitas.

#ifdef ANDROID

  // Set the add data directory in our configuration
  conf->setDataRoot( addDir );

  // As next we must wait, that the add data are installed. That is done
  // at the Java side.
  while( jniAddDataInstalled() == false )
    {
      qDebug() << " Waiting for Cumulus addData installed ...";
      usleep(250000);
    }

  // Gets the internal data directory from our App
  QString appDir = jniGetAppDataDir();

  while (appDir.isEmpty())
    {
      qDebug() << " Waiting for Cumulus appDir ...";
      usleep(250000);
      appDir = jniGetAppDataDir();
    }

  conf->setAppRoot( appDir );

#endif /* ANDROID */

  // @AP: we installing our own message handler
#if QT_VERSION < 0x050000
  qInstallMsgHandler(messageHandler);
#else
  qInstallMessageHandler(messageHandler);
#endif

  // @AP: to make trace output available, if process is started via
  // QT/X11, we can redirect all output into a file, if configuration option
  // Log2File is set to true.

#ifndef ANDROID
  // @AP: make install root of Cumulus available for other modules via
  // GeneralConfig. The assumption is that Cumulus is installed at
  // <root>/bin/cumulus. The <root> path will be passed to GeneralConfig.
  char *callPath = dirname(argv[0]);
  char *startDir = getcwd(0,0);
  chdir( callPath );
  char *callDir = getcwd(0,0);
  QString root = QString(dirname(callDir));
  conf->setAppRoot( root );
  conf->setDataRoot( root );

  // change back to start directory
  chdir( startDir );
  free( callDir );
  free( startDir );
#endif

#ifdef MAEMO
  bool isLog2File = true;
#else
  bool isLog2File = conf->getLog2FileMode();
#endif

  QString logDir = "/tmp";

#ifdef ANDROID

  // always log on Android for now
  isLog2File = true;
  logDir = QDir::homePath();

#endif

  if( isLog2File )
    {

#ifdef MAEMO
      // check for alternate paths under Maemo
      QDir path1("/media/mmc1"); // N8x0
      QDir path2("/media/mmc2"); // N8x0
      QDir path3("/home/user/MyDocs");

      if( path1.exists() && HwInfo::isMounted(path1.absolutePath()) )
        {
          logDir = path1.absolutePath();
        }
      else if( path2.exists() && HwInfo::isMounted(path2.absolutePath()) )
        {
          logDir = path2.absolutePath();
        }
      else if( path3.exists() )
         {
           logDir = path3.absolutePath();
         }

#endif

      logDir += "/cumulus.log";

      // Save one old log file version.
      rename( logDir.toLatin1().data(), (logDir + ".old").toLatin1().data() );

      int i = open( logDir.toLatin1().data(), O_RDWR|O_CREAT|O_TRUNC,
                    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );

      // Duplicate file descriptors 1, 2 that the output goes into a logfile

      // dup2( i, fileno(stdin) );
      dup2( i, fileno(stdout) );
      dup2( i, fileno(stderr) );
      close(i);
    }

#ifdef ANDROID

  qDebug() << "Cumulus addDir and QtHome:" << addDir;
  qDebug() << "Cumulus appDir:" << appDir;
  qDebug() << "Cumulus LogDir:" << logDir;

#endif

  /*
    @AP: check, if environment variable LD_BIND_NOW is set. In this case reset
    it to avoid a gps client crash during fork under Opie.

    If the process environment [see exec(base operating system)] contains a
    variable named LD_BIND_NOW with a non-null value, the dynamic linker processes
    all relocations before transferring control to the program. For example, all
    the following environment entries would specify this behavior.

    * LD_BIND_NOW=1
    * LD_BIND_NOW=on
    * LD_BIND_NOW=off

    Otherwise, LD_BIND_NOW either does not occur in the environment or has a null
    value. The dynamic linker is permitted to evaluate procedure linkage table
    entries lazily, thus avoiding symbol resolution and relocation overhead for
    functions that are not called. See the Procedure Linkage Table in this chapter
    of the processor supplement for more information.
  */

  char *env = getenv("LD_BIND_NOW");
  qDebug( "LD_BIND_NOW=%s", env ? env : "NULL" );

//  if( env != 0 )
//    {
//      unsetenv("LD_BIND_NOW");
//    }

  // Load language translation file for Cumulus.

#ifndef ANDROID

  conf->setLanguage( conf->getLanguage() );

#else

  // Gets the default language from the Android device.
   QString language = jniGetLanguage();

   // Put Android's default language into the program environment.
   qputenv( "LANG",  language.toLatin1().data() );

   qDebug() << "Android sets language to" << language;

   if( language.startsWith( "de" ) )
     {
       // In case of German there is a translation available.
       conf->setLanguage( "de" );
     }
   else
     {
       conf->setLanguage( conf->getLanguage() );
     }

  QFontDatabase database;

  foreach (const QString &family, database.families())
    {
      foreach (const QString &style, database.styles(family))
        {
          QString sizes;

          foreach (int points, database.smoothSizes(family, style))
            sizes += QString::number(points) + " ";

          qDebug() << "Installed Font:" << family << style << sizes.trimmed();
        }
    }

  QHash <QString, float> dmh = jniGetDisplayMetrics();

  QHashIterator<QString, float> i(dmh);

  qDebug() << "Android display metrics as key value list";

  while( i.hasNext() )
    {
      i.next();
      qDebug() << i.key() << "=" << i.value();
    }

  QHash <QString, QString> bdh = jniGetBuildData();

  QHashIterator<QString, QString> j(bdh);

  qDebug() << "Android build data as key value list";

  while( j.hasNext() )
    {
      j.next();
      qDebug() << j.key() << "=" << j.value();
    }

#endif

  // save done configuration settings
  conf->save();

  // create the Cumulus application window
  MainWindow *cumulus = new MainWindow( Qt::WindowContextHelpButtonHint );

  // start window manager event processing loop
  int result = QApplication::exec();

  // remove as first MainWindow because class objects inside can call GeneralConfig
  delete cumulus;

  // remove GeneralConfig, it is created during first call to it
  delete GeneralConfig::instance();

  return result;
}
