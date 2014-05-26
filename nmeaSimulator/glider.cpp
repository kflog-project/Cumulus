/***************************************************************************
                          glider.cpp  -  description
                             -------------------
    begin                : 23.12.2003
    copyright            : (C) 2003      by Eckhard Völlm
                               2009-2010 by Axel Pauli

    email                : axel@kflog.org

    $Id: b560ffa90f5b1a3a68ff0071c6ff69319578b7dc $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>
#include <stdlib.h>
#include <cmath>

#include "glider.h"

using namespace std;

glider::glider(double &alat, double &alon, float &aspeed, float &aheading, float awind, float awinddir, float &aaltitude, float aclimb ): lat (alat), lon(alon), speed(aspeed), heading(aheading), altitude(aaltitude)
{
  speedKnots = speed / 1.852;
  wind = awind;
  windKnots = awind / 1.852;
  winddir = awinddir;
  climb = aclimb;
  tick = 0;
}

void glider::setFd( int fd )
{
  myFd = fd;
}

void glider::FixedPos()
{
  altitude+=climb;

  if( !(tick++%5) )  // toggle heading each 5 seconds
    {
     heading = 360.0* rand()/ (double) RAND_MAX;
    }

  myGPGGA.send( lat,lon, altitude, myFd);
  myPGRMZ.send( altitude, myFd );
  myGPRMC.send( lat,lon, 0.0, heading, myFd);
}

void glider::FixedPosGround()
{
  if( !(tick++%5) )  // toggle heading each 5 seconds
    {
     heading = 360.0* rand()/ (double) RAND_MAX;
    }

  myGPGGA.send( lat,lon, altitude , myFd);
  myPGRMZ.send( altitude, myFd );
  myGPRMC.send( lat,lon, 0.0, heading, myFd);
}

void glider::Straight()
{
  cout << heading << " " << speed << endl;

  Vector gVec( (double)heading*M_PI/180, Speed( speed*1000.0/3600.0 ) );
  Vector wVec( (double)winddir*M_PI/180, Speed( wind*1000.0/3600.0 ) );
  cout << "Wind Speed:        " << wVec.getSpeed().getKph() << " km/h" << endl;
  // cout << "winddir:           " << winddir << endl;
  int wDir = int(wVec.getAngleDeg());
  wDir += 180;

  if(wDir > 360)
    {
      wDir -= 360;
    }

  cout << "Wind Dir (From):   " << wDir << endl;
  cout << "Speed (no wind)  : " << gVec.getSpeed().getKph() << " km/h" << endl;
  cout << "Heading (no wind): " << gVec.getAngleDeg() << endl;

  gVec.add( wVec );

  cout << "Speed:             " << gVec.getSpeed().getKph() << endl;
  // cout << "Speed knots X:     " << gVec.getX().getKnots() << endl;
  // cout << "Speed knots Y:     " << gVec.getY().getKnots() << endl;
  cout << "Heading:           " << gVec.getAngleDeg() << endl;

  double yDelta = gVec.getY().getKnots()/(3600*60.0); // this is Y in vector
  double xDelta = (gVec.getX().getKnots()/(3600*60.0)) / cos(lat*M_PI/180.0);
  // cout << cos(lat*PI/180.0) << endl;
  // cout << xDelta*60*1.852*3600*cos(lat*PI/180.0)  << endl;
  // cout << yDelta*60*1.852*3600 << endl;
  double speed = gVec.getSpeed().getKnots();
  double head = gVec.getAngleDeg();
  lon += xDelta; // Länge
  lat += yDelta; // Breite
  altitude += climb;
  myGPGGA.send( lat, lon, altitude, myFd );
  myPGRMZ.send( altitude, myFd );
  myGPRMC.send( lat, lon, speed, head, myFd );
}

void glider::setCircle(float radius, QString direction )
{
  double circle=M_PI*2*radius;
  double ctime = circle/(speed*1000.0/3600.0);
  cout << "Time for circle: " << ctime << " sec" << endl;
  courseChg = 360.0/ctime;

  if( direction == "left" )
    {
      courseChg = -courseChg;
    }

  cout << "Heading chg per second: " << courseChg << " deg" << endl;
}

void glider::Circle()
{
  // Calculate wind vector
  Vector wVec( (double)winddir*M_PI/180, Speed( wind*1000/3600 ) );
  cout << "Wind Speed:        " << wVec.getSpeed().getKph() << endl;
  int wDir = int(wVec.getAngleDeg());
  wDir += 180;

  if(wDir > 360)
    {
      wDir -= 360;
    }

  cout << "Wind Dir (From):   " << wDir << endl;

  // heading uses to change during circle
  heading += courseChg;

  if( heading > 360 )
    {
      heading -= 360;
    }

  // Calculate new glider vector
  Vector gVec( (double)heading*M_PI/180, Speed( speed*1000/3600 ) );
  cout << "Speed (no wind)  : " << gVec.getSpeed().getKph() << endl;
  cout << "Heading (no wind): " << gVec.getAngleDeg() << endl;

  // add wind
  gVec.add( wVec );
  cout << "Speed:             " << gVec.getSpeed().getKph() << endl;
  cout << "Heading:           " << gVec.getAngleDeg() << endl;

  double angW = gVec.getAngleDeg();
  double yDelta = gVec.getY().getKnots()/(3600*60.0); // this is Y in vector
  double xDelta = (gVec.getX().getKnots()/(3600*60.0)) /cos(lat*M_PI/180.0);

  // cout << yDelta << endl;
  // cout << xDelta << endl;

  lon += xDelta;  // Länge
  lat += yDelta;  // Breite

  altitude+=climb;
  double speedWind = gVec.getSpeed().getKnots();
  myGPGGA.send( lat,lon, altitude, myFd);
  myPGRMZ.send( altitude, myFd );
  myGPRMC.send( lat,lon, speedWind, angW, myFd);
}

