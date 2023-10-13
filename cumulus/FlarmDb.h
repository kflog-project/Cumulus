/***********************************************************************
**
**   FlarmDb.h
**
**   Created on: 12.10.2023
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2023 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class FlarmDb
 *
 * \author Axel Pauli
 *
 * \brief Class for reading and accessing special data.
 *
 * Class for reading and accessing special data.
 *
 * \see https://www.flarmnet.org
 *
 * \date 2023
 *
 * \version 1.0
 */

#pragma once

#include <QHash>
#include <QMutex>
#include <QString>

class FlarmDb
{
  public:

  /**
   * Constructor
   */
  FlarmDb()
  {
  };

  /**
   * Destructor
   */
  virtual ~FlarmDb()
  {
  };

  /**
   * Loads data from a file into a hash dictionary.
   *
   * @returns The number of successfully loaded items
   *
   */
  static int loadData();

  /**
   * unloads the data hash dictionary.
   *
   */
  static void unloadData();

  /**
   * Read the content of a compiled file and put it into the passed
   * list.
   *
   * @param id identifier key
   * @param data related data fetched by key.
   * @return true (success) or false (nothing found)
   */
  static bool getData( int id, QStringList &data );

 private:

  /**
   * A hash map containing data
   */
  static QHash<uint, QString> m_datamap;

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;
};

/******************************************************************************/

#include <QThread>

/**
* \class FlarmDbThread
*
* \author Axel Pauli
*
* \brief Class to read a data file in an extra thread.
*
* \date 2023
*
* \version 1.0
*/

class FlarmDbThread : public QThread
{
  Q_OBJECT

 public:

  FlarmDbThread( QObject *parent=0 );

  virtual ~FlarmDbThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();
};

