/***************************************************************************
                          gpgga.h  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003 by Eckhard Völlm
    email                : axel@kflog.org

    $Id: c1d71683b80a634f87fba6b8f07f27ade1dfcbbc $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GPGGA_H_
#define GPGGA_H_

#include <QString>

class GPGGA
{

public:

  GPGGA();
  int send( double lat, double lon, float altitude, int fd );

private:

  QString sentence;
  uint calcCheckSum (int pos, const QString& sentence);
  char * dmshh_format_lat(double degrees);
  char * dmshh_format_lon(double degrees);
};

#endif
