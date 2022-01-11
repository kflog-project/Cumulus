/***********************************************************************
 **
 **   main.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008-2022 by Axel Pauli
 **
 **   Email of maintainer: <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
 * Cumulus can be built with the Qt release 5.x.
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
#include "MainWindow.h"
#include "generalconfig.h"
#include "messagehandler.h"
#include "hwinfo.h"

/////////////////////
int main(int argc, char *argv[])
{
  // Workaround to start browser from QTextView
  qputenv( "BROWSER", "browser --url" );

  // @AP: Reset the locale that is used for number formatting to "C" locale.
  setlocale(LC_NUMERIC, "C");

  QApplication app(argc, argv, true);

  QCoreApplication::setApplicationName( "Cumulus" );
  QCoreApplication::setApplicationVersion( CU_VERSION );
  QCoreApplication::setOrganizationName( "KFLog" );
  QCoreApplication::setOrganizationDomain( "www.kflog.org" );

  qDebug() << "QT's default font is" << QApplication::font().family();

  // Note, that first $HOME must be overwritten under Android otherwise the
  // setting file is created/searched in the internal data area under:
  // /data/data/org.kflog.cumulus/files. That is the $HOME, set by Necessitas.
  GeneralConfig *conf = GeneralConfig::instance();

  // @AP: we installing our own message handler
  qInstallMessageHandler(messageHandler);

  // @AP: make install root of Cumulus available for other modules via
  // GeneralConfig. The assumption is that Cumulus is installed at
  // <root>/bin/cumulus. The <root> path will be passed to GeneralConfig.
  QDir rootDir( QFileInfo(argv[0]).canonicalPath() );

  if( rootDir.cdUp() == false )
    {
      qWarning() << "main: Cumulus App has no parent directory! InstallDir is" << rootDir;
    }

  QString rootPath = rootDir.canonicalPath();
  conf->setAppRoot( rootPath );

  // @AP: to make trace output available, if process is started via
  // QT/X11, we can redirect all output into a file, if configuration option
  // Log2File is set to true.
  bool isLog2File = conf->getLog2FileMode();

  QString logDir = "/tmp";

  if( isLog2File )
    {
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
  conf->setLanguage( conf->getLanguage() );

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
