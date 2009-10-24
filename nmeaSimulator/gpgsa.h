/***************************************************************************
                          gpgsa.h  -  description
                             -------------------
    begin                : 24.10.2009
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

#include <QString>
#include <QStringList>

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

class GPGSA
{

public:

  GPGSA();
  ~GPGSA();

  int send( QStringList& satIds, QString& pdop, QString& hdop, QString &vdop, int fd );

private:

  QString sentence;
  uint calcCheckSum (int pos, const QString& sentence);
};

