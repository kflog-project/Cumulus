/***********************************************************************
 **
 **   airport.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef AIRPORT_H
#define AIRPORT_H

#include <QString>
#include <QHash>
#include <QStringList>

#include "radiopoint.h"
#include "runway.h"

/**
 * @short Class to handle airports.
 *
 * This class is used for handling airports. The object can be one of
 * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
 * CivHeliport, MilHeliport, AmbHeliport.
 *
 * @see BaseMapElement#objectType
 */

class Airport : public RadioPoint
{
 public:

  /**
   * Used to define the surface of a runway.
   */
  enum SurfaceType {Unknown = 0, Grass = 1, Asphalt = 2, Concrete = 3};

  /**
   * Creates a new Airport-object.
   * @param  name  The name
   * @param  icao  The icao-name
   * @param  abbr  The abbreviation, used for the gps-logger
   * @param  typeID  The typeid
   * @param  pos  The position
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  vdf  "true",
   */
  Airport( const QString& name, const QString& icao, const QString& abbr,
           BaseMapElement::objectType typeID,
           const WGSPoint& wgsPos, const QPoint& pos, unsigned int elevation,
           const QString& frequency, bool vdf, runway* rw);

  /**
   * Destructor
   */
  ~Airport();

  /**
   * @return the frequency of the airport.
   */
  QString getFrequency() const;

  /**
   * @return a runway-struct, containing the data of the given runway.
   */
  runway getRunway(int index = 0) const;

  /**
   * @return the number of runways.
   */
  unsigned int getRunwayNumber() const;

  /**
   * Return a short html-info-string about the airport, containg the
   * name, the alias, the elevation and the frequency as well as a small
   * icon of the airporttype.
   *
   * Reimplemented from SinglePoint (@ref SinglePoint#getInfoString).
   * @return the infostring
   */
  virtual QString getInfoString() const;

  /**
   * Get translation string for surface type.
   */
  static QString item2Text( const int surfaceType, QString defaultValue=QString("") );

  /**
   * Get surface type for translation string.
   */
  static const int text2Item( const QString& text );

  /**
   * Get sorted translations
   */
  static QStringList& getSortedTranslationList()
    {
      return sortedTranslations;
    };

  // Draws the element into the given painter.
  virtual void drawMapElement(QPainter* targetP, QPainter* maskP);

 private:
  /**
   */
  bool vdf;

  /**
   * Contains the runway-data.
   */
  runway* rwData;

  /**
   * Contains the number of runways.
   */
  unsigned int rwNum;

  unsigned int shift;

  /**
   * Static pointer to surface translations
   */
  static QHash<int, QString> surfaceTranslations;
  static QStringList sortedTranslations;

  /**
   * Static method for loading of object translations
   */
  static void loadTranslations();
};

#endif
