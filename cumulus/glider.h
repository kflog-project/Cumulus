/***********************************************************************
**
**   glider.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003      by André Somers
**                   2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Glider
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Class to handle different glider attributes.
 *
 * An instance of a Glider object contains all the information available
 * on a glider, it's type, it's registration, call sign, polar,
 * single or double seater, maximum water capacity, etc.
 *
 * \date 2003-2015
 *
 * \version 1.1
 *
 */

#ifndef GLIDER_H
#define GLIDER_H

#include <QString>
#include <QSettings>

#include "polar.h"

class Glider
{

public:

    enum seat{ singleSeater=1, doubleSeater=2 };

    Glider();

    /**
     * @returns the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    QString type()const
    {
        return m_type;
    };

    /**
     * Sets the type of the Glider, such as for instance 'ASK 21' or 'Duo Discus', etc.
     */
    void setType(QString newValue)
    {
      m_type = newValue;
    };

    /**
     * @returns the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    QString registration() const
    {
        return m_registration;
    };

    /**
     * Sets the registration of the Glider, such as for instance 'PH-1024' or 'D-8482'.
     */
    void setRegistration(QString newValue)
    {
        m_registration = newValue;
    };

    /**
     * @returns the call sign of the Glider, such as for instance 'UT' or 'DW'.
     */
    QString callSign() const
    {
        return m_callSign;
    };

    /**
     * Sets the callSign of the Glider, such as for instance 'UT' or 'DW'.
     */
    void setCallSign(QString newValue)
    {
        m_callSign = newValue;
    };

    /**
     * @returns the number of seats of the glider.
     */
    Glider::seat seats() const
    {
        return m_seats;
    };

    /**
     * Sets the number of seats of the glider.
     */
    void setSeats(Glider::seat newValue)
    {
        m_seats = newValue;
    };

    /**
     * @returns the maximum amount of ballast water (in liters) that the glider can hold.
     */
    int maxWater()const
    {
        return m_maxWater;
    };

    /**
     * Sets the maximum amount of ballast water (in liters) that the glider can hold.
     */
    void setMaxWater(int newValue)
    {
      m_maxWater = newValue;
    };

    /**
     * @returns a pointer to the polar object belonging to this glider.
     */
    Polar *polar()
    {
      return &m_polar;
    };

    /**
     * Sets the polar object belonging to this glider.
     */
    void setPolar(Polar& newPolar)
    {
      m_polar = newPolar;
    };

    /**
     * This function loads the glider data from the configuration object.
     * The configuration object needs to be initialized to the correct
     * section beforehand! It tries to load the glider with id
     * and returns true if it succeeds, otherwise false.
     */
    bool load(QSettings *config, int id);

    /**
     * Stores the glider data permanently in the configuration object config
     * under id.
     */
    void safe(QSettings *config, int id);

    /**
     * @returns the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
     int lastSafeID()const
    {
        return m_lastSafeID;
    };

    /**
     * Sets the last ID used to save this object, or a temporary value for as-of-yet unsaved objects.
     */
     void setID(int newID)
    {
        m_lastSafeID = newID;
    };

    /**
     * @returns the name of the co-pilot.
     */
    QString coPilot() const
    {
      return m_coPilot;
    };

    /**
     * Sets the name of the co-pilot.
     */
    void setCoPilot(QString newValue)
    {
        m_coPilot = newValue;
    };

    /**
     * Sets the user selection flag.
     */
    void setSelectionFlag( const bool flag )
      {
	m_isSelected = flag;
      };

    /**
     * @returns the user selection flag.
     */
    bool getSelectionFlag() const
      {
	return m_isSelected;
      };

private:

    /**
     * Contains the type of glider, for instance Ka-8 or ASH 25
     */
    QString m_type;
    QString m_registration;
    QString m_callSign;
    QString m_coPilot;

    seat m_seats;
    Polar m_polar;
    int m_maxWater;
    int m_lastSafeID;

    /**
     * Selection flag. Set to true if glider is selected by the user.
     */
    bool m_isSelected;
};

#endif
