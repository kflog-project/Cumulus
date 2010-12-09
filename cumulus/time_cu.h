/***********************************************************************
**
**   time_cu.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
****************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * \class Time
 *
 * \author Axel Pauli
 *
 * \brief This class handles the time unit to be used for display.
 *
 * \date 2009
 */

#ifndef TIME_H
#define TIME_H

class Time
{
  public:

  /**
   * The time unit enumeration list contains the units that apply to time.
   */
  enum timeUnit {
      utc   = 0,   /** UTC */
      local = 1, /** Local */
  };

  Time() {};
  ~Time() {};

  static void setUtc()
  {
    _timeUnit = utc;
  };

  static void setLocal()
  {
    _timeUnit = local;
  };

  /**
   * Sets the time unit.
   */
  static void setUnit( const timeUnit unit )
  {
    _timeUnit = unit;
  };

  /**
   * Gets the time unit.
   */
  static timeUnit getTimeUnit()
  {
    return _timeUnit;
  };

  private:

  /** static element containing the current time unit */
  static timeUnit _timeUnit;
};

#endif /* TIME_H */
