/***************************************************************************
                          gpgga.cpp  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003 by Eckhard Völlm, 2009-2013 by Axel Pauli
    email                : kflog.cumulus@gmail.com

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gpgga.h"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <QDateTime>

using namespace std;

GPGGA::GPGGA()
{
}

char * GPGGA::dmshh_format_lat(double degrees)
{
  static char buf[16];
  int deg_part;
  double min_part;

  if( degrees < 0 )
    {
      degrees = -degrees;
    }

  deg_part = (int) degrees;
  min_part = 60.0 * (degrees - deg_part);

  sprintf( buf, "%02d%07.4f", deg_part, min_part );

  return buf;
}

char * GPGGA::dmshh_format_lon(double degrees)
{
  static char buf[16];
  int deg_part;
  double min_part;

  if( degrees < 0 )
    {
      degrees = -degrees;
    }

  deg_part = (int) degrees;
  min_part = 60.0 * (degrees - deg_part);

  sprintf( buf, "%03d%07.4f", deg_part, min_part );

  return buf;
}

uint GPGGA::calcCheckSum (int pos, const QString& sentence)
{
  uint sum = 0;

  for( int i = 1; i < pos; i++ )
    {
      sum ^= uint( sentence[i].toLatin1() );
    }

  return sum;
}

// Example of GPGGA:
// $GPGGA,223031.803,5228.1139,N,01334.0933,E,1,10,00.8,35.3,M,39.8,M,,*53
int GPGGA::send( double lat, double lon, float altitude, int fd )
{
  QDateTime dateTimeUtc = QDateTime::currentDateTime().toUTC();

  QString utcTime = dateTimeUtc.time().toString("hhmmss.zzz");

  sentence = "$GPGGA," + utcTime +",";

  QString lati;
  lati.sprintf( "%s,%c,", dmshh_format_lat( lat ), lat > 0 ? 'N' : 'S' );
  sentence += lati;
  QString longi;
  longi.sprintf( "%s,%c,", dmshh_format_lon( lon ), lon > 0 ? 'E' : 'W' );
  sentence += longi;
  sentence += "1,"; // valid
  sentence += "08,"; // Sats used
  sentence += "2.1,"; // HDOP
  QString alti;
  alti.sprintf( "%.1f,", altitude );
  sentence += alti;
  sentence += "M,,,,"; // unit in Meters, and some stuff
  sentence += "0000,*"; // station ID

  int pos = sentence.length() - 1;
  uint sum = calcCheckSum( pos, sentence );
  QString scheck;
  scheck.sprintf( "%02X\n", sum );
  sentence += scheck;

  int sent = write( fd, sentence.toLatin1().data(), (int) sentence.length() );

  cout << sentence.toLatin1().data();
  return sent;
}

