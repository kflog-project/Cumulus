/***********************************************************************
 **
 **   taskpointtypes.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2013-2016 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
 * \date 1999-2016
 *
 * \version 1.0
 */
class TaskPointTypes
{
  public:
  /**
   * The possible task point types according IGC declaration.
   */
  enum TaskPointType { Unknown = 0, Start = 2, Turn = 4, Finish = 8 };
};

#endif /* TASK_POINT_TYPES_H */
