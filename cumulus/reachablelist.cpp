/***********************************************************************
 **
 **   reachablelist.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by Eckhard Völlm,
 **                   2008-2013 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

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

#include <math.h>

#include <QtCore>

#include "reachablelist.h"
#include "calculator.h"
#include "mapcontents.h"
#include "mapcalc.h"
#include "polar.h"
#include "waypoint.h"
#include "airfield.h"

extern MapContents *_globalMapContents;

// Initialize static members
int  ReachableList::safetyAlt = 0;
QMap<QString, int> ReachableList::arrivalAltMap;
QMap<QString, Distance> ReachableList::distanceMap;
bool ReachableList::modeAltitude = false;

// Radius of reachables to be taken into account in kilometers
#define RANGE_RADIUS 100.0;

// number of created class instances
short ReachableList::instances = 0;

//***********************************************************************************

ReachableList::ReachableList(QObject *parent) : QObject(parent)
{
  if ( ++instances > 1 )
    {
      // There exists already a class instance as singleton.
      return;
    }

  ++instances;

  GeneralConfig *conf = GeneralConfig::instance();
  safetyAlt = (int) conf->getSafetyAltitude().getMeters();

  lastAltitude = 0.0;
  _maxReach = RANGE_RADIUS;
  tick = 0;
  modeAltitude = false;
  initValuesOK = false;
  calcMode = ReachableList::distance;
}

ReachableList::~ReachableList()
{
  if ( --instances > 0 )
    {
      // There exists other instances
      return;
    }

  clear();
}

void ReachableList::calculate(bool always)
{
  if ( !isOn() )
    {
      //qDebug("ReachableList::calculate is off");
      return;
    }

  tick++;

  QPoint currentPosition = calculator->getlastPosition();

  // Calculate distance to position of last full computed list.
  // The result has the unit kilometers.
  double dist2Last = dist(&currentPosition, &lastCalculationPosition);

  // qDebug("tick %d %d",tick, always );
  // The whole list is new computed, if the distance has become
  // greater than 5km to the last computing point.
  if ( dist2Last > 5.0 || always )
    {
      // save position where new calculation has been done
      lastCalculationPosition = currentPosition;
      calculateNewList();
      return;
    }

  if ( (tick %10) == 0 )
    {
      // Only update last computed list after 10 ticks,
      // normally after 10s in GPS mode.
      calculateDataInList();
    }
}

void ReachableList::addItemsToList(enum MapContents::MapContentsListID item)
{
  Distance distance;
  QRect bbox = areaBox(lastPosition, _maxReach);
  int r=0, a=0;
  //qDebug("bounding box: (%d, %d), (%d, %d) (%d x %d km)", bbox.left(),bbox.top(),bbox.right(),bbox.bottom(),0,0);

  if( item == MapContents::WaypointList )
    {
      // Waypoints have different structure treat them here
      QList<Waypoint> &wpList = _globalMapContents->getWaypointList();
      // qDebug("Nr of Waypoints: %d", wpList.count() );

      for ( int i=0; i < wpList.count(); i++ )
        {
          WGSPoint pt = wpList.at(i).wgsPoint;

          if (! bbox.contains(pt))
            {
              //qDebug("Not in bounding box, so ignore! (distance: %d, (%d, %d), %s)", (int)distance.getKilometers(), pt.x(),pt.y(), wpList.at(i)->name.latin1());
              r++;
              continue;
            }
          else
            {
              a++;
              //qDebug("In bounding box, so accept! (distance: %d, %s)", (int)distance.getKilometers(), wpList.at(i)->name.latin1());
            }

          distance.setKilometers(dist(&lastPosition, &pt));

          bool isLandable = false;

          if( wpList.at(i).rwyList.size() > 0 )
            {
              isLandable = wpList.at(i).rwyList.at(0).m_isOpen;
            }


          // check if point is a potential reachable candidate at best LD
          if ( (distance.getKilometers() > _maxReach ) ||
               ! (isLandable ||
                 (wpList.at(i).type == BaseMapElement::Outlanding) )  )
            {
              continue;
            }

          // calculate bearing
          double result = getBearing(lastPosition, pt);
          int bearing = int(rint(result * 180./M_PI));
          Altitude altitude(0);

          ReachablePoint rp( wpList[i],
                             false,
                             distance,
                             bearing,
                             altitude );

          append(rp);
        }
    }
  else
    {
      // get number of elements in the list
      int nr = _globalMapContents->getListLength(item);

      // qDebug("No of sites: %d type %d", nr, item );
      for (int i=0; i<nr; i++ )
        {
          // Get specific site data from current list. We have to distinguish
          // between AirfieldList, GilderSiteList and OutlandingList.
          Airfield* site;

          if( item == MapContents::AirfieldList )
            {
              // Fetch data from airport list
              site = _globalMapContents->getAirfield(i);
            }
          else if( item == MapContents::GliderfieldList )
            {
              // fetch data from glider site list
              site = _globalMapContents->getGliderfield(i);
            }
          else if( item == MapContents::OutLandingList )
            {
              // fetch data from glider site list
              site = _globalMapContents->getOutlanding(i);
            }
          else
            {
              qWarning( "ReachableList::addItemsToList: ListType %d is unknown",
                        item );
              break;
            }

          QString siteName = site->getWPName();
          QString siteIcao = site->getICAO();
          QString siteDescription = site->getName();
          QString siteCountry = site->getCountry();
          short siteType = site->getTypeID();
          float siteFrequency = site->getFrequency();
          WGSPoint siteWgsPosition = site->getWGSPosition();
          QPoint sitePosition = site->getPosition();
          float siteElevation = site->getElevation();
          QString siteComment = site->getComment();
          QList<Runway> siteRwyList = site->getRunwayList();

          if (! bbox.contains(siteWgsPosition) )
            {
              //qDebug("Not in bounding box, so ignore! (distance: %d, (%d, %d), %s)", (int)distance.getKilometers(), pt.x(),pt.y(), site->getName().latin1());
              r++;
              continue;
            }
          else
            {
              a++;
              //qDebug("In bounding box, so accept! (distance: %d, %s)", (int)distance.getKilometers(), site->getName().latin1());
            }
          distance.setKilometers(dist(&lastPosition,&siteWgsPosition));
          // qDebug("%d  %f %f", i, (float)distance.getKilometers(),_maxReach );
          // check if point is a potential reachable candidate at best LD
          if ( distance.getKilometers() > _maxReach )
            {
              continue;
            }

          // calculate bearing
          double result = getBearing(lastPosition, siteWgsPosition);
          short bearing = short(rint(result * 180./M_PI));
          Altitude altitude(0);

          // add all potential reachable points to the list, altitude is calculated later
          ReachablePoint rp( siteName,
                             siteIcao,
                             siteDescription,
                             siteCountry,
                             true,
                             siteType,
                             siteFrequency,
                             siteWgsPosition,
                             sitePosition,
                             siteElevation,
                             siteComment,
                             distance,
                             bearing,
                             altitude,
                             siteRwyList );
          append(rp);

          // qDebug("%s(%d) %f %d° %d", rp.getName().toLatin1().data(), rp.getElevation(),  rp.getDistance().getKilometers(), rp.getBearing(), (int)rp->getArrivalAlt().getMeters() );

        }
    }
  // qDebug("accepted: %d, rejected: %d. Percent reject: %f",a,r,(100.0*r)/(a+r));
}

QColor ReachableList::getReachColor( const QPoint& position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if ( arrivalAltMap.contains( ReachableList::coordinateString( position ) ) )
    {
      if ( arrivalAltMap[ ReachableList::coordinateString ( position ) ] > safetyAlt )
        {
          return( Qt::green );
        }
      else if ( arrivalAltMap[ ReachableList::coordinateString ( position ) ] > 0 )
        {
          return( Qt::magenta );
        }
      else
        {
          return( Qt::red );
        }
    }
  return( Qt::red );
}


int ReachableList::getArrivalAlt( const QPoint& position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if ( arrivalAltMap.contains( coordinateString ( position ) ) )
    {
      return( arrivalAltMap[ coordinateString ( position ) ] - safetyAlt );
    }

  return( -9999 );
}

Altitude ReachableList::getArrivalAltitude( const QPoint& position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if ( arrivalAltMap.contains( coordinateString ( position ) ) )
    {
      return (Altitude( arrivalAltMap[ coordinateString ( position ) ]) - safetyAlt) ;
    }

  return Altitude(); //return an invalid altitude
}

Distance ReachableList::getDistance( const QPoint& position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if ( distanceMap.contains( coordinateString ( position ) ) )
    {
      return( distanceMap[ coordinateString ( position ) ] );
    }

  return Distance();    //return an invalid distance
}

ReachablePoint::reachable ReachableList::getReachable( const QPoint& position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if ( arrivalAltMap.contains( coordinateString ( position ) ) )
    {
      if ( arrivalAltMap[ coordinateString ( position ) ] > safetyAlt )
        return ReachablePoint::yes;
      else if ( arrivalAltMap[ coordinateString ( position ) ] > 0 )
        return ReachablePoint::belowSafety;
      else
        return ReachablePoint::no;
    }

  return ReachablePoint::no;
}

/**
 * Calculate course, distance and reachability from the current position
 * to the elements contained in the limited list. If a glider is defined
 * the glide path is taken into account and the arrival altitude is
 * calculated too.
 */
