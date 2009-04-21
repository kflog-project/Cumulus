/***********************************************************************
**
**   wgspoint.h - general position representations
**
**   This file is part of Cumulus and has been extracted from mapmatrix.h
**
************************************************************************
**
**   Copyright (c):  2008-2009 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
  * @short WGS position
  *
  * This class is used to handle WGS-coordinates. It inherits QPoint. The only
  * difference is, that the methods to access the coordinates are called "lat"
  * and "lon". Furthermore it controls the unit to be used for position
  * representation.
  *
  */

#ifndef WGSPoint_H
#define WGSPoint_H

#include <QPoint>
#include <QString>

class WGSPoint : public QPoint
{
public:

    /**
     * defined position formats
     */
    enum Format {
        Unknown=-1,
        DMS=0,   // degrees, minutes, seconds
        DDM=1    // degrees, decimal minutes
    };

    /**
     * Creates a new, empty WGSPoint.
     */
    WGSPoint();

    /**
     * Creates a new WGSPoint with the given position-data.
     */
    WGSPoint(int lat, int lon);

    /**
     * Returns the latitude in the internal format.
     */
    int lat() const
    {
        return x();
    };

    /**
     * Returns the longitude in the internal format.
     */
    int lon() const
    {
        return y();
    };

    /**
     * Sets the latitude.
     */
    void setLat(int lat)
    {
        setX(lat);
    };

    /**
     * Sets the longitude.
     */
    void setLon(int lon)
    {
        setY(lon);
    };

    /**
     * Sets the position.
     */
    void setPos(int lat, int lon)
    {
        setX(lat);
        setY(lon);
    };

    /**
     *
     */
    WGSPoint &operator=( const QPoint &p );

    /**
     * Returns the current used format.
     */
    static int getFormat()
    {
        return _format;
    };

    /**
     * Sets the format to be used.
     */
    static void setFormat( Format format )
    {
        _format = format;
    };

    /**
     * Converts the given coordinate into separate values.
     */
    static void calcPos (int coord, int& degree, int& min, int &sec);

    /**
     * Converts the given coordinate into separate values.
     */
    static void calcPos (int coord, int& degree, double& min);

    /**
     * Converts the given coordinate into a readable string.
     * ( xxx,xxxx°[N,S,E,W] )
     */
    static QString printPos(int coord, bool isLat = true);

    /**
     * Converts a coordinate in string format to the internal KFLog format
     */
    static int degreeToNum(QString degree);

    /**
     * Returns a string representation of a position to be used as key
     * for checks a.s.o.
     */
    static QString coordinateString(const QPoint& position)
    {
      return QString("%1.%2").arg(position.x()).arg(position.y());
    };

protected:

    static Format _format;

};

#endif // WGSPoint_H
