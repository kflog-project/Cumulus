/***********************************************************************
 **
 **   mapmatrix.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Heiner Lamprecht, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include <QMessageBox>

#include "mapcalc.h"
#include "mapdefaults.h"
#include "mapmatrix.h"
#include "generalconfig.h"

// Projektions-Maßstab
// 10 Meter Höhe pro Pixel ist die stärkste Vergrößerung.
// Bei dieser Vergrößerung erfolgt die eigentliche Projektion
#define MAX_SCALE 50.0   // 40.0
#define MIN_SCALE 2000.0 // 800.0

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

// Mit welchem Radius müssen wir rechnen ???
// #define RADIUS 6370290 //6370289.509
// #define NUM_TO_RAD(num) ( ( M_PI * (double)(num) ) / 108000000.0 )
// this is faster !
#define NUM_TO_RAD(num) ( (M_PI / 108000000.0) * (double)(num) )
#define RAD_TO_NUM(rad) ( ( (rad) * (108000000.0 / M_PI) ) )

/*************************************************************************
 **
 **  MapMatrix
 **
 *************************************************************************/

MapMatrix::MapMatrix(QObject* parent)
  : QObject(parent),
    mapCenterLat(0), mapCenterLon(0), printCenterLat(0), printCenterLon(0),
    homeLat(0), homeLon(0), cScale(0), pScale(0), rotationArc(0), printArc(0),
    matrixSize(0,0)
{
  viewBorder.setTop(32000000);
  viewBorder.setBottom(25000000);
  viewBorder.setLeft(2000000);
  viewBorder.setRight(7000000);

  //  printBorder.setTop(32000000);
  //  printBorder.setBottom(25000000);
  //  printBorder.setLeft(2000000);
  //  printBorder.setRight(7000000);

  mapCenterArea=QRect(0,0,0,0);
  mapCenterAreaProj=QRect(0,0,0,0);

  // @AP: Load projection type from config data, to construct the
  // right type and to avoid a later change in the slotInitMatrix
  // call.

  GeneralConfig *conf = GeneralConfig::instance();
  int projectionType = conf->getMapProjectionType();

  if( projectionType == ProjectionBase::Lambert ) {
    // qDebug("MapMatrixConst: Lambert");
    currentProjection = new ProjectionLambert(conf->getLambertParallel1(),conf->getLambertParallel2(),conf->getLambertOrign());
  } else {
    // qDebug("MapMatrixConst: Cylindric");
    currentProjection = new ProjectionCylindric(conf->getCylinderParallel());
  }

  __moveMap(Home);

  qDebug("Map matrix initialized ...");
}


MapMatrix::~MapMatrix()
{
  writeMatrixOptions();
  delete currentProjection;
}


void MapMatrix::writeMatrixOptions()
{
  // qDebug ("MapMatrix::writeMatrixOptions()");
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setCenterLat( mapCenterLat );
  conf->setCenterLon( mapCenterLon );
  conf->setMapScale( cScale );
  conf->save();
}


