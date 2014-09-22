/***********************************************************************
 **
 **   radiopoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef RADIO_POINT_H
#define RADIO_POINT_H

#include <climits>

#include <QPoint>
#include <QString>

#include "singlepoint.h"

/**
 * \class RadioPoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief This class provides a map element for radio-navigation-facilities.
 *
 * This class provides a map element for radio navigation facilities. It is
 * derived from \ref SinglePoint. This class is used for: VOR, VORDME, VORTAC,
 * NDB and CompPoint.
 *
 * @see BaseMapElement#objectType
 *
 * \date 2000-2014
 */

class RadioPoint : public SinglePoint
{
  public:

  /**
   * Default constructor
   */
  RadioPoint() :
    SinglePoint(),
    m_frequency(0.0),
    m_range(0.0),
    m_declination(SHRT_MIN),
    m_aligned2TrueNorth(false)
   {
   };

  /**
   * Creates a new radio-point.
   *
   * @param  name  The name.
   * @param  icao  The ICAO name.
   * @param  shortName The abbreviation, used for the GPS logger.
   * @param  typeID The type identifier.
   * @param  wgsPos The original WGS84 position.
   * @param  pos    The projected position.
   * @param  frequency  The frequency.
   * @param  channel The channel.
   * @param  elevation The elevation.
   * @param  country The country location.
   * @param  range The service range in meters.
   * @param  declination The declination
   * @param  aligned2TrueNorth Alignment to true north
   */
  RadioPoint( const QString& name,
              const QString& icao,
              const QString& shortName,
              BaseMapElement::objectType typeID,
              const WGSPoint& wgsPos,
              const QPoint& pos,
              const float frequency,
              const QString channel = "",
              float elevation = 0.0,
              const QString country = "",
              const float range = 0.0,
              const short declination = SHRT_MIN,
              const bool aligned2TrueNorth = false );

  /**
   * Destructor
   */
  virtual ~RadioPoint();

  /**
   * @return The frequency
   */
  QString frequencyAsString() const
    {
      return (m_frequency > 0) ? QString("%1").arg(m_frequency, 0, 'f', 3) : QString("");
    };

  virtual float getFrequency() const
    {
      return m_frequency;
    };

  virtual void setFrequency( const float value)
    {
      m_frequency = value;
    };

  virtual QString getChannel() const
    {
      return m_channel;
    };

  virtual void setChannel( const QString& value )
    {
      m_channel = value;
    };

  /**
   * @return ICAO name
   */
  virtual QString getICAO() const
    {
      return m_icao;
    };

  /**
   * @param value ICAO name
   */
  virtual void setICAO( const QString& value )
    {
      m_icao = value;
    }

  bool isAligned2TrueNorth () const
    {
      return m_aligned2TrueNorth;
    }

  void setAligned2TrueNorth (bool aligned2TrueNorth)
    {
      m_aligned2TrueNorth = aligned2TrueNorth;
    }

  short getDeclination () const
    {
      return m_declination;
    }

  void setDeclination (short declination)
    {
      m_declination = declination;
    }

  float getRange () const
    {
      return m_range;
    }

  void setRange (float range)
    {
      m_range = range;
    }

 protected:
  /**
   * The frequency
   */
  float m_frequency;

  /**
   * The channel.
   */
  QString m_channel;

  /**
   * The icao name
   */
  QString m_icao;

  /**
   * Range of service in meters. 0 means unknown.
   */
  float m_range;

  /**
   * Declination, SHRT_MIN means undefined.
   */
  short m_declination;

  /**
   * Aligned to true north.
   */
  bool m_aligned2TrueNorth;
};

#endif
