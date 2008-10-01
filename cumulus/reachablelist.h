/***********************************************************************
 **
 **   reachablelist.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by Eckhard Völlm, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

// Calculates the destination, bearing, reachability of sites during
// flight.

#ifndef REACHABLE_LIST_H
#define REACHABLE_LIST_H

#include <QObject>
#include <QPoint>
#include <QList>
#include <QMap>

#include "generalconfig.h"
#include "mapmatrix.h"
#include "singlepoint.h"
#include "distance.h"
#include "mapcontents.h"
#include "altitude.h"
#include "vector.h"
#include "speed.h"
#include "waypoint.h"

// class for one entry in ReachableList
class ReachablePoint
{
 public:

   enum reachable{no, belowSafety, yes};

  ReachablePoint(QString name,
                 QString icao,
                 QString description,
                 bool orignAfl,
                 short type,
                 double frequency,
                 WGSPoint pos,
                 QPoint ppos,
                 unsigned int elevation,
                 Distance distance,
                 short bearing,
                 Altitude arrivAlt,
                 short rwDir,
                 short rwLen,
                 short rwSurf,
                 bool rwOpen );


  ReachablePoint(wayPoint& wp,
                 bool orignAfl,
                 Distance distance,
                 short bearing,
                 Altitude arrivAlt );

  Distance getDistance() const
  {
    return _distance;
  };

  void setDistance(Distance& d)
  {
    _distance=d;
  };

  QString getName() const
  {
    return _wp.name;
  };

  QString getDescription() const
  {
    return _wp.description;
  };

  void setOrignAfl(const bool orign)
  {
    _orignAfl = orign;
  };

  bool isOrignAfl() const
  {
    return _orignAfl;
  };

  int getElevation() const
  {
    return _wp.elevation;
  };

  short getType() const
  {
    return _wp.type;
  };

  Altitude getArrivalAlt() const
  {
    return _arrivalAlt;
  };

  short getBearing() const
  {
    return _bearing;
  };

  void setBearing(const short b)
  {
    _bearing = b;
  };

  WGSPoint& getWgsPos()
  {
    return _wp.origP;
  };

  const wayPoint *getWaypoint() const
  {
    return &_wp;
  };

  void setArrivalAlt( const Altitude& alt )
  {
    _arrivalAlt = alt;
  };

  ~ReachablePoint();

  reachable getReachable();

  /**
   * compares two entries to sort list either by distance or arrival altitude
   */
  bool operator < (const ReachablePoint& other) const;

 private:
  bool         _orignAfl; // Origin is taken from airfield list
  wayPoint     _wp;
  Distance     _distance;
  short        _bearing;
  Altitude     _arrivalAlt;
};

/**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * @short A list of reachable points
 * @author Eckhard Völlm
 *
 * The value based list of reachable points maintains the distance and
 * arrival altitudes for points in the region of the current position.
 * If no glider is defined only the nearest reachables in a radius of
 * 75 km are computed.
 *
 * It is assumed, that this class is a singleton.
 */
class ReachableList: public QObject, QList<ReachablePoint>
{
  Q_OBJECT

  public:

  // calculation mode used for the list
  enum CalculationMode{ distance, altitude };

  ReachableList(QObject *parent);
  ~ReachableList();

 signals:

  void newReachList();

 public:

  /**
   * Returns the current calculator switch state. Calculator can be
   * switched on/off by the user
   */
  const bool isOn() const
  {
    return GeneralConfig::instance()->getNearestSiteCalculatorSwitch();
  };

  /**
   * calculates scheduled glide path and full list
   */
  void calculate(bool always);

  /**
   * force to calculate full list
   */
  void calculateFullList();

  /**
   * add glider airport or waypoint site (if not out of reach) to the list
   */
  void addItemsToList(enum MapContents::MapContentsListID item);

  /**
   * print list via qDebug interface
   */
  void show();

  /**
   * returns number of sites
   */
  const int getNumberSites() const
  {
    return size();
  };

  /**
   * returns site at index (max = getNumberSites()-1)
   */
  ReachablePoint& getSite( const int index )
  {
    return (*this)[index];
  };

  QList<ReachablePoint> *getList()
  {
    return this;
  };

  /**
   * Returns configured maximum number of sites in list. Can be
   * modified by the user at run-time
   */
  const int getMaxNrOfSites() const
  {
    return GeneralConfig::instance()->getMaxNearestSiteCalculatorSites();
  };

  /**
   * Returns the mode, which was used during calculation
   */
  enum ReachableList::CalculationMode getCalcMode() const
  {
    return calcMode;
  };

  void clearList();

  /**
   * @returns the color indicating if the point with the given name
   * is reachable.
   */
  static QColor getReachColor( QPoint position );

  /**
   * @returns an enumeration value indicating if the point with the given name
   * is reachable.
   */
  static ReachablePoint::reachable getReachable( QPoint position );

  /**
   * @returns an integer with the arrival altitude. If the point is
   * not found, -9999 is returned.
   */
  static int getArrivalAlt( QPoint position );

  /**
   * @returns an Altitude object with the arrival altitude. If the
   * point is not found, an invalid Altitude is returned
   */

  static Altitude getArrivalAltitude( QPoint position );

  /**
   * @returns a Distance object representing the point. If the point
   * is not found, an invalid Distance is returned
   */

  static Distance getDistance( QPoint position );

  /**
   * @returns The safety altitude in meters
   */
  static const int getSafetyAltititude()
  {
    return safetyAlt;
  };

  /**
   * @returns The modeAltitude
   */
  static const bool getModeAltitude()
  {
    return modeAltitude;
  };

 private:

  void calculateGlidePath();

  void calcInitValues();

  /**
   * Removes double entries from the list. Double entries can occur
   * when a point is a waypoint as well as an airfield. In this case,
   * the one with the higher severity or longer name is preferred.
   * This function ASSUMES THE LIST IS SORTED!
   */
  void removeDoubles();

  static const QString coordinateString(QPoint position)
  {
    return QString("%1.%2").arg(position.x()).arg(position.y());
  };

  QPoint      lastPosition;
  double      lastAltitude;
  Vector      lastWind;
  Speed       lastMc;
  double      _maxReach;
  int         tick;
  bool        initValuesOK;

  // Used mode for calculation of list. Can be altitude or distance.
  enum ReachableList::CalculationMode calcMode;

  static bool modeAltitude;
  static int safetyAlt;

  static QMap<QString, int> arrivalAltMap;
  static QMap<QString, Distance> distanceMap;

  // number of created class instances
  static short instances;

};

struct CompareReachablePoints
{
  bool operator()(const ReachablePoint& rp1, const ReachablePoint& rp2) const
  {
    if( ReachableList::getModeAltitude() )
      {
        return (rp1.getArrivalAlt().getMeters() < rp2.getArrivalAlt().getMeters());
      }
    else
      {
        return (rp1.getDistance().getKilometers() > rp2.getDistance().getKilometers());
      }
  };
};

#endif
