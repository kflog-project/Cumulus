/***********************************************************************
 **
 **   main.cpp
 **
 **   This file is part of Cumulus. It contains the start procedure of
 **   the GUI. Currently we use the release QT/X11 4.6.x for the build
 **   process.
 **
 ************************************************************************
 **
 **   Copyright (c):  2008 by Axel Pauli
 **
 **   Email of maintainer: axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
#include <QDir>

#include "mainwindow.h"
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

  // @AP: make install root of Cumulus available for other modules via
  // general config. The assumption is that Cumulus is installed at
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

  QString logDir = "/tmp";

  if( isLog2File )
    {

#ifdef MAEMO
      // check for alternate paths under Maemo on MMC
      QDir path1("/media/mmc1"); // N8x0
      QDir path2("/media/mmc2"); // N8x0
      QDir path3("/media/mmc");  // N900

      if( path1.exists() )
        {
          logDir = path1.absolutePath();
        }
      else if( path2.exists() )
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

  /* Load selected language translations for Cumulus */

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

#define DISCLAIMERVERSION 1

  if( conf->getDisclaimerVersion() != DISCLAIMERVERSION )
    {
      QApplication::beep();

      QString disclaimer =
          QObject::tr(  //upon changing the text, you should also increase the value of DISCLAIMERVERSION with 1
            "<html>"
            "This program comes with"
            "<p><b>ABSOLUTELY NO WARRANTY!</b></p>"
            "Do not rely on this software program as your<br>"
            "primary source of navigation. You as user are<br>"
            "responsible for using official aeronautical<br>"
            "charts and proper methods for safe navigation.<br>"
            "The information presented in this software<br>"
            "program may be outdated or incorrect.<p>"
            "</html>");

      QString question =
          QObject::tr( "<b>Do You accept these terms?</b>" );

      QMessageBox msgBox;

      QFont font = msgBox.font();

      if( font.pixelSize() < 16 )
        {
          // adapt font size to a readable one
          font.setPixelSize( 16 );
          msgBox.setFont( font );
        }

      msgBox.setWindowTitle( QObject::tr("Cumulus Disclaimer") );
      msgBox.setIcon ( QMessageBox::Warning );
      msgBox.setText( disclaimer );
      msgBox.setInformativeText( question );
      msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
      msgBox.setDefaultButton( QMessageBox::Yes );

      int button = msgBox.exec();

      if( button == QMessageBox::Yes )
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

  // creates the Cumulus application
  MainWindow *cumulus = new MainWindow( Qt::WindowContextHelpButtonHint );

  // start window manager event processing loop
  int result = QApplication::exec();

  // remove first MainWindow because class objects inside can call GeneralConfig
  delete cumulus;

  // remove GeneralConfig, it is created during first call to it
  delete GeneralConfig::instance();

  return result;
}
