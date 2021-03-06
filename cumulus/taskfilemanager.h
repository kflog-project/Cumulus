/***********************************************************************
**
**   taskfilemanagerold.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2016 Axel Pauli
**
**   Created on: 16.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class TaskFileManagerOld
 *
 * \author Axel Pauli
 *
 * \brief Task file manager
 *
 * A class which handles the task file load and storage.
 *
 * \date 2013-2016
 *
 * \version 1.3
 */

#ifndef TASK_FILE_MANAGER_OLD_H_
#define TASK_FILE_MANAGER_OLD_H_

#include <QList>
#include <QString>
#include <QStringList>

#include "flighttask.h"
#include "speed.h"

class TaskFileManagerOld
{
 public:

  /**
   * Default constructor. Sets the task file name to its default value and the
   * TAS to zero.
   */
  TaskFileManagerOld();

  /**
   * Special constructor.
   *
   * \param taskFileName Task file name to be used
   *
   * \param tas TAS to be used.
   */
  TaskFileManagerOld( QString taskFileName, int tas=0 );

  /**
   * Loads all tasks found in the task file into the list.
   *
   * \param flightTaskList List which shall store the read flight tasks
   *
   * \param fileName File name of task, if empty a default is used.
   *
   * \return true in case of success otherwise false
   *
   */
  bool loadTaskList( QList<FlightTask*>& flightTaskList, QString fileName="" );

  /**
   * Do the same as method loadTaskList().
   */
  bool loadTaskListNew( QList<FlightTask*>& flightTaskList, QString fileName="" );

  /**
   * Returns all task names of a flight task list.
   *
   * \param fileName File name of task, if empty a default is used.
   *
   * \return A string list with all found task names. The list can be empty,
   *         if no task file exist or in error case.
   */
  QStringList getTaskListNames( QString fileName="" );

  /**
   * Loads a single flight task, selected by its name from a file.
   *
   * \param[in] taskName Name of flight task to be loaded.
   *
   * \param[in] fileName Name of task file to be used.
   *
   * \return FlightTask object in case of success otherwise NULL. Note, the
   *         returned object is created on the heap and the memory management
   *         is in responsible by the caller.
   */
  FlightTask* loadTask( QString taskName, QString fileName="" );

  /** Saves all tasks contained in the list in the task file.
   *
   * \param flightTaskList List with the flight tasks to be stored.
   *
   * \param fileNameFile name of task, if empty a default is used.
   *
   * \return true in case of success otherwise false
   */
  bool saveTaskList( QList<FlightTask*>& flightTaskList, QString fileName="" );

  /**
   * Sets a new task file name.
   *
   * @param fileName New Task file name to be used.
   */
  void setTaskFileName( QString fileName )
  {
    m_taskFileName = fileName;
  };

  /**
   * Returns the current used task file name.
   *
   * @return The current used task file name.
   */
  QString taskFileName() const
  {
    return m_taskFileName;
  }
  /**
   * Tas setter
   *
   * @param tas New tas to be used.
   */
  void setTas( const Speed& tas )
  {
    m_tas = tas;
  };

  /**
   * TAS getter
   *
   * @return Current set TAS.
   */
  const Speed& tas() const
  {
    return m_tas;
  };

 private:

  /** Global task file name. */
  QString m_taskFileName;

  /** True airspeed */
  Speed m_tas;
};

#endif /* TASK_FILE_MANAGER_H_ */
