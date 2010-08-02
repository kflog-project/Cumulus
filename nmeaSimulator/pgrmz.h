/***************************************************************************
                          pgrmz.h  -  description
                             -------------------
    begin                : 02.08.2010
    copyright            : (C) 2009 by Axel Pauli
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

#ifndef PGRMZ_H_
#define PGRMZ_H_

#include <QString>

class PGRMZ
{

public:

  PGRMZ();

  /** Altitude unit is expected as meters. */
  int send( float altitude, int fd );

private:

  uint calcCheckSum (int pos, const QString& sentence);
};

#endif /* PGRMZ_H_ */
