/***********************************************************************
 **
 **   messagehandler.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2007-2022 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 ***********************************************************************/

#include <QtCore>
#include <QString>

#include <cstdio>
#include <cstdlib>

#ifndef ANDROID
#include <syslog.h>
#include "generalconfig.h"
#endif

#include "messagehandler.h"

void messageHandler(QtMsgType type, const char *msg)
{
#ifndef ANDROID
  static GeneralConfig *conf = static_cast<GeneralConfig *> (0);
#endif

  static bool sysLogMode = false;
  static bool init = false;

  if (init)
    {
      // Init variable is used to avoid recursive calling during
      // initialization phase because GeneralConfig will also use the
      // log facility and this will end in an infinite loop.
      return;
    }

#ifndef ANDROID

  if (conf == 0)
    {
      // first call, do the initialization
      init = true;
      conf = GeneralConfig::instance();
      sysLogMode = conf->getSystemLogMode();
      init = false;

      if (sysLogMode == true)
        {
          // initialize system log connection
          openlog((const char *) "Cumulus", LOG_PID, LOG_USER);
        }
    }

#endif

  if( ! sysLogMode ) // normal logging via stderr, default for Android
    {
      switch( type )
        {
          case QtDebugMsg:
            fprintf( stderr, "Debug: %s\n", msg);
            break;
          case QtWarningMsg:
            fprintf( stderr, "Warning: %s\n", msg);
            break;
          case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", msg);
            break;
          case QtFatalMsg:
            fprintf( stderr, "Fatal: %s\n", msg);
            abort();
            break;
          default:
            fprintf( stderr, "Default: %s\n", msg);
            break;
        }

      return;
    }

#ifndef ANDROID

  // Logging via syslog daemon into system log file
  switch( type )
  {
    case QtDebugMsg:
      syslog( LOG_DEBUG, "Debug: %s", msg);
      break;
    case QtWarningMsg:
      syslog( LOG_WARNING, "Warning: %s", msg);
      break;
    case QtCriticalMsg:
      syslog( LOG_CRIT, "Critical: %s", msg);
      break;
    case QtFatalMsg:
      syslog( LOG_CRIT, "Fatal: %s", msg);
      abort();
      break;
    default:
      syslog( LOG_DEBUG, "Default: %s", msg);
      break;
  }

#endif

}
