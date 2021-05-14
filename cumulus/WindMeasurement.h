/***********************************************************************
**
**   WindMeasurement.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2007-2021 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <QTime>

#include "altitude.h"
#include "vector.h"

/**
 * \class WindMeasurement
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Data container for a single wind measurement.
 *
 * \date 2002-2021
 */
class WindMeasurement
{
public:

  WindMeasurement() :
    quality( 0.0 ),
    altitude( -1 )
  {}

  Vector vector;
  float quality; // 1...5
  QTime time; // time of measurement creation
  int altitude; // altitude in meters.

  bool operator < (const WindMeasurement& other) const;

  static bool lessThan(const WindMeasurement &w1, const WindMeasurement &w2)
  {
    return w1.altitude < w2.altitude;
  }
};
