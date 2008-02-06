/***********************************************************************
**
**   airregion.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2004 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <math.h>

#include "airregion.h"
#include "mapmatrix.h"
#include "mapcalc.h"
/**
 * @short Container for @ref Airspace objects with a QRegion
 * @author André Somers
 *
 * Contains the regions of all visible airspaces. The map maintains a list to
 * find the airspace-data when the users selects a airspace in the map.
 * Takes ownership of the region, but not of the airspace!
 */

AirRegion::AirRegion(QRegion * reg, Airspace * air)
{
    region   = reg;
    airspace = air;
    m_isNew = true;
    m_lastResult = Airspace::none;
    m_lastProjPos = QPoint(0,0);

    //set a reference to ourself
    airspace->setAirRegion(this);
}


AirRegion::~AirRegion()
{
    delete region;

    // @AP: Remove the class pointer in related AirSpace object. We
    // have a cross reference here. First deleted object will reset
    // its pointer in the other object. Check is necessary to avoid
    // usage of null pointer.

    if( airspace ) {
      airspace->setAirRegion(0);
    }
}


/**
 * Returns true if the given horizontal position conflicts with the
 * airspace properties
 */
Airspace::ConflictType AirRegion::conflicts( const QPoint& pos,
        const AirspaceWarningDistance& awd, bool* changed ) const
{
    //if the object is new, the last position is invalid, so we need a
    //full check
    bool fullCheck = m_isNew;

    if (parametersChanged(pos, awd)) {
        //the parameters changed, so we need to re-create the regions.
        createRegions();
    }

    if (!m_isNew && ms_lastProjPos == m_lastProjPos) {
        //The parameters have not changed, so we can use what we have.
        //The last result must still be valid!
        if (changed)
            *changed = false;
        return m_lastResult;
    }

    //We need a full check if the position change is large.
    fullCheck |= !ms_smallPositionChange;

    // dont know why this happens, but it does.
    if ( ms_lastProjPos.x() > 10000 || ms_lastProjPos.x() < -10000 ) {
        m_lastResult = Airspace::none;
        if (changed)
            *changed = true;
        //qDebug("returning no conflict because of invalid position");
        return Airspace::none;
    }

    Airspace::ConflictType hConflict = Airspace::none;
    m_lastProjPos = ms_lastProjPos;

    // check for horizontal conflicts
    if (fullCheck) {
        //we need to do a full check
        if (region->contains(ms_lastProjPos)) {
            hConflict=Airspace::inside;
        } else if (! region->intersect(ms_regVeryNear).isEmpty()) {
            hConflict=Airspace::veryNear;
        } else if(! region->intersect(ms_regNear).isEmpty()) {
            hConflict=Airspace::near;
        } else {
            hConflict=Airspace::none;
        }
    } else {
        // we only need to check around the last result. This is a potentially
        // significant save, as intersecting regions is quite expensive.
        switch (m_lastResult) {
            case Airspace::inside:
                if (region->contains(ms_lastProjPos)) {
                    hConflict=Airspace::inside;
                } else if (! region->intersect(ms_regVeryNear).isEmpty()) {
                    hConflict=Airspace::veryNear;
                };
                break;
            case Airspace::veryNear:
                if (region->contains(ms_lastProjPos)) {
                    hConflict=Airspace::inside;
                } else if (! region->intersect(ms_regVeryNear).isEmpty()) {
                    hConflict=Airspace::veryNear;
                } else if(! region->intersect(ms_regNear).isEmpty()) {
                    hConflict=Airspace::near;
                }
                break;
            case Airspace::near:
                if (! region->intersect(ms_regVeryNear).isEmpty()) {
                    hConflict=Airspace::veryNear;
                } else if(! region->intersect(ms_regNear).isEmpty()) {
                    hConflict=Airspace::near;
                } else {
                    hConflict=Airspace::none;
                }
                break;
            case Airspace::none:
                if(! region->intersect(ms_regNear).isEmpty()) {
                    hConflict=Airspace::near;
                } else {
                    hConflict=Airspace::none;
                }
                break;
            default:
                break;
        }
    }

    if (changed)
        *changed = (m_lastResult == hConflict);

    m_lastResult = hConflict;
    m_isNew = false;
    //qDebug("horizontal conflict: %d, airspace: %s", hConflict, airspace->getName().latin1());
    return hConflict;
}


bool AirRegion::parametersChanged(const QPoint& pos,
                                  const AirspaceWarningDistance& awd) const
{
    extern MapMatrix * _globalMapMatrix;

    if (pos == ms_lastPos &&
        ms_lastAwd == awd &&
        ms_lastScale == _globalMapMatrix->getScale(MapMatrix::CurrentScale))
        return false;

    // 30 meters is a small position change
    QPoint* p = &(const_cast<QPoint&>(pos));
    ms_smallPositionChange = (dist(p, &ms_lastPos) < 0.03);
    ms_lastPos = pos;
    ms_lastAwd = awd;
    ms_lastScale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

    
    //qDebug("parametersChanged, smallPositionChange: %d", ms_smallPositionChange);
    return true;
}


void AirRegion::createRegions() const
{
    extern MapMatrix * _globalMapMatrix;

    ms_lastProjPos = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(ms_lastPos));

    Distance dist = ms_lastAwd.horClose * 2;
    int projDist = (int)rint(dist.getMeters()/ms_lastScale);
    ms_regNear = QRegion(ms_lastProjPos.x()-projDist/2, ms_lastProjPos.y()-projDist/2, projDist, projDist, QRegion::Ellipse);

    dist = ms_lastAwd.horVeryClose*2;
    projDist = (int)rint(dist.getMeters()/ms_lastScale);
    ms_regVeryNear = QRegion(ms_lastProjPos.x()-projDist/2, ms_lastProjPos.y()-projDist/2, projDist, projDist, QRegion::Ellipse);

}

//initialize the static variables
bool AirRegion::ms_smallPositionChange = false;
QPoint AirRegion::ms_lastPos = QPoint();
double AirRegion::ms_lastScale = -1.0;
AirspaceWarningDistance AirRegion::ms_lastAwd;
QRegion AirRegion::ms_regNear = QRegion();
QRegion AirRegion::ms_regVeryNear = QRegion();
QPoint AirRegion::ms_lastProjPos = QPoint();

