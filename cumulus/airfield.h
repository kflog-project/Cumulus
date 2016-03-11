/***********************************************************************
 **
 **   airfield.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2015 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
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
 * \date 2000-2015
 *
 */

#ifndef AIRFIELD_H
#define AIRFIELD_H

#include <QList>
#include <QMutex>
#include <QPixmap>
#include <QString>

#include "runway.h"
#include "singlepoint.h"

class Airfield : public SinglePoint
{
 public:

  /**
   * Airfield default constructor
   */
  Airfield();

  /**
   * Creates a new Airfield-object.
   *
   * @param  name  The name of the airfield
   * @param  icao  The ICAO-name
   * @param  shortName  The abbreviation, used for the gps-logger
   * @param  typeId  The map element type identifier
   * @param  wgsPos The position as WGS84 datum
   * @param  pos  The position
   * @param  rwList A list with runway objects
   * @param  elevation  The elevation
   * @param  frequency  The frequency
   * @param  country The country of the airfield as two letter code
   * @param  comment An additional comment related to the airfield
   * @param  winch  "true", if winch launch is available
   * @param  towing "true", if aero towing is available
   * @param  landable "true", if airfield is landable
   * @param  atis ATIS
   */
  Airfield( const QString& name,
            const QString& icao,
            const QString& shortName,
            const BaseMapElement::objectType typeId,
            const WGSPoint& wgsPos,
            const QPoint& pos,
            const QList<Runway>& rwList,
            const float elevation,
            const float frequency,
            const QString country = "",
            const QString comment = "",
            bool winch = false,
            bool towing = false,
            bool landable = true,
            const float atis = 0.0 );

  /**
   * Destructor
   */
  virtual ~Airfield();

  /**
   * @return The frequency of the airfield as String.
   */
  QString frequencyAsString( const float frequency ) const
    {
      return (frequency > 0) ? QString("%1").arg(frequency, 0, 'f', 3) : QString("");
    };

  /**
   * @return The frequency of the airfield as float value.
   */
  float getFrequency() const
    {
      return m_frequency;
    };

  /**
   * @param value The frequency of the airfield as float value.
   */
  void setFrequency( const float value )
    {
      m_frequency = value;
    };

  /**
   * @return The ATIS frequency of the airfield.
   */
  float getAtis() const
    {
      return m_atis;
    };

  /**
   * @param The ATIS frequency of the airfield.
   */
  void setAtis( const float value )
    {
      m_atis = value;
    };

  /**
   * @return ICAO name
   */
  QString getICAO() const
    {
      return m_icao;
    };

  /**
   * @param value The ICAO name of the airfield
   */
  void setICAO( const QString& value )
    {
      m_icao = value;
    };

  /**
   * @return The runway list, containing the data of all runways.
   */
  QList<Runway>& getRunwayList()
    {
      return m_rwList;
    };

  /**
   * Adds a runway object to the runway list.
   *
   * @param value The runway object, containing the data of the runway.
   */
   void addRunway( const Runway& value )
     {
       m_rwList.append( value );
       calculateRunwayShift();
     };

  /**
   * @return "true", if winch launching is available.
   */
  bool hasWinch() const
    {
      return m_winch;
    };

  /**
   * \param value The winch flag of the airfield
   */
  void setWinch( const bool value )
    {
      m_winch = value;
    };

  /**
   * @return "true", if aero towing is available.
   */
  bool hasTowing() const
    {
      return m_towing;
    };

  /**
   * \param value The towing flag of the airfield
   */
  void setTowing( const bool value )
    {
      m_towing = value;
    };

  /**
   * @return "true", if it is landable
   */
  bool isLandable() const
    {
      return m_landable;
    };

  /**
   * \param value The landing flag of the airfield
   */
  void setLandable( const bool value )
    {
      m_landable = value;
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

  /**
   * Draws the element into the given painter.
   */
  virtual bool drawMapElement( QPainter* targetP );

  /**
   * Get a big airfield pixmap with the desired runway direction.
   *
   * \param runway Heading of runway in 1/10 degree.
   *
   * \return A big airfield pixmap.
   */
  static QPixmap& getBigAirfield( int runway=9 );

  /**
   * Get a small airfield pixmap with the desired runway direction.
   *
   * \param runway Heading of runway in 1/10 degree.
   *
   * \return A small airfield pixmap.
   */
  static QPixmap& getSmallAirfield( int runway=9 );

  /**
   * Get a big field pixmap with the desired runway direction.
   *
   * \param runway Heading of runway in 1/10 degree.
   *
   * \return A big field pixmap.
   */
  static QPixmap& getBigField( int runway=9 );

  /**
   * Get a small field pixmap with the desired runway direction.
   *
   * \param runway Heading of runway in 1/10 degree.
   *
   * \return A small field pixmap.
   */
  static QPixmap& getSmallField( int runway=9 );

  /**
   * Creates the static airfield and field icons.
   */
  static void createStaticIcons();

 protected:

  /**
   * Calculates the runway shift for the icon to be drawn.
   */
  void calculateRunwayShift()
  {
    // calculate the default runway shift in 1/10 degrees.
    m_rwShift = 90/10; // default direction is 90 degrees

    // We assume, that the first runway is always the main runway.
    if( m_rwList.size() > 0 )
      {
        Runway rw = m_rwList.first();

        // calculate the real runway shift in 1/10 degrees.
        if ( rw.m_heading/256 <= 36 )
          {
            m_rwShift = (rw.m_heading/256 >= 18 ? (rw.m_heading/256)-18 : rw.m_heading/256);
          }
      }
  };

  /**
  * The ICAO name
  */
  QString m_icao;

  /**
  * The speech frequency
  */
  float m_frequency;

  /**
   * The ATIS frequency
   */
  float m_atis;

  /**
   * Contains all runways.
   */
  QList<Runway> m_rwList;

  /**
   * The launching-type. "true" if the site has a m_winch.
   */
  bool m_winch;

  /**
   * The launching-type. "true" if the site has aero tow.
   */
  bool m_towing;

  /**
   * Contains the shift of the runway during drawing in 1/10 degrees.
   */
  unsigned short m_rwShift;

  /**
   * Flag to indicate the landability of the airfield.
   */
  bool m_landable;

  /**
   * Pixmaps with big airfields.
   */
  static QPixmap* m_bigAirfields;

  /**
   * Pixmaps with small airfields.
   */
  static QPixmap* m_smallAirfields;

  /**
   * Pixmaps with big fields.
   */
  static QPixmap* m_bigFields;

  /**
   * Pixmaps with small fields.
   */
  static QPixmap* m_smallFields;

  /**
   * Mutex to protect pixmap creation.
   */
  static QMutex mutex;
};

#endif
