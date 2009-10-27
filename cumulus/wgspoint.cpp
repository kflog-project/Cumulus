/***********************************************************************
**
**   WGSPoint.cpp - general position representations
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2008-2009 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * @short Abstract position
  *
  * This class is used to handle WGS-coordinates. It inherits QPoint. The only
  * difference is, that the methods to access the coordinates are called "lat"
  * and "lon". Furthermore it controls the unit to be used for position
  * representation and make conversions between the internal used KFLog format
  * and the WGS84 equivalents.
  *
  */

#include <stdlib.h>
#include <cmath>
#include <QRegExp>

#include "wgspoint.h"

// set static format variable to default (degrees, minutes, seconds)
WGSPoint::Format WGSPoint::_format = WGSPoint::DMS;


WGSPoint::WGSPoint() : QPoint()
{}

WGSPoint::WGSPoint(int lat, int lon) : QPoint(lat, lon)
{}

/**
 * Creates a new WGSPoint with the given position.
 */
WGSPoint::WGSPoint(const QPoint& pos) : QPoint(pos.x(), pos.y())
{}

WGSPoint &WGSPoint::operator=( const QPoint &p )
{
  setPos(p.x(), p.y());
  return *this;
}

/**
 * Converts the given integer KFLog coordinate into real degrees, minutes and seconds.
 */
void WGSPoint::calcPos (int coord, int& degree, int& min, int &sec)
{
  degree = coord / 600000;
  min = (coord % 600000) / 10000;
  sec = (coord % 600000) % 10000;
  sec = (int) rint((sec * 60) / 10000.0);

  // @AP: Rounding of seconds can lead to unwanted results. Therefore this is
  // checked and corrected here.
  if( sec > 59 )
    {
      sec = 59;
    }
  else if( sec < -59 )
    {
      sec = -59;
    }
}

/**
 * Converts the given integer KFLog coordinate into real degrees and minutes.
 */
void WGSPoint::calcPos (int coord, int& degree, double& min)
{
  degree = coord / 600000;
  min = (coord % 600000) / 10000.0;
  // qDebug("Coord=%d, degree=%d, decMin=%f", coord, degree, min);
}

/**
 * Converts the given integer KFLog coordinate into a real degree value.
 */
void WGSPoint::calcPos (int coord, double& degree)
{
  degree = coord / 600000.0;
  // qDebug("Coord=%d, Degree=%f", coord, degree);
}

/**
 * The function seems to have problems, if the position is near 0° W/E.
 */
QString WGSPoint::printPos(int coord, bool isLat)
{
  QString pos, posDeg, posMin, posSec;
  int degree, min, sec;
  double decDegree, decMin;

  if ( getFormat() == WGSPoint::DMS )
    {
      // default is always degrees, minutes, seconds
      calcPos (coord, degree, min, sec);

      if (isLat)
        {
          posDeg.sprintf("%02d\260 ", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d\260 ", (degree < 0)  ? -degree : degree);
        }

      min = abs(min);
      posMin.sprintf("%02d'", min);

      sec = abs(sec);
      posSec.sprintf(" %02d\"", sec);
    }
  else if ( getFormat() == WGSPoint::DDM )
    {
      // degrees and decimal minutes
      calcPos (coord, degree, decMin);

      if (isLat)
        {
          posDeg.sprintf("%02d\260 ", (degree < 0)  ? -degree : degree);
        }
      else
        {
          posDeg.sprintf("%03d\260 ", (degree < 0)  ? -degree : degree);
        }

      decMin = fabs(decMin);

      posMin.sprintf("%.3f'", decMin);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if ( decMin < 10.0 )
        {
          posMin.insert(0, "0");
        }
    }
  else if ( getFormat() == WGSPoint::DDD )
    {
      // decimal degrees
      calcPos (coord, decDegree);

      posDeg.sprintf("%.5f\260", (decDegree < 0)  ? -decDegree : decDegree);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if (isLat)
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "0");
            }
        }
      else
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "00");
            }
          else if ( decDegree < 100.0 )
            {
              posDeg.insert(0, "0");
            }
        }
    }

  pos = posDeg + posMin + posSec;

  if (isLat)
    {
      if (coord < 0)
        {
          pos += " S";
        }
      else
        {
          pos += " N";
        }
    }
  else
    {
      if (coord < 0)
        {
          pos += " W";
        }
      else
        {
          pos += " E";
        }
    }

  // qDebug( "Pos=%s", pos.toLatin1().data() );

  return pos;
}

/** Converts the degree input string into the internal KFLog format */
int WGSPoint::degreeToNum(QString inDegree)
{
  /*
   * needed formats:
   *
   *  [g]gg° mm' ss"
   *  [g]gg° mm.mmmm'
   *  [g]gg.ggggg°
   *  dddddddddd
   */

  // to prevent trouble with the degree sign coding
  QChar degreeChar = Qt::Key_degree;
  QString degreeString( degreeChar );
  QString input = inDegree;

  QRegExp degreeDMS("^[0-1]?[0-9][0-9]" + degreeString + "[ ]*[0-5][0-9]'[ ]*[0-5][0-9]\"");
  QRegExp degreeDDM("^[0-1]?[0-9][0-9]" + degreeString + "[ ]*[0-5][0-9].[0-9][0-9][0-9]'");
  QRegExp degreeDDD("^[0-1]?[0-9][0-9].[0-9][0-9][0-9][0-9][0-9]" + degreeString);
  QRegExp number("^-?[0-9]+$");

  if (number.indexIn(inDegree) != -1)
    {
      return inDegree.toInt();
    }

  int result = 0;

  if (degreeDMS.indexIn(inDegree) != -1)
    {
      int deg = 0, min = 0, sec = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg3("\"");
      sec = inDegree.mid(0, deg3.indexIn(inDegree)).toInt();

      result = (int)rint((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));
    }
  else if ( degreeDDM.indexIn(inDegree) != -1)
    {
      int deg = 0;
      double min = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
      inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

      QRegExp deg2("'");
      min = inDegree.mid(0, deg2.indexIn(inDegree)).toDouble();
      inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

      result = (int)rint((600000.0 * deg) + (10000.0 * (min)));
    }
  else if ( degreeDDD.indexIn(inDegree) != -1)
    {
      double deg = 0;

      QRegExp deg1(degreeString);
      deg = inDegree.mid(0, deg1.indexIn(inDegree)).toDouble();

      result = (int) rint( 600000.0 * deg );
    }
  else
    {
      // @AP: inform the user that something has going wrong
      qWarning("%s(%d) degreeToNum(): Wrong input format %s",
               __FILE__, __LINE__, inDegree.toLatin1().data() );

      return 0; ; // Auweia! That is a pitfall, all is set to zero on error.
    }

  QRegExp dir("[swSW]$");

  if (dir.indexIn(inDegree) >= 0)
    {
      result = -result;
    }

  // qDebug("WGSPoint::degreeToNum(%s)=%d", input.toLatin1().data(), result);

  return result;
}
