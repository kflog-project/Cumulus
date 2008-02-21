/***************************************************************************
                          altitude.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers
    email                : andre@kflog.org

    This file is part of Cumulus

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

#include "altitude.h"

// initialize static value
Altitude::altitude Altitude::_altitudeUnit=meters;

Altitude::Altitude()
{
  _isValid=false;
}


Altitude::Altitude(int meters):Distance(meters)
{}


Altitude::Altitude(double meters):Distance(meters)
{}


/** copy constructor */
Altitude::Altitude (const Altitude& alt): Distance (alt._dist)
{}


Altitude::Altitude (const Distance& dst): Distance (dst)
{}


Altitude::~Altitude()
{}


/** Get altitude as flightlevel (at standard pressure) */
double Altitude::getFL() const
{
  return Distance::getFeet()/100;
}


void Altitude::setUnit(altitude unit)
{
  _altitudeUnit=unit;
}


QString Altitude::getText(double meter, bool withUnit, int precision)
{
  QString result, unit;
  double dist;
  int defprec=1;

  switch (_altitudeUnit) {
  case meters:
    unit="m";
    dist=meter;
    defprec=0;
    break;
  case feet:
    unit="ft";
    dist=meter/mFromFeet;
    defprec=0;
    break;
  case kilometers:
    unit="Km";
    dist=meter/mFromKm;
    defprec=2;
    break;
  case miles:
    unit="M.";
    dist=meter/mFromMile;
    defprec=3;
    break;
  case nautmiles:
    unit="nM.";
    dist=meter/mFromNMile;
    defprec=3;
    break;
  case flightlevel:
    unit="FL";
    dist=meter/(mFromFeet*100.0);
    defprec=3;
    break;
  default:
    unit="m";
    dist=meter;
    defprec=0;
  }

  if (precision<0)
    precision=defprec;

  QString prec;
  prec.setNum(precision);

  if (dist<0) {
    if (withUnit) {
      result=unit;
    } else {
      result="";
    }
  } else {
    if (withUnit) {
      if( _altitudeUnit == flightlevel ) {
        QString fms = QString("%s %1.") + prec + "f";
        result.sprintf( fms.toLatin1().data(), unit.toLatin1().data(), dist );
      }
      else {
        QString fms = QString("%1.") + prec + "f %s";
        result.sprintf( fms.toLatin1().data(), dist, unit.toLatin1().data() );
      }
    } else {
      QString fms = QString("%1.") + prec + "f";
      result.sprintf( fms.toLatin1().data(), dist );
    }
  }
  return result;
}


/** Converts a distance from the current units to meters. */
double Altitude::convertToMeters(double dist)
{
  double res;

  switch (_altitudeUnit) {
  case 0: //meters:
    res=dist;
    break;
  case 1: //feet:
    res=dist*mFromFeet;
    break;
  case 2: //kilometers:
    res=dist*mFromKm;
    break;
  case 3: //miles:
    res=dist*mFromMile;
    break;
  case 4: //nautmiles:
    res=dist*mFromNMile;
    break;
  case 5: //flightlevel:
    res=dist*(mFromFeet/100.0);
  default:
    res=dist;
  }

  return res;
}


QString Altitude::getText(bool withUnit, uint precision) const
{
  QString result, unit;
  double dist;

  switch (_altitudeUnit) {
  case meters:
    unit="m";
    dist=getMeters();
    break;
  case feet:
    unit="ft";
    dist=getFeet();
    break;
  case kilometers:
    unit="Km";
    dist=getKilometers();
    break;
  case miles:
    unit="M.";
    dist=getMiles();
    break;
  case nautmiles:
    unit="nM.";
    dist=getNautMiles();
    break;
  case flightlevel:
    unit="FL";
    dist=getFL();
    break;
  default:
    unit="m";
    dist=getMeters();
  }

  QString prec;
  prec.setNum(precision);
  if (withUnit) {
    if( _altitudeUnit == flightlevel ) {
      QString fms = QString("%s %1.") + prec + "f";
      result.sprintf( fms.toLatin1().data(), unit.toLatin1().data(), dist );
    }
    else {
      QString fms = QString("%1.") + prec + "f %s";
      result.sprintf( fms.toLatin1().data(), dist, unit.toLatin1().data() );
    }
  } else {
    QString fms = QString("%1.") + prec + "f";
    result.sprintf( fms.toLatin1().data(), dist);
  }
  return result;
}


QString Altitude::getUnitText()
{
  QString unit;

  switch (_altitudeUnit) {
  case meters:
    unit="m";
    break;
  case feet:
    unit="ft";
    break;
  case kilometers:
    unit="Km";
    break;
  case miles:
    unit="M.";
    break;
  case nautmiles:
    unit="nM.";
    break;
  case flightlevel:
    unit="FL";
    break;
  default:
    unit="m";
  }

  return unit;
}
