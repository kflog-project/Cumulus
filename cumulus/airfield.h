/***********************************************************************
 **
 **   airfield.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \class Airfield
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Class to handle different types of airfields.
 *
 * This class is used for handling of airfields. The object can be one of
 * Airport, MilAirport, CivMilAirport, Airfield, ClosedAirfield,
 * CivHeliport, MilHeliport, AmbHeliport, UltraLight, Gliderfield
 *
 * \see BaseMapElement#objectType
 *
 * This class is derived from \ref SinglePoint
 *
 * \date 2000-2011
 *
 */

#ifndef AIRFIELD_H
#define AIRFIELD_H

#include <QString>

#include "singlepoint.h"
#include "runway.h"

class Airfield : public SinglePoint
{
 public:

  /**
   * Creates a new Airfield-object.
   *
   * @param  name  The name of the airfield
   * @param  icao  The ICAO-name
   * @param  shortName  The abbreviation, used for the gps-logger
   * @param  typeId  The map element type identifier
   * @param  wgsPos The position as WGS84 datum
   * @param  pos  The position
   * @param  rw The runway object
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  comment An additional comment related to the airfield
   * @param  country The country of the airfield as two letter code
   * @param  winch  "true", if winch launch is available
   * @param  towing "true", if aero towing is available
   * @param  landable "true", if airfield is landable
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const Runway& rw,
            const float elevation,
            const float frequency,
            const QString country = "",
            const QString comment = "",
            bool winch = false,
            bool towing = false,
            bool landable = true );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return The frequency of the airfield as String.
   */
  QString frequencyAsString() const
    {
      return (frequency > 0) ? QString("%1").arg(frequency, 0, 'f', 3) : QString("");
    };

  /**
   * @return The frequency of the airfield as float value.
   */
  float getFrequency() const
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
  bool hasWinch() const
    {
      return winch;
    };

  /**
   * @return "true", if aero towing is available.
   */
  bool hasTowing() const
    {
      return towing;
    };

  /**
   * @return "true", if it is landable
   */
  bool isLandable() const
    {
      return landable;
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
  virtual bool drawMapElement( QPainter* targetP );

 private:

   /**
    * The ICAO name
    */
   QString icao;

   /**
    * The frequency
    */
   float frequency;

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

  /**
   * Flag to indicate the landability of the airfield.
   */
  bool landable;
};

#endif
