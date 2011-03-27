/***********************************************************************
 **
 **   mapmatrix.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c): 2001      by Heiner Lamprecht, Florian Ehinger
 **                  2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef MAP_MATRIX_H
#define MAP_MATRIX_H

#include <QObject>
#include <QTransform>
#include <QPolygon>
#include <QString>

#include <stdint.h>
typedef int32_t fp24p8_t;
typedef int32_t fp8p24_t;

#include "projectionlambert.h"
#include "projectioncylindric.h"
#include "waypoint.h"

/**
 * \class MapMatrix
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map projection control class.
 *
 * This class provides functions for converting coordinates between
 * several coordinate-systems. It takes control over the map scale
 * and the projection-type. To avoid problems, there should be only
 * one element per application.
 *
 * \date 2001-2010
 */

class MapMatrix : public QObject
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MapMatrix )

public:
  /**
   * Creates a new MapMatrix object.
   */
  MapMatrix( QObject* object );

  /**
   * Destructor
   */
  virtual ~MapMatrix();

  /**
   * Converts the given geographic-data into the current map-projection.
   *
   * @param  point  The point to be converted. The point must be in
   *                the internal format of 1/10.000 minutes.
   *
   * @return the projected point
   */
  QPoint wgsToMap(const QPoint& point) const;

  /**
   * Converts the given geographic-data into the current map-projection.
   *
   * @param  lat  The latitude of the point to be converted. The point must
   *              be in the internal format of 1/10.000 minutes.
   * @param  lon  The longitude of the point to be converted. The point must
   *              be in the internal format of 1/10.000 minutes.
   *
   * @return the projected point
   */
  QPoint wgsToMap(int lat, int lon) const;

  /**
   * Converts the given geographic-data into the current map-projection.
   *
   * @param  latIn  The latitude of the point to be converted. The point must
   *                be in the internal format of 1/10.000 minutes.
   * @param  lonIn  The longitude of the point to be converted. The point must
   *                be in the internal format of 1/10.000 minutes.
   *
   * @param  latOut The latitude as projected point.
   *
   * @param  lonOut The longitude as projected point.
   *
   */
  void wgsToMap(int latIn, int lonIn, double& latOut, double& lonOut);

  /**
   * Converts the given geographic-data into the current map-projection.
   *
   * @param  rect  The rectangle to be converted. The points must
   *               be in the internal format of 1/10.000 minutes.
   *
   * @return the projected rectangle
   */
  QRect wgsToMap(const QRect& rect) const;

  /**
   * Maps the given projected polygon into the current map-matrix.
   *
   * @param  pPolygon  The polygon to be mapped
   *
   * @return the mapped polygon
   */
  QPolygon map(const QPolygon &pPolygon) const;
#if 0
  {
    return worldMatrix.map(pPolygon);
  };
#endif

  /**
   * Maps the given projected point into the current map-matrix.
   *
   * @param  point  The point to be mapped
   *
   * @return the mapped point
   */
  QPoint map(const QPoint &point) const;
#if 0
  {
    return worldMatrix.map(point);
  };
