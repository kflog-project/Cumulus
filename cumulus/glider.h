/***********************************************************************
**
**   glider.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GLIDER_H
#define GLIDER_H

#include <QString>
#include <QSettings>

#include "polar.h"

/**
 * \author André Somers
 *
 * \brief Class to handle different glider attributes.
 *
 * An instance of a Glider object contains all the information available
 * on a glider: it's type, it's registration, call sign, polar,
 * single or double seater, maximum water capacity, etc.
 */
class Glider
{

public:

    enum seat{ singleSeater=1, doubleSeater=2 };

    Glider();

    ~Glider();

    /**
     * @returns the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    QString type()const
    {
        return _type;
    };

    /**
     * Sets the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    void setType(QString newValue)
    {
      _type = newValue;
    };

    /**
     * @returns the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    QString registration() const
    {
        return _registration;
    };

    /**
     * Sets the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    void setRegistration(QString newValue)
    {
        _registration=newValue;
    };

    /**
     * @returns the call sign of the Glider, such as for instance 'UT' or 'DW'.
     */
    QString callSign() const
    {
        return _callSign;
    };

    /**
     * Sets the callSign of the Glider, such as for instance 'UT' or 'DW'.
     */
    void setCallSign(QString newValue)
    {
        _callSign = newValue;
    };

    /**
     * @returns the number of seats of the glider.
     */
    Glider::seat seats() const
    {
        return _seats;
    };

    /**
     * Sets the number of seats of the glider.
     */
    void setSeats(Glider::seat newValue)
    {
        _seats=newValue;
    };

    /**
     * @returns the maximum amount of ballast water (in liters) that the glider can hold.
     */
    int maxWater()const
    {
        return _maxWater;
    };

    /**
     * Sets the maximum amount of ballast water (in liters) that the glider can hold.
     */
    void setMaxWater(int newValue)
    {
      _maxWater=newValue;
    };

    /**
     * @returns a pointer to the polar object belonging to this glider.
     */
    Polar *polar()
    {
      return &_polar;
    };

    /**
     * Sets the polar object belonging to this glider.
     */
    void setPolar(Polar& newPolar)
    {
      _polar = newPolar;
    };

    /**
     * This funtion loads the glider-data from the config-object.
     * The config object needs to be initialized to the correct
     * section beforehand! It tries to load the glider with id id,
     * and returns true if it succeeds, and false otherwise.
     */
    bool load(QSettings *config, int id);

    /**
     * Stores the glider info contained in configuration object config under id.
     */
    void safe(QSettings *config, int id);

    /**
     * @returns the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
     int lastSafeID()const
    {
        return _lastSafeID;
    };

    /**
     * Sets the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
     void setID(int newID)
    {
        _lastSafeID=newID;
    };

    /**
     * @returns the name of the co-pilot.
     */
    QString coPilot() const
    {
      return _coPilot;
    };

    /**
     * Sets the name of the co-pilot.
     */
     void setCoPilot(QString newValue)
    {
        _coPilot=newValue;
    };

private:
    /**
     * Contains the type of glider, for instance Ka-8 or ASH 25
     */
    QString _type;
    QString _registration;
    QString _callSign;
    seat _seats;
    Polar _polar;
    int _maxWater;
    int _lastSafeID;

    QString _coPilot;
};

#endif
