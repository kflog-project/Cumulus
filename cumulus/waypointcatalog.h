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
**          modified 2008-2013 by Axel Pauli
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
 * This class reads or writes the waypoint catalog data into a file. Filters can
 * be set to reduce the data mount to be read.
 *
 * \date 2002-2013
 */

#ifndef WAYPOINT_CATALOG_H
#define WAYPOINT_CATALOG_H

#include <QString>
#include <QList>

#include "basemapelement.h"
#include "waypoint.h"
#include "wgspoint.h"

class WaypointCatalog
{
 public:

  enum WpType { All, Airfields, Gliderfields, Outlandings, OtherPoints };

  WaypointCatalog();

  virtual ~WaypointCatalog();

  /** read in binary data catalog from file name */
  int readBinary( QString catalog, QList<Waypoint>* wpList );

  /** write out binary data catalog to file name */
  bool writeBinary( QString catalog, QList<Waypoint>& wpList );

  /** read in KFLog xml data catalog from file name */
  int readXml( QString catalog, QList<Waypoint>* wpList, QString& errorMsg );

  /** write out KFLog xml data catalog to file name */
  bool writeXml( QString catalog, QList<Waypoint>& wpList );

  /**
   * Reads a SeeYou cup file, only the waypoint part.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readCup( QString catalog, QList<Waypoint>* wpList );

  /**
   * Reads a Cambridge Aero Instruments or a Winpilot turnpoint file.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readDat( QString catalog, QList<Waypoint>* wpList );

  /**
   * Reads the content of an OpenAip XML file.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \param errorInfo Additional text describing error situation more in detail.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readOpenAip( QString catalog, QList<Waypoint>* wpList, QString& errorInfo );

  /**
   * Sets a filter used during read.
   *
   * \param typeIn The waypoint type to be read in.
   *
   * \param radiusIn The radius around the center point.
   *
   * \param centerPointIn the Coordinates of the center point in KFLog format.
   */
  void setFilter( enum WpType typeIn,
                  const int radiusIn,
                  const WGSPoint& centerPointIn )
    {
      _type = typeIn;
      _radius = radiusIn;
      centerPoint = centerPointIn;
    };

  /**
   * Resets a defined filter.
   */
  void resetFilter()
    {
      _radius = -1;
    }

  /**
   * Toggles the wait screen display during file read/write.
   *
   * \param flag True switch wait screen usage on, false switch it off.
   */
  void showProgress( const bool flag )
  {
    _showProgress = flag;
  };

 private:

  /**
   * Reads the navigation aid content of an OpenAip XML file.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \param errorInfo Additional text describing error situation more in detail.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readOpenAipNavAids( QString catalog,
                          QList<Waypoint>* wpList,
                          QString& errorInfo );

  /**
   * Reads the hotspot content of an OpenAip XML file.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \param errorInfo Additional text describing error situation more in detail.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readOpenAipHotspots( QString catalog,
                           QList<Waypoint>* wpList,
                           QString& errorInfo );

  /**
   * Reads the airfield content of an OpenAip XML file.
   *
   * \param catalog Catalog file name with directory path.
   *
   * \param wpList Waypoint list where the read waypoints are stored. If the
   *               wpList is NULL, waypoints are counted only.
   *
   * \param errorInfo Additional text describing error situation more in detail.
   *
   * \return Number of read waypoints. In error case -1.
   */
  int readOpenAipAirfields( QString catalog,
                            QList<Waypoint>* wpList,
                            QString& errorInfo );

  /**
   * \return True, if waypoint type has passed the filter otherwise false.
   */
  bool takeType( enum BaseMapElement::objectType type );

  /**
   * \return True, if waypoint coordinates have passed the filter otherwise false.
   */
  bool takePoint( WGSPoint& point );

  /**
   * Splits a cup file line into its single elements.
   *
   * \param line Line to be split.
   *
   * \param ok True if split was ok otherwise false
   *
   * \return A list with the split elements.
   */
  QList<QString> splitCupLine( QString& line, bool &ok );

 private:

  enum WpType _type;
  int _radius;
  bool _showProgress;
  WGSPoint centerPoint;
};

#endif
