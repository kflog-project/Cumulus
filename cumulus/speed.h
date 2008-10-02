/***************************************************************************
                          speed.h  -  description
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

#ifndef SPEED_H
#define SPEED_H

#include <QString>

//the following constants define the factor by which to multiply meters per
//second to get the indicated unit
const double toKph=3.6;
const double toMph=2.2369;
const double toKnot=1.9438;
const double toFpm=196.8504;

/**
 * @short Abstract speed
 *
 * This class can contain a speed. It provides set and get functions for
 * all popular units. For this class, there are two default units you can
 * set: the horizontal and the vertical unit. The horizontal unit is used
 * for the airspeed, the vertical unit is used for climb, McCready settings,
 * etc. There are convenience functions to access the value using both
 * appropriate units.
 *
 * @author Andre Somers
 */
class Speed
{

public:
    /**
     * The speedUnit enum lists the units that apply to speed.
     */
    enum speedUnit{
        metersPerSecond=0,   /**< meters per second */
        kilometersPerHour=1, /**< kilometers per hour */
        knots=2,             /**< knots (nautical miles per hour) */
        milesPerHour=3,      /**< imperial miles per hour */
        feetPerMinute=4        /**< feet per minute */
    };

    /**
     * Constructor
     */
    Speed();

    /**
     * Constructor
     *
     * Sets the speed to Mps meters per second.
     */
    Speed(double Mps);

    /**
     * Destructor
     */
    virtual ~Speed();

    /**
     * Get speed in Knots.
     */
    double getKnots() const;

    /**
     * Get speed in Kilometers per hour
     */
    double getKph() const;

    /**
     * Get speed in Meters per Second
     */
    double getMps() const;

    /**
     * Get speed in Nautical Miles per hour
     */
    double getMph() const;

    /**
     * Get speed in Nautical Miles per hour
     */
    double getFpm() const;

    /**
     * Set speed in Nautical miles per hour
     */
    void setMph(double speed);

    /**
     * Set speed in knots
     */
    void setKnot(double speed);

    /**
     * Set speed in Kilometers per hour
     */
    void setKph(double speed);

    /**
     * Set speed in meters per second.
     */
    void setMps(double speed);

    /**
     * Set speed in feet per minute.
     */
    void setFpm(double speed);

    /**
     * Set speed in selected horizontal unit.
     */
    void setHorizontalValue(double speed);

    /**
     * Set speed in selected vertical unit.
     */
    void setVerticalValue(double speed);

    /**
     * set the horizontal unit
     */
    static void setHorizontalUnit(speedUnit unit);

    /**
     * set the vertical unit
     */
    static void setVerticalUnit(speedUnit unit);

    /**
     * get the horizontal unit
     */
    static speedUnit getHorizontalUnit();

    /**
     * get the vertical unit
     */
    static speedUnit getVerticalUnit();

    /**
     * Returns a formatted string for the default vertical speed units.
     * The string includes the value and optionally the unit.
     * @param withUnit set to true (default) to have the returned string
     *    include the unit, false otherwise
     * @param prec set to the number of digits after the decimal point you
     *    want in the string
     */
    QString getVerticalText(bool withUnit=true, uint prec=1)const;

    /**
     * Returns a formatted string for the default horizontal speed units.
     * The string includes the value and optionally the unit.
     * @param withUnit set to true (default) to have the returned string
     *    include the unit, false otherwise
     * @param prec set to the number of digits after the decimal point you
     *    want in the string
     */
    QString getHorizontalText(bool withUnit=true, uint prec=1)const;

    /**
     * @returns a string for the unit requested. This string only represents
     *    the unit, not the value.
     * @param unit the type of unit you want the string for.
     */
    static QString getUnitText(speedUnit unit);

    /**
     * @returns a string for the horizontal unit requested. This string only represents
     *    the unit, not the value.
     * @param unit the type of unit you want the string for.
     */
    static QString getHorizontalUnitText();
    static QString getVerticalUnitText();

    /**
     * Get the value for the vertical speed in the currently active unit.
     * @returns the current speed value
     */
    inline double getVerticalValue() const
    {
        return getValueInUnit(_verticalUnit);
    };

    /**
     * Get the value for the horizontal speed in the currently active unit.
     * @returns the current speed value
     */
    inline double getHorizontalValue() const
    {
        return getValueInUnit(_horizontalUnit);
    };

    /**
     * Get the value in the requested unit.
     * @returns the value converted to the requested unit
     * @param unit the unit in which to return the value
     */
    double getValueInUnit(speedUnit unit) const;

    /**
     * Set the value in the indicated unit.
     * @param speed the new speed to set
     * @param unit the unit in which to set the value
     */
    void setValueInUnit(double speed, speedUnit unit);

    /**
     * Sets the distance to be invalid
     */
    inline void setInvalid()
    {
        _isValid=false;
        _speed=0;
    };

    /**
     * Gets if the distance is valid
     */
    inline bool isValid()
    {
        return _isValid;
    };

    /*Operators */
    /**
     * + operator for speed.
     */
    Speed operator + (const Speed& x) const;

    /**
     * - operator for speed.
     */
    Speed operator - (const Speed& x) const;

    /**
     * / operator for speed.
     */
    double operator / (const Speed& x) const;

    /**
     * * operator for speed.
     */
    double operator * (const Speed& x) const;

    /**
     * != operator for Speed
     */
    bool operator != (const Speed& x) const;

    /**
     * == operator for Speed
     */
    bool operator == (const Speed& x) const;

    /**
     * minus prefix operator for speed
     */
    Speed operator - () const;

protected:
    /**
     * This attribute contains the stored speed in meters per second.
     */
    double _speed;

    /**
     * value valid?
     */
    bool _isValid;

    /**
     * Contains the unit used for horizontal speeds
     */
    static speedUnit _horizontalUnit;

    /**
     * Contains the unit used for vertical speeds
     */
    static speedUnit _verticalUnit;

    /**
     * called if the value has been changed. Can be overridden in derived classes
     */
    virtual void changed();
};

/**
 * - operator for speed.
 */
Speed operator - (double left, const Speed& right);

/**
 * + operator for speed.
 */
Speed operator + (double left, const Speed& right);

/**
 * * operator for speed.
 */
Speed operator * (double left, const Speed& right);

#endif
