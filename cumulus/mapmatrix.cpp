/***********************************************************************
 **
 **   mapmatrix.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001      by Heiner Lamprecht
 **                   2008-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

// #define MAP_FLOAT 1

#include <cmath>

#include <QtGlobal>

#include "calculator.h"
#include "mapcalc.h"
#include "mapdefaults.h"
#include "mapmatrix.h"
#include "generalconfig.h"

// Projektions-Massstab
// 50 Meter Hoehe pro Pixel ist die staerkste Vergroesserung.
// Bei dieser Vergroesserung erfolgt die eigentliche Projektion
#define MAX_SCALE 50.0   // 40.0
#define MIN_SCALE 2000.0 // 800.0

#define NUM_TO_RAD(num) ( (M_PI / 108000000.0) * (double)(num) )
#define RAD_TO_NUM(rad) ( ( (rad) * (108000000.0 / M_PI) ) )

// Macros borrowed from FPM ()
//
// Fixed point math improves polygon and point mapping;
// isoline drawing gains up to one second

/* int to fixed point */

#define itofp24p8(x)   ( (int32_t)  (x) << 8 )

/* double to fixed point */

#define dtofp24p8(x)   ( (fp24p8_t)   ((x) * 256.0) )
#define dtofp8p24(x)   ( (fp8p24_t)   ((x) * 16777216.0) )

/* fixed point to int */

#define fp24p8toi(x)   ( (x) >> 8 )
#define fp8p24toi(x)   ( (x) >> 24 )

/* multiplication (x*y) */

#define mulfp24p8(x,y)   ( ( (int64_t)(x) *  (int64_t)(y)) >> 8  )
#define mulfp8p24(x,y)   ( ( (int64_t)(x) *  (int64_t)(y)) >> 24 )


/*************************************************************************
 **
 **  MapMatrix
 **
 *************************************************************************/

MapMatrix::MapMatrix( QObject* parent ) :
  QObject(parent),
  rotationAngle(0.0),
  mapCenterLat(0), mapCenterLon(0),
  homeLat(0), homeLon(0), cScale(0), pScale(0), rotationArc(0),
  _MaxScaleToCScaleRatio(0),
  m11(0), m12(0), m21(0), m22(0), dx(0), dy(0), fx(0), fy(0),
  cylinderParallel(0)
{
  viewBorder.setTop(32000000);
  viewBorder.setBottom(25000000);
  viewBorder.setLeft(2000000);
  viewBorder.setRight(7000000);

  mapCenterArea=QRect(0,0,0,0);
  mapCenterAreaProj=QRect(0,0,0,0);

  // @AP: Load projection type from configuration data, to construct the
  // right type and to avoid a later change in the slotInitMatrix call.
  GeneralConfig *conf = GeneralConfig::instance();

  // get current map root directory to detect user changes during run-time
  mapRootDir = conf->getMapRootDir();

  // Save the last used cylinder parallel to detect changes during running.
  cylinderParallel = conf->getCylinderParallel();

  int projectionType = conf->getMapProjectionType();

  if (projectionType == ProjectionBase::Lambert)
    {
      // qDebug("MapMatrixConst: Lambert");
      currentProjection = new ProjectionLambert( conf->getLambertParallel1(),
                                                 conf->getLambertParallel2(),
                                                 conf->getLambertOrign() );
    }
  else
    {
      // qDebug("MapMatrixConst: Cylindric");
      currentProjection = new ProjectionCylindric( conf->getCylinderParallel() );
    }

  // Restore last saved settings
  cScale = conf->getMapScale();

  mapCenterLat = conf->getCenterLat();
  mapCenterLon = conf->getCenterLon();

  if( cScale <= 0 )
    {
      cScale = 200;
    }

  if( mapCenterLat <= 0 )
    {
      mapCenterLat = conf->getHomeLat();
    }

  if( mapCenterLon <= 0 )
    {
      mapCenterLon = conf->getHomeLon();
    }

  homeLat = conf->getHomeLat();
  homeLon = conf->getHomeLon();
}

