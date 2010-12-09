/***************************************************************************
                          polar.cpp  -  description
                             -------------------
    begin                : Okt 18 2002
    copyright            : (C) 2002      by Eggert Ehmke
                               2008-2010 by Axel Pauli

    email                : eggert.ehmke@berlin.de, axel@kflog.org

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
 * \class Polar
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Class for glider polar calculations and drawing.
 *
 * \date 2002-2010
 *
 */


#ifndef POLAR_H
#define POLAR_H

#include <QWidget>
#include <QString>

#include "speed.h"

class Polar
{
public:

    Polar();

    Polar(const QString& name,const Speed& v1, const Speed& w1,
          const Speed& v2, const Speed& w2,
          const Speed& v3, const Speed& w3,
          double wingload, double wingarea,
          double emptyWeight, double grossWeight);

    Polar (const Polar&);

    virtual ~Polar();

    /**
     * set bug factor as percentage
     */
    void setWater (int water, int bugs);

    Speed getSink (const Speed& speed) const;

    /**
     * calculate best airspeed for given wind, lift and McCready value;
     */
    Speed bestSpeed (const Speed& wind, const Speed& lift, const Speed& mc) const;

    /**
     * calculate best glide ratio
     */
    double bestLD (const Speed& speed, const Speed& wind, const Speed& lift) const;

    /** draw a graphical polar on the given widget;
     * draw glide path according to lift, wind and McCready value
     */
    void drawPolar (QWidget* view, const Speed& wind,
                    const Speed& lift, const Speed& mc) const;

    QString name()const
    {
        return _name;
    };

    Speed v1()const
    {
        return _v1;
    };

    Speed w1()const
    {
        return _w1;
    };

    Speed v2()const
    {
        return _v2;
    };

    Speed w2()const
    {
        return _w2;
    };

    Speed v3()const
    {
        return _v3;
    };

    Speed w3()const
    {
        return _w3;
    };

    double emptyWeight()const
    {
        return _emptyWeight;
    };

    double grossWeight()const
    {
        return _grossWeight;
    };

    void setGrossWeight(double newValue)
    {
        _grossWeight = newValue;
    };

    int water()const
    {
        return _water;
    };

    int bugs()const
    {
        return _bugs;
    };

    int seats()const
    {
        return _seats;
    };

    void setSeats(int seats)
    {
        _seats=qMax(1, qMin(2, seats));
    };

    int maxWater()const
    {
        return _maxWater;
    };

    void setMaxWater(int liters)
    {
        _maxWater=liters;
    };

    /**
     * compares two entries to sort list by name.
     */
    static bool lessThan(const Polar &p1, const Polar &p2)
    {
      return p1._name < p2._name;
    };

private:

    /** Glider type */
    QString _name;

    /** Data points of glider polar */
    Speed   _v1;
    Speed   _w1;
    Speed   _v2;
    Speed   _w2;
    Speed   _v3;
    Speed   _w3;

    /** these are the parabola parameters used for approximation */
    double _a, _aa, _b, _bb, _c, _cc;

    int    _water;
    int    _bugs;
    double _emptyWeight;
    double _grossWeight;
    int    _seats;
    int    _maxWater;
};

#endif
