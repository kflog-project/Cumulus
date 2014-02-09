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
#define FILE_VERSION_AIRSPACE_C 206

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>

#include "airspace.h"
#include "basemapelement.h"

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
   * Searches on default places for OpenAir and OpenAip airspace files.
   * That can be source files or compiled versions of them.
   *
   * @returns The number of successfully loaded files
   *
   * @param list The list where the Airspace objects should be added from the
   *        read files.
   */
  static int loadAirspaces( QList<Airspace*>& list );

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
                              ProjectionBase** projection );

  /**
   * Initialize a mapping from an airspace type string to the Cumulus integer type.
   */
  static QMap<QString, BaseMapElement::objectType>
         initializeAirspaceTypeMapping(const QString& path);

  /**
   * Checks, if a Cumulus airspace type name is contained in the airspace
   * type map.
   *
   * \return True in case of success otherwise false
   */
  static bool isAirspaceBaseTypeKnown( const QString& type )
  {
    if( m_airspaceTypeMap.isEmpty() )
      {
        loadAirspaceTypeMapping();
      }

    return m_airspaceTypeMap.contains( type );
  }

  /**
   * Maps a Cumulus airspace type name to its integer code.
   *
   * \param type airspace base type name
   *
   * \return short airspace type name or empty string, if mapping fails
   */
  static BaseMapElement::objectType mapAirspaceBaseType( const QString& type )
  {
    if( m_airspaceTypeMap.isEmpty() )
      {
        loadAirspaceTypeMapping();
      }

    return m_airspaceTypeMap.value( type, BaseMapElement::AirUkn );
  }

  /**
   * Reports, if a airspace object is already loaded or not.
   *
   * \param id airspace identifier
   *
   * \return true in case of contained otherwise false
   */
  static bool isAirspaceKnown( const int id )
  {
    return m_airspaceDictionary.contains( id );
  };

  /**
   * Adds an airspace identifier to the airspace dictionary.
   *
   * \param id airspace identifier
   *
   * \return true in case of added otherwise false
   */
  static bool addAirspaceIdentifier( const int id )
  {
    if( m_airspaceDictionary.contains( id ) )
      {
        return false;
      }

    m_airspaceDictionary.insert(id);
    return true;
  }

 private:

  /**
   * Creates a mapping from a string representation of the supported
   * airspace types in Cumulus to their integer codes.
   *
   * \return Map of airspace items as key value pair
   */
  static void loadAirspaceTypeMapping();

  /**
   * A map containing Cumulus's supported airspace types as key value pair.
   */
  static QMap<QString, BaseMapElement::objectType> m_airspaceTypeMap;

  /**
   * Contains all read airspaces to avoid duplicates.
   */
  static QSet<int> m_airspaceDictionary;

};

#endif /* AIRSPACE_HELPER_H_ */
