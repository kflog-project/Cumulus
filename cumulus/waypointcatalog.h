/***********************************************************************
**
**   waypointcatalog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**          modified 2002 by Andre Somers
**          modified 2008 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTCATALOG_H
#define WAYPOINTCATALOG_H

#include <QString>
#include <Q3PtrList>

#include "waypoint.h"


class WaypointCatalog
{
  public:

  WaypointCatalog();
  
  ~WaypointCatalog();

  /** read in catalog from file name */
  bool read( QString *catalog, Q3PtrList<wayPoint> *wpList );
  
  /** write out catalog to file name */
  bool write( QString *catalog, Q3PtrList<wayPoint> *wpList );
};

#endif
