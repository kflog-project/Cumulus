/***********************************************************************
**
**   openaip.h
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
 * \class OpenAip
 *
 * \author Axel Pauli
 *
 * \brief A class for reading data from OpenAip XML files.
 *
 * A class for reading data from OpenAip XML files provided by Butterfly
 * Avionics GmbH. The data are licensed under the CC BY-NC-SA license.
 *
 * See here for more info: http://www.openaip.net
 *
 * \date 2013
 *
 * \version $Id$
 */

#ifndef OpenAip_h
#define OpenAip_h

#include <QList>
#include <QString>
#include <QXmlStreamReader>

#include "airfield.h"
#include "radiopoint.h"

class OpenAip
{
 public:

  OpenAip();

  virtual ~OpenAip();

  /**
   * Opens the passed file and looks, which kind of OpenAip data is provided.
   *
   * \param filename File containing OpenAip XML data definitions.
   *
   * \param dataItem The second root element of the file after the OPENAIP tag.
   *
   * \return true as success otherwise false
   */
  bool getRootElement( QString fileName, QString& dataItem );

  /**
   * Reads in a navigation aid file provided as open aip xml format.
   *
   * \param filename File containing navigation aid definitions
   *
   * \return true as success otherwise false
   */
  bool readNavAids( QString fileName, QList<RadioPoint>& navAidList, QString& errorInfo );

  /**
   * Reads in a hotspot file provided as open aip xml format.
   *
   * \param filename File containing navigation aid definitions
   *
   * \return true as success otherwise false
   */
  bool readHotspots( QString fileName, QList<SinglePoint>& hotspotList, QString& errorInfo );

  /**
   * Reads in a airfield file provided as open aip xml format.
   *
   * \param filename File containing navigation aid definitions
   *
   * \return true as success otherwise false
   */
  bool readAirfields( QString fileName, QList<Airfield>& airfieldList, QString& errorInfo );

  /**
   * Upper and lower the words in the passed string.
   *
   * \param name The name to be processed.
   */
  void upperLowerName( QString& name );

  /**
   * Create a short name by removing undesired characters.
   *
   * \param name The name to be shorten.
   *
   * \return new short name 8 characters long
   */
  QString shortName( QString& name );

 private:

  bool readNavAidRecord( QXmlStreamReader& xml, RadioPoint& rp );

  bool readGeoLocation( QXmlStreamReader& xml, SinglePoint& sp );

  bool readRadio( QXmlStreamReader& xml, RadioPoint& rp );

  bool readHotspotRecord( QXmlStreamReader& xml, SinglePoint& sp );

  bool readAirfieldRecord( QXmlStreamReader& xml, Airfield& af );

  bool readAirfieldRadio( QXmlStreamReader& xml, Airfield& af );

  bool readAirfieldRunway( QXmlStreamReader& xml, Airfield& af );

  /**
   * Converts a string number with unit to an integer value.
   *
   * \param number The number to be converted
   *
   * \param unit The unit of the number, can be "M" or "FT"
   *
   * \param result The calculated integer value
   *
   * \return true in case of success otherwise false
   */
  bool getUnitValueAsInteger( const QString number, const QString unit, int& result );
};

#endif /* OpenAip_h */