MapMatrix::~MapMatrix()
{
  writeMatrixOptions();
  delete currentProjection;
}

void MapMatrix::writeMatrixOptions()
{
  GeneralConfig *conf = GeneralConfig::instance();
  conf->setCenterLat( mapCenterLat );
  conf->setCenterLon( mapCenterLon );
  conf->setMapScale( cScale );
  conf->save();
}

QPoint MapMatrix::wgsToMap(const QPoint& origPoint) const
{
  return wgsToMap( origPoint.x(), origPoint.y() );
}

QPoint MapMatrix::wgsToMap(int lat, int lon) const
{
  double rLat = NUM_TO_RAD(lat);
  double rLon = NUM_TO_RAD(lon);

  return QPoint((int) (rint(currentProjection->projectX(rLat, rLon) * (RADIUS / MAX_SCALE))),
                (int) (rint(currentProjection->projectY(rLat, rLon) * (RADIUS / MAX_SCALE))));
}


void MapMatrix::wgsToMap(int latIn, int lonIn, double& latOut, double& lonOut)
{
  double rLat = NUM_TO_RAD(latIn);
  double rLon = NUM_TO_RAD(lonIn);

  latOut = currentProjection->projectX(rLat, rLon) * (RADIUS / MAX_SCALE),
  lonOut = currentProjection->projectY(rLat, rLon) * (RADIUS / MAX_SCALE);
}


QRect MapMatrix::wgsToMap(const QRect& rect) const
{
  return QRect(wgsToMap(rect.topLeft()), wgsToMap(rect.bottomRight()));
}


QPoint MapMatrix::__mapToWgs(const QPoint& origPoint) const
{
  return __mapToWgs(origPoint.x(), origPoint.y());
}


QPoint MapMatrix::__mapToWgs(int x, int y) const
{

  double lat = RAD_TO_NUM(currentProjection->invertLat(x * (MAX_SCALE / RADIUS),
                                                       y * (MAX_SCALE / RADIUS)));
  double lon = RAD_TO_NUM(currentProjection->invertLon(x * (MAX_SCALE / RADIUS),
                                                       y * (MAX_SCALE / RADIUS)));

  return QPoint((int)rint(lon), (int)rint(lat));
}


bool MapMatrix::isVisible( const QRect& itemBorder, int typeID) const
{
  // Grenze: Nahe 15Bit
  // Vereinfachung kann zu Fehlern fuehren ...
  // qDebug("MapMatrix::isVisible(): w=%d h=%d", itemBorder.width(), itemBorder.height() );
  // ! check for > 20000 is a workaround for a bug other where
  //   that came out after fixing the scale criteria that was always true
  //   before

  if( itemBorder.width() >= 20000 || itemBorder.height() >= 20000 )
    {
      qCritical() << "MapMatrix::isVisible(): itemBorder to large" << itemBorder;
    }

  if( typeID == BaseMapElement::Motorway ||
      typeID == BaseMapElement::Road ||
      typeID == BaseMapElement::Trail ||
      typeID == BaseMapElement::Railway ||
      typeID == BaseMapElement::Railway_D ||
      typeID == BaseMapElement::River ||
      typeID == BaseMapElement::Canal ||
      typeID == BaseMapElement::Aerial_Cable )
    {
      return ( mapBorder.intersects(itemBorder) );
    }

  if( mapBorder.intersects( itemBorder ) == false )
    {
      return false;
    }

  int w = itemBorder.width();
  int h = itemBorder.height();

  if( typeID == BaseMapElement::Isohypse &&
      ( w*4 < cScale || h*4 < cScale ) )
    {
      return false;
    }

  if( w*8 < cScale || h*8 < cScale )
    {
      return false;
    }

  return true;
}

int MapMatrix::getScaleRange()  const
{
  if(cScale <= scaleBorders[Border1])
    return LowerLimit;
  else if(cScale <= scaleBorders[Border2])
    return Border1;
  else if(cScale <= scaleBorders[Border3])
    return Border2;
  else
    return Border3;
}

