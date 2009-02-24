/***********************************************************************
 **
 **   main.cpp
 **
 **   This file is part of Cumulus. It contains the start procedure of
 **   the GUI. Currently we use the release QT/X11 4.x for the build
 **   process.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008 by Axel Pauli
 **
 **   Email of maintainer  : axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include <QApplication>
#include <QMessageBox>
#include <QTranslator>
//#include <QSplashScreen>
//#include <QPixmap>

#include "cumulusapp.h"
#include "generalconfig.h"
#include "messagehandler.h"

/////////////////////
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  // @AP: we installing our own message handler
  qInstallMsgHandler(messageHandler);

  // @AP: to make trace output available, if process is started via
  // QT/X11, we can redirect all output into a file, if config option
  // Log2File is set to true.
  GeneralConfig *conf = GeneralConfig::instance();

  // @AP: make install root of cumulus available for other modules via
  // general config. The assumption is that cumulus is installed at
  // <root>/bin/cumulus. The <root> path will be passed to general
  // config.
  char *callPath = dirname(argv[0]);
  char *startDir = getcwd(0,0);
  chdir( callPath );
  char *callDir = getcwd(0,0);
  QString root = QString(dirname(callDir));
  conf->setInstallRoot( root );
  // change back to start dir
  chdir( startDir );
  free( callDir );
  free( startDir );

  bool isLog2File = conf->getLog2FileMode();

  if( isLog2File )
    {
      int i = open( "/tmp/cumulus.log", O_RDWR|O_CREAT|O_TRUNC,
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

  /* Load selected language translations for cumulus */

  QString langFile = QString("cumulus") + QString("_") + conf->getLanguage() + ".qm";
  QString langDir = root + "/locale/" + conf->getLanguage();

  QTranslator translator;

  if( translator.load( langFile, langDir ) )
    {
      app.installTranslator(&translator);
      qDebug( "Using translation file %s for language %s",
              langFile.toLatin1().data(),
              conf->getLanguage().toLatin1().data() );
    }
  else
    {
      qDebug( "No language translation file found in %s", langDir.toLatin1().data() );
    }

  // try to load the splash screen image
  //QPixmap pixmap( GeneralConfig::instance()->loadPixmap("splash.jpg") );
  //QSplashScreen splash( pixmap, Qt::WindowStaysOnTopHint );
  //splash.show();
  //app.processEvents();

#define DISCLAIMERVERSION 1

  if( conf->getDisclaimerVersion() != DISCLAIMERVERSION )
    {
      QApplication::beep();

      if (QMessageBox::warning (NULL, QObject::tr("Cumulus Disclaimer"),
                                QObject::tr(  //upon changing the text, you should also increase the value of DISCLAIMERVERSION with 1
                                  "<html><b>"
                                  "This program comes with ABSOLUTELY NO WARRANTY!<p>"
                                  "Do not rely on this software program as your<br>"
                                  "primary source of navigation. You as user are<br>"
                                  "responsible for using official aeronautical<br>"
                                  "charts and proper methods for safe navigation.<br>"
                                  "The information presented in this software<br>"
                                  "program may be outdated or incorrect.<p>"
                                  "Do you accept these terms?"
                                  "</html></b>"
                                ),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        {
          conf->setDisclaimerVersion( DISCLAIMERVERSION );
          conf->save();
        }
      else
        {
          qWarning("Closing application, user does not accept conditions.");
          return 0;
        }
    }

  // create the cumulus application
  CumulusApp *cumulus = new CumulusApp(0, Qt::WindowContextHelpButtonHint);

  //cumulus->show();
  //splash.finish( cumulus );

  // start window manager event processing
  int result = QApplication::exec();

  // remove first CumulusApp because class objects inside can call GeneralConfig
  delete cumulus;

  // remove GeneralConfig, it is created during first call to it
  delete GeneralConfig::instance();

  return result;
}
