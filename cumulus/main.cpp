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
 **   Copyright (c):  2008-2012 by Axel Pauli
 **
 **   Email of maintainer: axel@kflog.org
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
 * Application built with the QT/X11 SDK from Nokia. Qt is a cross-platform
 * application and UI framework. See here for more information:
 *
 * http://qt.nokia.com
 *
 * Cumulus is built with the release 4.8.x.
 *
 */

#include <clocale>
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

/** Define the disclaimer version */
#define DISCLAIMER_VERSION 1

/////////////////////
int main(int argc, char *argv[])
{
  // Workaround to start browser from QTextView
  qputenv ( "BROWSER", "browser --url" );

#ifdef ANDROID

  // Gets the additional data dir from our app. That is normally the storage
  // path to the SDCard.
  QString addDir = jniGetAddDataDir();

  while (addDir.isEmpty())
    {
      qDebug() << " Waiting for Cumulus addDir ...";
      sleep(1);
      addDir = jniGetAddDataDir();
    }

  // Nice trick to overwrite the HOME directory under Android ;-)
  qputenv ( "HOME", addDir.toLatin1().data() );

  // Note, that first $HOME is overwritten otherwise the setting file
  // is created in the internal data area!
  GeneralConfig::instance()->setDataRoot( addDir );

  // Gets the internal data dir from out app
  QString appDir = jniGetAppDataDir();

  while (appDir.isEmpty())
    {
      qDebug() << " Waiting for Cumulus appDir ...";
      sleep(1);
      appDir = jniGetAppDataDir();
    }

  GeneralConfig::instance()->setAppRoot( appDir );

  qDebug() << "Cumulus addDir and QtHome set to" << addDir;
  qDebug() << "Cumulus appDir set to" << appDir;

#endif /* ANDROID */

  GeneralConfig *conf = GeneralConfig::instance();

  QApplication::setGraphicsSystem( "raster" );

  QApplication app(argc, argv);

  // @AP: we installing our own message handler
  qInstallMsgHandler(messageHandler);

  // @AP: Reset the locale that is used for number formatting to "C" locale.
  setlocale(LC_NUMERIC, "C");

  QCoreApplication::setApplicationName( "Cumulus" );
  QCoreApplication::setApplicationVersion( CU_VERSION );
  QCoreApplication::setOrganizationName( "KFLog" );
  QCoreApplication::setOrganizationDomain( "www.kflog.org" );

  // Make sure the application uses utf8 encoding for translated widgets
  QTextCodec::setCodecForTr( QTextCodec::codecForName ("UTF-8") );

  // Store Build date
  conf->setBuiltDate( __DATE__ );

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

  bool isLog2File = conf->getLog2FileMode();

  QString logDir = "/tmp";

#ifdef ANDROID

  // always log on Android for now
  isLog2File = true;
  logDir = QDir::homePath();
  qDebug() << "Android LogDir=" << logDir;

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

      int i = open( logDir.toLatin1().data(), O_RDWR|O_CREAT|O_TRUNC,
                    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH );

      // Duplicate file descriptors 1, 2 that the output goes into a logfile

      // dup2( i, fileno(stdin) );
      dup2( i, fileno(stdout) );
      dup2( i, fileno(stderr) );

      close(i);
    }

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

  if( env != 0 )
    {
      unsetenv("LD_BIND_NOW");
    }

  // Load language translations for Cumulus.
  conf->setLanguage( conf->getLanguage() );

#ifdef ANDROID

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

  QDir fontDir( appDir + "/fonts" );

  if( fontDir.exists() )
    {
      QStringList fontList = fontDir.entryList ( QDir::Files|QDir::Readable );

      for( int i = 0; i < fontList.size(); i++ )
        {
          int res = QFontDatabase::addApplicationFont( fontList.at(i) );
          qDebug() << "Try to add font" << fontList.at(i) << "to data base with ID=" << res;
        }
    }

#endif

  if( conf->getGuiFont() == "" )
    {
      // No Gui font is defined, we try to define a sensefull default.
      QFont appFont;
      int   appFSize = 14;

#ifdef ANDROID
      appFont.setFamily( "Droid Sans" );
      appFSize = 6; // 14;
#else
#ifdef MAEMO
      appFont.setFamily("Nokia Sans");
      appFSize = 18;
#else
      appFont.setFamily("Sans Serif");
#endif
#endif

      // Check, what kind of font size is used by Qt.
      if( QApplication::font().pointSize() != -1 )
        {
          appFont.setPointSize( appFSize );
        }
      else
        {
          appFont.setPixelSize( appFSize );
        }

      QApplication::setFont( appFont );
    }

  if( conf->getDisclaimerVersion() != DISCLAIMER_VERSION )
    {
      QApplication::beep();

      // upon changing the text, you should also increase the value of DISCLAIMERVERSION with 1

      QString disclaimer =
          QObject::tr(
            "<html>"
            "This program comes with"
            "<p><b>ABSOLUTELY NO WARRANTY!</b></p>"
            "Do not rely on this software program as your<br>"
            "primary source of navigation. You as user are<br>"
            "responsible for using official aeronautical<br>"
            "charts and proper methods for safe navigation.<br>"
            "The information presented in this software<br>"
            "program may be outdated or incorrect.<br>"
            "<br><b>Do You accept these terms?</b>"
            "</html>");

      QMessageBox msgBox;

      int size = 6;
      QFont font = QApplication::font();

      // adapt font size to a readable one for the screen
      if( font.pointSize() != -1 )
        {
          font.setPointSize( size );
        }
      else
        {
          font.setPixelSize( size );
        }

      msgBox.setFont( font );
      msgBox.setWindowTitle( QObject::tr("Cumulus Disclaimer") );
      msgBox.setIcon ( QMessageBox::Warning );
      msgBox.setTextFormat( Qt::RichText );
      msgBox.setText( disclaimer );
      msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
      msgBox.setDefaultButton( QMessageBox::No );

#ifdef ANDROID

      QTextDocument td;
      td.setDefaultFont(font);
      td.setHtml(disclaimer);

      msgBox.setVisible(true);

      // Under Android the box must be moved into the center of the desktop screen.
      int dtw = QApplication::desktop()->availableGeometry().width();
      int dth = QApplication::desktop()->availableGeometry().height();

      QSize ts = td.size().toSize();

      // msgBox.setGeometry( 0, 0, dtw, dth );
      msgBox.move( (dtw-ts.width()) / 2 - 20, (dth-ts.height()) / 2 - 50);

#endif

      int button = msgBox.exec();

      if( button == QMessageBox::Yes )
        {
          conf->setDisclaimerVersion( DISCLAIMER_VERSION );
          conf->save();
        }
      else
        {
          qWarning("Closing application, user does not accept conditions!");
          return 0;
        }
    }

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