QPoint MapMatrix::wgsToMap(const QPoint& origPoint) const
{
  return wgsToMap(origPoint.x(), origPoint.y());
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
  // Vereinfachung kann zu Fehlern führen ...
  // qDebug("MapMatrix::isVisible(): w=%d h=%d", itemBorder.width(), itemBorder.height() );
  // ! check for < 10000 is a workaround for a bug otherwhere
  //   that came out after fixing the scale criteria that was always true
  //   before
#warning "FIXME: There is a bug, leading to very large with() and height() values treated by a workaround here"

  if( typeID == BaseMapElement::Highway ||
      typeID == BaseMapElement::Road ||
      typeID == BaseMapElement::Trail ||
      typeID == BaseMapElement::Railway ||
      typeID == BaseMapElement::Railway_D ||
      typeID == BaseMapElement::Aerial_Cable ) {
    return ( ( mapBorder.intersects(itemBorder) ) &&
             (itemBorder.width() < 10000) && (itemBorder.height() < 10000) );
  }

  return ( ( mapBorder.intersects(itemBorder) ) &&
           (itemBorder.width() < 10000) && (itemBorder.height() < 10000) &&
           (( itemBorder.width()*8  > ( cScale  ) ) ||
            ( itemBorder.height()*8 > ( cScale  ) )) );
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


bool MapMatrix::isSwitchScale() const
{
  return cScale <= scaleBorders[SwitchScale];
}


bool MapMatrix::isSwitchScale2() const
{
  return cScale <= scaleBorders[SwitchScale]*4;
}


QPoint MapMatrix::getMapCenter(bool) const
{
  return QPoint(mapCenterLat,mapCenterLon);
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
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
    // Houston, we've got a problem !!!
    qFatal("Cumulus: Cannot invert worldMatrix! File=%s, Line=%d", __FILE__, __LINE__);

  QPoint projCenter = __mapToWgs(invertMatrix.map(center));
  mapCenterLat = projCenter.y();
  mapCenterLon = projCenter.x();
}


void MapMatrix::centerToLatLon(const QPoint& center)
{
  centerToLatLon(center.x(), center.y());
}


void MapMatrix::slotCenterTo(int latitude, int longitude)
{
  centerToLatLon(latitude, longitude);
  //emit matrixChanged();
}


void MapMatrix::centerToLatLon(int latitude, int longitude)
{
  mapCenterLat = latitude;
  mapCenterLon = longitude;
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

  if(pS == QSize(0,0)) {
    xScaleDelta = width / mapViewSize.width();
    yScaleDelta = height / mapViewSize.height();
  } else {
    xScaleDelta = width / pS.width();
    yScaleDelta = height / pS.height();
  }

  double tempScale = MAX(cScale * MAX(xScaleDelta, yScaleDelta),
                         MAX_SCALE);

  // Only change when the difference is too big:
  if((tempScale / cScale) > 1.05 || (tempScale / cScale) < 0.95)
    cScale = tempScale;

  centerToPoint(QPoint(centerX, centerY));

  return cScale;
}


QPoint MapMatrix::mapToWgs(const QPoint& pos) const
{
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
    // Houston, we've got a problem !!!
    qFatal("Cumulus: Cannot invert worldMatrix! File=%s, Line=%d", __FILE__, __LINE__);

  return __mapToWgs(invertMatrix.map(pos));
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
  }

  // @AP: seems not to be necessary
  // createMatrix(matrixSize);
  // emit matrixChanged();
}


void MapMatrix::createMatrix(const QSize& newSize)
{
  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));
  worldMatrix.reset();

  /* Set rotating and scaling */
  const double scale = MAX_SCALE / cScale;
  rotationArc = currentProjection->getRotationArc(tempPoint.x(), tempPoint.y());// + 3.14;
  // qDebug("rotationArc: %f", rotationArc);
  double sinscaled = sin(rotationArc) * scale;
  double cosscaled = cos(rotationArc) * scale;
  worldMatrix.setMatrix(cosscaled, sinscaled, -sinscaled, cosscaled, 0, 0);

  /* Set the translation */
  const QPoint map (worldMatrix.map(tempPoint));
  QMatrix translateMatrix(1, 0, 0, 1,
                           currentProjection->getTranslationX(newSize.width(),map.x()),
                           currentProjection->getTranslationY(newSize.height(),map.y()));

  worldMatrix *= translateMatrix;

  //trying to rotate around center
  /*
    QPoint curProjCenter= worldMatrix * QPoint(mapCenterLat, mapCenterLon);
    worldMatrix.translate(-curProjCenter.x(),-curProjCenter.y());
    worldMatrix.rotate(180);
    worldMatrix.translate(curProjCenter.x(),curProjCenter.y());
  */

  // Setting the viewBorder
  bool result = true;
  QMatrix invertMatrix (worldMatrix.invert(&result));
  if(!result)
    // Houston, wir haben ein Problem !!!
    qFatal("Cumulus: Cannot invert worldMatrix! File=%s, Line=%d", __FILE__, __LINE__);

  //
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
  viewBorder.setBottom(MIN(blCorner.y(), brCorner.y()));

  mapBorder = invertMatrix.map(QRect(0,0, newSize.width(), newSize.height()));
  mapViewSize = newSize;

  //create the mapcenter area definition
  int vqDist=-viewBorder.height()/5;
  int hqDist=viewBorder.width()/5;
  mapCenterArea=QRect(mapCenterLat - vqDist, mapCenterLon - hqDist, 2* vqDist, 2* hqDist);

  vqDist=mapBorder.height()/5;
  hqDist=mapBorder.width()/5;

  mapCenterAreaProj=QRect(tempPoint.x() - vqDist,
                          tempPoint.y() - hqDist,
                          2* vqDist, 2* hqDist);
  emit displayMatrixValues(getScaleRange(), isSwitchScale());
  //  emit matrixChanged();
}


void MapMatrix::slotSetScale(const double& nScale)
{
  if (nScale <= 0)
    return;
  cScale=MAX(nScale,scaleBorders[LowerLimit]);
  cScale = MIN(cScale,scaleBorders[UpperLimit]);
  //createMatrix(matrixSize);
  //emit matrixChanged();
  _MaxScaleToCScaleRatio=int((MIN_SCALE/cScale)*(MAX_SCALE));
  qDebug("MapMatrix::slotSetScale(): Set new scale to %f ratio: %d ",cScale,_MaxScaleToCScaleRatio );
}