void ReachableList::calculateDataInList()
{
  // QTime t;
  // t.start();
  int counter = 0;
  setInitValues();
  arrivalAltMap.clear();
  distanceMap.clear();

  for (int i = 0; i < count(); i++)
    {
      // recalculate Distance
      ReachablePoint& p = (*this)[i];
      WGSPoint pt = p.getWaypoint()->wgsPoint;
      Altitude arrivalAlt;
      Distance distance;
      Speed bestSpeed;

      distance.setKilometers( dist(&lastPosition, &pt) );

      if ( lastPosition == pt || distance.getMeters() <= 100.0 )
        {
          // @AP: there is nearly no difference between the two points,
          // therefore we have no distance and no bearing
          distance.setMeters(0.0);
          p.setDistance( distance );
          p.setBearing( 0 );
          p.setArrivalAlt( calculator->getAltitudeCollection().gpsAltitude  );
        }
      else
        {
          p.setDistance( distance );

          // recalculate Bearing
          p.setBearing( short (rint(getBearingWgs(lastPosition, pt) * 180/M_PI)) );

          // Calculate glide path. Returns false, if no glider is known.
          calculator->glidePath( p.getBearing(), p.getDistance(),
                                 Altitude(p.getElevation()),
                                 arrivalAlt, bestSpeed );

          // Save arrival altitude. Is set to invalid, if no glider is defined in calculator.
          p.setArrivalAlt( arrivalAlt );
        }

      if ( arrivalAlt.isValid() )
        {
          // add only valid altitudes to the map
          arrivalAltMap[ coordinateString ( pt ) ] = (int) arrivalAlt.getMeters() + safetyAlt;
        }

      distanceMap[ coordinateString ( pt ) ] = distance;

      if ( arrivalAlt.getMeters() > 0 )
        {
          counter++;
        }
    }

  // sorting of items depends on the glider selection
  if ( calculator->glider() )
    {
      modeAltitude = true; // glider is known, sort by arrival altitudes
    }
  else
    {
      modeAltitude = false; // glider is unknown, sort by distances
    }

  qSort( begin(), end() );
  // qDebug("Number of reachable sites (arriv >0): %d", counter );
  // qDebug("Time for glide path calculation: %d msec", t.restart() );
  emit newReachList();
}

