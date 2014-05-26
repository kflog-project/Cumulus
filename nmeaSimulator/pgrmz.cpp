/***************************************************************************
                          pgrmz.cpp - description
                             -------------------
    begin                : 02.08.2010
    copyright            : (C) 2010-2013 by Axel Pauli
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

#include <iostream>
#include <cstdio>
#include <unistd.h>

#include "pgrmz.h"

using namespace std;

PGRMZ::PGRMZ()
{
}

/**
  Used by Garmin and Flarm devices
  $PGRMZ,93,f,3*21
         93,f         Altitude in feet
         3            Position fix dimensions 2 = FLARM barometric altitude
                                              3 = GPS altitude

  Flarm example: $PGRMZ,2963,F,2*04

  Input parameter altitude is expected as meters.
*/
int PGRMZ::send( float altitude, int fd )
{
  QString sentence = QString("$PGRMZ,%1,F,2*").arg( altitude*3.28095, 0, 'f', 0);

  int pos = sentence.length() - 1;
  uint sum = calcCheckSum( pos, sentence );
  QString scheck;
  scheck.sprintf ("%02X\n", sum);
  sentence += scheck;

  int sent = write( fd, sentence.toLatin1().data(), sentence.length() );

  cout << sentence.toLatin1().data();
  return sent;
}

uint PGRMZ::calcCheckSum (int pos, const QString& sentence)
{
  uint sum = 0;

  for( int i = 1; i < pos; i++ )
    {
      sum ^= uint( sentence[i].toLatin1() );
    }

  return sum;
}
