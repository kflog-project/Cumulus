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

#include "cumulusapp.h"
#include "generalconfig.h"
#include "messagehandler.h"

/////////////////////
int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  // @AP: Note, that Opie will install a default message handler in
  // QPEApplication constructor. This handler will suppress all debug
  // and warning messages in release mode. Therefore we will install
  // our own handler now.

  qInstallMsgHandler(messageHandler);

  // @AP: to make trace output available, if process is started via
  // QPE, we can redirect all output into a file, if config option
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

  /* Load language translations for cumulus */

  env = getenv("LANG");

  if( env == 0 )
    {
      env = "";
    }

  QString langFile = QString("cumulus") + QString("_") + QString(env) + ".qm";
  QString langDir = root + "/locale/" + QString(env);

  QTranslator translator;

  if( translator.load( langFile, langDir ) )
    {
      app.installTranslator(&translator);
      qDebug( "Installing translation file %s for language %s", langFile.latin1(), env );
    }
  else
    {
      qDebug( "No language translation file found in %s", langDir.toLatin1().data() );
    }

#define DISCLAIMERVERSION 1

  if( conf->getDisclaimerVersion() < DISCLAIMERVERSION )
    {
      QApplication::beep();

      if (QMessageBox::warning (NULL, QObject::tr("Cumulus Disclaimer"),
                                QObject::tr(  //upon changing the text, you should also increase the value of DISCLAIMERVERSION with 1
                                  "This program comes with\n"
                                  "ABSOLUTELY NO WARRANTY!\n"
                                  "Do not rely on this software\n"
                                  "program as your primary source\n"
                                  "of navigation. You as user are\n"
                                  "responsible for using official\n"
                                  "aeronautical charts and proper\n"
                                  "methods for safe navigation.\n"
                                  "The information presented in this\n"
                                  "software program may be outdated\n"
                                  "or incorrect.\n\n"
                                  "Do you accept these terms?"
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

  // cumulus as local variable important to get the destructor called !
  CumulusApp cumulus(0, Qt::WindowContextHelpButtonHint);

  // start window manager event processing
  int result = QApplication::exec();

  delete GeneralConfig::instance();
  return result;
}