void ReachableList::setInitValues()
{
  // This info we do need from the calculator
  lastPosition = calculator->getlastPosition();
  lastAltitude = calculator->getlastAltitude().getMeters();
  _maxReach = RANGE_RADIUS; // default range radius
  Polar* polar = calculator->getPolar();
  calcMode = ReachableList::altitude;

  if ( ! polar )
    {
      calcMode = ReachableList::distance;
      // If no glider selected calculate nearest sites only.
      return;
    }

  Speed speed = polar->bestSpeed(0.0, 0.0, Speed(0));

  // qDebug("speed for best LD= %f", speed.getKph() );
  double ld = polar->bestLD( speed, speed, 0.0 );  // for coarse estimation (no wind)

  _maxReach = qMax( (lastAltitude/1000) * ld, _maxReach ); // look at least within 75km
  // Thats the maximum range we can reach
  // it in hard to get under sea level :-)
  // qDebug("maxReach: %f %f", _maxReach, (float)ld );
}

void ReachableList::calculateNewList()
{
  // qDebug( "ReachableList::calculateNewList() is called" );

  // calculateNewList is also called from calculator, so on check has to be
  // executed
  if ( !isOn() )
    {
      return;
    }

  // QTime t; // timer for performance measurement
  // t.start();

  setInitValues();
  clearLists();  // clear all lists

  // Now add items of different type to the list
  addItemsToList(MapContents::AirfieldList);
  addItemsToList(MapContents::GliderfieldList);
  addItemsToList(MapContents::OutLandingList);
  addItemsToList(MapContents::WaypointList);
  modeAltitude = false;
  //qDebug("Number of potential reachable sites: %d", count() );

  // sort list according to distances
  qSort(begin(), end() );
  removeDoubles();
  // qDebug("Number of potential reachable sites (after pruning): %d", count() );

  // Remove all elements over the maximum. That are the far away elements.
  int nr = getMaxNrOfSites();

  while ( nr < size() )
    {
      removeFirst();
    }

  // qDebug("Limited Number of potential reachable sites: %d", count() );
  calculateDataInList();
  //qDebug("Time for full calculation: %d msec", t.restart() );
}

