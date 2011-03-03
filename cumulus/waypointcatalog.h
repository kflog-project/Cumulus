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
**          modified 2008-2011 by Axel Pauli
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
 * This class reads or writes the waypoint catalog data into a file.
 *
 * \date 2002-2011
 */

#ifndef WAYPOINT_CATALOG_H
#define WAYPOINT_CATALOG_H

#include <QString>
#include <QList>

#include "waypoint.h"
#include "wgspoint.h"

class WaypointCatalog
{
 public:

  enum wpType { All, Airfields, Gliderfields, Outlandings, UlFlields, OtherPoints };

  WaypointCatalog();

  ~WaypointCatalog();

  /** read in binary data catalog from file name */
  bool readBinary( QString catalog, QList<Waypoint>& wpList );

  /** write out binary data catalog to file name */
  bool writeBinary( QString catalog, QList<Waypoint>& wpList );

  /** read in KFLog xml data catalog from file name */
  bool readXml( QString catalog, QList<Waypoint>& wpList );

  /** write out KFLog xml data catalog to file name */
  bool writeXml( QString catalog, QList<Waypoint>& wpList );

  /** read SeeYou cup file, only waypoint part */
  bool readCup( QString catalog, QList<Waypoint>& wpList );

  /**
   * Sets a filter used during read.
   *
   * \param typeIn The waypoint type to be read in.
   *
   * \param radiusIn The radius around the center point.
   *
   * \param centerPointIn the Coordinates of the center point in KFLog format.
   */
  void setFilter( enum wpType typeIn,
                  const int radiusIn,
                  const WGSPoint& centerPointIn )
    {
      type = typeIn;
      radius = radiusIn;
      centerPoint = centerPointIn;
    };

  /**
   * Resets a defined filter.
   */
  void resetFilter()
    {
      radius = -1;
    }

 private:
  /**
   * Splits a cup file line into its single elements.
   *
   * \param line Line to be split.
   *
   * \param ok True if split was ok otherwise false
   *
   * \return A list with the splt elements.
   */
  QList<QString> splitCupLine( QString& line, bool &ok );

 private:

  enum wpType type;
  int radius;
  WGSPoint centerPoint;
};

#endif