#endif

  /**
   * Maps the given projected rectangle into the current map-matrix.
   *
   * @param  rect  The rectangle to be mapped
   *
   * @return the mapped rectangle
   */
  QRect map(const QRect& rect) const
  {
    return worldMatrix.mapRect(rect);
  };

  /**
   * Maps the given bearing into the current map-matrix.
   *
   * @param  bearing  The bearing to be mapped
   *
   * @return the mapped bearing
   */
  double map(double bearing) const
  {
    return (bearing + rotationArc);
  };

  /**
   * @param  type  The type of scale to be returned.
   *
   * @return the selected scale
   */
  double getScale(unsigned int type = MapMatrix::CurrentScale);

  /**
   * The ration of MaxScale to the current scale is returned as integer.
   *
   * @return The wanted scale ratio.
   */
  int getScaleRatio()
  {
    return _MaxScaleToCScaleRatio;
  };

  /**
   * Note, the returned map uses the x-axis for longitude and the y-axis
   * for latitude. That is reverse to getMapBorder.
   *
   * @return The lon/lat-border of the current map in KFLog coordinates.
   *
   * @see getMapBorder()
   */
  QRect getViewBorder() const
  {
    return viewBorder;
  };

  /**
   * @return The lat/lon-border of the current map in projected coordinates.
   *
   * @see getViewBorder()
   */
  QRect getMapBorder() const
  {
    return mapBorder;
  };

  /**
   * Initializes the matrix for displaying the map.
   */
  void createMatrix(const QSize& newSize);

  /**
   * @return "true", if the given point in visible in the current map.
   */
  bool isVisible(const QPoint& pos) const
  {
    return (mapBorder.contains(pos));
  };

  /**
   * @return "true", if the given rectangle intersects with the current map.
   */
  bool isVisible( const QRect& itemBorder, int typeID) const;

  /** */
  enum MoveDirection {NotSet = 0, North = 1, West = 2, East = 4,
                      South = 8, Home = 16, Waypoint = 32};
  /**
   * CurrentScale muss immer die groesste Zahl sein!
   */
  enum ScaleType {LowerLimit = 0, Border1 = 1, Border2 = 2, Border3 = 3,
                  UpperLimit = 4, SwitchScale = 5, CurrentScale = 6};
  /**
   * Centers the map to the given point.
   */
  void centerToPoint(const QPoint&);

  /**
   * Centers the map to the given rectangle and scales the map, so that
   * the rectangle will be seen completely.
   */
  double centerToRect(const QRect&, const QSize& = QSize(0,0));

  /** */
  QPoint mapToWgs(const QPoint& pos) const;

  /**
   *
   */
  int getScaleRange() const;

  /**
   * @return "true", if the current scale is smaller than the switch-scale.
   */
  bool isSwitchScale() const;

  /**
   * @return "true", if the current scale is smaller than the second switch-scale.
   */
  bool isSwitchScale2() const;

  /**
   * @return the lat/lon-position of the map center.
   */
  QPoint getMapCenter(bool isPrint = false) const;

  /** */
  void centerToLatLon(const QPoint& center);

  /** */
  void centerToLatLon(int latitude, int longitude);

  /** */
  void writeMatrixOptions();

  /** This function tries to make the given point visible on the
   * map, using only scaling.  This may fail if the point is too
   * far away, so that the required scale is bigger than the
   * limit. If the function fails, the current projection is not
   * changed and false is returned. If the function succeeds, the
   * scale is changed so that the greatest level of detail can be
   * seen while keeping the point on the map. The new scale is
   * returned. */
  double ensureVisible(const QPoint& point);

  /**
   * @returns an indication of the current draw scale, 0 to 2. 0 is small scale, 2 very big scale
   */
  unsigned int currentDrawScale() const;

  /**
   * @returns an indication, if a waypoint can be drawn according to the current
   * scale setting.
   */
  bool isWaypoint2Draw( Waypoint::Priority importance ) const;

  /**
   * @returns the coordinates of the home site
   */
  QPoint getHomeCoord() const
  {
    return QPoint(homeLat, homeLon);
  };

  /**
   * @returns true if the point is close to the center of the map, and false otherwise.
   */
  bool isInCenterArea(const QPoint& coord)
  {
    return mapCenterArea.contains(coord);
  }


  /**
   * @returns true if the rectangle @arg r intersects with the center rectangle of the map
   */
  bool isInCenterArea(const QRect& r)
  {
    return mapCenterArea.intersects(r);
  };


  /**
   * @returns true if the projected rectangle @arg r intersects
   * with the center rectangle of the map
   */
  bool isInProjCenterArea(const QRect& r)
  {
    return mapCenterAreaProj.intersects(r);
  };

  /**
   * @returns the current projection type
   */
  ProjectionBase* getProjection() const
    {
      return currentProjection;
    };

  public slots:

  /** Sets all mapping parameters of the projection matrix. */
  void slotInitMatrix();

  /**
   * Sets the scale to the indicated scale.
   *
   * @param scale The scale as meters per pixel
   */
  void slotSetScale(const double& scale);

  /**
   * Centers the map on the indicated coordinates
   */
  void slotCenterTo(int latitude, int longitude);

  /**
   * set new home position
   */
  void slotSetNewHome(const QPoint& newHome);


 signals:

  /** */
  void displayMatrixValues(int, bool);

  /**
   * Emitted each time the projection or map root directory were changed.
   */
  void projectionChanged();

  /**
   * Emitted each time the home position is changed.
   */
  void homePositionChanged();

  /**
   * Emitted if a move to the home position is requested.
   */
  void gotoHomePosition();

 private:
  /**
   * Moves the map into the given direction.
   */
  void __moveMap(int dir);

  /**
   */
  QPoint __mapToWgs(const QPoint&) const;

  /**
   */
  QPoint __mapToWgs(int x, int y) const;

  /**
   * Used map transformation matrix.
   */
  QTransform worldMatrix;

  /**
   * Used map invert transformation matrix.
   */
  QTransform invertMatrix;

  /**
   * The mapCenter is the position displayed in the center of the map.
   * It is used in two different ways:
   * 1.: Determine the area shown in the map-widget
   * 2.: Calculating the difference in latitude between a point in the
   * map and the center.
   *
   * The latitude of the center of the map.
   */
  int mapCenterLat;

  /**
   * The longitude of the center of the map.
   */
  int mapCenterLon;

  /** latitude of home position */
  int homeLat;

  /** longitude of home position */
  int homeLon;

  /**
   * Contains the geographical border of the map (lat/lon).
   */
  QRect viewBorder;
  // QRect printBorder;
  QRect mapBorder;

  /** The mapCenterArea is the rectangle in which the glider symbol
   *  can move around before the map is redrawn */
  QRect mapCenterArea;

  QRect mapCenterAreaProj;
  /** */
  QSize mapViewSize;

  /** Number of meters per pixel? */
  double cScale;
  /** */
  double pScale;
  /** */
  double rotationArc;
  /** */
  int scaleBorders[7];

  /** current selected type of map projection */
  ProjectionBase* currentProjection;

  /** Optimization to prevent recurring recalculation of this value */
  int _MaxScaleToCScaleRatio;

  fp24p8_t m11, m12, m21, m22, dx, dy, fx, fy;

  /** Root path to the map directories */
  QString mapRootDir;

};

#endif
