/***********************************************************************
**
**   wgspoint.h - general position representations
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
  * @short WGS position
  *
  * This class is used to handle WGS-coordinates. It inherits QPoint. The only
  * difference is, that the methods to access the coordinates are called "lat"
  * and "lon". Furthermore it controls the unit to be used for position
  * representation.
  *
  */

#ifndef WGS_Point_H
#define WGS_Point_H

#include <QPoint>
#include <QString>

class WGSPoint : public QPoint
{
public:

    /**
     * defined position formats
     */
    enum Format
    {
        Unknown=-1,
        DMS=0, // degrees, minutes, seconds
        DDM=1, // degrees, decimal minutes
        DDD=2  // decimal degrees
    };

    /**
     * Creates an empty WGSPoint.
     */
    WGSPoint();

    /**
     * Creates a WGSPoint with the given coordinates. Input coordinates are expected
     * in KFLog format.
     */
    WGSPoint(int lat, int lon);

    /**
     * Creates a WGSPoint with the given position. Input coordinates are expected
     * in KFLog format.
     */
    WGSPoint(const QPoint& pos);

    /**
     * Returns the latitude in the internal KFLog format.
     */
    int lat() const
    {
        return x();
    };

    /**
     * Returns the longitude in the internal KFLog format.
     */
    int lon() const
    {
        return y();
    };

    /**
     * Sets the latitude. Input latitude is expected in KFLog format.
     */
    void setLat(int lat)
    {
        setX(lat);
    };

    /**
     * Sets the longitude. Input longitude is expected in KFLog format.
     */
    void setLon(int lon)
    {
        setY(lon);
    };

    /**
     * Sets the position. Input coordinates are expected in KFLog format.
     */
    void setPos(int lat, int lon)
    {
        setX(lat);
        setY(lon);
    };

    /**
     * Sets the position. Input coordinates are expected in KFLog format.
     */
    void setPos( const QPoint& pos )
    {
        setX(pos.x());
        setY(pos.y());
    };

    /**
     *
     */
    WGSPoint &operator=( const QPoint &p );

    /**
     * Returns the current used coordinate format.
     */
    static int getFormat()
    {
        return _format;
    };

    /**
     * Sets the coordinate format to be used.
     */
    static void setFormat( Format format )
    {
        _format = format;
    };

    /**
     * Converts the given KFLog coordinate into separate WGS84 values.
     */
    static void calcPos (int coord, int& degree, int& min, int &sec);

    /**
     * Converts the given KFLog coordinate into separate WGS84 values.
     */
    static void calcPos (int coord, int& degree, double& min);

    /**
     * Converts the given KFLog coordinate into separate WGS84 values.
     */
    static void calcPos (int coord, double& degree);

    /**
     * Converts the given KFLOG coordinate into a readable WGS84 string.
     * ( xxx,xxxx°[N,S,E,W] )
     */
    static QString printPos(int coord, bool isLat = true);

    /**
     * Converts a WGS84 coordinate string into the internal KFLog format.
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

#endif // WGS_Point_H
