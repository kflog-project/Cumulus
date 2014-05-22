/***********************************************************************
**
**   vector.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**
**   polished for nmea application by Eckhard Völlm
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>
#include <iostream>
#include "vector.h"

using namespace std;

Vector::Vector(){
  _angle=0;
  _speed=0;
  _x=0;
  _y=0;
  dirtyXY=false;
  dirtyDR=false;
}

double Vector::normalize(double angle) {
    if (angle<0) return normalize(angle+PI2);
    if (angle>=PI2) return normalize(angle-PI2);
    return angle;
}

double Vector::polar(double y, double x)
{
    double angle = 0.0;
    //
    //  dX = 0 ???
    //
    if(x >= -0.001 && x <= 0.001)
    {
        if(y < 0.0) return ( 1.5 * PI );
        else  return ( 0.5 * PI );
    }

    // Punkt liegt auf der neg. X-Achse
    if(x < 0.0)  angle = atan( y / x ) + PI;
    else  angle = atan( y / x );

    // Neg. value not allowed.
    if(angle < 0.0)  angle = PI2 + angle;

    if(angle > (PI2))  angle = angle - (PI2);

    return angle;
}

Vector::Vector(double x, double y){
    setX(x);
    setY(y);
    dirtyXY=false;
    dirtyDR=true;
}

Vector::Vector(Speed x, Speed y){
    setX(x);
    setY(y);
    dirtyXY=false;
    dirtyDR=true;
}

Vector::Vector(double angle, Speed R){
  setAngleRad((float)angle);
  // qDebug("angle:  %f", angle*180/PI);
  _speed=R.getMps();
  // qDebug("_speed:  %f", _speed);
  dirtyDR=false;
  dirtyXY=true;
}

Vector::~Vector(){
}

/** Read property of int angle. */
float Vector::getAngleDeg() {
    if (dirtyDR) recalcDR();
    // qDebug("getAngleDeg:  %f", _angle*180/PI);
    // qDebug("_x %f", _x );
    // qDebug("_y %f", _y );

    float ang = (_angle / PI) * 180.0;
    return float(ang);
}

/** Get angle in rad */
double Vector::getAngleRad() {
    if (dirtyDR) recalcDR();
    return (double)_angle;
}

/** Set property of int angle in degrees*/
void Vector::setAngle(float angle){
    if (dirtyDR) recalcDR();
    _angle=(double(normalize(angle*PI/180.0)));
    dirtyXY=true;
}

/** Set property of int angle in degrees*/
void Vector::setAngle(int angle){
  if (dirtyDR) recalcDR();
  _angle=(double(normalize(angle))/180.0) * PI;
  dirtyXY=true;
}

/** Set property of int angle in rad*/
void Vector::setAngleRad(double angle){
  if (dirtyDR) recalcDR();
  _angle=normalize(angle);
  dirtyXY=true;
  dirtyDR=false;
}

/** Re-implemented from Speed */
void Vector::changed() {
    dirtyXY=true;
}

/** Recalculates the the angle and the speed from the known x and y values. */
void Vector::recalcDR() {
    // qDebug("recalcDR");
    // qDebug("_x %f", _x );
    // qDebug("_y %f", _y );

    _angle=normalize(polar(_x,_y));
    // qDebug("_angle:  %f", _angle*180/PI);
    _speed=(sqrt((_y * _y) + (_x * _x)));
    dirtyDR=false;
}

/** Recalculates the X and Y values from the known angle and speed. */
void Vector::recalcXY() {
   _y=_speed * cos(_angle);
  _x=_speed * sin(_angle);
  dirtyXY=false;
}

/** returns the speed in X (longtide) direction (north is positive, south is negative) */
Speed Vector::getX() {
    if (dirtyXY) recalcXY();
    return Speed(_x);
}

/** Returns the speed in Y (latitude) direction (east is positive, west is negative) */
Speed Vector::getY() {
    if (dirtyXY) recalcXY();
    return Speed(_y);
}

