/***********************************************************************
**
**   WGSPoint.cpp - general position representations
**
**   This file is part of Cumulus and has been extracted from mapmatrix.cpp
**
************************************************************************
**
**   Copyright (c):  2008 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
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
  * representation.
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

WGSPoint::WGSPoint(int lat, int lon)
        : QPoint(lat, lon)
{}

WGSPoint &WGSPoint::operator=( const QPoint &p )
{
    setPos(p.x(), p.y());
    return *this;
}

/**
  * Converts the given coordinate into separate values.
  */
void WGSPoint::calcPos (int coord, int& degree, int& min, int &sec)
{
    degree = coord / 600000;
    min = (coord % 600000) / 10000;
    sec = (coord % 600000) % 10000;
    sec = (int) rint((sec * 60) / 10000.0);
}

void WGSPoint::calcPos (int coord, int& degree, double& min)
{
    degree = coord / 600000;
    min = (coord % 600000) / 10000.0;
    // qDebug("Coord=%d, degree=%d, decMin=%f", coord, degree, min);
}

/**
 * The function seems to have problems, if the position is near 0° W/E.
 */
QString WGSPoint::printPos(int coord, bool isLat)
{
    QString pos, posDeg, posMin, posSec;
    int degree, min, sec;
    double decMin;

    if( getFormat() != WGSPoint::DDM ) {
        // default is always degrees, minutes, seconds
        calcPos (coord, degree, min, sec);

        // qDebug("coord=%d, degree=%d, min=%d, sec=%d",
        //     coord, degree, min, sec );

        min = abs(min);
        posMin.sprintf("%02d'", min);

        sec = abs(sec);
        posSec.sprintf(" %02d\"", sec);
    } else {
        // degrees and decimal minutes
        calcPos (coord, degree, decMin);

        decMin = fabs(decMin);

        posMin.sprintf("%.3f'", decMin);

        // Unfortunately sprintf does not support leading zero in float
        // formating. So we must do it alone.
        if( decMin < 10.0 ) {
            posMin.insert(0, "0");
        }
    }

    if(isLat) {
        if(coord < 0) {
            posDeg.sprintf("%02d\260 ", -degree);
            pos = posDeg + posMin + posSec + " S";
        } else {
            posDeg.sprintf("%02d\260 ", degree);
            pos = posDeg + posMin + posSec + " N";
        }
    } else {
        if(coord < 0) {
            posDeg.sprintf("%03d\260 ", -degree);
            pos = posDeg + posMin + posSec + " W";
        } else {
            posDeg.sprintf("%03d\260 ", degree);
            pos = posDeg + posMin + posSec + " E";
        }
    }

    return pos;
}


int WGSPoint::degreeToNum(QString inDegree)
{
    /*
     * needed formats:
     *
     *  [g]gg° mm' ss"
     *  [g]gg° mm.mmmm'
     *  dddddddddd
     */

    QString input = inDegree;

    QRegExp degreeDMS("^[0-1]?[0-9][0-9]\260[ ]*[0-5][0-9]'[ ]*[0-5][0-9]\"");
    QRegExp degreeDMM("^[0-1]?[0-9][0-9]\260[ ]*[0-5][0-9].[0-9][0-9][0-9]'");
    QRegExp number("^-?[0-9]+$");

    if(number.indexIn(inDegree) != -1) {
        return inDegree.toInt();
    } else if(degreeDMS.indexIn(inDegree) != -1) {
        int deg = 0, min = 0, sec = 0, result = 0;

        QRegExp deg1("\260");
        deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
        inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

        QRegExp deg2("'");
        min = inDegree.mid(0, deg2.indexIn(inDegree)).toInt();
        inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

        QRegExp deg3("\"");
        sec = inDegree.mid(0, deg3.indexIn(inDegree)).toInt();

        result = (int)rint((600000.0 * deg) + (10000.0 * (min + (sec / 60.0))));

        // We add 1 to avoid rounding-errors and to make it possible to use the
        // zero value with a minus sign!
        // result += 1;

        QRegExp dir("[swSW]$");
        if(dir.indexIn(inDegree) >= 0) {
            // qDebug("WGSPoint::degreeToNum(%s)=%d", input.latin1(), -result);
            return -result;
        }

        // qDebug("WGSPoint::degreeToNum(%s)=%d", input.latin1(), result);
        return result;
    } else if( degreeDMM.indexIn(inDegree) != -1) {
        int deg = 0, result = 0;
        double min = 0;

        QRegExp deg1("\260");
        deg = inDegree.mid(0, deg1.indexIn(inDegree)).toInt();
        inDegree = inDegree.mid(deg1.indexIn(inDegree) + 1, inDegree.length());

        QRegExp deg2("'");
        min = inDegree.mid(0, deg2.indexIn(inDegree)).toDouble();
        inDegree = inDegree.mid(deg2.indexIn(inDegree) + 1, inDegree.length());

        result = (int)rint((600000.0 * deg) + (10000.0 * (min)));

        // We add 1 to avoid rounding-errors and to make it possible to use the
        // zero value with a minus sign!
        // result += 1;

        QRegExp dir("[swSW]$");

        if(dir.indexIn(inDegree) >= 0) {
            // qDebug("WGSPoint::degreeToNum(%s)=%d", input.latin1(), -result);
            return -result;
        }

        // qDebug("WGSPoint::degreeToNum(%s)=%d", input.latin1(), result);
        return result;
    }

    // @AP: inform the user that something has going wrong
    qWarning("%s(%d) degreeToNum(): Wrong input format %s",
             __FILE__, __LINE__, inDegree.toLatin1().data() );

    return 0; // that is the pitfall, all is set to zero on error
}
