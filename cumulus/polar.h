/***************************************************************************
                          polar.cpp  -  description
                             -------------------
    begin                : Okt 18 2002
    copyright            : (C) 2002      by Eggert Ehmke
                               2008-2015 by Axel Pauli

    email                : kflog.cumulus@gmail.com

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
 * \date 2002-2021
 *
 * \version 1.2
 *
 */

#pragma once

#include <QString>
#include <QWidget>

#include "speed.h"

class Polar
{
 public:

  Polar();

  Polar( const QString& name,
	 const Speed& v1, const Speed& w1,
	 const Speed& v2, const Speed& w2,
	 const Speed& v3, const Speed& w3,
	 double wingArea,
	 int emptyWeight,
	 int grossWeight,
	 int addLoad=0 );

  Polar(const Polar&);

  virtual ~Polar();

  /**
   * recalculates the polar data
   */
  void recalculatePolarData();

  /**
   * set additional load as Kg
   * set additional water as liters or Kg
   * set bug factor as percentage
   */
  void setLoad (int addLoad, int water, int bugs);

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
  void drawPolar( QWidget* view, const Speed& wind,
                  const Speed& lift, const Speed& mc) const;

  QString name() const
  {
      return _name;
  };

  void setName( const QString& name )
  {
    _name = name;
  }

  Speed v1() const
  {
      return _v1;
  };

  void setV1( const Speed& speed )
  {
    _v1 = speed;
  }

  Speed w1() const
  {
      return _w1;
  };

  void setW1( const Speed& speed )
  {
    _w1 = speed;
  }

  Speed v2() const
  {
      return _v2;
  };

  void setV2( const Speed& speed )
  {
    _v2 = speed;
  }

  Speed w2() const
  {
      return _w2;
  };

  void setW2( const Speed& speed )
  {
    _w2 = speed;
  }

  Speed v3() const
  {
      return _v3;
  };

  void setV3( const Speed& speed )
  {
    _v3 = speed;
  }

  Speed w3() const
  {
      return _w3;
  };

  void setW3( const Speed& speed )
  {
    _w3 = speed;
  }

  int emptyWeight() const
  {
      return _emptyWeight;
  };

  int grossWeight() const
  {
      return _grossWeight;
  };

  void setGrossWeight(int newValue)
  {
      _grossWeight = newValue;
  };

  double wingArea() const
  {
      return _wingArea;
  };

  void setWingArea(double newValue)
  {
      _wingArea = newValue;
  };

  int addLoad() const
  {
      return _addLoad;
  };

  void setAddLoad( int addLoad )
  {
    setLoad( addLoad, _water, _bugs );
  };


  int water() const
  {
      return _water;
  };

  void setWater( int water )
  {
    setLoad( _addLoad, water, _bugs );
  };

  int bugs() const
  {
      return _bugs;
  };

  void setBugs( int bugs )
  {
    setLoad( _addLoad, _water, bugs );
  };

  int seats() const
  {
      return _seats;
  };

  void setSeats(int seats)
  {
      _seats=qMax(1, qMin(2, seats));
  };

  int maxWater() const
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
  int    _emptyWeight;
  int    _grossWeight;
  int    _addLoad;
  double _wingArea;
  int    _seats;
  int    _maxWater;
};