QPoint MapMatrix::getMapCenter() const
{
  return QPoint(mapCenterLat, mapCenterLon);
}

double MapMatrix::getScale(unsigned int type)
{
  if(type == MapMatrix::CurrentScale)
    return cScale;
  else if(type < MapMatrix::CurrentScale)
    return scaleBorders[type];
  else
    qFatal("MapMatrix::getScale(): Value too large!");

  return 0.0;
}

void MapMatrix::centerToPoint(const QPoint& center)
{
  QPoint projCenter = __mapToWgs( invertMatrix.map( center ) );
  mapCenterLat = projCenter.y();
  mapCenterLon = projCenter.x();
  writeMatrixOptions();
}

void MapMatrix::centerToLatLon(const QPoint& center)
{
  centerToLatLon(center.x(), center.y());
}

void MapMatrix::slotCenterTo(int latitude, int longitude)
{
  centerToLatLon(latitude, longitude);
}

void MapMatrix::centerToLatLon(int latitude, int longitude)
{
  mapCenterLat = latitude;
  mapCenterLon = longitude;
  writeMatrixOptions();
}

double MapMatrix::centerToRect(const QRect& center, const QSize& pS)
{
  const int centerX = (center.left() + center.right()) / 2;
  const int centerY = (center.top() + center.bottom()) / 2;

  // We add 6.5 km to ensure, that the sectors will be visible,
  // when the user centers to the task.
  const double width = fabs((double)center.width()) + (6.5 * 1000.0 / cScale);
  const double height = fabs((double)center.height()) + (6.5 * 1000.0 / cScale);

  double xScaleDelta, yScaleDelta;

  if( pS == QSize( 0, 0 ) )
    {
      xScaleDelta = width / mapViewSize.width();
      yScaleDelta = height / mapViewSize.height();
    }
  else
    {
      xScaleDelta = width / pS.width();
      yScaleDelta = height / pS.height();
    }

  double tempScale = qMax(cScale * qMax(xScaleDelta, yScaleDelta), MAX_SCALE);

  // Only change if difference is too large:
  if((tempScale / cScale) > 1.05 || (tempScale / cScale) < 0.95)
    {
      cScale = tempScale;
      GeneralConfig::instance()->setMapScale( cScale );
    }

  centerToPoint(QPoint(centerX, centerY));

  return cScale;
}

QPoint MapMatrix::mapToWgs(const QPoint& pos) const
{
  return __mapToWgs( invertMatrix.map(pos) );
}

void MapMatrix::__moveMap(int dir)
{
  switch(dir) {
  case North:
    mapCenterLat = viewBorder.top();
    break;
  case North | West:
    mapCenterLat = viewBorder.top();
    mapCenterLon = viewBorder.left();
    break;
  case North | East:
    mapCenterLat = viewBorder.top();
    mapCenterLon = viewBorder.right();
    break;
  case West:
    mapCenterLon = viewBorder.left();
    break;
  case East:
    mapCenterLon = viewBorder.right();
    break;
  case South:
    mapCenterLat = viewBorder.bottom();
    break;
  case South | West:
    mapCenterLat = viewBorder.bottom();
    mapCenterLon = viewBorder.left();
    break;
  case South | East:
    mapCenterLat = viewBorder.bottom();
    mapCenterLon = viewBorder.right();
    break;
  case Home:
    mapCenterLat = homeLat;
    mapCenterLon = homeLon;
    break;
  }

  writeMatrixOptions();
}

void MapMatrix::createMatrix(const QSize& newSize)
{
  // qDebug() << "MapMatrix::createMatrix()" << newSize;

  mapViewSize = newSize;

  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));

  // qDebug() << "Center" << float(mapCenterLat/600000.0) << float(mapCenterLon/600000.0);

  /* Set rotating and scaling */
  const double scale = MAX_SCALE / cScale;
  rotationArc = currentProjection->getRotationArc(tempPoint.x(), tempPoint.y());
  // qDebug("rotationArc: %f", rotationArc);
  double sinscaled = sin(rotationArc) * scale;
  double cosscaled = cos(rotationArc) * scale;
  worldMatrix = QTransform( cosscaled, sinscaled, -sinscaled, cosscaled, 0, 0 );

  /* Map WGS Center point to map center */
  const QPoint map = worldMatrix.map(tempPoint);

