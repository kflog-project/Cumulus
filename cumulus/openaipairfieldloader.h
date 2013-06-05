/***********************************************************************
**
**   openaipairfieldloader.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class OpenAipAirfieldLoader
 *
 * \author Axel Pauli
 *
 * \brief A class for reading airfield data from OpenAip XML files.
 *
 * A class for reading airfield data from OpenAip XML files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013
 *
 * \version $Id$
 */

#ifndef OpenAip_Airfield_Loader_h_
#define OpenAip_Airfield_Loader_h_

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QPoint>
#include <QString>
#include <QRect>

#include "airfield.h"
#include "projectionbase.h"

class OpenAipAirfieldLoader
{
 public:

  OpenAipAirfieldLoader();

  virtual ~OpenAipAirfieldLoader();

  /**
   * Searches on default places openAIP airfield files and load them. A source
   * can be an original XML file or a compiled version of it. The
   * results are put in the passed list.
   *
   * \param airfieldList All read airfields are stored in this list.
   *
   * \return number of loaded airfield files
   */
  int load( QList<Airfield>& airfieldList );

  /**
   * Creates a compiled file from the passed airfield list beginning at the
   * given start position.
   *
   * \param fileName Name of compiled file
   *
   * \param airfieldList List with airfields
   *
   * \param airfieldListStart Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  bool createCompiledFile( QString& fileName,
                           QList<Airfield>& airfieldList,
                           int airfieldListStart );

  /**
   * Read the content of a compiled file and put it into the related
   * lists.
   *
   * \param path Full name with path of the openAIP binary file
   *
   * \param airfieldList All airports have to be stored in this list
   *
   * \return true (success) or false (error occurred)
   */
  bool readCompiledFile( QString &fileName,
                         QList<Airfield>& airfieldList );


 private:

  /**
   * Gets the header data of a compiled file and put it in the class
   * variables.
   *
   * @param path Full name with path of openAIP binary file
   *
   * @return true (success) or false (error occurred)
   */
  bool setHeaderData( QString &path );

  // header data members of compiled file
  quint32         h_magic;
  qint8           h_fileType;
  quint16         h_fileVersion;
  QDateTime       h_creationDateTime;
  double          h_homeRadius;
  QPoint          h_homeCoord;
  QRect           h_boundingBox;
  ProjectionBase *h_projection;
  bool            h_headerIsValid;

  /** Mutex to ensure thread safety. */
  static QMutex mutex;
};

#endif /* OpenAip_Airfield_Loader_h_ */
