/***********************************************************************
**
**   distance.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Andre Somers
**                   2007-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Distance
 *
 * \author Heiner Lamprecht, Andre Somers, Axel Pauli
 *
 * \brief This class handles different distance units and arithmetics.
 *
 * An object of this class represents a distance. It can express this
 * distance in a number of major formats.
 * It also features a couple of static methods to convert from the set
 * unit to meters or to display a distance in the currently selected unit.
 *
 * \date 2002-2013
 */

#pragma once

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
   * Copy assignment operator
   */
  Distance& operator=(const Distance& dst);

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
   * Sets the distance value in the current used distance unit.
   */
  void setValueInCurrentUnit( const double value );

  /**
   * Returns distance value of the current set distance unit.
   */
  double getValueOfCurrentUnit() const;

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
   * \return The distance in Nautical Miles.
   */
  double getNautMiles() const
  {
    return _dist/mFromNMile;
  };

  /**
   * \return The distance in miles:
   */
  double getMiles() const
  {
    return _dist/mFromMile;
  };

  /**
   * \return The distance in kilometers.
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
