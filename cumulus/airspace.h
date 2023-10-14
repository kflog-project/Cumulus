/***********************************************************************
**
**   airspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
**                   2009-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Airspace
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Class to handle airspaces.
 *
 * This class is used to handle different airspace categories.
 *
 * Due to the cross pointer reference to the air region this class do not
 * allow copies and assignments of an existing instance.
 *
 * \date 2000-2023
 *
 * \version 1.5
 *
 */

#pragma once

#include <algorithm>
#include <cmath>

#include <QDateTime>
#include <QElapsedTimer>
#include <QPolygon>
#include <QPainter>
#include <QPainterPath>
#include <QRect>

#include "altitude.h"
#include "lineelement.h"
#include "airspacewarningdistance.h"
#include "flarmbase.h"
#include "Frequency.h"

class AirRegion;

class Airspace : public LineElement
{

private:

/**
 * Don't allow copies.
 */
Airspace(const Airspace& );

public:

  enum ConflictType { none=0, near=1, veryNear=2, inside=3 };

  /**
   * ICAO airspace classes of openAIP.
   */
  enum icaoClass {
    AS_A=0,
    AS_B=1,
    AS_C=2,
    AS_D=3,
    AS_E=4,
    AS_F=5,
    AS_G=6,
    AS_SUA=8,
    AS_Unkown=255
  };

  /**
   * Kind of activity used by openAIP
   */
  enum activity {
    None=0, // No specific activity (default)
    Parachuting=1,
    Aerobatics=2,
    AeroclubAndArialWorkArea=3,
    UltraLightMachines=4, // (ULM) Activity
    HangGlidingAndParagliding=5
  };

  Airspace();

  /**
   * Creates a new Airspace object.
   *
   * \param name The name of the airspace
   * \param oType The object type identifier.
   * \param pP The projected coordinates of the airspace as polygon.
   * \param upper The upper altitude limit of the airspace in feet
   * \param upperType The upper altitude reference
   * \param lower The lower altitude limit of the airspace in feet
   * \param lowerType The lower altitude reference
   * \param frequencyList A list with the frequency objects
   * \param icaoClass openAIP ICAO class identifier
   * \param country The country as two letter code, where the airspace is located
  */
  Airspace( QString name,
            BaseMapElement::objectType oType,
            QPolygon pP,
            const float upper,
            const BaseMapElement::elevationType upperType,
            const float lower,
            const BaseMapElement::elevationType lowerType,
            const QList<Frequency> frequencyList,
            const int icaoClass=AS_Unkown,
            QString country="",
            quint8 activity=0,
            bool byNotam=false );

  /**
   * Destructor
   */
  virtual ~Airspace();

  /**
   * Creates a new airspace object using the current set airspace data.
   */
  Airspace* createAirspaceObject();

  /**
   * Draws the airspace into the given painter.
   *
   * @param targetP The painter to draw the element into.
   *
   * @param opacity Sets the opacity of the painter to opacity. The
   * value should be in the range 0.0 to 100.0, where 0.0 is fully
   * transparent and 100.0 is fully opaque.
   */
  void drawRegion( QPainter* targetP, qreal opacity = 0.0 );

  /**
   * Draws the Flarm alert zone into the given painter.
   *
   * @param targetP The painter to draw the element into.
   *
   * @param opacity Sets the opacity of the painter to opacity. The
   * value should be in the range 0.0 to 100.0, where 0.0 is fully
   * transparent and 100.0 is fully opaque.
   */
  void drawFlarmAlertZone( QPainter* targetP, qreal opacity );

  /**
   * Tells the caller, if the airspace is drawable or not
   */
  bool isDrawable() const;

  /**
   * Return a pointer to the mapped airspace region data. The caller takes
   * the ownership about the returned object.
   */
  QPainterPath* createRegion();

  /**
   * Sets the upper limit of the airspace.
   */
  void setUpperL( const Altitude& alt )
  {
    m_uLimit = alt;
  };

