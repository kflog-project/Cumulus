/***************************************************************************
                          play.h - description
                             -------------------
    begin                : 11.01.2012

    copyright            : (C) 2012 by Axel Pauli

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

/**
 * \class Play
 *
 * \author Axel Pauli
 *
 * \brief A class to play GPS data from a file.
 *
 * This class reads GPS sentence data from a file and writes it into
 * a fifo. After every written $GPRMC sentence a pause is made. The read
 * data file must contain such $GPRMC sentences!
 *
 * \date 2012
 *
 * \version $Id$
 *
*/

class Play
{
  public:

    /**
     * Constructor of class.
     *
     * \param fileName Path to file to be read in
     *
     * \fifo Fifo file descriptor, used to write the GPS data out.
     */
    Play( QString& fileName, int fifo ) :
      m_pause(0),
      m_fifo(fifo),
      m_fileName(fileName)
      {
      }

    /**
     * Starts the playing of the GPS data.
     *
     * \param skip Lines to be skipped in the file to be played.
     *
     * \param pause Pause after each $GPRMC sentence in ms. Default is 1000ms.
     *
     */
    int startPlaying( const int skip=0, const int pause=1000 );

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