Speed Vector::getSpeed() {
    if (dirtyXY) recalcXY();
    return Speed( sqrt((_y*_y)+(_x*_x)) );
}



/** returns the speed in X (longtide) direction (north is positive, south is negative) */
double Vector::getXMps() {
    if (dirtyXY) recalcXY();
    return _x;
}

/** Returns the speed in Y (latitude) direction (east is positive, west is negative) */
double Vector::getYMps() {
    if (dirtyXY) recalcXY();
    return _y;
}

/** Sets the X (longitudinal) speed in meters per second. */
void Vector::setX(double x){
    if (dirtyXY) recalcXY();
    _x=x;
    dirtyDR=true;
}

/** Sets the Y (latitudinal) speed in meters per second. */
void Vector::setY(double y){
    if (dirtyXY) recalcXY();
    _y=y;
    dirtyDR=true;
}

/** Sets the X (longitudinal) speed in meters per second. */
void Vector::setX(Speed x){
    if (dirtyXY) recalcXY();
    _x=x.getMps();
    dirtyDR=true;
}

/** Sets the Y (latitudinal) speed in meters per second. */
void Vector::setY(Speed y){
    if (dirtyXY) recalcXY();
    _y=y.getMps();
    dirtyDR=true;
}

/** + operator for Vector. */
Vector Vector::operator + (const Vector& x)  {
    return Vector(x._x + _x, x._y + _y);
}

/** - operator for Vector. */
Vector Vector::operator - (const Vector& x)  {
    return Vector(_x - x._x,_y - x._y);
}


/** * operator for Vector. */
Vector Vector::operator * (double left)  {
    return Vector(_angle, Speed(left * this->getMps()));
}

/** / operator for Vector. */
double Vector::operator / (const Vector& x)  {
    return _speed / x._speed;
}

/** * operator for Vector. */
double Vector::operator * (const Vector& x)  {
    return _speed * x._speed;
}


/** - prefix operator for Vector */
Vector Vector::operator - ()
{
 if (!dirtyDR) {
    return Vector (_angle+PI, Speed(_speed));
  } else if (!dirtyXY) {
    return Vector (-_x, -_y);
  } else { //should not happen! Either XY or DR should be clean, or both.
    recalcXY();
    return Vector (-_x, -_y);
  }
}

/** * operator for vector. */
Vector operator * (Vector& left, double right) {

    return Vector(left.getAngleRad(), Speed(right * left.getSpeed().getMps()) );
}

/** * operator for vector. */
Vector operator * (double left, Vector& right) {

    return Vector(right.getAngleRad(), Speed(left * right.getSpeed().getMps()) );
}

/** / operator for vector. */
Vector operator / (Vector& left, double right) {

    return Vector(left.getAngleRad(), Speed(left.getSpeed().getMps()/right) );
}

/** / operator for vector. */
Vector operator / (Vector& left, int right) {

    return Vector(left.getAngleRad(), Speed(left.getSpeed().getMps()/right) );
}

/** Poor man's solution for not getting the + operator to work properly. */
void Vector::add(Vector& arg){
  if (dirtyXY) {
    recalcXY();
    // qDebug("Vector::add - recalculated XY");
  } else {
    // qDebug("Vector::add - XY clean");
  }
  // qDebug("_x %f", _x );
  // qDebug("_y %f", _y );
  // qDebug("arg._x %f",arg.getXMps() );
  // qDebug("arg._y %f",arg.getYMps() );
  _x+=arg.getXMps();
  _y+=arg.getYMps();
  // qDebug("_x %f", _x );
  // qDebug("_y %f", _y );

  dirtyDR=true;
}

/** Returns a copy of the object */
Vector Vector::Clone(){
    Vector result;
    result._speed=_speed;
    result.setAngleRad(this->getAngleRad());
    result.dirtyDR=false;
    return result;
}
