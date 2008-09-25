/***********************************************************************
 **
 **   reachablelist.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by Eckhard V�llm, 2008 Axel pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

// Calculates the destination, bearing, reachability of sites during
// flight.

#include <math.h>

#include <QString>
#include <Qt>
#include <QList>
#include <QColor>
#include <QtAlgorithms>
#include <QDateTime>

#include "reachablelist.h"
#include "cucalc.h"
#include "mapcontents.h"
#include "glidersite.h"
#include "mapcalc.h"
#include "polar.h"
#include "waypoint.h"
#include "airport.h"

extern MapContents *_globalMapContents;

// Initialize static members
int  ReachableList::safetyAlt = 0;
QMap<QString, int> ReachableList::arrivalAltMap;
QMap<QString, Distance> ReachableList::distanceMap;
bool ReachableList::modeAltitude = false;

// construct from airfield database
ReachablePoint::ReachablePoint(QString name,
                               QString icao,
                               QString description,
                               bool orignAfl,
                               int type,
                               double frequency,
                               WGSPoint pos,
                               QPoint   ppos,
                               int elevation,
                               Distance distance,
                               int bearing,
                               Altitude arrivAlt,
                               int rwDir,
                               int rwLen,
                               int rwSurf,
                               bool rwOpen )
{
  _wp = new wayPoint;
  _wp->name = name;
  _wp->icao = icao;
  _wp->description = description;
  _wp->frequency = frequency;
  _wp->elevation = elevation;
  _wp->importance = wayPoint::High; // high to make sure it is visible
  _wp->isLandable = rwOpen;
  _wp->surface = rwSurf;
  _wp->runway = rwDir;
  _wp->length = rwLen;
  _wp->sectorFAI = 0;
  _wp->sector1 = 0;
  _wp->sector2 = 0;
  _wp->origP = pos;
  _wp->projP = ppos;
  _wp->type = type;

  _orignAfl = orignAfl;
  _distance = distance;
  _arrivalAlt=arrivAlt;
  _bearing = bearing;
};


// Construct from another WP
ReachablePoint::ReachablePoint(wayPoint *wp,
                               bool orignAfl,
                               Distance distance,
                               int bearing,
                               Altitude arrivAlt )
{
  _wp = new wayPoint(*wp); // deep copy
  _orignAfl = orignAfl;
  _distance = distance;
  _arrivalAlt=arrivAlt;
  _bearing = bearing;
};

ReachablePoint::~ReachablePoint()
{
  delete _wp;
}

reachable ReachablePoint::getReachable()
{
  if( _arrivalAlt.isValid() && _arrivalAlt.getMeters()> 0 )
    {
      return yes;
    }
  else if( _arrivalAlt.isValid() && _arrivalAlt.getMeters()> -ReachableList::getSafetyAltititude() )
    {
      return belowSafety;
    }
  else
    {
      return no;
    }
}

bool ReachablePoint::operator < (const ReachablePoint& other) const
{
  if( ReachableList::getModeAltitude() )
    {
      return (getArrivalAlt().getMeters() < other.getArrivalAlt().getMeters());
    }
  else
    {
      return (getDistance().getKilometers()> other.getDistance().getKilometers());
    }
}

//***********************************************************************************

ReachableList::ReachableList(QObject *parent) : QObject(parent)
{
  GeneralConfig *conf = GeneralConfig::instance();
  safetyAlt = (int) conf->getSafetyAltitude().getMeters();

  lastAltitude = 0.0;
  _maxReach = 0.0;
  tick = 0;
  modeAltitude = false;
  initValuesOK = false;
  calcMode = ReachableList::altitude;
}

ReachableList::~ReachableList()
{
  while (!isEmpty()) delete takeFirst();
}

void ReachableList::calculate(bool always)
{
  if( !isOn() ) {
    //qDebug("ReachableList::calculate is off");
    return;
  }
  tick++;
  // qDebug("tick %d %d",tick, always );
  // in manual Mode calculate more often full list from due to big distance changes
  if( (tick %64) == 0 || (always &&((tick %8) == 0)) )
    calculateFullList();
  else if( ((tick %8) == 0) || always )
    calculateGlidePath();
}


void ReachableList::addItemsToList(enum MapContents::MapContentsListID item)
{
  Distance distance;
  QRect bbox;
  bbox=areaBox(lastPosition,_maxReach);
  int r=0, a=0;
  //qDebug("bounding box: (%d, %d), (%d, %d) (%d x %d km)", bbox.left(),bbox.top(),bbox.right(),bbox.bottom(),0,0);

  if( item == MapContents::WaypointList ) {
    // Waypoints have different structure treat them here
    QList<wayPoint*> *pWPL = _globalMapContents->getWaypointList();
    // qDebug("Nr of Waypoints: %d", pWPL->count() );
    for( int i=0; i<pWPL->count(); i++ ) {
      WGSPoint pt = pWPL->at(i)->origP;
      if (!bbox.contains(pt)) {
        //qDebug("Not in bounding box, so ignore! (distance: %d, (%d, %d), %s)", (int)distance.getKilometers(), pt.x(),pt.y(), pWPL->at(i)->name.latin1());
        r++;
        continue;
      }  else {
        a++;
        //qDebug("In bounding box, so accept! (distance: %d, %s)", (int)distance.getKilometers(), pWPL->at(i)->name.latin1());
      }
      distance.setKilometers(dist(&lastPosition,&pt));
      // check if point is a potential reachable candidate at best LD
      if( (distance.getKilometers() > _maxReach ) ||
          !(pWPL->at(i)->isLandable || (pWPL->at(i)->type == BaseMapElement::Outlanding) )  )
        continue;
      // calculate bearing
      double result=getBearing(lastPosition, pt);
      int bearing =int(rint(result * 180./M_PI));
      ReachablePoint *rp = new ReachablePoint( pWPL->at(i),
                                               false,
                                               distance,
                                               bearing,
                                               Altitude( 0 )  );
      append(rp);
    }
  } else {
    // get number of elements in the list
    int nr = _globalMapContents->getListLength(item);
    // qDebug("Nr of sites: %d type %d", nr, item );
    for(int i=0; i<nr; i++ ) {
      // get specific site via base class, that contains the needed infos
      Airport* site = (Airport*)_globalMapContents->getElement( item, i );
      WGSPoint pt  = site->getWGSPosition();

      if (!bbox.contains(pt)) {
        //qDebug("Not in bounding box, so ignore! (distance: %d, (%d, %d), %s)", (int)distance.getKilometers(), pt.x(),pt.y(), site->getName().latin1());
        r++;
        continue;
      }  else {
        a++;
        //qDebug("In bounding box, so accept! (distance: %d, %s)", (int)distance.getKilometers(), site->getName().latin1());
      }
      distance.setKilometers(dist(&lastPosition,&pt));
      // qDebug("%d  %f %f", i, (float)distance.getKilometers(),_maxReach );
      // check if point is a potential reachable candidate at best LD
      if( distance.getKilometers() > _maxReach )
        continue;
      // calculate bearing
      double result=getBearing(lastPosition, pt);
      int bearing =int(rint(result * 180./M_PI));
      // add all potential reachable points to the list, altitude is calculated later
      ReachablePoint *rp = new ReachablePoint( site->getWPName(),
                                               site->getICAO(),
                                               site->getName(),
                                               true,
                                               site->getTypeID(),
                                               site->getFrequency().toDouble(),
                                               site->getWGSPosition(),
                                               site->getPosition(),
                                               site->getElevation(),
                                               distance,
                                               bearing,
                                               Altitude( 0 ),
                                               site->getRunway().direction,
                                               site->getRunway().length,
                                               site->getRunway().surface,
                                               site->getRunway().isOpen );
      append(rp);
      // qDebug("%s(%d) %f %d� %d", (const char *)rp->getName(), rp->getElevation(),  rp->getDistance().getKilometers(), rp->getBearing(), (int)rp->getArrivalAlt().getMeters() );

    }
  }
  // qDebug("accepted: %d, rejected: %d. Percent reject: %f",a,r,(100.0*r)/(a+r));
}

QColor ReachableList::getReachColor( QPoint position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if( arrivalAltMap.contains( ReachableList::coordinateString( position ) ) ) {
    if( arrivalAltMap[ ReachableList::coordinateString ( position ) ] > safetyAlt ) {
      return( Qt::green );
    } else if( arrivalAltMap[ ReachableList::coordinateString ( position ) ] > 0 ) {
      return( Qt::magenta );
    } else {
      return( Qt::red );
    }
  }
  return( Qt::red );
}


int ReachableList::getArrivalAlt( QPoint position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if( arrivalAltMap.contains( coordinateString ( position ) ) ) {
    return( arrivalAltMap[ coordinateString ( position ) ] - safetyAlt );
  }
  return( -9999 );
}


Altitude ReachableList::getArrivalAltitude( QPoint position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if( arrivalAltMap.contains( coordinateString ( position ) ) ) {
    return (Altitude( arrivalAltMap[ coordinateString ( position ) ]) - safetyAlt) ;
  }
  return Altitude(); //return an invalid altitude
}


Distance ReachableList::getDistance( QPoint position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if( distanceMap.contains( coordinateString ( position ) ) ) {
    return( distanceMap[ coordinateString ( position ) ] );
  }
  return Distance();    //return an invalid distance
}


//reachable ReachableList::getReachable( QString name )
reachable ReachableList::getReachable( QPoint position )
{
  // qDebug("name: %s: %d", (const char *)name, arrivalAltMap[name] );

  if( arrivalAltMap.contains( coordinateString ( position ) ) ) {
    if( arrivalAltMap[ coordinateString ( position ) ] > safetyAlt )
      return yes;
    else if( arrivalAltMap[ coordinateString ( position ) ] > 0 )
      return belowSafety;
    else
      return no;
  }
  return no;
}


// calculate glide path, based on the estimated limited list
void ReachableList::calculateGlidePath()
{
  QTime t;
  t.start();
  int counter = 0;
  calcInitValues();
  Distance distance;
  arrivalAltMap.clear();
  distanceMap.clear();

  for (int i = 0; i < count(); i++) {
    // recalculate Distance
    ReachablePoint *p = at(i);
    WGSPoint pt = p->getWaypoint()->origP;
    Altitude arrivalAlt;
    Speed bestSpeed;

    distance.setKilometers( dist(&lastPosition, &pt) );

    if( lastPosition == pt || distance.getMeters() <= 100.0 ) {
      // @AP: there is nearly no difference between the two points,
      // therefore we have no distance and no bearing
      distance.setMeters(0.0);
      p->setDistance( distance );
      p->setBearing( 0 );
      p->setArrivalAlt( calculator->getAltitudeCollection().gpsAltitude  );
    }
    else {
      p->setDistance( distance );

      // recalculate Bearing
      p->setBearing( int(rint(getBearingWgs(lastPosition, pt) * 180/M_PI)) );

      // Calculate glide path. Returns false, if no glider is known.
      calculator->glidePath( p->getBearing(), p->getDistance(),
                             Altitude(p->getElevation()),
                             arrivalAlt, bestSpeed );

      // Save arrival altitude. Is set to invalid, if no glider is defined in calculator.
      p->setArrivalAlt( arrivalAlt );
    }

    if( arrivalAlt.isValid() ) {
      // add only valid altitudes to the map
      arrivalAltMap[ coordinateString ( pt ) ] = (int) arrivalAlt.getMeters() + safetyAlt;
    }

    distanceMap[ coordinateString ( pt ) ] = distance;

    if( arrivalAlt.getMeters() > 0 )
      counter++;
  }

  // sorting of items depends on the glider selection
  if( calculator->glider() ) {
    modeAltitude = true; // glider is known
  }
  else {
    modeAltitude = false; // glider is unknown
  }

  std::sort(begin(), end(), CompareReachablePoints());
  // qDebug("Number of reachable sites (arriv >0): %d", counter );
  // qDebug("Time for glide path calculation: %d msec", t.restart() );
  emit newReachList();
}


void ReachableList::calcInitValues()
{
  // This info we do need from the calculator
  lastPosition = calculator->getlastPosition();
  lastAltitude = calculator->getlastAltitude().getMeters();
  _maxReach = 50.0; // default radius is 50km
  Polar* polar = calculator->getPolar();
  calcMode = ReachableList::altitude;

  if( ! polar ) {
    calcMode = ReachableList::distance;
    // If no glider selected exit!
    return;
  }

  Speed speed = polar->bestSpeed(0.0, 0.0, Speed(0));

  // qDebug("speed for best LD= %f", speed.getKph() );
  double ld = polar->bestLD( speed, speed, 0.0 );  // for coarse estimation (no wind)

  _maxReach = MAX( (lastAltitude/1000)*ld, 50.0 ); // look at least within 50km
  // Thats the maximum range we can reach
  // it in hard to get under sea level :-)
  // qDebug("maxReach: %f %f", _maxReach, (float)ld );
}


void ReachableList::calculateFullList()
{
  // calculateFullList also called from cucalc, so on check has to be
  // executed
  if( !isOn() ) {
    return;
  }

  QTime t; // timer for performance measurement
  t.start();
  // This info we need from calculator
  calcInitValues();
  clearList();  // clear list
  // Now add items of different type to the list
  addItemsToList(MapContents::AirportList);
  addItemsToList(MapContents::GliderSiteList);
  addItemsToList(MapContents::AddSitesList);
  addItemsToList(MapContents::WaypointList);
  // qDebug("Number of potential reachable sites: %d", count() );
  modeAltitude = false;
  std::sort(begin(), end(), CompareReachablePoints() );
  removeDoubles();
  // qDebug("Number of potential reachable sites (after pruning): %d", count() );
  int size = count();
  int nr;
  // Keep the coarse list maximum of double of size as the precise list
  for( nr= getMaxNrOfSites()*2; nr<size; nr++ ) {
    removeLast();  // remove elements > max number of sites
  }
  // qDebug("Limited Number of potential reachable sites: %d", count() );
  calculateGlidePath();
  // qDebug("Time for full calculation: %d msec", t.restart() );
}


// prints list to qDebug interface
void ReachableList::show()
{
  for (int i = 0; i < count(); i++) {
    ReachablePoint *p = at(i);
    qDebug("%s(%d) %f %d° %d",
           p->getName().toLatin1().data(),
           p->getElevation(),
           p->getDistance().getKilometers(),
           p->getBearing(),
           (int)p->getArrivalAlt().getMeters() );
  }
}

// returns number of sites
const int ReachableList::getNumberSites() const
{
  return count();
}


// returns site at index (max =  getNumberSites()-1)
ReachablePoint *ReachableList::getSite( int index )
{
  ReachablePoint * p = at(index);
  return( p );
}

void ReachableList::clearList() {
  while (!isEmpty()) delete takeFirst();
  arrivalAltMap.clear();
  distanceMap.clear();
}


void ReachableList::removeDoubles()
{
  ReachablePoint * p1;
  ReachablePoint * p2;
  QList<int> removeList;

  for (int i=count()-1; i>=1; i--) {
    p1=at(i);

    for (int j=0; j<i; j++) {
      p2=at(j);

      //qDebug("i=%d j=%d, P1=%s, P2=%s,", i, j, p1->getWaypoint()->name.latin1(), p2->getWaypoint()->name.latin1());

      if ( p2->getWaypoint()->origP == p1->getWaypoint()->origP ) {
        //qDebug("  points are the same");
        // ok, the points are the same. Now we choose which one to remove

        if( p2->getWaypoint()->importance != p1->getWaypoint()->importance ) {
          // the waypoint with the lower importance will be removed
          if( p2->getWaypoint()->importance > p1->getWaypoint()->importance ) {
            removeList.append(i);
          } else {
            removeList.append(j);
          }
          continue;
        }

        if( p2->getWaypoint()->name.length() == p1->getWaypoint()->name.length() ) {
          // the lengths of the names are the same, remove
          // the one with the lowest alphabetical value
          // (remember that A<a)
          if (p2->getWaypoint()->name > p1->getWaypoint()->name) {
            removeList.append(i);
          } else {
            removeList.append(j);
          }
          continue;
        }

        // if the names are not of equal length, remove the shortest.
        if ( p2->getWaypoint()->name.length() > p1->getWaypoint()->name.length() ) {
          removeList.append(i);
        } else {
          removeList.append(j);
        }
        continue;
      }

      // qDebug("  points are NOT the same: (%d, %d) vs (%d, %d)",p1->getWaypoint()->origP.x(),p1->getWaypoint()->origP.y(),p2->getWaypoint()->origP.x(),p2->getWaypoint()->origP.y());
      //p1=at(i);

    } // End of for j
  } // End of for i

  qSort( removeList.begin(), removeList.end(), qLess<int>() );

  for (int i=removeList.count()-1; i>=0; i--) {
    //qDebug("Removing point %d (%s)", removeList.at(i),at(removeList.at(i))->getWaypoint()->name.latin1());
    delete takeAt( removeList.at(i) );
  }
}