void MapMatrix::slotInitMatrix()
{
  // qDebug("MapMatrix::slotInitMatrix() is called");

  GeneralConfig *conf = GeneralConfig::instance();

  //
  // The scale is set to 0 in the constructor. Here we read the scale and
  // the mapcenter only the first time. Otherwise the values would change
  // after configuring KFLog.
  //
  //                                                Fixed 2001-12-14
  if(cScale <= 0) {
    // @ee we want to center to the last position !
    mapCenterLat = conf->getCenterLat();
    mapCenterLon = conf->getCenterLon();

    if ((mapCenterLat == 0) && (mapCenterLon == 0)) {
      // ok, that point is not valid
      qWarning("Center Latitude not valid");
      mapCenterLat = conf->getHomeLat();
      mapCenterLon = conf->getHomeLon();
    }

    cScale = conf->getMapScale();
  }

  homeLat = conf->getHomeLat();
  homeLon = conf->getHomeLon();

  int newProjectionType = conf->getMapProjectionType();

  bool projChanged = newProjectionType != currentProjection->projectionType();

  if (projChanged) {
    delete currentProjection;
    switch(newProjectionType) {
    case ProjectionBase::Lambert:
      currentProjection = new ProjectionLambert(conf->getLambertParallel1(),conf->getLambertParallel2(),conf->getLambertOrign());
      // qDebug("XXXXXXXXXXXXXXXX %i %i",conf->getLambertParallel1(),conf->getLambertParallel2());
      qDebug ("Map projection changed to Lambert");
      break;
      //case ProjectionBase::Cylindric:
    default:
      // fallback is cylindrical
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

  cScale = MIN(cScale, scaleBorders[UpperLimit]);
  cScale = MAX(cScale, scaleBorders[LowerLimit]);

  bool initChanged = false;

  if (currentProjection->projectionType() == ProjectionBase::Lambert) {
    initChanged = ((ProjectionLambert*)currentProjection)->initProjection(
                                                                          conf->getLambertParallel1(),
                                                                          conf->getLambertParallel2(),
                                                                          conf->getLambertOrign() );
  } else if (currentProjection->projectionType() == ProjectionBase::Cylindric) {
    initChanged = ((ProjectionCylindric*)currentProjection)->initProjection(
                                                                            conf->getCylinderParallel() );
  }

  if( initChanged ) {
    // qDebug ("Map init has detected a change");
  }

  if(projChanged || initChanged) {
    // emit projectionChanged();
    // @AP: Notice the user, that compiled maps must be removed and
    // original maps be reinstalled if necessary

    QMessageBox::warning( 0, "Cumulus",
                          tr( "<qt>"
                              "<b>Map projection has been changed.</b><p>"
                              "Please ensure that original "
                              "map files are available for recompiling!"
                              "</qt>" ) );

    emit projectionChanged();
  }
}


double MapMatrix::ensureVisible(const QPoint& point)
{
  //get distances in both x and y directions, and add some extra space
  double xDist=dist(point.x(), mapCenterLon, mapCenterLat, mapCenterLon)*1.10;
  double yDist=dist(mapCenterLat, point.y(), mapCenterLat, mapCenterLon)*1.10;

  //calculate the scale needed to display that distance on the map
  double xScale=2.0 * (xDist*1000.0) / mapViewSize.height();
  double yScale=2.0 * (yDist*1000.0) / mapViewSize.width();

  //we obviously need the bigger of the two scales
  double newScale=MAX(xScale, yScale);
  if (newScale < MIN_SCALE) {  //we only zoom if we can fit it on the map with the minimum scale or more
    newScale=MAX(newScale, MAX_SCALE); //maximum zoom is the minimum scale
    cScale=newScale;

    return cScale;
  } else {
    return 0;
  }

}


/** This function returns an integer between 0 and 2. 0 is returned if the map is zoomed in far enough to display all waypoints, 1 is an intermediate zoomfactor and 2 is such a big scale that only important waypoints should be drawn. */
unsigned int MapMatrix::currentDrawScale() const
{
  //these numbers need to be made configurable at some point.

  if (cScale<=250)
    return 0;
  else if (cScale<=500)
    return 1;
  else
    return 2;
}

/** set new home position */
void MapMatrix::slotSetNewHome(const QPoint* newHome)
{
  homeLat = newHome->x();
  homeLon = newHome->y();
  emit homePositionChanged();
}
