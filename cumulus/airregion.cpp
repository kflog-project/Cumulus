/***********************************************************************
**
**   airregion.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004      by André Somers
**                  2008-2015 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <cmath>

#include <QtCore>

#include "airregion.h"
#include "mapmatrix.h"
#include "mapcalc.h"

// Initialize the static class variables
bool   AirRegion::ms_smallPositionChange = false;
QPoint AirRegion::ms_lastPos = QPoint();
double AirRegion::ms_lastScale = -1.0;
AirspaceWarningDistance AirRegion::ms_lastAwd;
QPainterPath AirRegion::ms_regNear = QPainterPath();
QPainterPath AirRegion::ms_regVeryNear = QPainterPath();
QPoint  AirRegion::ms_lastProjPos = QPoint();
QPoint  AirRegion::ms_lastMapCenter = QPoint();

AirRegion::AirRegion( QPainterPath* region, Airspace* airspace ) :
  m_lastResult(Airspace::none),
  m_region(region),
  m_airspace(airspace),
  m_isNew(true),
  m_lastProjPos(QPoint(0,0))
{
  // set a reference to the related airspace instance
  if( m_airspace )
    {
      m_airspace->setAirRegion(this);
    }
}

AirRegion::~AirRegion()
{
  if( m_region )
    {
      delete m_region;
    }

  // @AP: Remove the class pointer in related AirSpace object. We
  // have a cross reference here. First deleted object will reset
  // its pointer in the other object. Check is necessary to avoid
  // usage of null pointer.
  if( m_airspace )
    {
      m_airspace->setAirRegion( static_cast<AirRegion*> (0) );
    }
}

/**
 * Returns true if the given horizontal position conflicts with the
 * airspace properties
 */
Airspace::ConflictType AirRegion::conflicts( const QPoint& pos,
                                             const AirspaceWarningDistance& awd,
                                             bool* changed )
{
  // If the object is new, the last position is invalid, so we need a full check.
  bool fullCheck = m_isNew;

  if ( parametersChanged(pos, awd) )
    {
      // the parameters are changed, so we need to re-create the regions.
      createRegions();
    }

  if ( ! m_isNew && ! m_lastProjPos.isNull() && ms_lastProjPos == m_lastProjPos )
    {
      // The parameters have not been changed, so we can use what we have.
      // The last result must still be valid!
      if (changed)
	{
	  *changed = false;
	}

      return m_lastResult;
    }

  // We need a full check if the position change is large.
  fullCheck |= !ms_smallPositionChange;

  // don't know why this happens, but it does.
  if ( ms_lastProjPos.x() > 20000 || ms_lastProjPos.x() < -20000 )
    {
      qWarning() << "AirRegion::conflicts(): ms_lastProjPos" << ms_lastProjPos << "is out of range!";

      m_lastResult = Airspace::none;

      if (changed)
	{
	  *changed = true;
	}
      // returning no conflict because of invalid position";
      return Airspace::none;
    }

  Airspace::ConflictType hConflict = Airspace::none;
  m_lastProjPos = ms_lastProjPos;

  // check for horizontal conflicts
  if (fullCheck)
    {
      // we need to do a full check
      if ( m_region->contains(ms_lastProjPos) )
	{
	  hConflict=Airspace::inside;
	}
      else if ( m_region->intersects(ms_regVeryNear) )
	{
	  hConflict=Airspace::veryNear;
	}
      else if ( m_region->intersects(ms_regNear) )
	{
	  hConflict=Airspace::near;
	}
      else
	{
	  hConflict=Airspace::none;
	}
    }
  else
    {
      // we only need to check around the last result. This is a potentially
      // significant save, as intersecting regions is quite expensive.
      switch (m_lastResult)
	{
	case Airspace::inside:

	  if (m_region->contains(ms_lastProjPos))
	    {
	      hConflict=Airspace::inside;
	    }
	  else if ( m_region->intersects(ms_regVeryNear) )
	    {
	      hConflict=Airspace::veryNear;
	    };
	  break;

	case Airspace::veryNear:

	  if (m_region->contains(ms_lastProjPos))
	    {
	      hConflict=Airspace::inside;
	    }
	  else if ( m_region->intersects(ms_regVeryNear) )
	    {
	      hConflict=Airspace::veryNear;
	    }
	  else if ( m_region->intersects(ms_regNear) )
	    {
	      hConflict=Airspace::near;
	    }
	  break;

	case Airspace::near:

	  if ( m_region->intersects(ms_regVeryNear) )
	    {
	      hConflict=Airspace::veryNear;
	    }
	  else if ( m_region->intersects(ms_regNear) )
	    {
	      hConflict=Airspace::near;
	    }
	  else
	    {
	      hConflict=Airspace::none;
	    }
	  break;

	case Airspace::none:

	  if ( m_region->intersects(ms_regNear) )
	    {
	      hConflict=Airspace::near;
	    }
	  else
	    {
	      hConflict=Airspace::none;
	    }
	  break;

	default:
	  break;
	}
    }

  if (changed)
    {
      *changed = (m_lastResult == hConflict);
    }

  m_lastResult = hConflict;
  m_isNew = false;

  return hConflict;
}

bool AirRegion::parametersChanged(const QPoint& pos,
                                  const AirspaceWarningDistance& awd)
{
  extern MapMatrix* _globalMapMatrix;

  if (pos == ms_lastPos &&
      ms_lastAwd == awd &&
      ms_lastScale == _globalMapMatrix->getScale(MapMatrix::CurrentScale) &&
      ms_lastMapCenter == _globalMapMatrix->getMapCenter() )
    {
      return false;
    }

  // 30 meters is a small position change
  QPoint* p = &(const_cast<QPoint&>(pos));
  ms_smallPositionChange = (MapCalc::dist(p, &ms_lastPos) < 0.03);
  ms_lastPos = pos;
  ms_lastAwd = awd;
  ms_lastScale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  ms_lastMapCenter = _globalMapMatrix->getMapCenter();

  return true;
}

void AirRegion::createRegions()
{
  extern MapMatrix* _globalMapMatrix;

  ms_lastProjPos = _globalMapMatrix->map( _globalMapMatrix->wgsToMap(ms_lastPos) );

  Distance dist = ms_lastAwd.horClose * 2;
  int projDist = (int) rint(dist.getMeters()/ms_lastScale);

  ms_regNear = QPainterPath();
  ms_regNear.addEllipse( ms_lastProjPos.x()-projDist/2,
			 ms_lastProjPos.y()-projDist/2,
			 projDist, projDist );

  dist = ms_lastAwd.horVeryClose * 2;
  projDist = (int) rint(dist.getMeters()/ms_lastScale);

  ms_regVeryNear = QPainterPath();
  ms_regVeryNear.addEllipse( ms_lastProjPos.x()-projDist/2,
			     ms_lastProjPos.y()-projDist/2,
			     projDist, projDist );
}
