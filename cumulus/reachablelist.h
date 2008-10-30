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

#ifndef REACHABLE_LIST_H
#define REACHABLE_LIST_H

#include <QObject>
#include <QPoint>
#include <QList>
#include <QMap>

#include "generalconfig.h"
#include "mapmatrix.h"
#include "distance.h"
#include "mapcontents.h"
#include "altitude.h"
#include "vector.h"
#include "speed.h"
#include "reachablepoint.h"

/************************************************************************
 * @short A list of reachable points
 *
 * @author Eckhard Völlm
 *
 * The value based list of reachable points maintains the distance, bearings
 * and arrival altitude for points in the range of the current position.
 * If no glider is defined only the nearest reachables in a radius of
 * 75 km are computed.
 *
 * It is assumed, that this class is a singleton.
 ***********************************************************************/

class ReachableList: public QObject, QList<ReachablePoint>
{
  Q_OBJECT

  public:

  // calculation mode used for sorting of the list
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
   * forces to calculate a new list
   */
  void calculateNewList();

  /**
   * returns the number of sites in the list
   */
  const int getNumberSites() const
  {
    return size();
  };

  /**
   * returns site at index (max = getNumberSites()-1)
   */
  const ReachablePoint& getSite( const int index ) const
  {
    return at(index);
  };

  /**
   * returns a pointer to the list class instance
   */
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

  /**
   * Removes all data in the different lists.
   */
  void clearLists()
  {
    clear();
    arrivalAltMap.clear();
    distanceMap.clear();
  };

  /**
   * @returns the color indicating if the point with the given name
   * is reachable.
   */
  static QColor getReachColor( const QPoint& position );

  /**
   * @returns an enumeration value indicating if the point with the given name
   * is reachable.
   */
  static ReachablePoint::reachable getReachable( const QPoint& position );

  /**
   * @returns an integer with the arrival altitude. If the point is
   * not found, -9999 is returned.
   */
  static int getArrivalAlt( const QPoint& position );

  /**
   * @returns an Altitude object with the arrival altitude. If the
   * point is not found, an invalid Altitude is returned
   */
  static Altitude getArrivalAltitude( const QPoint& position );

  /**
   * @returns a Distance object representing the point. If the point
   * is not found, an invalid Distance is returned
   */
  static Distance getDistance( const QPoint& position );

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

   /**
    * Calculate course, distance and reachability from the current position
    * to the elements contained in the limited list. If a glider is defined
    * the glide path is taken into account and the arrival altitude is
    * calculated too.
    */
  void calculateDataInList();

  /**
   * Sets the initial values needed for the calculation.
   */
  void setInitValues();

  /**
   * adds glider, airport or waypoint site (if not out of reach) to the list
   */
  void addItemsToList(enum MapContents::MapContentsListID item);

  /**
   * print list via qDebug interface
   */
  void show();

  /**
   * Removes double entries from the list. Double entries can occur
   * when a point is a waypoint as well as an airfield. In this case,
   * the one with the higher severity or longer name is preferred.
   * This function ASSUMES THE LIST IS SORTED!
   */
  void removeDoubles();

  static QString coordinateString(const QPoint& position)
  {
    return QString("%1.%2").arg(position.x()).arg(position.y());
  };

  QPoint      lastCalculationPosition; // position at last calculation
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

#endif
