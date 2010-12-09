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
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WaypointCatalog
 *
 * \author Harald Maier, André Somers, Axel Pauli
 *
 * \brief Waypoint catalog file handling.
 *
 * This class reads or writes the waypoint catalog into a file.
 *
 * \date 2002-2008
 */

#ifndef WAYPOINT_CATALOG_H
#define WAYPOINT_CATALOG_H

#include <QString>
#include <QList>

#include "waypoint.h"

class WaypointCatalog
{
  public:

  WaypointCatalog();

  ~WaypointCatalog();

  /** read in catalog from file name */
  bool read( QString *catalog, QList<wayPoint>& wpList );

  /** write out catalog to file name */
  bool write( QString *catalog, QList<wayPoint>& wpList );
};

#endif