  /**
   * Returns the upper limit of the airspace in meters.
   */
  unsigned int getUpperL() const
  {
    return (unsigned int) rint(m_uLimit.getMeters());
  };

  /**
   * Returns the upper limit of the airspace.
   */
  const Altitude& getUpperAltitude() const
  {
    return m_uLimit;
  };

  /**
   * Sets the lower limit of the airspace.
   */
  void setLowerL( const Altitude& alt )
  {
    m_lLimit = alt;
  };

  /**
   * Returns the lower limit of the airspace in meters.
   */
  unsigned int getLowerL() const
  {
    return (unsigned int) rint(m_lLimit.getMeters());
  };

  /**
   * Returns the lower limit of the airspace.
   */
  const Altitude& getLowerAltitude() const
  {
    return m_lLimit;
  };

  /**
   * Returns the type of the upper limit (MSL, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #uLimitType
   */
  BaseMapElement::elevationType getUpperT() const
  {
    return m_uLimitType;
  };

  void setUpperT( const BaseMapElement::elevationType ut )
  {
    m_uLimitType = ut;
  };

  /**
   * Returns the type of the lower limit (MSL, GND, FL)
   * @see BaseMapElement#elevationType
   * @see #lLimitType
   */
  BaseMapElement::elevationType getLowerT() const
  {
    return m_lLimitType;
  };

  void setLowerT( const BaseMapElement::elevationType lt )
  {
    m_lLimitType = lt;
  };

  /**
   * Returns a html-text-string about the airspace containing the name,
   * the type and the borders.
   * @return the infostring
   */
  QString getInfoString() const;

  /**
   * Returns a text representing the type of the airspace
   */
  static QString getTypeName (objectType);

  /**
   * Returns true if the given altitude conflicts with the airspace properties
   */
  ConflictType conflicts (const AltitudeCollection& alt,
                          const AirspaceWarningDistance& dist) const;

  /**
   * Returns the last vertical conflict type
   */
  ConflictType lastVConflict() const
  {
      return m_lastVConflict;
  }

  /**
   * sets the touch time of air space to current time
   */
  void setLastNear()
  {
      m_lastNear.start();
  }

  void setLastVeryNear()
  {
      m_lastVeryNear.start();
  }

  void setLastInside()
  {
      m_lastInside.start();
  }

  /**
  * returns the touch time of air space
  */
  QElapsedTimer& getLastNear()
  {
      return m_lastNear;
  }

  QElapsedTimer& getLastVeryNear()
  {
      return m_lastVeryNear;
  }

  QElapsedTimer& getLastInside()
  {
      return m_lastInside;
  }

  /**
   * @returns the associated AirRegion object
   */
  AirRegion* getAirRegion() const
  {
      return m_airRegion;
  }

  /**
   * Sets the associated AirRegion object
   */
  void setAirRegion(AirRegion* region)
  {
      m_airRegion = region;
  }

  /**
   * Compares two items, in this case, Airspaces.
   * The items are compared on their levels. Because cumulus provides a view
   * where the user looks down on the map, the first airspace you'll see is the
   * one with the highest ceiling. So, an item with a higher ceiling is
   * bigger than an item with a lower ceiling. In case these are the same,
   * the item with the bigger floor will be the bigger item.
   *
   * The airspaces in the list are sorted in this way to make sure they are
   * stacked correctly. This has become important with the introduction of
   * transparent airspaces. By sorting the airspaces like this, the lower ones
   * will be drawn first, and the higher ones on top of them.
   */
  bool operator < (const Airspace& other) const;

  /**
   * Get icao airspace identifier.
   *
   * \return airspace identifier
   */
  quint8 getIcaoClass() const
  {
    return m_icaoClass;
  }

  /**
   * Set icao airspace identifier
   *
   * \param id airspace identifier
   */
  void setIcaoClass( int icaoClass)
  {
    m_icaoClass = icaoClass;
  }

