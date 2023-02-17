/***********************************************************************
**
**   vector.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2009-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include "speed.h"

/**
 * \class Vector
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A vector represents a speed in a certain (2d) direction.
 *
 * A vector represents a speed in a certain (2d) direction.
 * It is a subclass from the @ref Speed class, meaning you can use it as a
 * normal Speed object. The values returned or set in that way are the
 * values in the given direction. You can read or set that direction using
 * the getAngle... and setAngle member functions (or their variants).
 * You can also access the components of the speed in the X and Y directions.
 * Note that X counts latitudinal, Y count longitudinal.
 *
 * \date 2002-2023
 */
class Vector
{
public:

  Vector();
  Vector(const double& x, const double& y);
  Vector(const Speed& x, const Speed& y);
  Vector(const double& angle, const Speed& R);
  Vector(const int angle, const Speed& R);

  Vector(const Vector&) = default;

  Vector& operator=( const Vector& other )
  {
    if( this != &other)
      {
        // protect against invalid self-assignment
        _angle = other._angle;
        dirtyXY = other.dirtyXY;
        dirtyDR = other.dirtyDR;
        _x = other._x;
        _y = other._y;
        _speed = other._speed;
        _isValid = other._isValid;
      }

    return *this;
  }

  ~Vector();

  /**
   * Get angle in degrees as integer.
   */
  int getAngleDeg();

  /**
   * Get angle in degrees as double.
   */
  double getAngleDegDouble();

  /**
   * Get angle in radian.
   */
  double getAngleRad();

  /**
   * set the angle in degrees
   */
  void setAngle(const int angle);

  /**
   * set the angle in degrees
   */
  void setAngle(const double angle);

  /**
   * set the angle in degrees  and the speed
   */
  void setAngleAndSpeed(const int angle, const Speed&);

  /**
   * set the angle in radian
   */
  void setAngleRad(const double& angle);

  /**
   * set the speed
   */
  void setSpeed(const Speed& speed);

  /**
   * Set the speed. Expected unit is meter per second.
   */
  void setSpeed(const double mps);

  /**
   * @return The speed
   */
  Speed getSpeed();

  /**
   * @return The speed in Y (longitude) direction
   * (east is positive, west is negative)
   */
  Speed getY();

  /**
   * @return The speed in X (latitude) direction
   * (north is positive, south is negative)
   */
  Speed getX();

  /**
   * @returns The speed in Y (longitude) direction
   * (east is positive, west is negative) in meters per second
   */
  double getYMps();

  /**
   * @return The speed in X (latitude) direction
   * (north is positive, south is negative) in meters per second
   */
  double getXMps();

  /**
   * Sets the X (latitudinal) speed in meters per second.
   */
  void setX(const double& x);

  /**
   * Sets the Y (longitudinal) speed in meters per second.
   */
  void setY(const double& y);

  /**
   * Sets the X (latitudinal) speed in meters per second.
   */
  void setX(const Speed& x);

  /**
   * Sets the Y (longitudinal) speed in meters per second.
   */
  void setY(const Speed& y);

  /**
   * + operator for Vector.
   */
  Vector operator + (Vector& x);

  /**
   * - operator for Vector.
   */
  Vector operator - (Vector& x);

  /**
   * / operator for Vector.
   */
  double operator / (Vector& x);

  /**
   * operator for Vector.
   */
  double operator * (Vector& x);

  /**
   * != operator for Vector
   */
  bool operator != ( Vector& x);

  /**
    * == operator for Vector
    */
  bool operator == ( Vector& x);

  /**
   * minus prefix operator for Vector
   */
  Vector operator - ();

  /**
   * * prefix operator for Vector
   */
  Vector operator * (double left);

  Vector operator * (int left);

  /**
   * Poor man's solution for not getting the +
   * operator to work properly.
   */
  void add(Vector arg);

  /**
   * Returns a copy of the object
   */
  Vector clone();

  /**
   * Sets the distance to be invalid
   */
  void setInvalid()
  {
      _isValid=false;
      dirtyXY=true;
      dirtyDR=true;
      _angle=0;
      _x=0;
      _y=0;
      _speed=0;
  };

  /**
   * Gets if the distance is valid
   */
  bool isValid() const
  {
    return _isValid;
  };

protected: // Protected attributes
  /**
   * Contains the angle of the speed. 0 is north, pi/2 east, pi south, etc.
   */
  double _angle;

  /**
   *  True if the speed and/or direction have been set, and XY need to be recalculated
   */
  bool dirtyXY;

  /**
   * True if X and/or Y have been set, and speed and direction need to be recalculated
   */
  bool dirtyDR;

  /**
   * Speed in X (latitudinal) direction in meters per second, north being positive.
   */
  double _x;

  /**
   * Speed in Y (longitudinal) direction in meters per second, east being positive.
   */
  double _y;

  /**
   * Speed in mps
   */
  double _speed;

  /**
   * value valid?
   */
  bool _isValid;

private:
  /**
   * Recalculates the X and Y values from the known angle and speed.
   */
  void recalcXY();

  /**
   * Recalculates the the angle and the distance from the known x and y values.
   */
  void recalcDR();
};
