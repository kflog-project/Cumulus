/***********************************************************************
 **
 **   airfield.h
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

#ifndef AIRFIELD_H
#define AIRFIELD_H

#include <QString>

#include "singlepoint.h"
#include "runway.h"

/**
 * @short Class to handle different types of airfields.
 *
 * This class is used for handling of airfields. The object can be one of
 * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
 * CivHeliport, MilHeliport, AmbHeliport, UltraLight, GliderSite
 *
 * @see BaseMapElement#objectType
 */

class Airfield : public SinglePoint
{
 public:

  /**
   * Creates a new Airfield-object.
   * @param  name  The name
   * @param  icao  The icao-name
   * @param  shortName  The abbreviation, used for the gps-logger
   * @param  typeID  The typeid
   * @param  pos  The position
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  winch  "true", if winch-launch is available
   * @param  towing "true", if aero towing is available
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const Runway& rw,
            const unsigned int elevation,
            const QString& frequency,
            bool winch = false,
            bool towing = false );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return the frequency of the airport.
   */
  QString getFrequency() const
    {
      return frequency;
    };

  /**
   * @return ICAO name
   */
  QString getICAO() const
    {
      return icao;
    };

  /**
   * @return a runway object, containing the data of the runway.
   */
  const Runway& getRunway()
    {
      return rwData;
    };

  /**
   * @return "true", if winch launching is available.
   */
  const bool hasWinch() const
    {
      return winch;
    };

  /**
   * @return "true", if aero towing is available.
   */
  const bool hasTowing() const
    {
      return towing;
    };

  /**
   * Return a short html-info-string about the airport, containing the
   * name, the alias, the elevation and the frequency as well as a small
   * icon of the airport type.
   *
   * Reimplemented from SinglePoint (@ref SinglePoint#getInfoString).
   * @return the info string
   */
  virtual QString getInfoString() const;

  // Draws the element into the given painter.
  virtual void drawMapElement(QPainter* targetP);

 private:

   /**
    * The ICAO name
    */
   QString icao;

   /**
    * The frequency
    */
   QString frequency;

  /**
   * Contains the runway data.
   */
  Runway rwData;

  /**
   * The launching-type. "true" if the site has a winch.
   */
  bool winch;

  /**
   * The launching-type. "true" if the site has aero tow.
   */
  bool towing;

  /**
   * Contains the shift of the runway during drawing.
   */
  unsigned short rwShift;

};

#endif
