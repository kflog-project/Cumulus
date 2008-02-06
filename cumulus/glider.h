/***********************************************************************
**
**   glider.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GLIDER_H
#define GLIDER_H

#include <QObject>
#include <QSettings>

#include "polar.h"

/**
 * An instance of a Glider object contains all the information available
 * on a glider: it's type, it's registration, callsign, polaire,
 * two- or single seater, maximum water capacity, etc.
 * 
 * @author André Somers
 */

class Glider : public QObject
{
    Q_OBJECT
public: //types
    enum seat{singleSeater, doubleSeater};

public:
    Glider();
    ~Glider();

    /**
     * @returns the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    inline QString type()const
    {
        return _type;
    };

    /**
     * Sets the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    void setType(QString newValue);

    /**
     * @returns the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    inline QString registration()const
    {
        return _registration;
    };

    /**
     * Sets the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    inline void setRegistration(QString newValue)
    {
        _registration=newValue;
    };

    /**
     * @returns the callsign of the Glider, such as for instance 'UT' or 'DW'.
     */
    inline QString callsign()const
    {
        return _callsign;
    };

    /**
     * Sets the callsign of the Glider, such as for instance 'UT' or 'DW'.
     */
    inline void setCallsign(QString newValue)
    {
        _callsign=newValue;
    };

    /**
     * @returns the number of seats of the glider.
     */
    inline Glider::seat seats() const
    {
        return _seats;
    };

    /**
     * Sets the number of seats of the glider.
     */
    inline void setSeats(Glider::seat newValue)
    {
        _seats=newValue;
    };

    /**
     * @returns the maximum amount of ballast water (in liters) that the glider can hold.
     */
    inline int maxWater()const
    {
        return _maxWater;
    };

    /**
     * Sets the maximum amount of ballast water (in liters) that the glider can hold.
     */
    inline void setMaxWater(int newValue)
    {
        _maxWater=newValue;
    };

    /**
     * @returns a pointer to the polar object belonging to this glider. 
     */
    inline Polar *polar()
    {
        return _polar;
    };

    /**
     * Sets the polar object belonging to this glider. 
     */
    void setPolar(Polar * newPolar);

    /**
     * This funtion loads the glider-data from the config-object.
     * The config object needs to be initialized to the correct
     * section beforehand! It tries to load the glider with id id,
     * and returns true if it succeeds, and false otherwise. 
     */
    bool load(QSettings *config, int id);

    /**
     * Stores the gliderinfo contained in config-object config under ID id. 
     */
    void safe(QSettings *config, int id);

    /**
     * @returns the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
    inline int lastSafeID()const
    {
        return _lastSafeID;
    };

    /**
     * Sets the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
    inline void setID(int newID)
    {
        _lastSafeID=newID;
    };

    /**
     * @returns the name of the co-pilot.
     */
    inline QString coPilot()const
    {
        return _coPilot;
    };

    /**
     * Sets the name of the co-pilot.
     */
    inline void setCoPilot(QString newValue)
    {
        _coPilot=newValue;
    };

private: // Private attributes
    /**
     * Contains the type of glider, for instance Ka-8 or ASH 25
     */
    QString _type;
    QString _registration;
    QString _callsign;
    seat _seats;
    Polar * _polar;
    int _maxWater;
    int _lastSafeID;

    QString _coPilot;
};

#endif