// prints list to qDebug interface
void ReachableList::show()
{
  for (int i = 0; i < count(); i++)
    {
      const ReachablePoint& p = at(i);
      qDebug("[%i] %s(%d) %f %d° %d",
             i,
             p.getName().toLatin1().data(),
             p.getElevation(),
             p.getDistance().getKilometers(),
             p.getBearing(),
             (int)p.getArrivalAlt().getMeters() );
    }
}

/**
 * Removes double entries from the list. Double entries can occur
 * when a point is a waypoint as well as an airfield. In this case,
 * the one with the higher severity or longer name is preferred.
 * This function ASSUMES THE LIST IS SORTED!
 */
void ReachableList::removeDoubles()
{
  QList<int> removeList;

  for (int i=count()-1; i>=1; i--)
    {
      const ReachablePoint& p1 = at(i);

      for (int j=0; j<i; j++)
        {
          const ReachablePoint& p2 = at(j);

          //qDebug("i=%d j=%d, P1=%s, P2=%s,", i, j, p1.getWaypoint()->name.toLatin1().data(), p2.getWaypoint()->name.toLatin1().data());

          if ( p2.getWaypoint()->wgsPoint == p1.getWaypoint()->wgsPoint )
            {
              //qDebug("  points are the same");
              // ok, the points are the same. Now we choose which one to remove

              if ( p2.getWaypoint()->priority != p1.getWaypoint()->priority )
                {
                  // the waypoint with the lower priority will be removed
                  if ( p2.getWaypoint()->priority > p1.getWaypoint()->priority )
                    {
                      removeList.append(i);
                    }
                  else
                    {
                      removeList.append(j);
                    }
                  continue;
                }

              if ( p2.getWaypoint()->name == p1.getWaypoint()->name )
                {
                  // both name are identical. remove the one which has not set
                  // the airfield origin flag.
                  if( ! p1.isOrignAfl() )
                     {
                       removeList.append(i);
                       continue;
                     }

                  if( ! p2.isOrignAfl() )
                     {
                       removeList.append(j);
                       continue;
                     }

                  // both origin flags are set, remove one of the objects
                  removeList.append(j);
                  continue;
                }

              if ( p2.getWaypoint()->name.length() == p1.getWaypoint()->name.length() )
                {
                  // the lengths of the names are the same
                  // remove the one with the lowest alphabetical value
                  // (remember that A<a)
                  if (p2.getWaypoint()->name > p1.getWaypoint()->name)
                    {
                      removeList.append(i);
                    }
                  else
                    {
                      removeList.append(j);
                    }
                  continue;
                }

              // if the names are not of equal length, remove the shortest.
              if ( p2.getWaypoint()->name.length() > p1.getWaypoint()->name.length() )
                {
                  removeList.append(i);
                }
              else
                {
                  removeList.append(j);
                }
              continue;
            }

          // qDebug("  points are NOT the same: (%d, %d) vs (%d, %d)",p1.getWaypoint()->origP.x(),p1.getWaypoint()->origP.y(),p2.getWaypoint()->origP.x(),p2.getWaypoint()->origP.y());
        } // End of for j
    } // End of for i

  // sort remove list
  qSort( removeList.begin(), removeList.end(), qLess<int>() );

  // Start remove at the end of the list so that access index is always valid.
  for (int i = removeList.count()-1; i >= 0; i--)
    {
      // qDebug("Removing point %d (%s)", removeList.at(i), at(removeList.at(i)).getWaypoint()->name.toLatin1().data());
      // remove doubles from global list
      removeAt( removeList.at(i) );
    }
}

