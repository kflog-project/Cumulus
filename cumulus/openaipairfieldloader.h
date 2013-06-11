/***********************************************************************
**
**   openaipairfieldloader.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
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
 * \brief A class for reading airfield data from OpenAIP XML files.
 *
 * A class for reading airfield data from OpenAIP XML files provided by Butterfly
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

#include <QByteArray>
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
   * results are appended to the passed list.
   *
   * \param airfieldList All read airfields have to be appended to this list.
   *
   * \param readSource If true the source files have to be read instead of
   * compiled sources.
   *
   * \return number of loaded airfield files
   */
  int load( QList<Airfield>& airfieldList, bool readSource=false );

  /**
   * Creates a compiled file from the passed airfield list beginning at the
   * given start position and ending at the end of the list.
   *
   * \param fileName Name of the compiled file
   *
   * \param airfieldList List with airfield records
   *
   * \param airfieldListStart Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  bool createCompiledFile( QString& fileName,
                           QList<Airfield>& airfieldList,
                           int airfieldListStart );

  /**
   * Read the content of a compiled file and append it to the passed list.
   *
   * \param path Full name with path of the openAIP binary file
   *
   * \param airfieldList All read airfields have to be appended to this list.
   *
   * \return true (success) or false (error occurred)
   */
  bool readCompiledFile( QString &fileName,
                         QList<Airfield>& airfieldList );

 private:

  /**
   * Reads the header data of a compiled file and put it in the class
   * variables.
   *
   * \param path Full name with path of openAIP binary file
   *
   * \return true (success) or false (error occurred)
   */
  bool setHeaderData( QString &path );

  // header data members of compiled file
  quint32         h_magic;
  QByteArray      h_fileType;
  quint8          h_fileVersion;
  QDateTime       h_creationDateTime;
  double          h_homeRadius;
  QPoint          h_homeCoord;
  QRect           h_boundingBox;
  ProjectionBase* h_projection;

  /** Flag to signal that set header data are valid. */
  bool h_headerIsValid;

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;
};

/******************************************************************************/

#include <QThread>

/**
* \class OpenAipThread
*
* \author Axel Pauli
*
* \brief Class to read OpenAIP data files in an extra thread.
*
* This class can read, parse and filter OpenAIP airfield data files and store
* the content in a binary format. All work is done in an extra thread.
* The results are returned via the signal \ref loadedList.
*
* \date 2013
*
* \version $Id$
*/

class OpenAipThread : public QThread
{
  Q_OBJECT

 public:

  OpenAipThread( QObject *parent=0, bool readSource=false );

  virtual ~OpenAipThread();

 protected:

  /**
   * That is the main method of the thread.
   */
  void run();

 signals:

  /**
  * This signal emits the results of the OpenAIP load. The receiver slot is
  * responsible to delete the dynamic allocated list in every case.
  *
  * \param ok              The result of the load action.
  * \param airfieldList    The list with the airfield data
  *
  */
  void loadedList( bool ok, QList<Airfield>* airfieldList );

 private:

  bool m_readSource;
};

#endif /* OpenAip_Airfield_Loader_h_ */
