/***************************************************************************
                          IgcPlay.cpp - description
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

#include <cmath>
#include <unistd.h>
#include <iostream>

#include <QtCore>

#include "altitude.h"
#include "IgcPlay.h"
#include "sentence.h"
#include "speed.h"

/**
 * The earth's radius used for calculation, given in Meters
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6371000 // FAI Radius

int IgcPlay::startPlaying( const QString& startPoint,
                           const int skip,
                           const int playFactor )
{
  qDebug() << "igcPlay::startPlaying(): startPoint=" << startPoint
           << "Skip=" << skip << "Factor=" << playFactor;

  int i_skip = skip;
  m_factor = playFactor;

  QString date;

  QString time0;
  QString status0;
  QString latDeg0;
  QString lonDeg0;
  QString latHem0;
  QString lonHem0;
  QString baroAlt0;
  QString gssnAlt0;
  QString fixAcc0;
  QString satsInUse0;

  QString time1;
  QString status1;
  QString latDeg1;
  QString lonDeg1;
  QString latHem1;
  QString lonHem1;
  QString baroAlt1;
  QString gssnAlt1;
  QString fixAcc1;
  QString satsInUse1;

  if( m_fileName.isEmpty() )
    {
      qWarning() << "IgcPlay::startPlaying: No file name is defined!";
      return -1;
    }

  QFile file(m_fileName);

  if( ! file.open(QIODevice::ReadOnly) )
    {
      qWarning() << "IgcPlay::startPlaying: Cannot open file" << m_fileName;
      return -1;
    }

  QTextStream inStream(&file);

  bool firstBRecord = true;
  bool startPositionFound = false;

  while( ! inStream.atEnd() )
    {
      QString line = inStream.readLine().trimmed();

      if( line.isEmpty() )
        {
          continue;
        }

      if( line.startsWith("HFDTE"))
	{
	  // H-Record, Date, Example HFDTE270614
	  date = line.mid( 5 );
	  continue;
	}

      if( line.startsWith("B") == false )
	{
	  continue;
	}

      if( i_skip > 0 )
        {
          i_skip--;
          continue;
        }

      if( startPoint.isEmpty() == false && startPositionFound == false )
	{
	  if( line.left(1).startsWith(startPoint) == false )
	    {
	      // start point not yet reached
	      continue;
	    }

	  startPositionFound = true;
	}

      if( line.size() < 40 )
	{
	  qWarning() << "B-Record to short: " << line;
	  continue;
	}

      qDebug() << "B:" << line;

      // Only B-Records are taken into account
      //
      // 0           1          2            3
      // 0 123456 78901234 567890123 4 56789 01234 567 89
      // B 155706 5229791N 01331393E A 00000 00081 001 08
      //
      // Extract time
      time1 = line.mid( 1, 6 );

      // Extract coordinates
      latHem1 = line.mid( 14, 1 );
      lonHem1 = line.mid( 23, 1 );

      latDeg1 = line.mid(7, 4) + "." + line.mid(11, 3);
      lonDeg1 = line.mid(15, 5) + "." + line.mid(20, 3);

      status1 = line.mid( 24, 1 );

      baroAlt1 = line.mid( 25, 5 );
      gssnAlt1 = line.mid( 30, 5 );

      fixAcc1 = line.mid( 35, 3 );
      satsInUse1 = line.mid( 38, 2 );

      if( firstBRecord == true )
	{
	  // Store first extracted B-Record data and continue;
	  firstBRecord = false;

	  time0 = time1;
	  status0 = status1;
	  latDeg0 = latDeg1;
	  lonDeg0 = lonDeg1;
	  latHem0 = latHem1;
	  lonHem0 = lonHem1;
	  baroAlt0 = baroAlt1;
	  gssnAlt0 = gssnAlt1;
	  fixAcc0 = fixAcc1;
	  satsInUse0 = satsInUse1;
	  continue;
	}

      // Calculate distance between coordinate points
      double lat0, lon0, lat1, lon1;

      lat0 = latDeg0.left(2).toDouble() + latDeg0.mid(2).toDouble() / 60.;
      if( latHem0 == "S" ) lat0 = -lat0;

      lat1 = latDeg1.left(2).toDouble() + latDeg1.mid(2).toDouble() / 60.;
      if( latHem1 == "S" ) lat1 = -lat1;

      lon0 = lonDeg0.left(3).toDouble() + lonDeg0.mid(3).toDouble() / 60.;
      if( lonHem0 == "W" ) lon0 = -lon0;

      lon1 = lonDeg1.left(3).toDouble() + lonDeg1.mid(3).toDouble() / 60.;
      if( lonHem1 == "W" ) lon1 = -lon1;

      // Distance in meters
      double distBetweenPoints = distC1( lat0, lon0, lat1, lon1);

      // Calculate time difference in seconds
      QTime qtime0 = QTime::fromString( time0, "HHmmss");
      QTime qtime1 = QTime::fromString( time1, "HHmmss");

      int timeDiff = qtime0.secsTo( qtime1 );

      qDebug() << "timeDiff" << timeDiff;

      // Check time difference, if negative, make it positive
      if( timeDiff < 0 )
	{
	  qWarning() << "TimeDiff is negative!" << timeDiff;
	  timeDiff = -timeDiff;
	}

      // Take over new values
      time0 = time1;
      status0 = status1;
      latDeg0 = latDeg1;
      lonDeg0 = lonDeg1;
      latHem0 = latHem1;
      lonHem0 = lonHem1;
      baroAlt0 = baroAlt1;
      gssnAlt0 = gssnAlt1;
      fixAcc0 = fixAcc1;
      satsInUse0 = satsInUse1;

      // Check time difference, if zero something is wrong
      if( timeDiff == 0 )
	{
	  continue;
	}

      // Calculate speed in m/s
      Speed speed( distBetweenPoints / (double) timeDiff );

      // Calculate heading
      double bearing = 0.0;

      if( distBetweenPoints > 0.5 )
	{
	  bearing = getBearingWgs( lat0, lon0, lat1, lon1 );
	}

      /**
        See http://www.nmea.de/nmea0183datensaetze.html

        RMC - Recommended Minimum Navigation Information

                                                                     12
                1         2 3       4 5        6 7   8   9     10  11 | 13
                |         | |       | |        | |   |   |      |   | | |
         $--RMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a,a*hh<CR><LF>

         $GPRMC,132217.00,A,5228.19856,N,01408.32249,E,47.100,267.38,300710,,,A*58

         Field Number:
          1) UTC Time
          2) Status, A = valid position, V = Navigation receiver warning
          3) Latitude ddmm.mmm
          4) Latitude hemisphere, N or S
          5) Longitude dddmm.mm
          6) Longitude hemisphere, E or W
          7) Speed over ground, knots 0.0 ... 999.9
          8) Course over ground, degrees true
          9) UTC date of position fix, ddmmyy format
         10) Magnetic Variation, degrees
         11) Magnetic Variation direction, E or W
         12) Signal integrity, A=Autonomous mode
         13) Checksum, hh
      */

      // Prepare RMC sentence
      QString rmc = "$GPRMC," + time1 + "," + status1 + "," +
	            latDeg1 + "," + latHem1 + "," +
	            lonDeg1 + "," + lonHem1 + "," +
	            QString("%1").arg( speed.getKnots(), 0, 'f', 1 ) + "," +
	            QString("%1").arg( bearing, 0, 'f', 0 ) + "," +
	            date + ",,," + "A";

      Sentence sentence;

      sentence.send( rmc, m_fifo );

      /**
        GLL - Geographic Position - Latitude/Longitude

               1       2 3        4 5         6 7
               |       | |        | |         | |
        $--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh<CR><LF>

         Field Number:
          1) Latitude
          2) N or S (North or South)
          3) Longitude
          4) E or W (East or West)
          5) Universal Time Coordinated (UTC)
          6) Status A - Data Valid, V - Data Invalid
          7) Checksum

          $GPGLL,4916.45,N,12311.12,W,225444,A
      */

      QString ggl = "$GPGLL," +
	            latDeg1 + "," + latHem1 + "," +
	  	    lonDeg1 + "," + lonHem1 + "," +
	  	    time1 + "," +
	  	    status1;

      //sentence.send( ggl, m_fifo );

      /**
        GGA - Global Positioning System Fix Data, Time, Position and fix related data for a GPS receiver.

                1         2       3 4        5 6 7  8   9  10 11 12 13  14   15
                |         |       | |        | | |  |   |   | |   | |   |    |
         $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh<CR><LF>

         Field Number:
          1) Universal Time Coordinated (UTC) of position fix, hhmmss format
          2) Latitude, ddmm.mmmm format (leading zeros will be transmitted)
          3) Latitude hemisphere, N or S
          4) Longitude, dddmm.mmmm format (leading zeros will be transmitted)
          5) Longitude hemisphere, E or W
          6) GPS Quality Indicator,
             0 - fix not available,
             1 - GPS fix,
             2 - Differential GPS fix
             6 - Estimated
          7) Number of satellites in view, 00 - 12
          8) Horizontal Dilution of precision 0.5...99.9
          9) Antenna Altitude above/below mean-sea-level (geoid)
         10) Units of antenna altitude, meters
         11) Geoidal separation, the difference between the WGS-84 earth
             ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level
             below ellipsoid
         12) Units of geoidal separation, meters
         13) Age of differential GPS data, time in seconds since last SC104
             type 1 or 9 update, null field when DGPS is not used
         14) Differential reference station ID, 0000-1023
         15) Checksum

         $GPGGA,110959.00,5237.999230,N,01312.523391,E,1,14,1.6,81.4,M,43.0,M,,*54
      */
      QString gga = "$GPGGA," +
	            time1 + "," +
	            latDeg1 + "," + latHem1 + "," +
	            lonDeg1 + "," + lonHem1 + "," +
	            "1," +
	            satsInUse1 + "," +
	            fixAcc1 + "," +
	            gssnAlt1 + ",M,0,M,,";

      sentence.send( gga, m_fifo );

      /**
        Used by Garmin and Flarm devices
        $PGRMZ,93,f,3*21
               93,f         Altitude in feet
               3            Position fix dimensions 2 = FLARM barometric altitude
                                                    3 = GPS altitude
      */
      Altitude alt(baroAlt1.toDouble());
      QString rmz = "$PGRMZ," +
	            QString("%1").arg(alt.getFeet(), 0, 'f', 0) + ",f,2";

      sentence.send( rmz, m_fifo );

      // make a break defined by m_factor
      usleep( timeDiff * 1000000 / m_factor);
    }

  file.close();
  return 0;
}

