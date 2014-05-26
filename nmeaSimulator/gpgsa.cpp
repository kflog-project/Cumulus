/***************************************************************************
                          gpgsa.cpp  -  description
                             -------------------
    begin                : 24.10.2009
    copyright            : (C) 2009-2013 by Axel Pauli
    email                : axel@kflog.org

    $Id: 0d1799326f6b968b88765ce0c63efc6aa476cfea $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <unistd.h>
#include <iostream>

#include "gpgsa.h"

using namespace std;

GPGSA::GPGSA()
{
}

/**
GSA - GPS DOP and active satellites

        1 2 3                    14 15  16  17  18
        | | |                    |  |   |   |   |
 $--GSA,a,a,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x.x,x.x*hh<CR><LF>

 Field Number:
  1) Selection mode
  2) Mode
  3) ID of 1st satellite used for fix
  4) ID of 2nd satellite used for fix
  ...
  14) ID of 12th satellite used for fix
  15) PDOP in meters
  16) HDOP in meters
  17) VDOP in meters
  18) checksum
 */

// $GPGSA,A,3,14,32,17,20,11,23,28,,,,,,1.7,1.1,1.2*3C

int GPGSA::send( QStringList& satIds, QString& pdop, QString& hdop, QString &vdop, int fd )
{
  QString sentence = "$GPGSA,A,3";

  for( int i = 0; i < satIds.size(); i++ )
    {
      sentence += "," + satIds[i];
    }

  if( satIds.size() < 12 )
    {
      for( int i = satIds.size(); i < 12; i++ )
        {
          sentence += ",";
        }
    }

  sentence += "," + pdop + "," + hdop + "," + vdop + "*";

  int pos = sentence.length()-1;
  uint sum = calcCheckSum( pos, sentence );
  QString scheck;
  scheck.sprintf ("%02X\n", sum);
  sentence += scheck;

  int sent = write( fd, sentence.toLatin1().data(), sentence.length() );

  cout << sentence.toLatin1().data();
  return sent;
}

uint GPGSA::calcCheckSum (int pos, const QString& sentence)
{
  uint sum = 0;

  for (int i = 1; i < pos; i++)
    {
      sum ^= uint(sentence[i].toLatin1());
    }

  return sum;
}