#if 0
  qDebug() << "MapCenter" << map;
  qDebug() << "getTranslationX=" << currentProjection->getTranslationX(newSize.width(),map.x());
  qDebug() << "getTranslationY=" << currentProjection->getTranslationX(newSize.height(),map.y());
#endif

  QTransform translateMatrix( 1, 0, 0, 1,
                              currentProjection->getTranslationX( newSize.width(), map.x()),
                              currentProjection->getTranslationY( newSize.height(), map.y()));

  worldMatrix *= translateMatrix;

  /*
  //trying to rotate around center
  // QPoint curProjCenter= worldMatrix * QPoint(mapCenterLat, mapCenterLon);
  worldMatrix.translate(-map.x(),-map.y());
  worldMatrix.rotate(0);
  worldMatrix.translate(map.x(),map.y());
  */

  // Setting the viewBorder
  bool result = true;
  invertMatrix = worldMatrix.inverted( &result );

  if( !result )
    {
      // Houston, wir haben ein Problem !!!
      qFatal("Cumulus: Cannot invert worldMatrix! File=%s, Line=%d", __FILE__, __LINE__);
    }

  // Die Berechnung der Kartengrenze funktioniert so nur auf der
  // Nordhalbkugel. Auf der Südhalbkugel stimmen die Werte nur
  // näherungsweise.
  //
  QPoint tCenter  = __mapToWgs(invertMatrix.map(QPoint(newSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, newSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(),newSize.height())));

  viewBorder.setTop(tCenter.y());
  viewBorder.setLeft(tlCorner.x());
  viewBorder.setRight(trCorner.x());
  viewBorder.setBottom( qMin(blCorner.y(), brCorner.y()) );

  mapBorder = invertMatrix.mapRect( QRect( 0, 0, newSize.width(), newSize.height() ) );

  //create the map center area definition
  int vqDist = -viewBorder.height() / 5;
  int hqDist = viewBorder.width() / 5;

  mapCenterArea = QRect(mapCenterLat - vqDist, mapCenterLon - hqDist, 2* vqDist, 2* hqDist);

  vqDist = mapBorder.height() / 5;
  hqDist = mapBorder.width() / 5;

  mapCenterAreaProj = QRect(tempPoint.x() - vqDist,
                            tempPoint.y() - hqDist,
                            2 * vqDist, 2 * hqDist);

  // fixed math mapping value assignment
  m11 = (fp24p8_t)( worldMatrix.m11() * 16777216.0 );
  m12 = (fp24p8_t)( worldMatrix.m12() * 16777216.0 );
  m21 = (fp24p8_t)( worldMatrix.m21() * 16777216.0 );
  m22 = (fp24p8_t)( worldMatrix.m22() * 16777216.0 );
  dx = dtofp24p8( worldMatrix.dx() );
  dy = dtofp24p8( worldMatrix.dy() );

  emit displayMatrixValues(getScaleRange(), isSwitchScale());
}

void MapMatrix::slotSetScale(const double& nScale)
{
  if (nScale <= 0)
    {
      return;
    }

  cScale = qMax( (int) nScale, scaleBorders[LowerLimit]);
  cScale = qMin( (int) cScale, scaleBorders[UpperLimit]);

  GeneralConfig::instance()->setMapScale( cScale );
  GeneralConfig::instance()->save();

  _MaxScaleToCScaleRatio = int((MIN_SCALE/cScale)*(MAX_SCALE));
  // qDebug("MapMatrix::slotSetScale(): Set new scale to %f ratio: %d ",cScale,_MaxScaleToCScaleRatio );
}

