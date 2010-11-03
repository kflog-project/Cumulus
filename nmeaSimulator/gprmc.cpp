/***************************************************************************
                          gprmc.cpp  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003 by Eckhard Völlm, by 2009 Axel Pauli
    email                : axel@kflog.org

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

#include "gprmc.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <QDateTime>

using namespace std;

GPRMC::GPRMC()
{
}

char* GPRMC::dmshh_format_lat(double degrees)
{
  static char buf[16];
  int deg_part;
  double min_part;

  if( degrees < 0 )
    {
      degrees = -degrees;
    }

  deg_part = (int) rint(degrees);
  min_part = 60.0 * (degrees - deg_part);

  sprintf( buf, "%02d%07.4f", deg_part, min_part );

  return buf;
}

char* GPRMC::dmshh_format_lon(double degrees)
{
  static char buf[16];
  int deg_part;
  double min_part;

  if( degrees < 0 )
    {
      degrees = -degrees;
    }

  deg_part = (int) rint(degrees);
  min_part = 60.0 * (degrees - deg_part);

  sprintf( buf, "%03d%07.4f", deg_part, min_part );

  return buf;
}

uint GPRMC::calcCheckSum (int pos, const QString& sentence)
{
  uint sum = 0;

  for( int i = 1; i < pos; i++ )
    {
      sum ^= uint( sentence[i].toAscii() );
    }

  return sum;
}

// Example of GPRMC:
// $GPRMC,223030.803,A,5228.1139,N,01334.0933,E,0.00,329.74,251009,,,A*6A
int GPRMC::send( double lat, double lon, float speed, float course, int fd )
{
  QDateTime dateTimeUtc = QDateTime::currentDateTime().toUTC();

  QString utcDate = dateTimeUtc.date().toString("ddMMyy");
  QString utcTime = dateTimeUtc.time().toString("hhmmss.zzz");

  sentence = "$GPRMC," + utcTime + ",A,";
  QString lati;
  lati.sprintf( "%s,%c,", dmshh_format_lat( lat ), lat > 0 ? 'N' : 'S' );
  sentence += lati;
  QString longi;
  longi.sprintf( "%s,%c,", dmshh_format_lon( lon ), lon > 0 ? 'E' : 'W' );
  sentence += longi;

  QString sp;
  sp.sprintf( "%0.3f,", speed );
  sentence += sp;

  QString co;
  co.sprintf( "%0.2f,", course );
  sentence += co;

  sentence += utcDate;

  sentence += ",,,A*";
  int pos = sentence.length() - 1;
  uint sum = calcCheckSum( pos, sentence );
  QString scheck;
  scheck.sprintf( "%02X\n", sum );
  sentence += scheck;

  int sent = write( fd, sentence.toAscii().data(), (int) sentence.length() );

  cout << sentence.toAscii().data();
  return sent;
}

