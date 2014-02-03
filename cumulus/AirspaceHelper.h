/***********************************************************************
**
**   AirspaceHelper.h
**
**   Created on: 03.02.2014
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class AirspaceHelper
 *
 * \author Axel Pauli
 *
 * \brief Helper class for airspaces
 *
 * This class contains methods for airspace compilation, storage and reading.
 *
 * \date 2014
 *
 * \version $Id$
 */

#ifndef AIRSPACE_HELPER_H_
#define AIRSPACE_HELPER_H_

// type definition for compiled airspace files
#define FILE_TYPE_AIRSPACE_C 0x61

// version used for files created from OpenAir data
#define FILE_VERSION_AIRSPACE_C 205

#include <QDateTime>
#include <QList>
#include <QString>

#include "airspace.h"

class ProjectionBase;

class AirspaceHelper
{
  public:

  /**
   * Constructor
   */
  AirspaceHelper();

  /**
   * Destructor
   */
  virtual ~AirspaceHelper();

  /**
   * Read the content of a compiled file and put it into the passed
   * list.
   *
   * @param path Full name with path of OpenAir binary file
   * @param list All airspace objects have to be stored in this list
   * @return true (success) or false (error occurred)
   */
  static bool readCompiledFile( QString &path, QList<Airspace*>& list );

  /**
   * Creates a compiled file from the passed airspace list beginning at the
   * given start position and ending at the end of the list.
   *
   * \param fileName Name of the compiled file
   *
   * \param airspaceList List with airspace records
   *
   * \param airspaceListStart Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  static bool createCompiledFile( QString& fileName,
                                  QList<Airspace*>& airspaceList,
                                  int airspaceListStart );

  /**
   * Get the header data of a compiled file and put it in the class
   * variables.
   *
   * \param path Full name with path of OpenAir binary file
   *
   * \param creationDateTime Date and time of file creation
   *
   * \param ProjectionBase stored projection type
   *
   * \returns true (success) or false (error occured)
   */
  static bool readHeaderData( QString &path,
                              QDateTime& creationDateTime,
                              ProjectionBase* projection );
};

#endif /* AIRSPACE_HELPER_H_ */