void MapMatrix::slotInitMatrix()
{
  GeneralConfig *conf = GeneralConfig::instance();

  // The scale is set to 0 in the constructor. Here we read the scale and
  // the map center only the first time. Otherwise the values would change
  // after configuring Cumulus.
  if (cScale <= 0)
    {
      // @ee we want to center to the last position !
      mapCenterLat = conf->getCenterLat();
      mapCenterLon = conf->getCenterLon();

      if ((mapCenterLat == 0) && (mapCenterLon == 0))
        {
          // ok, that point is not valid
          qWarning("Center coordinates not valid");
          mapCenterLat = conf->getHomeLat();
          mapCenterLon = conf->getHomeLon();
        }

      cScale = conf->getMapScale();
    }

  homeLat = conf->getHomeLat();
  homeLon = conf->getHomeLon();

  int newProjectionType = conf->getMapProjectionType();

  bool projChanged = newProjectionType != currentProjection->projectionType();

  if (projChanged)
    {
      delete currentProjection;
      switch(newProjectionType)
      {
        case ProjectionBase::Lambert:
          currentProjection = new ProjectionLambert(conf->getLambertParallel1(),
                                                    conf->getLambertParallel2(),
                                                    conf->getLambertOrign());
          qDebug ("Map projection changed to Lambert");
          break;

        case ProjectionBase::Cylindric:
        default:
          // fall back is cylindrical
          currentProjection = new ProjectionCylindric(conf->getCylinderParallel());
          qDebug ("Map projection changed to Cylinder");
          break;
      }
  }

  scaleBorders[UpperLimit]  = conf->getMapUpperLimit();
  scaleBorders[LowerLimit]  = conf->getMapLowerLimit();
  scaleBorders[Border1]     = conf->getMapBorder1();
  scaleBorders[Border2]     = conf->getMapBorder2();
  scaleBorders[Border3]     = conf->getMapBorder3();
  scaleBorders[SwitchScale] = conf->getMapSwitchScale();

  cScale = qMin( (int) cScale, scaleBorders[UpperLimit]);
  cScale = qMax( (int) cScale, scaleBorders[LowerLimit]);

  bool initChanged = false;

  if (currentProjection->projectionType() == ProjectionBase::Lambert)
    {
      initChanged = ((ProjectionLambert*)currentProjection)->initProjection( conf->getLambertParallel1(),
                                                                             conf->getLambertParallel2(),
                                                                             conf->getLambertOrign() );
    }
  else if (currentProjection->projectionType() == ProjectionBase::Cylindric)
    {
      initChanged = ((ProjectionCylindric*)currentProjection)->initProjection( conf->getCylinderParallel() );

      if( cylinderParallel != conf->getCylinderParallel() )
        {
          cylinderParallel = conf->getCylinderParallel();
          initChanged = true;
    		}
    }

  if( mapRootDir != conf->getMapRootDir() )
    {
      // The user has defined a new map root directory at run-time. We should take
      // that as new source for map files and trigger a reload of all map files.
      mapRootDir = conf->getMapRootDir();
      initChanged = true;
    }

  if( initChanged )
    {
      // qDebug ("MapMatrix::slotInitMatrix has detected a change");
    }

  if( projChanged || initChanged )
    {
      emit projectionChanged();
    }
}

