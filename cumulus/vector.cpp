/***********************************************************************
**
**   vector.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include "vector.h"
#include "mapcalc.h"

Vector::Vector() :
  _angle(0.0),
  dirtyXY(false),
  dirtyDR(false),
  _x(0.0),
  _y(0.0),
  _speed(0.0),
  _isValid(false)
{
}

Vector::Vector(const double& x, const double& y)
{
    _angle=0.0;
    _speed=0.0;
    dirtyXY=false;
    dirtyDR=false;
    setX(x);
    setY(y);
    dirtyXY=false;
    dirtyDR=true;
    _isValid=true;
}

Vector::Vector(const Speed& x, const Speed& y)
{
    _angle=0.0;
    _speed=0.0;
    dirtyXY=false;
    dirtyDR=false;
    setX(x);
    setY(y);
    dirtyXY=false;
    dirtyDR=true;
    _isValid=true;
}

Vector::Vector(const double& angle, const Speed& R)
{
    _x=0.0;
    _y=0.0;
    dirtyXY=false;
    dirtyDR=false;
    _speed=R.getMps();
    setAngleRad(angle);
    dirtyDR=false;
    dirtyXY=true;
    _isValid=true;
}

Vector::Vector(const int angle, const Speed& R)
{
    _x=0.0;
    _y=0.0;
    dirtyXY=false;
    dirtyDR=false;
    _speed=R.getMps();
    setAngle(angle);
    dirtyDR=false;
    dirtyXY=true;
    _isValid=true;
}

Vector::~Vector()
{}

/** Read property of int angle. */
int Vector::getAngleDeg()
{
  if( dirtyDR )
    {
      recalcDR();
    }

  return int( rint( (_angle / M_PI) * 180.0 ) );
}

/** Get angle in rad */
double Vector::getAngleRad()
{
  if( dirtyDR )
    {
      recalcDR();
    }

  return _angle;
}

/** Set property of int angle in degrees*/
void Vector::setAngle(const int angle)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  _angle = (double( normalize( angle ) ) / 180.0) * M_PI;
  dirtyXY = true;
  _isValid = true;
}

/**
 * set the angle in degrees  and the speed
 */
void Vector::setAngleAndSpeed(const int angle, const Speed & spd)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  setAngle( angle );
  _speed = spd.getMps();
  dirtyDR = false;
  dirtyXY = true;
  _isValid = true;
}

/** Set property of int angle in rad*/
void Vector::setAngleRad(const double& angle)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  _angle = normalize( angle );
  dirtyXY = true;
  dirtyDR = false;
  _isValid = true;
}

/**
 * Set the speed
 */
void Vector::setSpeed(const Speed & s)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  _speed = s.getMps();
  _isValid = true;
}

/**
 * @return The speed
 */
Speed Vector::getSpeed()
{
  if( dirtyDR )
    {
      recalcDR();
    }

  return Speed( _speed );
}

/** Recalculates the the angle and the speed from the known x and y values. */
void Vector::recalcDR()
{
  _angle = normalize( polar( _y, _x ) );
  _speed = hypot( _y, _x );
  dirtyDR = false;
}


/** Recalculates the X and Y values from the known angle and speed. */
void Vector::recalcXY()
{
  _y = _speed * sin( _angle );
  _x = _speed * cos( _angle );
  dirtyXY = false;
}


/** returns the speed in X (latitude) direction (north is positive, south is negative) */
Speed Vector::getX()
{
  if( dirtyXY )
    {
      recalcXY();
    }

  return Speed( _x );
}


/** Returns the speed in Y (longitude) direction (east is positive, west is negative) */
Speed Vector::getY()
{
  if( dirtyXY )
    {
      recalcXY();
    }

  return Speed( _y );
}


/** returns the speed in X (latitude) direction (north is positive, south is negative) */
double Vector::getXMps()
{
  if( dirtyXY )
    {
      recalcXY();
    }

  return _x;
}


/** Returns the speed in Y (longitude) direction (east is positive, west is negative) */
double Vector::getYMps()
{
    if (dirtyXY)
        recalcXY();
    return _y;
}


/** Sets the X (latitudinal) speed in meters per second. */
void Vector::setX(const double& x)
{
  if( dirtyXY )
    {
      recalcXY();
    }

  _x = x;
  dirtyDR = true;
}


/** Sets the Y (longitudinal) speed in meters per second. */
void Vector::setY(const double& y)
{
  if( dirtyXY )
    {
      recalcXY();
    }

  _y = y;
  dirtyDR = true;
}