/**
 * Distance calculation according to great circle. A mathematically equivalent formula,
 * which is less subject to rounding error for short distances.
 */
double IgcPlay::distC1( double lat1, double lon1, double lat2, double lon2 )
{
  const double rad = M_PI / 180.0;

  // See here for formula: http://williams.best.vwh.net/avform.htm#Dist
  // d = 6371km * 2 * asin( sqrt( sin((lat1-lat2)/2) ^2 + cos(lat1) * cos(lat2) * sin((lon1-lon2)/2) ^2 ) )
  double dlon = (lon1-lon2) * rad / 2;
  double dlat = (lat1-lat2) * rad / 2;

  double sinLatd = sin(dlat);
  sinLatd = sinLatd * sinLatd;

  double sinLond = sin(dlon);
  sinLond = sinLond * sinLond;

  double arc = 2 * asin( sqrt( sinLatd + cos(lat1*rad) * cos(lat2*rad) * sinLond ) );

  // distance in meters
  double dist = arc * RADIUS;

  // qDebug() << "Dist=" << dist;
  return dist;
}

/**
   Calculate the bearing from point p1 to point p2 from WGS84
   coordinates to avoid distortions caused by projection to the map.
*/
double IgcPlay::getBearingWgs( double lat1, double lon1, double lat2, double lon2 )
{
  // See here: http://www.naviuser.at/forum/showthread.php?t=1949
  const double pi_180 = M_PI / 180.0;

  // qDebug( "x1=%d y1=%d, x2=%d y2=%d",  p1.x(), p1.y(), p2.x(), p2.y() );

  double dlat = lat2 - lat1; // latitude
  double dlon = lon2 - lon1; // longitude

  // compute latitude average
  double latAv = ( ( lat2 + lat1 ) / 2.0);

  if( dlat == 0 && dlon > 0 )
    {
      return 90;
    }

  if( dlat == 0 && dlon < 0 )
    {
      return 270;
    }

  if( dlat > 0 && dlon == 0 )
    {
      return 360;
    }

  if( dlat < 0 && dlon == 0 )
    {
      return 180;
    }

  if( dlat == 0 && dlon == 0 )
    {
      return 0;
    }

  // β = arctan [cos B1 * (L2 - L1) / (B2 – B1)].......-90° < β < 90°

  // compute angle
  double angle = atan( cos(latAv * pi_180) * dlon / dlat );

  //  qDebug() << "Heading=" << angle * 180 / M_PI
  //           << "dlat=" << dlat
  //           << "dlon=" << dlon;

  // assign computed angle to the right quadrant

  if (dlat >= 0 && dlon < 0)
    {
      angle = (M_PI * 2) + angle;
    }
  else if (dlat <= 0 && dlon <= 0)
    {
      angle = M_PI + angle;
    }
  else if (dlat < 0 && dlon >= 0)
    {
      angle = M_PI / 2 - angle;
    }

  // qDebug() << "Heading=" << angle * 180 / M_PI;
  return angle * 180 / M_PI;
}
