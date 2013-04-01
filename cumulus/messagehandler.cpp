/***********************************************************************
 **
 **   messagehandler.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2007-2013 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>
#include <QString>

#include <cstdio>
#include <cstdlib>
#include <syslog.h>

#include "generalconfig.h"
#include "messagehandler.h"

#if QT_VERSION >= 0x050000
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void messageHandler(QtMsgType type, const char *msg)
#endif
{
  static GeneralConfig *conf = static_cast<GeneralConfig *> (0);
  static bool sysLogMode = false;
  static bool init = false;

  if (init)
    {
      // Init variable is used to avoid recursive calling during
      // initialization phase because GeneralConfig will also use the
      // log facility and this will end in an infinite loop.
      return;
    }

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

  if( ! sysLogMode ) // normal logging via stderr
    {
#if QT_VERSION >= 0x050000
#define MSG msg.toLocal8Bit().constData()
#else
#define MSG msg
#endif
      switch( type )
        {
          case QtDebugMsg:
            fprintf( stderr, "Debug: %s\n", MSG);
            break;
          case QtWarningMsg:
            fprintf( stderr, "Warning: %s\n", MSG);
            break;
          case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", MSG);
            break;
          case QtFatalMsg:
            fprintf( stderr, "Fatal: %s\n", MSG);
            abort();
            break;
          default:
            fprintf( stderr, "Default: %s\n", MSG);
            break;
        }

      return;
    }

  // Logging via syslog daemon into system log file
  switch( type )
  {
    case QtDebugMsg:
      syslog( LOG_DEBUG, "Debug: %s", MSG);
      break;
    case QtWarningMsg:
      syslog( LOG_WARNING, "Warning: %s", MSG);
      break;
    case QtCriticalMsg:
      syslog( LOG_CRIT, "Critical: %s", MSG);
      break;
    case QtFatalMsg:
      syslog( LOG_CRIT, "Fatal: %s", MSG);
      abort();
      break;
    default:
      syslog( LOG_DEBUG, "Default: %s", MSG);
      break;
  }
}