/** Sets the X (latitudinal) speed in meters per second. */
void Vector::setX(const Speed& x)
{
  if( dirtyXY )
    {
      recalcXY();
    }

  _x = x.getMps();
  dirtyDR = true;
  _isValid = true;
}


/** Sets the Y (longitudinal) speed in meters per second. */
void Vector::setY(const Speed& y)
{
  if( dirtyXY )
    {
      recalcXY();
    }

  _y = y.getMps();
  dirtyDR = true;
  _isValid = true;
}


/** = operator for Vector. */
Vector& Vector::operator = (const Vector& x)
{
  setX( x._x );
  setY( x._y );
  _speed = x._speed;
  _angle = x._angle;
  dirtyXY = x.dirtyXY;
  dirtyDR = x.dirtyDR;

  return *this;
}


/** + operator for Vector. */
Vector Vector::operator + (Vector& x)
{
  if( dirtyXY )
    {
      recalcXY();
    }
  if( x.dirtyXY )
    {
      x.recalcXY();
    }

  return Vector( x._x + _x, x._y + _y );
}


/** - operator for Vector. */
Vector Vector::operator - (Vector& x)
{
  if( dirtyXY )
    {
      recalcXY();
    }
  if( x.dirtyXY )
    {
      x.recalcXY();
    }

  return Vector( _x - x._x, _y - x._y );
}

/** * operator for Vector. */
Vector Vector::operator * (double left)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  return Vector( _angle, Speed( left * _speed ) );
}


Vector Vector::operator * (int left)
{
    if( !dirtyDR )
    {
      return Vector( _angle, Speed( double( left * _speed ) ) );
    }
  else if( !dirtyXY )
    {
      return Vector( left * _x, left * _y );
    }
  else
    {
      recalcXY();
      return Vector( left * _x, left * _y );
    }
}


/** / operator for Vector. */
double Vector::operator / ( Vector& x)
{
    if (dirtyDR)
        recalcDR();
    if (x.dirtyDR)
        x.recalcDR();
    return _speed / x._speed;
}


/** * operator for Vector. */
double Vector::operator * ( Vector& x)
{
    if (dirtyDR)
        recalcDR();
    if (x.dirtyDR)
        x.recalcDR();
    return _speed * x._speed;
}


/** == operator for Vector */
bool Vector::operator == ( Vector& x)
{
  Vector t( x );
  Vector u( *this );

  if( u.dirtyDR )
    {
      u.recalcDR();
    }

  if( t.dirtyDR )
    {
      t.recalcDR();
    }

  return ((t._speed == u._speed) && (t._angle == u._angle));
}


/** != operator for Vector */
bool Vector::operator != ( Vector& x)
{
  if( dirtyDR )
    {
      recalcDR();
    }

  return ((x._speed != _speed) || (x._angle != _angle));
}


/** - prefix operator for Vector */
Vector Vector::operator - ()
{
  //there are two options for this. We use the one that involves the least conversions.
  if( !dirtyDR )
    {
      return Vector( _angle + M_PI, Speed( _speed ) );
    }
  else if( !dirtyXY )
    {
      return Vector( -_x, -_y );
    }
  else
    { //should not happen! Either XY or DR should be clean, or both.
      recalcXY();
      return Vector( -_x, -_y );
    }
}


/** * operator for vector. */
Vector operator * (Vector& left, double right)
{
  return Vector( left.getAngleRad(), Speed( right * left.getSpeed().getMps() ) );
}


/** * operator for vector. */
Vector operator * (double left, Vector& right)
{

  return Vector( right.getAngleRad(), Speed( left * right.getSpeed().getMps() ) );
}


/** / operator for vector. */
Vector operator /( Vector& left, double right )
{

  return Vector( left.getAngleRad(), Speed( left.getSpeed().getMps() / right ) );
}

/** / operator for vector. */
Vector operator /( Vector& left, int right )
{

  return Vector( left.getAngleRad(), Speed( left.getSpeed().getMps() / right ) );
}


/** Poor man's solution for not getting the + operator to work properly. */
void Vector::add(Vector arg)
{
  if( dirtyXY )
    {
      recalcXY();
    }

  _x += arg.getXMps();
  _y += arg.getYMps();
  dirtyDR = true;
}


/** Returns a copy of the object */
Vector Vector::clone()
{
  Vector result;
  result._speed = _speed;
  result.setAngleRad( this->getAngleRad() );
  result.dirtyDR = false;
  return result;
}
