/***********************************************************************
 **
 **   reachablelist.h 
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by Eckhard V�llm, 2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

// Calculates the destination, bearing, reachability of sites during
// flight.

#ifndef REACHABLELIST_H
#define REACHABLELIST_H

#include <QObject>
#include <QPoint>
#include <QList>
#include <QMap>
#include <Q3PtrCollection>

#include "generalconfig.h"
#include "mapmatrix.h"
#include "singlepoint.h"
#include "distance.h"
#include "mapcontents.h"
#include "altitude.h"
#include "vector.h"
#include "speed.h"
#include "waypoint.h"

enum reachable{no, belowSavety, yes};

// class for one entry in ReachableList
class ReachablePoint
{
 public:
  ReachablePoint(QString name,
                 QString icao,
                 QString description,
                 bool orignAfl,
                 int type,
                 double frequency,
                 WGSPoint pos,
                 QPoint ppos,
                 int elevation,
                 Distance distance,
                 int bearing,
                 Altitude arrivAlt,
                 int rwDir,
                 int rwLen,
                 int rwSurf,
                 bool rwOpen );


  ReachablePoint(wayPoint *wp,
                 bool orignAfl,
                 Distance distance,
                 int bearing,
                 Altitude arrivAlt );

  Distance getDistance() const
  {
    return _distance;
  };

  void setDistance(Distance d)
  {
    _distance=d;
  };

  QString getName() const
  {
    return _wp->name;
  };

  QString getDescription() const
  {
    return _wp->description;
  };

  void setOrignAfl(bool o)
  {
    _orignAfl=o;
  };

  bool isOrignAfl() const
  {
    return _orignAfl;
  };

  int getElevation() const
  {
    return _wp->elevation;
  };

  int getType() const
  {
    return _wp->type;
  };

  Altitude getArrivalAlt() const
  {
    return _arrivalAlt;
  };

  int getBearing() const
  {
    return _bearing;
  };

  void setBearing(int b)
  {
    _bearing=b;
  };

  WGSPoint& getWgsPos() const
  {
    return _wp->origP;
  };

  wayPoint * getWaypoint() const
  {
    return _wp;
  };

  void setArrivalAlt( Altitude alt )
  {
    _arrivalAlt = alt;
  };

  ~ReachablePoint();

  reachable getReachable();

  /**
   * compares two entries to sort list either by distance or arrival alt
   */
  bool operator < (const ReachablePoint& other) const;

 private:
  bool         _orignAfl; // orign is from airfield list
  wayPoint     *_wp;
  Distance     _distance;
  int          _bearing;
  Altitude     _arrivalAlt;
};


/**
 * @short A list of reachable points
 * @author Eckhard V�llm
 * The list of reachables points maintains the distance and arrival
 * altitudes for points in the region of the current position.
 */
class ReachableList: public QObject, QList<ReachablePoint*>
{
  Q_OBJECT

  public:

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
  const int getNumberSites() const;

  /**
   * returns site at index (max =  getNumberSites()-1)
   */
  ReachablePoint *getSite( const int index );

  QList<ReachablePoint*> *getList()
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

  void clearList();

  /**
   * @returns the colour indicating if the point with the given name
   * is reachable.
   */
  static QColor getReachColor( QPoint position );

  /**
   * @returns an enum value indicating if the point with the given name
   * is reachable.
   */
  static reachable getReachable( QPoint position );

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
   * Removes double entries from the list.  Double entries can occur
   * when a point is a waypoint as well as an airfield. In this case,
   * the one with the higher serverity or longer name is prefered.
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

  static bool modeAltitude;
  static int safetyAlt;
  static QMap<QString, int> arrivalAltMap;
  static QMap<QString, Distance> distanceMap;

};

#endif
