/***********************************************************************
**
**   TaskFileManager.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2013-2018 Axel Pauli
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
 * \class TaskFileManager
 *
 * \author Axel Pauli
 *
 * \brief Task file manager
 *
 * A class which handles the task files load and storage.
 *
 * \date 2013-2018
 *
 * \version 1.4
 */

#ifndef TaskFileManager_h
#define TaskFileManager_h

#include <QDir>
#include <QList>
#include <QString>
#include <QStringList>

#include "flighttask.h"
#include "generalconfig.h"
#include "speed.h"

class TaskFileManager
{
 public:

  /**
   * TAS constructor.
   *
   * \param tas TAS to be used.
   */
  TaskFileManager( const int tas=0 );

  /**
   * Check, if an task file upgrade has to be done.
   */
  void check4Upgrade();

  /**
   * Loads an old Task list file.
   */
  bool loadOldTaskList( QList<FlightTask*>& flightTaskList,
                        QString fileName );

  /**
   * Creates the task file directory
   *
   * \return true in case of success otherwise false
   */
  bool createTaskDirectory()
  {
    QDir dir;
    return dir.mkpath( m_taskFileDirectory );
  }

  /**
   * Loads all tasks found in the task file directory into the list.
   *
   * \param flightTaskList List which shall store the read flight tasks
   *
   * \return true in case of success otherwise false
   *
   */
  bool loadTaskList( QList<FlightTask*>& flightTaskList );

  /**
   * Reads a single task file and returns a FlightTask object.
   *
   * \param Pure name of the task to be read.
   *
   * \return FlightTask object in case of success other wise 0.
   */
  FlightTask* readTaskFile( QString taskName );

  /**
   * Writes a single task file.
   *
   * \param task Flight task object to be stored.
   *
   * \return true in case of success otherwise false.
   */
  bool writeTaskFile( FlightTask *task );

  /**
   * Remove a task file.
   *
   * \param taskName Name of the task to be removed.
   *
   */
  void removeTaskFile( QString taskName );

  /**
   * Returns all task names of a flight task list.
   *
   * \return A string list with all found task names. The list can be empty,
   *         if no task file exist or in error case.
   */
  QStringList getTaskListNames();

  /** Saves all tasks contained in the list in the task file.
   *
   * \param flightTaskList List with the flight tasks to be stored.
   *
   * \return true in case of success otherwise false
   */
  bool saveTaskList( QList<FlightTask*>& flightTaskList );

  /**
   * Sets a new task directory.
   *
   * @param dirPath New Task directory name to be used.
   */
  void setTaskFileDirectory( QString dirPath )
  {
    m_taskFileDirectory = dirPath;
    createTaskDirectory();
  };

  /**
   * Returns the current used task directory name.
   *
   * @return The current used task directory name.
   */
  QString taskTaskFileDirectory() const
  {
    return m_taskFileDirectory;
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

  /** Cumulus task's file directory. */
  QString m_taskFileDirectory;

  /** True airspeed */
  Speed m_tas;
};

#endif /* TaskFileManager_h */
