/***********************************************************************
 **
 **   gpscona.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c): 2004-2012 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>

#include "androidevents.h"
#include "flarm.h"
#include "generalconfig.h"
#include "gpsconandroid.h"
#include "gpsnmea.h"

// static members
QByteArray GpsConAndroid::rcvBuffer;
QMutex     GpsConAndroid::mutex;



GpsConAndroid::GpsConAndroid(QObject* parent) : QObject(parent)
{
  setObjectName( "GpsConAndroid" );
}

GpsConAndroid::~GpsConAndroid()
{
}

void GpsConAndroid::rcvByte( const char byte )
{
  // Called if a byte is read from the GPS port on the java part.
  QMutexLocker locker(&mutex);

  rcvBuffer.append( byte );

  if( Flarm::getProtocolMode() == Flarm::text && byte == '\n' )
    {
      // Flarm works in text mode and the complete GPS sentence must be
      // forwarded to GpsNmea.
      QString ns( rcvBuffer.data() );
      forwardNmea( ns );
      rcvBuffer.clear();
      return;
    }

  // Flarm works in binary mode, do nothing more as to store the byte.
  // Another thread will read it.
  return;
}

static void GpsConAndroid::forwardNmea( QString& qnmea )
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

  if( verifyCheckSum( qnmea.toAscii().data() ) == false )
    {
      return;
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

  // Hand over the GPS data as event to the GUI thread.
  GpsNmeaEvent *ne = new GpsNmeaEvent(qnmea);
  QCoreApplication::postEvent( GpsNmea::gps, ne, Qt::HighEventPriority );
}

/**
 * Verify the checksum of the passed sentences.
 *
 * @returns true (success) or false (error occurred)
 */
bool GpsConAndroid::verifyCheckSum( const char *sentence )
{
  // Filter out wrong data messages read in from the GPS port. Known messages
  // do start with a dollar sign or an exclamation mark.
  if( sentence[0] != '$' && sentence[0] != '!' )
    {
      qWarning() << "GpsConAndroid::CheckSumError:" << sentence;
    }

  for( int i = strlen(sentence) - 1; i >= 0; i-- )
    {
      if( sentence[i] == '*' )
        {
          if( (strlen(sentence) - 1 - i) < 2 )
            {
              // too less characters
              return false;
            }

          char checkBytes[3];
          checkBytes[0] = sentence[i+1];
          checkBytes[1] = sentence[i+2];
          checkBytes[2] = '\0';

          bool ok = false;
          uchar checkSum = (uchar) QString( checkBytes ).toUShort( &ok, 16 );

          if( ok && checkSum == GpsNmea::calcCheckSum( sentence ) )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  return false;
}

