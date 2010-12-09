/***************************************************************************
                          distance.h  -  general distance representation
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2007 Axel Pauli
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

/**
 * \class Distance
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief This class handles different distance units and arithmetics.
 *
 * An object of this class represents a distance. It can express this
 * distance in a number of major formats.
 * It also features a couple of static methods to convert from the set
 * unit to meters or to display a distance in the currently selected unit.
 *
 * \date 2002-2007
 */

#ifndef DISTANCE_H
#define DISTANCE_H

#include <QString>

class Distance
{
 public:

  // define conversion constants
  static const double mFromKm;
  static const double mFromMile;
  static const double mFromNMile;
  static const double mFromFeet;

  enum distanceUnit{meters=0, feet=1, kilometers=2, miles=3, nautmiles=4};

 public:

  Distance();

  /**
   * Constructor
   *
   * Initializes the object to meters.
   */
  Distance(int meters);

  /**
   * Constructor
   *
   * Initializes the object to meters.
   */
  Distance(double meters);

  /**
   * Copy constructor
   */
  Distance(const Distance& dst);

  /**
   * Destructor
   */
  ~Distance();

  /**
   * Set the distance in meters
   */
  void setMeters(int meters)
  {
    _dist=double(meters);
    _isValid=true;
  };

  /**
   * Set the distance in meters
   */
  void setMeters(double meters)
  {
    _dist=meters;
    _isValid=true;
  };

  /**
   * Set distance in feet
   */
  void setFeet(int feet)
  {
    _dist=double(feet)*mFromFeet;
    _isValid=true;
  };

  /**
   * Set distance in feet
   */
  void setFeet(double feet)
  {
    _dist=feet*mFromFeet;
    _isValid=true;
  };

  /**
   * Get distance in feet
   */
  double getFeet() const
  {
    return _dist / mFromFeet;
  };

  /**
   * Returns distance in meters
   */
  double getMeters() const
  {
    return _dist;
  };

  /**
   * implements == operator for distance
   */
  bool operator == (const Distance& x) const;

  /**
   * implements != operator for distance
   */
  bool operator != (const Distance& x) const;

  /**
   * implements minus operator
   */
  Distance operator - (const Distance& op) const;

  /**
   * implements divide operator
   */
  double operator / (const Distance& op) const;

  /**
   * implements multiply operator
   */
  Distance operator * (const int& op) const
  {
    return Distance(_dist * op);
  };

  /**
   * implements multiply operator
   */
  Distance operator * (const double& op) const
  {
    return Distance(_dist * op);
  };

  /**
   * @returns a string for the currently set distance unit.
   */
  static QString getUnitText();

  /**
   * Represent a distance as a string.
   *
   * @return A string containing the distance in the set unit (see @ref setUnit).
   * The string that identifies the unit used is added if withUnit is true.
   *
   * example:
   * <pre>
   *    getText(1,true,1)
   * </pre>
   * with the distance unit set to feet would return "3.2 ft".
   *
   * @param meters The distance expressed in meters
   * @param withUnit determines if the unit-string is included in the output
   * @param precision number of digits after the decimal separator
   */
  static QString getText(double meters, bool withUnit, int precision=1);

  /**
   * Basicly the same as @ref getText, but returns the internally stored distance.
   */
  QString getText(bool withUnit, uint precision=1, uint chopOrder=0) const;

  /**
   * Converts a distance from the current unit set with @ref setUnit to meters.
   */
  static double convertToMeters(double dist);

  /**
   * Sets the distance in kilometers
   */
  void setKilometers(double km)
  {
    _dist=km*mFromKm;
    _isValid=true;
  };

  /**
   * Sets the distance in kilometers
   */
  void setKilometers(int km)
  {
    _dist=double(km)*mFromKm;
    _isValid=true;
  };

  /**
   * sets the distance in Nautical miles
   */
  void setNautMiles(double value)
  {
    _dist=value*mFromNMile;
    _isValid=true;
  };

  /**
   * Sets the distance in miles
   */
  void setMiles(double value)
  {
    _dist=value*mFromMile;
    _isValid=true;
  };

  /**
   * Sets the unit for distances.
   */
  static void setUnit(distanceUnit unit)
  {
    _distanceUnit=unit;
  };

  /**
   * Return the currently set unit for distances
   */
  static distanceUnit getUnit()
  {
    return _distanceUnit;
  };

  /**
   * Returns the distance in Nautical Miles
   */
  double getNautMiles() const
  {
    return _dist/mFromNMile;
  };

  /**
   * returns the distance in miles
   */
  double getMiles() const
  {
    return _dist/mFromMile;
  };

  /**
   * returns the distance in kilometers
   */
  double getKilometers() const
  {
    return _dist/mFromKm;
  };

  /**
   * Sets the distance to be invalid
   */
  void setInvalid()
  {
    _isValid=false;
    _dist=0;
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
   * internal representation (in meters)
   */
  double _dist;

  /**
   * value valid?
   */
  bool _isValid;

 private:

  static distanceUnit _distanceUnit;
};

#endif
