/***************************************************************************
                          glider.h  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003 by Eckhard Völlm
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

#include "vector.h"
#include "gpgga.h"
#include "gprmc.h"

class glider
{

public:

    glider( double &lat, double &lon, float &speed, float &heading, float wind, float winddir, float &altitude, float climb );
    ~glider();
    void setFd( int fd );
    void Straight();
    void setCircle( float radius, QString direction );
    void Circle();
    void FixedPos();
    void FixedPosGround();

private:

    int myFd;
    GPGGA myGPGGA;
    GPRMC myGPRMC;
    double &lat;
    double &lon;
    float  &speed;
    double speedKnots;
    float  &heading;
    float  wind;
    double windKnots;
    float  winddir;
    float  &altitude;
    float  climb;
    double circle;
    double ctime;
    double courseChg;
    int tick;
};

