/***********************************************************************
**
**   airspacewarningdistance.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef AIRSPACE_WARNING_DISTANCE_H
#define AIRSPACE_WARNING_DISTANCE_H

#include "distance.h"

/**
  * @short Collection of distances to airspaces
  *
  * This class holds a set of six distances to airspaces, used to warn the user if he's getting
  * (too) close to an airspace.
  *
  * @author André Somers
  */

class AirspaceWarningDistance
{
public:

  Distance horClose;
  Distance horVeryClose;
  Distance verAboveClose;
  Distance verAboveVeryClose;
  Distance verBelowClose;
  Distance verBelowVeryClose;

  bool operator==(const AirspaceWarningDistance& x) const {
      return (
              horClose == x.horClose &&
              horVeryClose == x.horVeryClose &&
              verAboveClose == x.verAboveClose &&
              verAboveVeryClose == x.verAboveVeryClose &&
              verBelowClose == x.verBelowClose &&
              verBelowVeryClose == x.verBelowVeryClose
             );
  }

  bool operator!=(const AirspaceWarningDistance& x) const {
      return !operator==(x);
  }

};

#endif