double MapMatrix::ensureVisible(const QPoint& point2Show, QPoint center )
{
  if( center.isNull() )
    {
      // No center point is defined, so we take the normal map center point.
      center = QPoint( mapCenterLat, mapCenterLon );
    }

  // get distances in both x and y directions in km and add some extra space
  double xDist = MapCalc::dist(point2Show.x(), center.y(), center.x(), center.y()) * 1.3;
  double yDist = MapCalc::dist(center.x(), point2Show.y(), center.x(), center.y()) * 1.3;

  // qDebug() << "MapMatrix::ensureVisible xdist=" << xDist << "ydist=" << yDist;

  //calculate the scale needed to display that distance on the map
  double xScale = 2.0 * (xDist * 1000.0) / mapViewSize.height();
  double yScale = 2.0 * (yDist * 1000.0) / mapViewSize.width();

  // we obviously need the bigger of the two scales
  double newScale = qMax(xScale, yScale);

  // qDebug() << "MapMatrix::ensureVisible xscale=" << xScale << "yscale=" << yScale
  //          << "NewScale=" << newScale;

  if (newScale <= MIN_SCALE)
    {
      // we only zoom if we can fit it on the map with the minimum scale or more.
      // maximum zoom is the minimum scale border
      newScale = qMax(newScale, static_cast<double>(VAL_BORDER_L));
      cScale = newScale;
      GeneralConfig::instance()->setMapScale( cScale );
      return cScale;
    }
  else
    {
      return 0;
    }
}

/** This function returns an integer between 0 and 2.
 * 0 is returned if the map is zoomed in far enough to
 * display all waypoints, 1 is an intermediate zoom factor
 * and 2 is such a big scale that only important waypoints
 * should be drawn.
 */
unsigned int MapMatrix::currentDrawScale() const
{
  //these numbers need to be made configurable at some point.

  if (cScale <= 125)
    {
      return 0;
    }
  else if (cScale <= 200 )
    {
      return 1;
    }
  else
    {
      return 2;
    }
}

/**
 * @returns an indication, if a waypoint can be drawn on the map according to the current
 * scale setting.
 */
bool MapMatrix::isWaypoint2Draw( Waypoint::Priority importance ) const
{
  int wpScaleLimit = GeneralConfig::instance()->getWaypointScaleBorder( importance );

  if( wpScaleLimit < cScale )
    {
      return true;
    }

  return false;
}

/**
 * Sets a new home position and is called after a configuration change done
 * by the user.
 */
void MapMatrix::slotSetNewHome(const QPoint& newHome)
{
  // qDebug( "MapMatrix::slotSetNewHome() is called" );
  Q_UNUSED( newHome )

  // The former change of the cylinder standard parallel was removed but it
  // makes to much effort to changed all the signal-slot handling in the code.

  // Emit an update trigger for home position change.
  emit homePositionChanged();
}

#ifdef MAP_FLOAT

// The old function using Qt (floating point)
QPolygon MapMatrix::map(const QPolygon &a) const
{
  return worldMatrix.map(a);
}

QPoint MapMatrix::map(const QPoint& p) const
{
  return worldMatrix.map(p);
}

#else

// The new function using fixed point multiplication
QPolygon MapMatrix::map(const QPolygon &a) const
{
  int size = a.size();
  int64_t fx;
  int64_t fy;
  int32_t curx;
  int32_t cury;
  int32_t lastx = 0;
  int32_t lasty = 0;

  QPolygon p;

  for( int i = 0; i < size; i++ )
    {
      a.point(i, &curx, &cury);
      fx = itofp24p8( curx );
      fy = itofp24p8( cury );
      // some cheating involved; multiplication with the "wrong" macro
      // after "left shifting" the "m" value in createMatrix
      curx = fp24p8toi( mulfp8p24(m11,fx) + mulfp8p24(m21,fy) + dx);
      cury = fp24p8toi( mulfp8p24(m22,fy) + mulfp8p24(m12,fx) + dy);

      if ( (i==0) | ( ((curx - lastx) | (cury - lasty)) != 0) )
        {
          p.append(QPoint(curx, cury));
          lastx = curx;
          lasty = cury;
        }
    }

  return p;
}

QPoint MapMatrix::map(const QPoint& p) const
{
  int64_t fx = itofp24p8( p.x() );
  int64_t fy = itofp24p8( p.y() );
  // some cheating involved; multiplication with the "wrong" macro
  // after "left shifting" the "m" value in createMatrix
  return QPoint( fp24p8toi( mulfp8p24(m11,fx) + mulfp8p24(m21,fy) + dx),
                 fp24p8toi( mulfp8p24(m22,fy) + mulfp8p24(m12,fx) + dy) );
}

#endif
