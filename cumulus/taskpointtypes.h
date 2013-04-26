/***********************************************************************
 **
 **   taskpointtypes.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef TASK_POINT_TYPES_H
#define TASK_POINT_TYPES_H

/**
 * \class TaskPointTypes
 *
 * \author Heiner Lamprecht, Florian Ehinger, André Somers, Axel Pauli
 *
 * \brief Kinds of a task point.
 *
 * Definitions of possible task point types.
 *
 * \date 1999-2013
 *
 * \version $Id$
 */
class TaskPointTypes
{
  public:
  /**
   * The possible task point types.
   */
  enum TaskPointType { NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                       End = 8, FreeP = 16, Landing = 32 };
};

#endif /* TASK_POINT_TYPES_H */