  /**
   * Get Flarm Alert Zone.
   *
   * \return Flarm Alert Zone.
   */
  FlarmBase::FlarmAlertZone& getFlarmAlertZone()
  {
    return m_flarmAlertZone;
  }

  /**
   * Set Flarm Alert Zone.
   *
   * \param Flarm Alert Zone.
   */
  void setFlarmAlertZone( const FlarmBase::FlarmAlertZone& faz)
  {
    m_flarmAlertZone = faz;
  }

  /**
   * Prints out all relevant airspace data.
   */
  void debug();

  /**
   * Kind of activity used by openAip.
   *
   * @return
   */
  quint8 getActivity() const
  {
    return m_activity;
  }

  /**
   * Kind of activity used by openAip.
   *
   * @param activity enumeration
   */
  void setActivity( quint8 activity )
  {
    m_activity = activity;
  }

  bool isByNotam() const
  {
    return m_byNotam;
  }

  void setByNotam( bool byNotam )
  {
    m_byNotam = byNotam;
  }

  /**
   * @return The frequency as String.
   */
  QString frequencyAsString( const float frequency ) const
    {
      return (frequency > 0) ? QString("%1").arg(frequency, 0, 'f', 3) : QString("");
    };

  /**
   * @return The airspace frequency list.
   */
  QList<Frequency>& getFrequencyList()
    {
      return m_frequencyList;
    };

  /**
   * @param freq The airspace frequency and its type.
   */
  void addFrequency( Frequency freqencyAndType)
    {
      m_frequencyList.append( freqencyAndType );
    };

private:
  /**
   * Contains the lower limit.
   * @see #getLowerL
   */
  Altitude m_lLimit;

  /**
   * Contains the type of the lower limit
   * @see #lLimit
   * @see #getLowerT
   */
  BaseMapElement::elevationType m_lLimitType;

  /**
   * Contains the upper limit.
   * @see #getUpperL
   */
  Altitude m_uLimit;

  /**
   * Contains the type of the upper limit
   * @see #uLimit
   * @see #getUpperT
   */
  BaseMapElement::elevationType m_uLimitType;

  /**
   * Specifies the frequencies, e.g. "132.505", and the ground stations
   * responsible for this airspace.
   */
  QList<Frequency> m_frequencyList;

  mutable ConflictType m_lastVConflict;

  /** save time of last touch of airspace */
  QElapsedTimer m_lastNear;
  QElapsedTimer m_lastVeryNear;
  QElapsedTimer m_lastInside;

  // pointer to associated airRegion object
  AirRegion* m_airRegion;

  /**
   * ICAO identifier used by openAip.
   */
  quint8 m_icaoClass;

  /**
   * Kind of activity used by openAip.
   */
  quint8 m_activity;

  /**
   * Activation by NOTAM
   */
  bool m_byNotam;

  /**
   * Flarm Alert Zone object.
   */
  FlarmBase::FlarmAlertZone m_flarmAlertZone;
};

/**
 * \struct CompareAirspaces
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Compare method for two \ref Airspace items.
 *
 * \see Airspace
 *
 * \date 2002-2010
 */
struct CompareAirspaces
{
  // The operator sorts the airspaces in the expected order
  bool operator()(const Airspace *as1, const Airspace* as2) const
  {
    int a1C = as1->getUpperL(), a2C = as2->getUpperL();

    if (a1C > a2C)
      {
        return false;
      }

    if (a1C < a2C)
      {
        return true;
      }

    // equal
    int a1F = as1->getLowerL();
    int a2F = as2->getLowerL();
    return (a1F < a2F);
  };
};

/**
 * \class SortableAirspaceList
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Specialized QList for Airspaces.
 *
 * \see Airspace
 *
 * Specialized QList for \ref Airspace elements. The sort member function
 * has been re-implemented to make it possible to sort items based on their
 * levels.
 *
 * \date 2002-2010
 */

class SortableAirspaceList : public QList<Airspace*>
{
public:

  void sort ()
  {
    std::sort( begin(), end(), CompareAirspaces() );
  }
};
