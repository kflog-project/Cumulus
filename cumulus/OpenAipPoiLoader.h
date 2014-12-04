/***********************************************************************
**
**   OpenAipPoiLoader.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class OpenAipPoiLoader
 *
 * \author Axel Pauli
 *
 * \brief A class for reading openAIP POI data from OpenAIP XML files.
 *
 * A class for reading openAIP POI data from OpenAIP XML files provided by
 * Butterfly Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013-2014
 *
 * \version 1.0
 */

#ifndef OpenAip_Poi_Loader_h_
#define OpenAip_Poi_Loader_h_

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QPoint>
#include <QSet>
#include <QString>
#include <QRect>

#include "airfield.h"
#include "projectionbase.h"
#include "radiopoint.h"
#include "singlepoint.h"

class OpenAipPoiLoader
{
 public:

  OpenAipPoiLoader();

  virtual ~OpenAipPoiLoader();

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
   * Searches on default places openAIP navAid files and load them. A source
   * can be an original XML file or a compiled version of it. The
   * results are appended to the passed list.
   *
   * \param navAidList All read navAids have to be appended to this list.
   *
   * \param readSource If true the source files have to be read instead of
   * compiled sources.
   *
   * \return number of loaded navAid files
   */
  int load( QList<RadioPoint>& navAidList, bool readSource=false );

  /**
   * Searches on default places openAIP hotspot files and load them. A source
   * can be an original XML file or a compiled version of it. The
   * results are appended to the passed list.
   *
   * \param hotspotList All read hotspots have to be appended to this list.
   *
   * \param readSource If true the source files have to be read instead of
   * compiled sources.
   *
   * \return number of loaded hotspot files
   */
  int load( QList<SinglePoint>& hotspotList, bool readSource=false );

  /**
   * Creates a compiled file from the passed airfield list beginning at the
   * given start position and ending at the end of the list.
   *
   * \param fileName Name of the compiled file
   *
   * \param airfieldList List with airfield records
   *
   * \param listBegin Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  bool createCompiledFile( QString& fileName,
                           QList<Airfield>& airfieldList,
                           int listBegin );


  /**
   * Creates a compiled file from the passed navaid list beginning at the
   * given start position and ending at the end of the list.
   *
   * \param fileName Name of the compiled file
   *
   * \param navAidList List with navAid records
   *
   * \param listBegin Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  bool createCompiledFile( QString& fileName,
                           QList<RadioPoint>& navAidList,
                           int listBegin );

  /**
   * Creates a compiled file from the passed single point list beginning at the
   * given start position and ending at the end of the list.
   *
   * \param fileName Name of the compiled file
   *
   * \param spList List with single point records
   *
   * \param listBegin Begin index in passed list
   *
   * \return true in case of success otherwise false
   */
  bool createCompiledFile( QString& fileName,
                           QList<SinglePoint>& spList,
                           int listBegin );

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

  /**
   * Read the content of a compiled file and append it to the passed list.
   *
   * \param path Full name with path of the openAIP binary file
   *
   * \param navAidList All read navaids have to be appended to this list.
   *
   * \return true (success) or false (error occurred)
   */
  bool readCompiledFile( QString &fileName,
                         QList<RadioPoint>& navAidList );

  /**
   * Read the content of a compiled file and append it to the passed list.
   *
   * \param path Full name with path of the openAIP binary file
   *
   * \param spList All read single points have to be appended to this list.
   *
   * \return true (success) or false (error occurred)
   */
  bool readCompiledFile( QString &fileName,
                         QList<SinglePoint>& spList );

  /**
   * Reads the header data of a compiled file and put them in the class
   * variables.
   *
   * \param path Full name with path of openAIP binary file
   *
   * \param fileType The expected file type
   *
   * \param fileVersion The expected file version
   *
   * \return true (success) or false (error occurred)
   */
  bool getHeaderData( QString &path, QString fileType, int fileVersion );

 private:

  /**
   * Reads the header data of a compiled file from the opened data stream and
   * put them in the class variables.
   *
   * \param dataStream A data stream to the openAIP binary file.
   *
   * \param fileType The expected file type
   *
   * \param fileVersion The expected file version
   *
   * \return true (success) or false (error occurred)
   */
  bool readHeaderData( QDataStream& dataStream, QString fileType, int fileVersion );

  /**
   * Header data members of compiled openAIP file.
   */
  class HeaderData
  {
    public:

      HeaderData() :
	h_magic(0),
	h_fileType(),
	h_fileVersion(0),
	h_homeRadius(0.0),
	h_runwayLengthFilter(0.0),
	h_projection(static_cast<ProjectionBase *> (0)),
	h_headerIsValid(false)
	{
	};

      ~HeaderData()
	{
	  if( h_projection )
	    {
	      delete h_projection;
	    }
	};

      quint32         h_magic;
      QByteArray      h_fileType;
      quint8          h_fileVersion;
      QDateTime       h_creationDateTime;
      float           h_homeRadius;
      float           h_runwayLengthFilter;
      QPoint          h_homeCoord;
      QRect           h_boundingBox;
      ProjectionBase* h_projection;

      /** Flag to signal that set header data are valid. */
      bool h_headerIsValid;
  };

  HeaderData m_hd;

  /** Mutex to ensure thread safety. */
  static QMutex m_mutex;
};

#endif /* OpenAip_Poi_Loader_h_ */
