/***********************************************************************
**
**   airregion.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/


#ifndef AirRegion_h
#define AirRegion_h

#include <QRegion>
#include <QPoint>

#include "airspace.h"

/**
 * @short Container for @ref Airspace objects with a QRegion
 * @author André Somers
 *
 * Contains the regions of all visible airspaces. The map maintains a list to
 * find the airspace-data when the users selects a airspace in the map.
 * Takes ownership of the region, but not of the airspace!
 */
class AirRegion
{
public:

    QRegion* region;
    Airspace* airspace;

    AirRegion(QRegion* reg, Airspace* air);
    virtual ~AirRegion();

    /**
     * Returns true if the given horizontal position conflicts with
     * the airspace properties
     * @param pos position in WSG coordinates
     * @param awd collection of distances to use for the warnings
     * @param changed set to true if the warning is different from the
     *                last one issued for this airspace
     * @returns a conflict type
     */
    Airspace::ConflictType conflicts( const QPoint& pos,
                                      const AirspaceWarningDistance& awd,
                                      bool* changed = 0 ) const;

    /**
     * @returns the last known horizontal conflict for this airspace
     */
    Airspace::ConflictType currentConflict()
    {
        return m_lastResult;
    }

private:
    /**
     * Create new regions around the current position to check for collisions
     */
    void createRegions() const;
    /**
     * @returns true if the position, the airspace warning distances or the
     * scale has changed. If so, the static parameters that contain these
     * values as well as ms_smallPostionChange are set to reflect the
     * new situation.
     */
    bool parametersChanged(const QPoint& pos, const AirspaceWarningDistance& awd) const;

    /** the result returned last time */
    mutable Airspace::ConflictType m_lastResult;

    /** true if this is the first time this region is used */
    mutable bool m_isNew;

    /**
     * Projected pos *this* instance has last been used with.
     * ms_lastProjPos is the static value, used for the whole class.
     * This value is used to check if we need to re-check for
     * horizontal conflicts.
     */
    mutable QPoint m_lastProjPos;

private: //static values
    //given values
    /** contains the last known position */
    static QPoint ms_lastPos;
    /** contains the last known scale */
    static double ms_lastScale;
    /** contains the last known set of warning distances */
    static AirspaceWarningDistance ms_lastAwd;

    //calculated values based on the values above
    /** contains a large circular region around ms_lastProjPos */
    static QRegion ms_regNear;
    /** contains a small circular region around ms_lastProjPos */
    static QRegion ms_regVeryNear;
    /** contains the last position, projected */
    static QPoint ms_lastProjPos;
    /** true if the last position change was a small change */
    static bool ms_smallPositionChange;

};

#endif
