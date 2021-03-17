/***********************************************************************
**
**   XCSoar.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2021 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class XCSoar
 *
 * \author Axel Pauli
 *
 * \brief An interface class to XCSoar
 *
 * A class for handling XCSoar data
 *
 * \date 2021
 *
 * \version 1.0
 */

#pragma once

#include <QString>
#include "flighttask.h"
#include "taskpointtypes.h"

class XCSoar
{
 public:

  XCSoar()
  {
  }

  virtual ~XCSoar()
  {
  }

  /**
   * Reads a single XCSoar task file created by WeGlide and returns a
   * FlightTask object.
   *
   * \param fileName of task with extention .tsk
   *
   * \param errorInfo info in case of error.
   *
   * \return FlightTask object in case of success other wise 0.
   */

  static FlightTask* reakTaskFile( QString fileName, QString& errorInfo );

 private:

  /**
   * Map a point type to the related cumulus type.
   *
   * @param type
   * @return
   */
  static TaskPointTypes::TaskPointType mapPointType( QString& type );
};
