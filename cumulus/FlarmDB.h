/***********************************************************************
**
**   FlarmDB.h
**
**   Created on: 12.10.2023
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2023-2026 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class FlarmDB
 *
 * \author Axel Pauli
 *
 * \brief Class for reading and accessing Flarm datadase data.
 *
 * Class for reading and Flarm datadase data.
 *
 * \see https://www.flarmnet.org
 * \see https://ddb.glidernet.org
 *
 * \date 2023-2026
 *
 * \version 1.2
 */

#pragma once

#include <QHash>
#include <QMutex>
#include <QString>
#include <QStringList>

class FlarmDB
{
  public:

  /**
   * Constructor
   */
  FlarmDB()
  {
  };

  /**
   * Destructor
   */
  virtual ~FlarmDB()
  {
  };

  /**
   * Loads data from a FlarmNet file into a hash dictionary.
   *
   * @returns The number of successfully loaded items
   *
   */
  static int loadFNData( QHash<uint, QString>& dict );

  /**
   * Loads data from an OGN file into a hash dictionary.
   *
   * @returns The number of successfully loaded items
   *
   */
  static int loadOGNData( QHash<uint, QString>& dict );

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

  /**
   * Get number of FlarmDB records.
   */
  static int getRecords()
  {
    return m_datamap.size();
  }

  /**
   * Count filtered elements and return them.
   */
  static int applyFilter( QString& filter );

  /**
   * Return data dictionary
   */
  static QHash<uint, QString>& dictionary()
  {
    return m_datamap;
  }

 private:

  /**
   * Check item for usage.
   */
  static bool checkFilter( QString& item )
  {
    if( filterList.size() > 0 )
      {
        // Check item for usage
        for( int i=0; i < filterList.size(); i++ )
          {
            if( item.startsWith( filterList.at(i) ) == true )
              {
                return true;
              }
          }

        return false;
      }
    else
      {
        return true;
      }
  }

  /**
   * A hash map containing data
   */
  static QHash<uint, QString> m_datamap;

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;

  /**
   * Filterlist
   */
  static QStringList filterList;
};

/******************************************************************************/

#include <QThread>

/**
* \class FlarmDBThread
*
* \author Axel Pauli
*
* \brief Class to read a data file in an extra thread.
*
* \date 2023-2026
*
* \version 1.1
*/

class FlarmDBThread : public QThread
{
  Q_OBJECT

 public:

  FlarmDBThread( QObject *parent=0 );

  virtual ~FlarmDBThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

  signals:

   /**
   * This signal emits the number of loaded FlarmNet records
   *
   * \param loadedRecords  The number of loaded records
   */
   void loadedRecords( int loadedRecords );
};
