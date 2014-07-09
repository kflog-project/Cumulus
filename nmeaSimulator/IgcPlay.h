/***************************************************************************
                          IgcPlay.h - description
                             -------------------
    begin                : 05.07.2014

    copyright            : (C) 2014 by Axel Pauli

    email                : kflog.cumulus@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IGC_PLAY_H_
#define IGC_PLAY_H_

#include <QString>

/**
 * \class IgcPlay
 *
 * \author Axel Pauli
 *
 * \brief A class to play IGC data from a recorded file.
 *
 * This class reads IGC sentence data from a file, convert it to GPS NMEA and
 * writes it into a fifo. The time difference between to recorded IGC records
 * is automatically waited before the next record is processed.
 *
 * \date 2014
 *
 * \version $Id$
 *
*/

class IgcPlay
{
  public:

    /**
     * Constructor of class.
     *
     * \param fileName Path to file to be read in
     *
     * \fifo Fifo file descriptor, used to write the GPS data out.
     */
    IgcPlay( QString& fileName, int fifo ) :
      m_factor(1),
      m_fifo(fifo),
      m_fileName(fileName)
      {
      };

    /**
     * Starts the playing of the IGC data.
     *
     * \param startPoint If start point is valid, all records laying before
     *                   this time are skipped.
     *
     * \param skip Lines to be skipped in the file to be played.
     *
     * \param playFactor Factor which is applied to playing time.
     *
     */
    int startPlaying( const QString& startPoint,
	              const int skip=0,
                      const int playFactor=1 );

    void setFileName( QString& newFileName )
    {
      m_fileName = newFileName;
    };

    void setFifo( const int fifo )
    {
      m_fifo = fifo;
    };

    double distC1( double lat1, double lon1, double lat2, double lon2 );

    /**
     * Calculates the bearing to the next point with wgs84 coordinates
     */
    double getBearingWgs( double lat1, double lon1, double lat2, double lon2 );

  private:

    int m_factor; // play factor which is applied to playing time. Normally 1.
    int m_fifo;  // fifo descriptor

    QString m_fileName; // path to IGC file
};

#endif
