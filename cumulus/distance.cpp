/***************************************************************************
                          distance.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers
                         :     2007-2010 by Axel Pauli
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

#include <cmath>

#include "distance.h"

//initializer for static member variable
Distance::distanceUnit Distance::_distanceUnit=kilometers;

const double Distance::mFromKm=1000.0;     // 1000.0 meters in 1 km.
const double Distance::mFromMile=1609.344; // 1609.344 meters in a statute mile
const double Distance::mFromNMile=1852.0;  // 1852 meters in a nautical mile
const double Distance::mFromFeet=0.3048;   // a foot is a bit more than 30 cm


Distance::Distance() : _dist(0), _isValid(false)
{
}


Distance::Distance(int meters) : _dist((double)meters), _isValid(true)
{
}


Distance::Distance(double meters) : _dist(meters), _isValid(true)
{
}

Distance::Distance(const Distance& dst) :
  _dist(dst._dist),
  _isValid(dst._isValid)
{
}

Distance::~Distance()
{}

/** implements == operator for distance */
bool Distance::operator == (const Distance& x) const
{
  return (_dist == x._dist && _isValid && x._isValid);
}


/** implements != operator for distance */
bool Distance::operator != (const Distance& x) const
{
  return(_dist != x._dist);
}


/** implements minus operator */
Distance Distance::operator - (const Distance& op) const
{
  return Distance (_dist - op._dist);
}


/** implements divide operator */
double Distance::operator / (const Distance& op) const
{
  return _dist / op._dist;
}

/**
 * @returns a string for the currently set distance unit.
 */
QString Distance::getUnitText()
{
  QString unit;

  switch( _distanceUnit )
    {
    case meters:
      unit = "m";
      break;
    case feet:
      unit = "ft";
      break;
    case kilometers:
      unit = "km";
      break;
    case miles:
      unit = "SM";
      break;
    case nautmiles:
      unit = "NM";
      break;
    default:
      unit = "m";
      break;
    }

  return unit;
}

QString Distance::getText( bool withUnit, uint precision, uint chopOrder ) const
{
  QString result;
  double dist;

  switch( _distanceUnit )
    {
    case meters:
      dist = getMeters();
      break;
    case feet:
      dist = getFeet();
      break;
    case kilometers:
      dist = getKilometers();
      break;
    case miles:
      dist = getMiles();
      break;
    case nautmiles:
      dist = getNautMiles();
      break;
    default:
      dist = getMeters();
      break;
    }

  // see if we need to lower the precision
  if( chopOrder > 0 )
    {
      while( precision > 0 && pow( 10, chopOrder ) <= dist )
        {
          precision--;
          chopOrder++;
        }
    }

  if( withUnit )
    {
      result = QString("%1 %2").arg( dist, 0, 'f', precision )
                               .arg( getUnitText() );
    }
  else
    {
      result = QString("%1").arg( dist, 0, 'f', precision );
    }

  return result;
}

QString Distance::getText(double meters, bool withUnit, int precision)
{
  QString result;
  double dist;
  int defprec;

  switch( _distanceUnit )
    {
    case 0: // meters:
      dist = meters;
      defprec = 0;
      break;
    case 1: // feet:
      dist = meters / mFromFeet;
      defprec = 0;
      break;
    case 2: // kilometers:
      dist = meters / mFromKm;
      defprec = ( dist < 1.0 ) ? 2 : 1;
      break;
    case 3: // statute miles:
      dist = meters / mFromMile;
      defprec = ( dist < 1.0 ) ? 3 : 2;
      break;
    case 4: // nautical miles:
      dist = meters / mFromNMile;
      defprec = ( dist < 1.0 ) ? 3 : 2;
      break;
    default:
      dist = meters;
      defprec = 0;
      break;
    }

  if( precision < 0 )
    {
      precision = defprec;
    }

  if( dist < 0 )
    {
      if( withUnit )
        {
          result = getUnitText();
        }
      else
        {
          result = "";
        }
    }
  else
    {
      if( withUnit )
        {
          result = QString("%1 %2").arg( dist, 0, 'f', precision )
                                   .arg( getUnitText() );
        }
      else
        {
          result = QString("%1").arg( dist, 0, 'f', precision );
        }
    }

  return result;
}

/** Converts a distance from the current units to meters. */
double Distance::convertToMeters(double dist)
{
  double res;

  switch( _distanceUnit )
    {
      case 0: // meters:
        res = dist;
        break;
      case 1: // feet:
        res = dist * mFromFeet;
        break;
      case 2: // kilometers:
        res = dist * mFromKm;
        break;
      case 3: // statute miles:
        res = dist * mFromMile;
        break;
      case 4: // nautical miles:
        res = dist * mFromNMile;
        break;
      default:
        res = dist;
        break;
    }

  return res;
}
