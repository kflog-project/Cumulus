/***********************************************************************
 **
 **   glidersite.h
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

#ifndef GLIDERSITE_H
#define GLIDERSITE_H

#include "singlepoint.h"
#include "runway.h"

/**
 * This class provides handling the glider-sites.
 */

class GliderSite : public SinglePoint
{
 public:
  /**
   * Creates a new GliderSite-object.
   *
   * @param  name  The name
   * @param  icao  The icao-name
   * @param  shortName  The short name, used for the gps-logger
   * @param  wgsPos  The original WGS-position
   * @param  pos  The projected position
   * @param  rw The runway data
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  winch  "true", if winch-launch is available
   * @param  towing "true", if aero towing is available
   */
  GliderSite( const QString& name,
              const QString& icao,
              const QString& shortName,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const Runway& rw,
              const unsigned int elevation,
              const QString& frequency,
              bool winch = false,
              bool towing = false );

  /**
   * Destructor.
   */
  virtual ~GliderSite();

  /**
   * @return the frequency of the glider site.
   */
  QString getFrequency() const
    {
      return frequency;
    };

  /**
   * @return the ICAO name, if available
   */
  QString getICAO() const
    {
      return icao;
    };

  /**
   * @return a runway struct, containing the data of the runway.
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
   * Reimplemented from SinglePoint.
   *
   * @return the info string
   */
  virtual QString getInfoString() const;

  /**
   * Draws the element into the given painter. Reimplemented from BaseMapElement.
   */
  virtual void drawMapElement(QPainter* targetP);

 private:

    /**
     * The icao name
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
