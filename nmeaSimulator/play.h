/***************************************************************************
                          play.h - description
                             -------------------
    begin                : 11.01.2012
    copyright            : (C) 2010 by Axel Pauli
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

#ifndef PLAY_H_
#define PLAY_H_

#include <QFile>
#include <QString>

class Play
{
  public:

    Play( QString& fileName, int fifo ) :
      m_pause(0),
      m_fifo(fifo),
      m_fileName(fileName)
      {
      }

    int startPlaying( const int pause=1000 );

    void setFileName( QString& newFileName )
    {
      m_fileName = newFileName;
    };

    void setFifo( const int fifo )
    {
      m_fifo = fifo;
    };

  private:

    int m_pause; // pause time in milli seconds
    int m_fifo;  // fifo descriptor

    QString m_fileName; // path to NMEA file

};

#endif /* PLAY_H_ */
