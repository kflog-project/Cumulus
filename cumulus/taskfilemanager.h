/***********************************************************************
**
**   taskfilemanager.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013 Axel Pauli
**
**   Created on: 16.01.2013
**
**   Author: Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class TaskFileManager
 *
 * \author Axel Pauli
 *
 * \brief Task file manager
 *
 * A class which handles the task file load and storage.
 *
 * \date 2013-2014
 *
 * \version $Id$
 */

#ifndef TASK_FILE_MANAGER_H_
#define TASK_FILE_MANAGER_H_

#include <QList>
#include <QString>

#include "flighttask.h"
#include "speed.h"

class TaskFileManager
{
 public:

  /**
   * Default constructor. Sets the task file name to its default value and the
   * TAS to zero.
   */
  TaskFileManager();

  /**
   * Special constructor.
   *
   * \param taskFileName Task file name to be used
   *
   * \param tas TAS to be used.
   */
  TaskFileManager( QString taskFileName, int tas=0 );

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

  bool loadTaskListOld( QList<FlightTask*>& flightTaskList, QString fileName="" );

  bool loadTaskListNew( QList<FlightTask*>& flightTaskList, QString fileName="" );

  /** Saves all tasks contained in the list in the task file.
   *
   * \param flightTaskList List with the flight tasks to be stored
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
