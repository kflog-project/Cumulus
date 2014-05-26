/***************************************************************************
                          sentence.cpp - description
                             -------------------
    begin                : 17.08.2010
    copyright            : (C) 2010-2013 by Axel Pauli
    email                : axel@kflog.org

    $Id: 443c6b2abddc8c55ef6c5de5629faf29052428e0 $

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
#include <stdio.h>
#include <unistd.h>

#include "sentence.h"

using namespace std;

Sentence::Sentence()
{
}

/**
 * A complete GPS sentence is expected starting after the $ with the keyword
 * and ending without the asterix. Starting $ sign, ending asterix and
 * checksum are added by this method before sending.
 */
int Sentence::send( QString& sentence, int fd )
{
  QString string;

  if( ! sentence.startsWith('$') && ! sentence.startsWith('!') )
    {
      string = "$";
    }

  string += sentence;

  if( ! sentence.endsWith('*') )
    {
      string += "*";
    }

  int pos = string.length() - 1;

  QString sum = QString("%1").arg(calcCheckSum( pos, string ), 2, 16,  QChar('0')).toUpper();
  string = QString("%1%2\r\n").arg(string).arg(sum);

  int sent = write( fd, string.toLatin1().data(), string.length() );

  cout << string.toLatin1().data();
  return sent;
}

uint Sentence::calcCheckSum (int pos, const QString& sentence)
{
  uint sum = 0;

  for( int i = 1; i < pos; i++ )
    {
      sum ^= uint( sentence[i].toLatin1() );
    }

  return sum;
}
