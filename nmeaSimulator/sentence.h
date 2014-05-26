/***************************************************************************
                          sentence.h - description
                             -------------------
    begin                : 17.08.2010
    copyright            : (C) 2010 by Axel Pauli
    email                : axel@kflog.org

    $Id: 22b0336a42a07ce92275cedc6ac5de538e9ec344 $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SENTENCE_H_
#define SENTENCE_H_

#include <QString>

class Sentence
{

public:

  Sentence();

  /**
   * A complete GPS sentence is expected starting after the $ with the keyword
   * and endingwithout the asterix. Starting $ sign, ending asterix and
   * checksum are added by this method before sending.
   */
  int send( QString& sentence, int fd );

private:

  uint calcCheckSum (int pos, const QString& sentence);
};

#endif /* SENTENCE_H_ */
