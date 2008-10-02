/***************************************************************************
                          speed.cpp  -  description
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by Andre Somers, 2007 Axel Pauli
    email                : axel@kflog.org

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

#include <cmath>
#include <QByteArray>

#include "speed.h"

// initialize static values
Speed::speedUnit Speed::_horizontalUnit=kilometersPerHour;
Speed::speedUnit Speed::_verticalUnit=metersPerSecond;

Speed::Speed()
{
    _speed=0;
    _isValid=false;
    changed();
}


Speed::Speed(double Mps)
{
    _speed=Mps;
    _isValid=true;
    changed();
}


Speed::~Speed()
{}


void Speed::setVerticalUnit(speedUnit unit)
{
    _verticalUnit=unit;
}


void Speed::setHorizontalUnit(speedUnit unit)
{
    _horizontalUnit=unit;
}


Speed::speedUnit Speed::getVerticalUnit()
{
    return _verticalUnit;
}


Speed::speedUnit Speed::getHorizontalUnit()
{
    return _horizontalUnit;
}


/** Set speed in meters per second. */
void Speed::setMps(double speed)
{
    _speed=speed;
    _isValid=true;
    changed();
}


/** Set speed in Kilometers per hour */
void Speed::setKph(double speed)
{
    _speed=speed/toKph;
    _isValid=true;
    changed();
}


/** Set speed in knots */
void Speed::setKnot(double speed)
{
    _speed=speed/toKnot;
    _isValid=true;
    changed();
}


/** Set speed in Nautical miles per hour */
void Speed::setMph(double speed)
{
    _speed=speed/toMph;
    _isValid=true;
    changed();
}


/** Set speed in feet per minute */
void Speed::setFpm(double speed)
{
    _speed=speed/toFpm;
    _isValid=true;
    changed();
}


/** Get speed in Meters per Second */
double Speed::getMps() const
{
    return (_speed);
}


/** Get speed in Kilometers per hour */
double Speed::getKph() const
{
    return (_speed*toKph);
}


/** Get speed in Knots. */
double Speed::getKnots() const
{
    return (_speed*toKnot);
}


/** Get speed in Nautical Miles per hour */
double Speed::getMph() const
{
    return (_speed*toMph);
}


/** Get speed in feet per minute */
double Speed::getFpm() const
{
    return (_speed*toFpm);
}


/** + operator for speed. */
Speed Speed::operator + (const Speed& x) const
{
    return Speed(x._speed + _speed);
}


/** - operator for speed. */
Speed Speed::operator - (const Speed& x) const
{
    return Speed(_speed - x._speed);
}


/** - operator for speed. */
Speed operator - (double left, const Speed& right)
{
    return Speed(left - right.getMps());
}


/** + operator for speed. */
Speed operator + (double left, const Speed& right)
{
    return Speed(left + right.getMps());
}


/** * operator for speed. */
Speed operator * (double left, const Speed& right)
{
    return Speed(left * right.getMps());
}


/** / operator for speed. */
double Speed::operator / (const Speed& x) const
{
    return _speed / x._speed;
}


/** * operator for speed. */
double Speed::operator * (const Speed& x) const
{
    return _speed * x._speed;
}


/** == operator for Speed */
bool Speed::operator == (const Speed& x) const
{
    return (x._speed==_speed);
}


/** != operator for Speed */
bool Speed::operator != (const Speed& x) const
{
    return (x._speed !=_speed);
}


/** - prefix operator for speed */
Speed Speed::operator - () const
{
    return Speed (-_speed);
}


/** Returns a formatted string for the default horizontal unit-setting. */
QString Speed::getHorizontalText(bool withUnit, uint precision) const
{
    QString result, unit;
    double speed;
    unit = getUnitText(_horizontalUnit);
    speed = getHorizontalValue();

    QByteArray prec;
    prec.setNum(precision);
    if (withUnit) {
      result.sprintf("%1." + prec + "f %s",speed,unit.toLatin1().data());
    } else {
        result.sprintf("%1." + prec + "f",speed);
    }
    return result;

}


/** Returns a formatted string for the default vertical speed units. */
QString Speed::getVerticalText(bool withUnit, uint precision) const
{
    QString result, unit;
    double speed;

    unit = getUnitText(_verticalUnit);
    speed = getVerticalValue();

    //@JD: If unit is feet/minute set precision to 0. Saves display space
    //     and should really be sufficient (1 foot/min is 0.005 m/s ...)
    if ( _verticalUnit == feetPerMinute )
      precision = 0;

    QByteArray prec;
    prec.setNum(precision);

    if (withUnit) {
      result.sprintf("%1." + prec + "f %s", speed, unit.toLatin1().data());
    } else {
        result.sprintf("%1." + prec + "f", speed);
    }
    return result;
}


/** Set speed in selected horizontal unit. */
void Speed::setHorizontalValue(double speed)
{
    setValueInUnit(speed, _horizontalUnit);
}


/** Set speed in selected vertical unit. */
void Speed::setVerticalValue(double speed)
{
    setValueInUnit(speed, _verticalUnit);
}


void Speed::setValueInUnit(double speed, speedUnit unit)
{
    switch (unit) {
    case knots:
        setKnot(speed);
        break;
    case milesPerHour:
        setMph(speed);
        break;
    case metersPerSecond:
        setMps(speed);
        break;
    case kilometersPerHour:
        setKph(speed);
        break;
    case feetPerMinute:
        setFpm(speed);
        break;
    default:
        setMps(speed);
        break;
    }
}


QString Speed::getUnitText(speedUnit unit)
{
    switch (unit) {
    case knots:
        return "Knt";
        break;
    case milesPerHour:
        return "Mph";
        break;
    case metersPerSecond:
        return "m/s";
        break;
    case kilometersPerHour:
        return "Km/h";
        break;
    case feetPerMinute:
        return "Fpm";
        break;
    default:
        return "m/s";
    }
}

QString Speed::getHorizontalUnitText()
{
  return getUnitText( _horizontalUnit );
}

QString Speed::getVerticalUnitText()
{
  return getUnitText( _verticalUnit );
}

double Speed::getValueInUnit(speedUnit unit) const
{
    switch (unit) {
    case knots:
        return getKnots();
        break;
    case milesPerHour:
        return getMph();
        break;
    case metersPerSecond:
        return getMps();
        break;
    case kilometersPerHour:
        return getKph();
        break;
    case feetPerMinute:
        return getFpm();
        break;
    default:
        return getMps();
    }
}


void Speed::changed()
{
    // @AP Note: we have negative speed values in the variometer
    // culculation. Setting to zero is not a good idea.

    // speed should not be less than 0
    // if (_speed<0) _speed=0;
}

