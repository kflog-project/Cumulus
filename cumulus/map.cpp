/***********************************************************************
 **
 **   map.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 modified by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <cmath>

#include <QDesktopWidget>
#include <QPainter>
#include <QPen>
#include <QWhatsThis>
#include <QDesktopWidget>
#include <Q3SimpleRichText>
#include <QPolygon>

#include "airport.h"
#include "airspace.h"
#include "cucalc.h"
#include "cumulusapp.h"
#include "distance.h"
#include "generalconfig.h"
#include "hwinfo.h"
#include "map.h"
#include "mapcalc.h"
#include "mapconfig.h"
#include "mapcontents.h"
#include "mapdefaults.h"
#include "mapmatrix.h"
#include "mapview.h"
#include "radiopoint.h"
#include "reachablelist.h"
#include "runway.h"
#include "singlepoint.h"
#include "wgspoint.h"
#include "whatsthat.h"
#include "waypoint.h"
#include "wpeditdialog.h"

extern CumulusApp      *_globalCumulusApp;
extern MapContents     *_globalMapContents;
extern MapMatrix       *_globalMapMatrix;
extern MapConfig       *_globalMapConfig;
extern MapView         *_globalMapView;

Map *Map::instance=0;

Map::Map(QWidget* parent) : QWidget(parent),
    preSnapPoint(-999, -999), prePos(-50, -50), preCur1(-50, -50),
    preCur2(-50, -50), planning(-1)
{
  qDebug( "Map window size is %dx%d, width=%d, height=%d",
          parent->size().width(),
          parent->size().height(),
          parent->size().width(),
          parent->size().height() );

  setObjectName("Map");

  instance = this;
  _lastASInfo = "";
  _isEnable = false;  // Disable map redrawing at startup
  _isResizeEvent = false;
  _isRedrawEvent = false;
  mapRot = 0;
  curMapRot = 0;
  heading = 0;
  bearing = 0;
  mode = northUp;
  m_sceduledFromLayer = topLayer;
  ShowGlider = false;
  setMutex(false);

  //install an event filter on the map object itself, so all events are
  //routed through eventFilter()
  //installEventFilter(this);

  //setup progressive zooming values
  zoomProgressive = 0;
  zoomProgressiveVal[0] = 1.25;

  for( int i=1; i<8; i++ )
    {
      zoomProgressiveVal[i] = zoomProgressiveVal[i-1] * 1.25;
    }

  // Set pixmaps first to the size of the parent
  m_pixBaseMap        = QPixmap( parent->size() );
  m_pixAeroMap        = QPixmap( parent->size() );
  m_pixNavigationMap  = QPixmap( parent->size() );
  m_pixInformationMap = QPixmap( parent->size() );
  m_pixPaintBuffer    = QPixmap( parent->size() );

  m_pixBaseMap.fill(Qt::white);
  m_pixAeroMap.fill(Qt::white);
  m_pixNavigationMap.fill(Qt::white);
  m_pixInformationMap.fill(Qt::white);
  m_pixPaintBuffer.fill(Qt::white);

  setBackgroundColor(Qt::white);

  redrawTimerShort = new QTimer(this);
  redrawTimerShort->setSingleShot(true);

  redrawTimerLong  = new QTimer(this);
  redrawTimerLong->setSingleShot(true);

  connect( redrawTimerShort, SIGNAL(timeout()),
           this, SLOT(slotRedrawMap()));

  connect( redrawTimerLong, SIGNAL(timeout()),
           this, SLOT(slotRedrawMap()));

  zoomFactor = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  curMANPos  = _globalMapMatrix->getMapCenter();
  curGPSPos  = _globalMapMatrix->getMapCenter();

  /** @ee load icons */
  _cross  = GeneralConfig::instance()->loadPixmap("cross.png");
  _glider = GeneralConfig::instance()->loadPixmap("gliders80pix-15.png");
}


Map::~Map()
{
  qDeleteAll(airspaceRegList);
  airspaceRegList.clear();
}

/**
 * Display Info about Airspace items
*/

void Map::__displayMapInfo(const QPoint& current)
{
  /*
   * Gliding sites, if available, will be the first entry
   */
  if( mutex() )
    {
      //qDebug("Map::__displayMapInfo: Map drawing in progress: return");
      return;
    }

  if (!isVisible())
    {
      return;
    }

  int itemcount=0;

  QString text;

  bool show = false;

  text += "<html><table border=1><tr><th align=left>" +
          tr("Airspace&nbsp;Structure") +
          "</th></tr>";

  for( int loop = 0; loop < airspaceRegList.count(); loop++ )
    {
      if(airspaceRegList.at(loop)->region->contains(current))
        {
          Airspace* pSpace = airspaceRegList.at(loop)->airspace;
          //      qDebug ("name: %s", pSpace->getName().latin1());
          //      qDebug ("lower limit: %d", pSpace->getLowerL());
          //      qDebug ("upper limit: %d", pSpace->getUpperL());
          //      qDebug ("lower limit type: %d", pSpace->getLowerT());
          //      qDebug ("upper limit type: %d", pSpace->getUpperT());
          //work around the phenemenon that airspaces tend to appear in the list twice -> this should be dealt with properly!
          if (text.find(pSpace->getInfoString())==-1)
            {     //only add if the string has not been added before
              text += "<tr><td align=left>" + pSpace->getInfoString() + "</td></tr>";
              itemcount++;
            }
          show = true;
        }
    }

  text += "</table></html>";

  // @AP: replace long labels to small one, otherwise the window is
  // sometimes to small for display whole line.
  // remove term Restricted, to display all info behind
  text.replace( QRegExp("Restricted "), "AR " );
  text.replace( QRegExp("Danger "), "AD " );
  text.replace( QRegExp("Prohibited "), "AP " );
  text.replace( QRegExp("AS-E low "), "AS-El " );
  text.replace( QRegExp("AS-E high "), "AS-Eh " );

  if( show && WhatsThat::getInstance() == 0 )
    {
      // display text only, if no other display is active
      GeneralConfig *conf = GeneralConfig::instance()
                            ;
      int airspaceTime = conf->getAirspaceDisplayTime();
      int showTime = 0;

      if( airspaceTime != 0 )
        {
          //qDebug("airspace itemcount=%d", itemcount);
          showTime = MAX((itemcount * airspaceTime), MIN_POPUP_DISPLAY_TIME);
          showTime *= 1000;
        }

      // qDebug("airspace timer %dms", showTime);

      WhatsThat *box = new WhatsThat(this, text, showTime);
      box->show();
    }
}


/**
 * Display detailed Info about one MapItem
*/

void Map::__displayDetailedMapInfo(const QPoint& current)
{
  if( mutex() )
    {
      //qDebug("Map::__displayDetailedMapInfo: Map drawing in progress: return");
      return;
    }

  // select WayPoint
  QRegExp blank("[ ]");
  bool found = false;

  // Radius for Mouse Snapping
  int delta=0, dX=0, dY=0;

  // add WPList !!!
  int searchList[] = {MapContents::GliderList, MapContents::AirportList};
  wayPoint *w = (wayPoint *) 0;

  // scale uses unit meter/pixel
  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  // snap distance should be 3000m considering scale, otherwise the
  // range is to high on higher sacles.
  delta = (int) rint( 3000. / cs );
  // Manhattan-distance to found point.
  int lastDist=2*delta+1;

  // @AP: On map scale higher as 1024 we don't evelute anything
  for (int l = 0; l < 2 && cs < 1024.0; l++)
    {
      for(unsigned int loop = 0;
          loop < _globalMapContents->getListLength(searchList[l]); loop++)
        {
          Airport* hitElement = (Airport*)_globalMapContents->getElement(
                                  searchList[l], loop);
          QPoint sitePos (hitElement->getMapPosition());

          dX = abs(sitePos.x() - current.x());
          dY = abs(sitePos.y() - current.y());

          // qDebug( "delta=%d, dX=%d, dY=%d, lastDist=%d", delta, dX, dY, lastDist );

          // Abstand entspricht der Icon-Groesse
          if (dX < delta && dY < delta)
            {
              if (found && ((dX+dY) >= lastDist))
                {
                  continue; //the point we found earlier was closer
                }

              //      qDebug ("waypoint: %s", hitElement->getName().latin1());
              //w = new wayPoint;  // create new wp later, only if added to list
              w = &wp;
              // w->name = w->name.replace(blank, QString::null).left(6).upper();
              w->name = hitElement->getWPName();
              w->description = hitElement->getName();
              w->type = hitElement->getTypeID();
              w->origP = hitElement->getWGSPosition();
              w->elevation = hitElement->getElevation();
              w->icao = hitElement->getICAO();
              // qDebug("|%s|", hitElement->getFrequency().latin1());
              w->frequency = hitElement->getFrequency().toDouble();
              // qDebug(" hitElement->getRunway(0) %d",hitElement->getRunway(0).direction );
              w->isLandable = hitElement->getRunway(0).isOpen;
              w->surface = hitElement->getRunway(0).surface;
              w->runway = hitElement->getRunway(0).direction;
              w->length = hitElement->getRunway(0).length;
              w->sectorFAI = 0;
              w->sector1 = 0;
              w->sector2 = 0;

              found = true;
              lastDist = dX+dY;
              if( lastDist < (delta/3) ) //if we're very near, stop searching the list
                {
                  break;
                }
            }
        }
    }

  // let's show waypoints
  // @AP: On map scale higher as 1024 we don't evelute anything
  for (int i = 0; i < _globalMapContents->getWaypointList()->count(); i++)
    {
      if (cs > 1024.0)
        break;
      wayPoint* wp = _globalMapContents->getWaypointList()->at(i);

      // consider only points, which are visible on the map
      if( (uint) wp->importance < _globalMapMatrix->currentDrawScale() )
        {
          continue;
        }

      QPoint sitePos (_globalMapMatrix->map(wp->projP));
      dX = abs(sitePos.x() - current.x());
      dY = abs(sitePos.y() - current.y());

      // Abstand entspricht der Icon-Groesse
      if (dX < delta && dY < delta)
        {
          if (found && ((dX+dY) > lastDist))
            { //subtle difference with airfields: replace allready found waypoint if we find a waypoint at the same distance.
              continue; //the point we found earlier was closer
            }
          found = true;
          w=wp;
          lastDist = dX+dY;
          if (lastDist< (delta/3)) //if we're very near, stop searching the list
            {
              break;
            }
        }
    }

  if (found)
    {
      emit waypointSelected(w);
      return;
    }

  // @ee maybe we can show airspace info
  if (!found)
    __displayMapInfo(current);
}


void Map::mousePressEvent(QMouseEvent* event)
{
  // qDebug("Map::mousePressEvent():");

  if( mutex() )
    {
      // qDebug("Map::mousePressEvent(): mutex is locked, returning");
      return;
    }

  // qDebug("Map::mousePressEvent");
  switch (event->button())
    {
      // we don't have a mid button on the Z, but ...
      /*
        case Qt::MidButton:
        // Move Map
        _globalMapMatrix->centerToPoint(event->pos());
        _globalMapMatrix->createMatrix(this->size());
        __redrawMap();
        break;
      */

      // On Zaurus we use the press-on-hold feature
    case Qt::RightButton: // press and hold generates mouse RightButton
      // qDebug("RightButton");
      break;
    case Qt::LeftButton: // press generates mouse LeftButton immediately
      // qDebug("LeftButton");
      __displayDetailedMapInfo(event->pos());
      break;
    case Qt::MidButton:
      // qDebug("MidButton");
      break;

    default:
      break;
    }
}


void Map::mouseReleaseEvent(QMouseEvent* event)
{
  // qDebug("Map::mouseReleaseEvent():");

  if( mutex() )
    {
      // qDebug("Map::mouseReleaseEvent(): mutex is locked, returning");
      return;
    }

  switch (event->button())
    {
    case Qt::LeftButton:
      // __displayMapInfo(event->pos());
      break;
    default:
      break;
    }
}


void Map::paintEvent(QPaintEvent* event)
{
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString(Qt::ISODate);

//   qDebug("%s: Map::paintEvent(): RecW=%d, RecH=%d, RecLeft(X)=%d, RecTop(Y)=%d",
//          dtStr.toAscii().data(),
//          event->rect().width(), event->rect().height(),
//          event->rect().left(), event->rect().top() );

  if( mutex() )
    {
      qDebug("Map::paintEvent(): mutex is locked");
    }

  // We copy always the content from the m_pixPaintBuffer to the paint
  // device.
  QPainter p(this);
  p.drawPixmap( event->rect().left(), event->rect().top(), m_pixPaintBuffer,
                0, 0, event->rect().width(), event->rect().height() );

  // qDebug("Map.paintEvent(): return");
}


void Map::slotNewWind()
{
  Vector wind = calculator->getlastWind();

  if( wind.getSpeed().getMps() != 0 )
    {
      int angle = wind.getAngleDeg();
      angle = ((angle+5)/10)*10;  // quantisize modulo 10
      while( angle > 360 )
        angle -= 360;
      QString resource;
      resource.sprintf("windarrows/wind-arrow-80px-%03d.png", angle );
      // qDebug("Loading resource %s", (const char *) resource );
      windArrow = GeneralConfig::instance()->loadPixmap(resource);
    }
}


void Map::__drawAirspaces(bool reset)
{
  QPainter cuAeroMapP;

  cuAeroMapP.begin(&m_pixAeroMap);

  QTime t;
  t.start();

  Airspace* currentAirS;
  QRegion * reg = 0;
  AirRegion* region = 0;

  if( reset )
    {
      qDeleteAll(airspaceRegList); airspaceRegList.clear();
    }

  GeneralConfig* settings     = GeneralConfig::instance();
  bool fillAirspace           = settings->getAirspaceFillingEnabled();
  bool forcedDrawing          = settings->getForceAirspaceDrawingEnabled();
  AirspaceWarningDistance awd = settings->getAirspaceWarningDistances();
  AltitudeCollection alt      = calculator->getAltitudeCollection();
  QPoint pos                  = calculator->getlastPosition();

  qreal airspaceOpacity = 100.0; // no transparency

  uint forcedMinimumCeil = (uint) rint(settings->getForceAirspaceDrawingDistance().getMeters() +
                                   calculator->getlastAltitude().getMeters());

  for( uint loop = 0;
       loop < _globalMapContents->getListLength( MapContents::AirspaceList);
       loop++ )
    {
      currentAirS = (Airspace*) _globalMapContents->getElement(MapContents::AirspaceList, loop);

      //airspaces we don't draw, we don't warn for either (and vise versa)
      if( !settings->getAirspaceWarningEnabled( currentAirS->getTypeID() ) )
        {
          //currentAirS->drawRegion(&cuAeroMapP, false);
          if (forcedDrawing && currentAirS->getLowerL() <= forcedMinimumCeil)
            {
              //draw anyway, because it's getting close...
            }
          else
            {
              continue;
            }
        }

      if( ! currentAirS->isDrawable() )
        {
          // Not of interest, step away
          continue;
        }

      if( reset )
        {
          // we have to create a new region for that airspace and put
          // it in the airspace region list.
          region = new AirRegion( currentAirS->createRegion(), currentAirS ) ;
          airspaceRegList.append(region);
        }
      else
        {
          // try to reuse an existing region
          region = currentAirS->getAirRegion();

          if( region )
            {
              reg = region->region;
            }
        }

      if( region && fillAirspace )
        {
          Airspace::ConflictType hConflict = region->conflicts( pos, awd );
          Airspace::ConflictType vConflict = currentAirS->conflicts( alt, awd );
          Airspace::ConflictType rConflict = Airspace::none; // resulting conflict

          if( hConflict != Airspace::none && vConflict != Airspace::none )
            {
              // we have a real conflict, determine the higher priority
              rConflict = (hConflict < vConflict ? hConflict : vConflict);
            }

          airspaceOpacity = (qreal) settings->airspaceFilling( vConflict, rConflict );
        }

      currentAirS->drawRegion(&cuAeroMapP, this->rect(), airspaceOpacity );
    }

  cuAeroMapP.end();
  qDebug("Airspace drawing took %d ms", t.elapsed());
}


void Map::__drawGrid()
{
  const QRect mapBorder = _globalMapMatrix->getViewBorder();

  QPainter gridP;

  gridP.begin(&m_pixAeroMap);
  gridP.setBrush(Qt::NoBrush);
  gridP.setClipping(true);

  // die Kanten des Bereichs
  const int lon1 = mapBorder.left() / 600000 - 1;
  const int lon2 = mapBorder.right() / 600000 + 1;
  const int lat1 = mapBorder.top() / 600000 + 1;
  const int lat2 = mapBorder.bottom() / 600000 - 1;

  // Step between two degree-lines (in 1/60 degree)
  int step = 60;
  int gridStep = 1;
  int lineWidth = 1;

  switch(_globalMapMatrix->getScaleRange())
    {
    case 0:
      step = 10;
      break;
    case 1:
      step = 30;
      break;
    case 2:
      gridStep = 2;
      break;
    default:
      gridStep = 4;
    }

  QPoint cP, cP2;

  // First the latitudes:
  for(int loop = 0; loop < (lat1 - lat2 + 1) ; loop += gridStep)
    {
      int size = (lon2 - lon1 + 1) * 10;
      QPolygon pointArray(size);

      for(int lonloop = 0; lonloop < size; lonloop++)
        {
          cP = _globalMapMatrix->wgsToMap( ( lat2 + loop ) * 600000,
                                           (int)rint(( lon1 + ( lonloop / 10.0 ) ) * 600000));
          pointArray.setPoint(lonloop, cP);
        }

      // Draw the small lines between:
      int number = 60 / step;

      for(int loop2 = 1; loop2 < number; loop2++)
        {
          QPolygon pointArraySmall(size);

          for(int lonloop = 0; lonloop < size; lonloop++)
            {
              cP = _globalMapMatrix->wgsToMap(
                     (int)rint(((lat2 + loop + ( loop2 * ( step / 60.0 ) ) ) * 600000)),
                     (int)rint(((lon1 + (lonloop / 10.0)) * 600000)));

              pointArraySmall.setPoint(lonloop, cP);
            }

          if(loop2 == (number / 2))
            gridP.setPen(QPen(Qt::black, 1, Qt::DashLine));
          else
            gridP.setPen(QPen(Qt::black, 1, Qt::DotLine));

          gridP.drawPolyline(_globalMapMatrix->map(pointArraySmall));
        }
      // Draw the main lines
      gridP.setPen(QPen(Qt::black, 1));
      gridP.drawPolyline(_globalMapMatrix->map(pointArray));
    }

  // Now the longitudes:
  for(int loop = lon1; loop <= lon2; loop += gridStep)
    {
      cP = _globalMapMatrix->wgsToMap(lat1 * 600000, (loop * 600000));
      cP2 = _globalMapMatrix->wgsToMap(lat2 * 600000, (loop * 600000));

      // Draw the main longitudes:
      gridP.setPen(QPen(Qt::black, 1));
      gridP.drawLine(_globalMapMatrix->map(cP), _globalMapMatrix->map(cP2));

      // Draw the small lines between:
      int number = 60 / step;

      for(int loop2 = 1; loop2 < number; loop2++)
        {
          cP = _globalMapMatrix->wgsToMap((lat1 * 600000),
                                          (int)rint(((loop + (loop2 * step / 60.0)) * 600000)));

          cP2 = _globalMapMatrix->wgsToMap((lat2 * 600000),
                                           (int)rint(((loop + (loop2 * step / 60.0)) * 600000)));

          if(loop2 == (number / 2))
            gridP.setPen(QPen(Qt::black, 1, Qt::DashLine));
          else
            gridP.setPen(QPen(Qt::black, lineWidth, Qt::DotLine));

          gridP.drawLine(_globalMapMatrix->map(cP), _globalMapMatrix->map(cP2));
        }
    }

  gridP.end();
}


void Map::__drawPlannedTask()
{
  FlightTask* task = (FlightTask*) _globalMapContents->getCurrentTask();

  if( task == 0 )
    {
      return;
    }

  if(task && task->getTypeID() == BaseMapElement::Task)
    {

      // Draw the task
      QPainter taskP;
      taskP.begin( &m_pixInformationMap );

      // Draw task including sectors
      task->drawMapElement(&taskP);

      taskP.end();
    }
}


/**
 * Draw a trail displaying the flight path if feature is turned on
 */
void Map::__drawTrail()
{
  if (!calculator->matchesFlightMode(GeneralConfig::instance()->getDrawTrail()))
    return;

  if (calculator->samplelist.count()<5)
    {
      return;
    }

  QPainter p;
  p.begin(&m_pixInformationMap);

  QPen pen1(Qt::black, 2);
  QTime minTime=QTime::currentTime().addSecs(-150);
  //minTime.addSecs(-maxSecs);
  QPoint pos;
  QPoint lastPos;
  uint loop = 0;
  uint sampleCnt = calculator->samplelist.count();
  lastPos = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(calculator->samplelist.at(0).position));
  p.setPen(pen1);

  while (loop < sampleCnt && calculator->samplelist.at(loop).time >= minTime)
    {
      pos = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(calculator->samplelist.at(loop).position));
      p.drawLine(pos,lastPos);
      lastPos=pos;

      loop += 5;
    }
  p.end();
}


void Map::setDrawing(bool isEnable)
{
  if (_isEnable != isEnable)
    {
      if (isEnable)
        sceduleRedraw(baseLayer);
      _isEnable = isEnable;
    }
}


void Map::resizeEvent(QResizeEvent* event)
{
//   qDebug("Map::resizeEvent(): w=%d, h=%d, pbw=%d, pbh=%d",
//          event->size().width(), event->size().height(),
//          m_pixPaintBuffer.width(), m_pixPaintBuffer.height() );

  if( event->size().width()  == m_pixPaintBuffer.width() &&
      event->size().height() < m_pixPaintBuffer.height() &&
      (m_pixPaintBuffer.height() - event->size().height()) <= 35 )
    {
      qDebug("Map::resizeEvent(): EventSize.height <= PaintBufferSize.height-35 ->ignore Event");
      return; // we assume, that the menu bar has been opened and ignore this event
    }

  // set resize flag
  _isResizeEvent = true;
  // start redrawing of map
  slotDraw();
}


void Map::__redrawMap(mapLayer fromLayer)
{
  // qDebug("Map::__redrawMap from layer=%d", fromLayer);

  if( ! isVisible() )
    {
      // AP: ignore draw request when the window is hidden or not
      // visible to get the user all power of the device for interactions
       return;
    }

  if( ! _isEnable )
    {
      qDebug("Map::__redrawMap() disabled");
      // @AP: we reset also the redraw request flag.
      _isRedrawEvent = false;
      return;
    }

  if( mutex() )
    {
      // @AP: we queue only the redraw request, timer will be started
      // again by __redrawMap() method.
      _isRedrawEvent = true;
      // qDebug("Map::__redrawMap(): mutex is locked, returning");
      return;
      //TODO make a bit smarter. We only need to scedule a redraw if
      //the layer is beneath the one we are currently redrawing, and
      //then only to the level requested.
    }

  // set mutex to block recursive entries and unwanted data
  // modifications.

  setMutex(true);

  // Check, if a resize event is queued. In this case it must be
  // considered to create the right matrix size.
  if( _isResizeEvent )
    {
      _isResizeEvent = false;
      // reset layer to base layer, that all will be redrawn
      fromLayer = baseLayer;
    }

  //set the map rotation
  curMapRot = mapRot;

  // draw the layers we need to refresh
  if (fromLayer < aeroLayer)
    {
      // Draw the base layer, which contains the landscape elements.
      // First initialize map matrix
      _globalMapMatrix->slotSetScale(zoomFactor);

      if(calculator->isManualInFlight() || !ShowGlider)
        {
          _globalMapMatrix->slotCenterTo(curMANPos.x(), curMANPos.y());
        }
      else
        {
          _globalMapMatrix->slotCenterTo(curGPSPos.x(), curGPSPos.y());
        }

      zoomFactor = _globalMapMatrix->getScale(); //read back from matrix!
      
      _globalMapMatrix->createMatrix(this->size());
      
      // initialize the base pixmap
      m_pixBaseMap.resize(size());
      m_pixBaseMap.fill(Qt::white);

      //actually start doing our drawing
      __drawBaseLayer();
    }

  if (fromLayer < navigationLayer)
    __drawAeroLayer(fromLayer < aeroLayer);
  if (fromLayer < informationLayer)
    __drawNavigationLayer();
  if (fromLayer < topLayer)
    __drawInformationLayer();

  // copy the new map content into the paint buffer
  m_pixPaintBuffer = m_pixInformationMap;

  // unlock mutex
  setMutex(false);

  // Repaints the widget directly by calling paintEvent() immediately,
  // unless updates are disabled or the widget is hidden.
  // @AP: Qt 4.3 complains about it, when paintEvent is called directly
  QDateTime dt = QDateTime::currentDateTime();
  QString dtStr = dt.toString(Qt::ISODate);

  // qDebug("%s: Map::__redrawMap: repaint(%dx%d) is called",
  //       dtStr.toAscii().data(), this->rect().width(),this->rect().height() );
  repaint( this->rect() );

  // @AP: check, if a pending redraw request is active. In this case
  // the scheduler timers will be restarted to handle it.
  if( _isRedrawEvent )
    {
      qDebug("============== Map::__redrawMap(): queued redraw event found, schedule Redraw =======");
      _isRedrawEvent = false;
      sceduleRedraw();
    }

  // @AP: check, if a pending resize event exists. In this case the
  // scheduler timers will be restarted to handle it.
  if( _isResizeEvent )
    {
      qDebug("Map::__redrawMap(): queued resize event found, schedule Redraw");
      sceduleRedraw();
    }

  return;
}

/**
 * Draws the base layer of the map.
 * The base layer consists of the basic map, containing everything up
 * to the features of the landscape.
 * It is drawn on an empty canvas.
 */
void Map::__drawBaseLayer()
{
  if( !_isEnable ) return;

  // erase the base layer (fill with light blue color)
  m_pixBaseMap.fill(QColor(96,128,248));

  //set the cursor position to be outside of our view
  preCur1.setX(-50);
  preCur2.setX(-50);

  // make sure we have all the mapfiles we need loaded
  _globalMapContents->proofeSection();


  qDebug("Map::__drawBaseLayer(): zoomFactor=%f, QAppLoopLevel=%d",
         zoomFactor, qApp->loopLevel() );

  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  // create a painter on the pixmap
  QPainter baseMapP;

  baseMapP.begin(&m_pixBaseMap);

  //first, draw the iso lines
  _globalMapContents->drawIsoList(&baseMapP);

  // next, draw the topographical elements and the cities
  _globalMapContents->drawList(&baseMapP, MapContents::TopoList);
  _globalMapContents->drawList(&baseMapP, MapContents::CityList);

  // draw the (rail-) roads
  if( cs < 819.2 )
    {
      _globalMapContents->drawList(&baseMapP, MapContents::RoadList);
      _globalMapContents->drawList(&baseMapP, MapContents::RailList);
    }

  // draw the landmarks, the obstacles and the waterways
  if( cs < 1024.0 )
    {
      _globalMapContents->drawList(&baseMapP, MapContents::LandmarkList);
      _globalMapContents->drawList(&baseMapP, MapContents::ObstacleList);
      _globalMapContents->drawList(&baseMapP, MapContents::ReportList);
      _globalMapContents->drawList(&baseMapP, MapContents::HydroList);
    }

  // draw the highways and the lakes
  _globalMapContents->drawList(&baseMapP, MapContents::HighwayList);
  _globalMapContents->drawList(&baseMapP, MapContents::LakeList);
  _globalMapContents->drawList(&baseMapP, MapContents::NavList);

  // end the painter
  baseMapP.end();
}


/**
 * Draws the aero layer of the map.
 * The aero layer consists of the airspace structures and the navigation
 * grid.
 * It is drawn on top of the base layer
 */
void Map::__drawAeroLayer(bool reset)
{
  // first, copy the base map to the aero map
  m_pixAeroMap = m_pixBaseMap;

  __drawAirspaces(reset);
  __drawGrid();
}


/**
 * Draws the navigation layer of the map.
 * The navigation layer consists of the airfields, outlanding sites and
 * waypoints.
 * It is drawn on top of the aero layer.
 */
void Map::__drawNavigationLayer()
{
  m_pixNavigationMap = m_pixAeroMap;

  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  if( !_isEnable || cs > 1024.0 )
    {
      return;
    }

  QPainter navP;

  navP.begin(&m_pixNavigationMap);

  _globalMapContents->drawList(&navP, MapContents::AirportList);
  _globalMapContents->drawList(&navP, MapContents::OutList);
  _globalMapContents->drawList(&navP, MapContents::GliderList);
  __drawWaypoints(&navP);

  navP.end();
}


/**
 * Draws the information layer of the map.
 * The information layer consists of the flight task, windarrow, the
 * trail, the position indicator and the scale.
 * It is drawn on top of the navigation layer.
 */
void Map::__drawInformationLayer()
{
  m_pixInformationMap = m_pixNavigationMap;

  //draw the task (if enabled)
  __drawPlannedTask();

  //draw the trail (if enabled)
  __drawTrail();

  //draw the wind arrow, if pixmap was initialized by slot slotNewWind
  //drawing the wind arrow is a matter of bit blitting a piece of the windArrow pixmap
  if( ! windArrow.isNull() )
    {
      QPainter p(&m_pixInformationMap);
//      p.drawPixmap( 4, 4, windArrow, windArrow.width()/4, windArrow.width()/4, 36, 36);
      p.drawPixmap( 8, 8, windArrow );
    }
    
  //draw a location symbol on the buffer
  if (ShowGlider)
    {
      __drawGlider();
    }
  if(!ShowGlider || calculator->isManualInFlight())
    {
      __drawX();
    }

  // and finaly draw a scale indicator on top of this
  __drawScale();
}


//Performs an unsceduled, immediate redraw of the entire map.
//Any allready pending redraws are cancelled.
void Map::slotDraw()
{
  m_sceduledFromLayer = baseLayer;
  slotRedrawMap();
}


// This slot is called, if one of the timers redrawTimerShort or
// redrawTimerLong is expired. It starts the redrawing of the map, if
// the mutex is not locked. In the other case the new redraw request
// will be queued only.
void Map::slotRedrawMap()
{
  qDebug("Map::slotRedrawMap(): LoopLevel=%d", qApp->loopLevel());

  // @AP: Don't allows changes on matrix data during drawing!!!

  if( mutex() )
    {
      // @AP: we queue only the redraw request, timer will be
      // started again by __redrawMap() method.
      _isRedrawEvent = true;
      qDebug("Map::slotRedrawMap(): is locked by mutex, returning");
      return;
    }

  // stop timers
  redrawTimerShort->stop();
  redrawTimerLong->stop();

  __redrawMap(m_sceduledFromLayer);
  m_sceduledFromLayer = topLayer;
}


void Map::slotCenterToWaypoint(const unsigned int) // id)
{
  // qDebug("Map::slotCenterToWaypoint");
  /*
    if(id >= _globalMapContents->getFlight()->getWPList().count())
    {
    warning("KFLog: Map::slotCenterToWaypoint: wrong Waypoint-ID");
    return;
    }

    _globalMapMatrix->centerToPoint(_globalMapMatrix->map(
    _globalMapContents->getFlight()->getWPList().at(id)->projP));
    _globalMapMatrix->slotSetScale(_globalMapMatrix->getScale(MapMatrix::LowerLimit));

    emit changed(this->size());
  */
}


void Map::slotCenterToFlight()
{
  // qDebug("Map::slotCenterToFlight");
  /*
    unsigned int i;

    Flight *f = (Flight *)_globalMapContents->getFlight();
    if (f) {
    QRect r;
    QRect r2;
    Q3PtrList<Flight> fl;

    switch (f->getTypeID()) {
    case BaseMapElement::Flight:
    r = f->getFlightRect();
    break;
    case BaseMapElement::FlightGroup:
    fl = ((FlightGroup *)f)->getFlightList();
    r = fl.at(0)->getFlightRect();
    for (i = 1; i < fl.count(); i++) {
    r2 = fl.at(i)->getFlightRect();
    r.setLeft(qMin(r.left(), r2.left()));
    r.setTop(qMin(r.top(), r2.top()));
    r.setRight(qMax(r.right(), r2.right()));
    r.setBottom(qMax(r.bottom(), r2.bottom()));
    }
    break;
    default:
    return;
    }

    // check if the Rectangle is zero
    // is it necessary here?
    if (!r.isNull()) {
    _globalMapMatrix->centerToRect(r);
    _globalMapMatrix->createMatrix(this->size());
    __redrawMap();
    }

    emit changed(this->size());
    }
  */
}


void Map::slotCenterToTask()
{
  // qDebug("Map::slotCenterToTask");
  /*
    unsigned int i;
    BaseFlightElement *f = _globalMapContents->getFlight();

    if(f)
    {
    QRect r;
    QRect r2;
    Q3PtrList<Flight> fl;

    switch (f->getTypeID())
    {
    case BaseMapElement::Flight:
    r = ((Flight *)f)->getTaskRect();
    break;
    case BaseMapElement::Task:
    r = ((FlightTask *)f)->getRect();
    break;
    case BaseMapElement::FlightGroup:
    fl = ((FlightGroup *)f)->getFlightList();
    r = fl.at(0)->getTaskRect();
    for (i = 1; i < fl.count(); i++) {
    r2 = fl.at(i)->getTaskRect();
    r.setLeft(qMin(r.left(), r2.left()));
    r.setTop(qMin(r.top(), r2.top()));
    r.setRight(qMax(r.right(), r2.right()));
    r.setBottom(qMax(r.bottom(), r2.bottom()));
    }
    break;
    default:
    return;
    }

    // check if the Rectangle is zero
    if(!r.isNull())
    {
    _globalMapMatrix->centerToRect(r);
    _globalMapMatrix->createMatrix(this->size());
    __redrawMap();

    emit changed(this->size());
    }
    }
  */
}

/**
 * search for a waypoint
 * First look in task itself
 * Second look in map contents
 */
bool Map::__getTaskWaypoint(QPoint current, struct wayPoint *wp, QList<wayPoint*> &taskPointList)
{
  // Radius for Mouse Snapping
  double delta(12.0);

  if(_globalMapMatrix->isSwitchScale())
    {
      delta = 6.0;
    }

  bool found = false;


  for(int i = 0; i < taskPointList.count(); i++)
    {
      wayPoint *tmpPoint = taskPointList.at(i);
      QPoint sitePos ( _globalMapMatrix->map(_globalMapMatrix->wgsToMap(tmpPoint->origP)));
      double dX = abs(sitePos.x() - current.x());
      double dY = abs(sitePos.y() - current.y());

      // Abstand entspricht der Icon-Groesse
      if (dX < delta && dY < delta)
        {
          *wp = *tmpPoint;
          found = true;
          break;
        }
    }

  if(!found)
    {
      /*
       *  Should be done for all Point-data
       */
      QList<int> contentArray;

      contentArray.append( MapContents::GliderList );
      contentArray.append( MapContents::AirportList );

      for( int n = 0; n < contentArray.count(); n++)
        {
          for(unsigned int loop = 0; loop < _globalMapContents->getListLength(contentArray.at(n)); loop++)
            {
              RadioPoint *hitElement = (RadioPoint*)_globalMapContents->getElement(contentArray.at(n), loop);
              QPoint sitePos (hitElement->getMapPosition());
              double dX = abs(sitePos.x() - current.x());
              double dY = abs(sitePos.y() - current.y());

              if (dX < delta && dY < delta)
                {
                  wp->name = hitElement->getWPName();
                  wp->origP = hitElement->getWGSPosition();
                  wp->elevation = hitElement->getElevation();
                  wp->projP = hitElement->getPosition();
                  wp->description = hitElement->getName();
                  wp->type = hitElement->getTypeID();
                  wp->elevation = hitElement->getElevation();
                  wp->icao = hitElement->getICAO();
                  wp->frequency = hitElement->getFrequency().toDouble();
                  wp->runway = 0;
                  wp->length = 0;
                  wp->surface = 0;
                  wp->comment = "";
                  wp->isLandable = true;

                  found = true;
                  break;
                }
            }
        }
    }

  return found;
}


/** Draws the waypoints of the active waypoint catalog on the map */
void Map::__drawWaypoints(QPainter* wpPainter)
{
  int i, n;
  QList<wayPoint*> *wpList;
  wayPoint * wp;
  bool isSelected;

  int w = this->size().width();
  int h = this->size().height();

  QRect testRect(-10, -10, w + 20, h + 20);
  QString labelText;
  Altitude alt;
  Distance dist;

  wpList = _globalMapContents->getWaypointList();
  extern MapConfig * _globalMapConfig;

  wpPainter->setBrush(Qt::NoBrush);

  // now do complete list
  n =  wpList->count();
  for (i=0; i < n; i++)
    {
      wp = (wayPoint*)wpList->at(i);

      //show now only is used for the currently selected wp, but
      //could also be used for waypoints in the task or other
      //super-important waypoints later on.
      isSelected=false;

      if (calculator && calculator->getselectedWp() && wp )
        {
          if (calculator->getselectedWp()->name == wp->name &&
              calculator->getselectedWp()->origP == wp->origP)
            {
              isSelected = true;
            }
        }

      if( _globalMapMatrix->getScale(MapMatrix::CurrentScale) > 1024.0 && ! isSelected )
        {
          // Don't draw waypoints at this high scale
          continue;
        }

      if (((uint) wp->importance >= _globalMapMatrix->currentDrawScale()) || isSelected)
        { //if the waypoint is important enough for the current mapscale
          // make sure projection is ok, and map to screen

          QPoint P = _globalMapMatrix->map(wp->projP);

          if (testRect.contains(P))
            {
              // draw marker
              // QColor col = ReachableList::getReachColor(wp->name);
              reachable reachable=ReachableList::getReachable(wp->origP);
              if( isSelected )
                {
                  wpPainter->setPen(QPen(Qt::white, 3, Qt::SolidLine));
                }
              else
                {
                  wpPainter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
                }

              QPixmap pm;

              if( _globalMapConfig->isRotatable(wp->type) )
                pm = _globalMapConfig->getPixmapRotatable( wp->type,false );
              else
                pm= _globalMapConfig->getPixmap(wp->type,false); //,col);

              int xOffset=16;
              int yOffset=16;
              int iconSize = 32;
              if (_globalMapConfig->useSmallIcons())
                {
                  xOffset=8;
                  yOffset=8;
                  iconSize = 16;
                }

              if (reachable == yes)
                {
                  //draw green circle
                  wpPainter->drawPixmap(P.x() - 9, P.y() -9, _globalMapConfig->getPixmap("green_circle.xpm"));
                }
              else if (reachable == belowSavety)
                {
                  //draw magenta circle
                  wpPainter->drawPixmap(P.x() - 9, P.y() -9, _globalMapConfig->getPixmap("magenta_circle.xpm"));
                }

              int rw2 = wp->runway >= 180 ? wp->runway-180 : wp->runway;
              int shift = ((rw2)/10);
              if( _globalMapConfig->isRotatable(wp->type) )
                wpPainter->drawPixmap(P.x() - iconSize/2, P.y() - iconSize/2, pm,
                                      shift*iconSize, 0, iconSize, iconSize);
              else
                wpPainter->drawPixmap(P.x()-xOffset,P.y()-yOffset, pm);

              // draw name of wp
              if (_globalMapConfig->getShowWpLabels())
                {
                  // do we need to show the labels at all?
                  labelText=wp->name;
                  if ( _globalMapConfig->getShowWpLabelsExtraInfo() )
                    { //just the name, or also additional info?
                      if ( wp->isLandable )
                        {
                          if (reachable==yes)
                            { //reachable? then the label will become bold
                              labelText = "<qt><b>" + labelText + "</b><br>";
                            }
                          else
                            {
                              labelText = "<qt>" + labelText + "<br>";
                            }

                          dist = ReachableList::getDistance( wp->origP );
                          if ( dist.isValid() )
                            { //check if the distance is valid...
                              labelText += dist.getText( false, uint(0), uint(0) )
                                           +  "/";
                              alt = ReachableList::getArrivalAltitude( wp->origP );
                              if ( alt.getMeters()<0 ) //the color is dependent on the arrival altitude
                                {
                                  labelText += "<font color=\"#FF0000\">"
                                               + alt.getText( false, 0 )
                                               + "</font></qt>";
                                } else
                                {
                                  labelText += "&nbsp;"
                                               + alt.getText( false, 0 )
                                               + "</qt>";
                                }
                            }
                        }
                    }
                  else
                    {
                      if ( wp->isLandable )
                        {
                          if (reachable==yes)
                            {
                              labelText = "<qt><b>" + labelText + "</b></qt>";
                            }
                        }
                    }

                  Q3SimpleRichText rtext ( labelText, this->font() );
                  rtext.adjustSize();

                  QRect textbox( 0, 0, rtext.widthUsed(), rtext.height() );
                  if (wp->origP.lon()<_globalMapMatrix->getMapCenter(false).y())
                    {
                      //the wp is on the left side of the map, so draw the textlabel on the right side
                      xOffset=10;
                      yOffset=(textbox.height()/4)*3;
                      if (_globalMapConfig->useSmallIcons())
                        {
                          xOffset=6;
                          yOffset=-0;
                        }
                    }
                  else
                    {
                      //the wp is on the right side of the map, so draw the textlabel on the left side
                      xOffset=-textbox.width()-14;
                      yOffset=(textbox.height()/4)*3;
                      if (_globalMapConfig->useSmallIcons())
                        {
                          xOffset=-textbox.width()-6;
                          yOffset=0;
                        }
                    }

                  QColorGroup cg;

                  if (!isSelected)
                    {
                      wpPainter->fillRect(P.x() + xOffset - 1,
                                          P.y() + yOffset + 1,
                                          textbox.width() + 2,
                                          -textbox.height() + 1,
                                          Qt::white);

                      cg.setColor( QColorGroup::Background, Qt::white );
                      cg.setColor( QColorGroup::Foreground, Qt::black );
                      cg.setColor( QColorGroup::Text, Qt::black );
                    }
                  else
                    {
                      wpPainter->fillRect(P.x() + xOffset - 1,
                                          P.y() + yOffset + 1,
                                          textbox.width() + 2,
                                          -textbox.height() + 1,
                                          Qt::black);

                      cg.setColor( QColorGroup::Background, Qt::black );
                      cg.setColor( QColorGroup::Foreground, Qt::white );
                      cg.setColor( QColorGroup::Text, Qt::white );
                    }

                  rtext.draw(wpPainter,
                             P.x() + xOffset + 1,
                             P.y() + yOffset - textbox.height() + 2,
                             QRect( 0, 0, width() + 2, height() + 2 ), //textbox,
                             cg);
                }//draw labels?
            } //in visible area?
        } //important enough?
    } //for loop
}


/** Slot signalled when user selects another waypointcatalog.  */
void Map::slotWaypointCatalogChanged(WaypointCatalog* /*c*/)
{
  // qDebug("Map::slotWaypointCatalogChanged");
  emit changed(this->size());
  __redrawMap(waypoints);
}


/** This function sets the map rotation and redraws the map if the
    map rotation differs too much from the current map rotation. */
void Map::setMapRot(int newRotation)
{
  // qDebug("Map::setMapRot");
  mapRot=newRotation;

  if (abs(mapRot-curMapRot) >= 2)
    {
      sceduleRedraw(baseLayer);
    }
}


/** Read property of int heading. */
const int& Map::getHeading()
{
  return heading;
}


/** Write property of int heading. */
void Map::setHeading( const int& _newVal)
{
  heading = _newVal;
  setMapRot(calcMapRotation());
}


/** Read property of int bearing. */
const int& Map::getBearing()
{
  return bearing;
}


/** Write property of int bearing. */
void Map::setBearing( const int& _newVal)
{
  bearing = _newVal;
  setMapRot(calcMapRotation());
}


/** calculates the maprotation based on the map mode, the heading and
    the bearing. In degrees counter clockwise. */
int Map::calcMapRotation()
{
  switch (mode)
    {
    case northUp:
      return 0;
    case headUp:
      return -heading;
    case trackUp:
      return -bearing;
    default:
      return 0;
    }
}


/** calculates the rotation of the glider symbol based on the map mode,
    the heading and the bearing. In degrees counter clockwise. */
int Map::calcGliderRotation()
{
  switch (mode)
    {
    case northUp:
      return heading;
    case headUp:
      return 0;
    case trackUp:
      return heading-bearing;
    default:
      return heading;
    }
}


/** Read property of mapMode mode. */
Map::mapMode Map::getMode() const
  {
    return mode;
  }


/** Write property of mapMode mode. */
void Map::setMode( const mapMode& )
{ //_newVal){
  //  mode = _newVal;
  mode = Map::northUp; //other modes are not supported yet
  setMapRot(calcMapRotation());
}


/** Write property of bool ShowGlider. */
void Map::setShowGlider( const bool& _newVal)
{
  if (ShowGlider!=_newVal)
    {
      ShowGlider = _newVal;
      __redrawMap(informationLayer);
    }
}


/** Draws a scale indicator on the pixmap. */
void Map::__drawScale()
{
  QPainter scaleP;

  scaleP.begin(&m_pixInformationMap);

  QPen pen;
  QBrush brush(Qt::white);

  pen.setColor(Qt::black);
  pen.setWidth(3);
  pen.setCapStyle(Qt::RoundCap);
  scaleP.setPen(pen);

  double scale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  Distance barLen;

  /** select appropriate length of bar. This needs to be done for each unit
   * seperately, because else you'd get weird, broken barlengts.
   * Note: not all possible units are taken into account (meters, feet),
   * because we are not going to use these for horizontal distances (at least
   * not externaly.) */
  int len=1;

  switch (barLen.getUnit())
    {
    case Distance::kilometers:
      len = 100;
      if (scale<1000)
        len=50;
      if (scale<475)
        len=25;
      if (scale<240)
        len=10;
      if (scale<100)
        len=5;
      if (scale<50)
        len=3;
      if (scale<20)
        len=1;
      barLen.setKilometers(len);
      break;

    case Distance::miles:
      len=60;
      if (scale<1000)
        len=30;
      if (scale<475)
        len=12;
      if (scale<200)
        len=6;
      if (scale<95)
        len=4;
      if (scale<60)
        len=2;
      if (scale<30)
        len=1;
      barLen.setMiles(len);
      break;

    case Distance::nautmiles:
      len=50;
      if (scale<1000)
        len=25;
      if (scale<450)
        len=10;
      if (scale<175)
        len=5;
      if (scale<90)
        len=3;
      if (scale<55)
        len=1;
      barLen.setNautMiles(len);
      break;

    default: //should not happen, other units are not used for horizontal distances.
      len = 100;
      if (scale<1000)
        len=50;
      if (scale<475)
        len=25;
      if (scale<240)
        len=10;
      if (scale<100)
        len=5;
      if (scale<50)
        len=3;
      if (scale<20)
        len=1;
      barLen.setKilometers(len);
      break;
    };

  //determine how long the bar should be in pixels
  int drawLength = (int)rint(barLen.getMeters()/scale);
  //...and where to start drawing.
  int leftXPos=this->width()-drawLength-5;
  //Now, draw the bar
  scaleP.drawLine(leftXPos,this->height()-5,this->width()-5,this->height()-5);       //main bar
  pen.setWidth(2);
  scaleP.setPen(pen);
  scaleP.drawLine(leftXPos,this->height()-9,leftXPos,this->height()-1);              //left endbar
  scaleP.drawLine(this->width()-5,this->height()-9,this->width()-5,this->height()-1);//right endbar

  //get the string to draw
  QString scaleText=barLen.getText(true,0);
  //get some metrics for this string
  QRect txtRect=scaleP.fontMetrics().boundingRect(scaleText);
  int leftTPos=this->width()+int((drawLength-txtRect.width())/2)-drawLength-5;

  //draw white box to draw text on
  scaleP.setBrush(brush);
  scaleP.setPen(Qt::NoPen);
  scaleP.drawRect(leftTPos,this->height()-txtRect.height()+2, txtRect.width()+1, txtRect.height()-2);

  //draw text itself
  scaleP.setPen(pen);
  scaleP.drawText(leftTPos,this->height()-10+txtRect.height()/2,scaleText);
  scaleP.end();
}


/** This slot is called to set a new position. The map object determines if it is necessary to recenter the map, or if the glider can just be drawn on a different position. */
void Map::slot_position(const QPoint& newPos, const int source)
{
  // qDebug("Map::slot_position x=%d y=%d", newPos.x(), newPos.y() );

  if( !_isEnable )
    return;

  if(source == CuCalc::GPS)
    {
      if (curGPSPos!=newPos)
        {
          curGPSPos=newPos;

          if(!calculator->isManualInFlight())
            {
              // let cross be at the GPS position so that if we switch to manual mode
              // show up at the same location
              curMANPos = curGPSPos;

              if (!_globalMapMatrix->isInCenterArea(newPos) || mutex() )
                {
                  // qDebug("Map::slot_position:sceduleRedraw()");
                  // this is the slow redraw
                  sceduleRedraw();
                }
              else
                {
                  // this is the fast redraw
                  __redrawMap(informationLayer);
                }
            }
          else
            {
              // if we are in manual mode, the real center is the cross, not the glider
              __redrawMap(informationLayer);
            }
        }
    }
  else
    { // source is CuCalc::MAN
      if (curMANPos!=newPos)
        {
          curMANPos=newPos;

          if( !_globalMapMatrix->isInCenterArea(newPos) || mutex() )
            {
              // qDebug("Map::slot_position:sceduleRedraw()");
              sceduleRedraw();
            }
          else
            {
              __redrawMap(informationLayer);
            }
        }
    }
}

void Map::slot_switchManualInFlight()
{
  // show or hide the cross; for switchOn == true, positions are identical
  // the cross is drawn in the scale layer

  // if positions are the same, calculator->setPosition(curGPSPos)
  // would not redraw which is required to show the cross
  if(curMANPos != curGPSPos)
    {
      // if switch off, the map is redrawn which takes some time
      // and in this the the cross is still visible. So the user may think
      // the switch off was not successfully; To hide the cross immediately,
      // call redraw of scale layer
      __redrawMap(scale);
      calculator->setPosition(curGPSPos);
    }
  else
    {
      __redrawMap(scale);
    }
}


/** Draws the glidersymbol on the pixmap */
void Map::__drawGlider()
{
  //get the projected coordinates of the current position
  QPoint projPos=_globalMapMatrix->wgsToMap(curGPSPos);
  //map them to a coordinate on the pixmap
  int Rx = _globalMapMatrix->map(projPos).x();
  int Ry = _globalMapMatrix->map(projPos).y();

  QRect rect( QPoint(), size() );

  //don't continue if position is outside of window's viewport
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  int rot=calcGliderRotation();
  rot=((rot+7)/15) % 24;  //we only want to rotate in steps of 15 degrees. Finer is not usefull.

  //now, draw the line from the glider symbol to the waypoint
  __drawDirectionLine(QPoint(Rx,Ry));

  // @ee the glider pixmap contains all rotated glider symbols.
  QPainter p(&m_pixInformationMap);
  p.drawPixmap( Rx-40, Ry-40, _glider, rot*80, 0, 80, 80 );
}


/** Draws the Xsymbol on the pixmap */
void Map::__drawX()
{
  //get the projected coordinates of the current position
  QPoint projPos=_globalMapMatrix->wgsToMap(curMANPos);
  //map them to a coordinate on the pixmap
  int Rx = _globalMapMatrix->map(projPos).x();
  int Ry = _globalMapMatrix->map(projPos).y();

  QRect rect( QPoint(), size() );

  //don't continue if position is outside of window's viewport
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  if(!ShowGlider)
    {
      //now, draw the line from the X symbol to the waypoint
      __drawDirectionLine(QPoint(Rx,Ry));
    }

  // @ee draw preloaded pixmap
  QPainter p(&m_pixInformationMap);
  p.drawPixmap(  Rx-20, Ry-20, _cross );
}


/** Used to zoom in on the map. Will schedule a redraw. */
void Map::slotZoomIn()
{
  // @AP: block zoom events, if map is redrawn
  if( mutex() )
    {
      //qDebug("Map::slotZoomIn(): mutex is locked, returning");
      return;
    }

  if( _globalMapMatrix->getScale() == _globalMapMatrix->getScale(MapMatrix::LowerLimit) )
    {
      // @AP: lower limit reached, ignore event
      return;
    }

  if( zoomProgressive < 7 && redrawTimerShort->isActive() )
    zoomProgressive++;
  else if( zoomProgressive == 7 && redrawTimerShort->isActive() )
    return;
  else
    zoomProgressive=0;

  zoomFactor/=zoomProgressiveVal[zoomProgressive];

  sceduleRedraw();
  QString msg;
  msg.sprintf(tr("Map zoom in, scale: %.1f"),zoomFactor/L_LIMIT );
  _globalMapView->message( msg );
}


/** Used . Will schedule a redraw. */
void Map::slotRedraw()
{
  // qDebug("Map::slotRedraw");
  sceduleRedraw();
}


/** Used to zoom the map out. Will scedule a redraw. */
void Map::slotZoomOut()
{

  // @AP: block zoom events, if map is redrawn
  if( mutex() )
    {
      //qDebug("Map::slotZoomOut(): mutex is locked, returning");
      return;
    }

  if( _globalMapMatrix->getScale() == _globalMapMatrix->getScale(MapMatrix::UpperLimit) )
    {
      // @AP: upper limit reached, ignore event
      return;
    }

  if( zoomProgressive < 7 && redrawTimerShort->isActive() )
    zoomProgressive++;
  else if( zoomProgressive == 7 && redrawTimerShort->isActive() )
    return;
  else
    zoomProgressive=0;

  zoomFactor*=zoomProgressiveVal[zoomProgressive];

  sceduleRedraw();
  QString msg;
  msg.sprintf(tr("Map zoom out, scale: %.1f"),zoomFactor/L_LIMIT );

  _globalMapView->message( msg );
}


/**
 * This function scedules a redraw of the map. It sets two timers:
 * The first timer is set for a small interval, and reset every time sceduleRedraw
 * is called. This allows for several modifications to the map being used for the
 * redraw at once.
 * The second timer is set for a larger interval, and is not reset. It makes sure
 * the redraw occurs once in awhile, even if events modifying the map keep comming
 * in and would otherwise prevent the map from being redrawn.
 *
 * If either of the two timers expires, the status of redrawScheduled is reset
 * and the map is redrawn to reflect the current position and zoom factor.
 */
void Map::sceduleRedraw(mapLayer fromLayer)
{
  qDebug("Map::sceduleRedraw(): mapLayer=%d, loopLevel=%d", fromLayer, qApp->loopLevel() );

  if( !_isEnable )
    {
      _isRedrawEvent = false;
      return;
    }

  // scedule requested layer
  m_sceduledFromLayer = qMin(m_sceduledFromLayer, fromLayer);

  if( mutex() )
    {
      // Map drawing is running therefore queue redraw request
      // only. The timers will be restarted at the end of the
      // drawing routine, if the _isRedrawEvent flag is set.
      _isRedrawEvent = true;
      return;
    }

  // qDebug("Map::sceduleRedraw");
  if (!redrawTimerLong->isActive() && ShowGlider)
    {
      redrawTimerLong->start(1500);

    }

  redrawTimerShort->start(1000);
}


/** sets a new scale */
void Map::slotSetScale(const double& newScale)
{
  // qDebug("Map::slotSetScale");
  if (newScale!=zoomFactor)
    {
      zoomFactor=newScale;
      sceduleRedraw();
    }
}


/**
 * This function draws a "direction line" on the map if a waypoint has been
 * selected. The QPoint is the projected & mapped coordinate of the positionsymbol
 * on the map, so we don't have to calculate that all over again.
 */
void Map::__drawDirectionLine(const QPoint& from)
{
  if (!_globalMapConfig->getdrawBearing()) return;

  if (calculator && calculator->getselectedWp())
    {
      QPoint to  = _globalMapMatrix->map(calculator->getselectedWp()->projP);

      if( from == to )
        {
          // no point in drawing - a line without length
          return;
        }

      QColor col = ReachableList::getReachColor(calculator->getselectedWp()->origP);

      QPainter lineP;
      lineP.begin(&m_pixInformationMap);
      lineP.setClipping(true);
      lineP.setPen(QPen(col, 5, Qt::DashLine));
      lineP.drawLine(from, to);
      lineP.end();
    }
}


/**
 * check if the new position is near to an airspace.
 */
void Map::checkAirspace(const QPoint& pos)
{
  if ( mutex() )
    {
      // qDebug("Map::checkAirspace: Map drawing in progress: return");
      return;
    }

  bool warningEnabled = GeneralConfig::instance()->getAirspaceWarningEnabled();
  bool fillingEnabled = GeneralConfig::instance()->getAirspaceFillingEnabled();

  bool needAirspaceRedraw = false;
  QString inside("");
  QString veryNear("");
  QString near("");

  AltitudeCollection alt = calculator->getAltitudeCollection();
  AirspaceWarningDistance awd = GeneralConfig::instance()->getAirspaceWarningDistances();
  Airspace* pSpace = (Airspace *) 0;
  Airspace::ConflictType hConflict=Airspace::none, lastHConflict=Airspace::none,
                                   vConflict=Airspace::none, lastVConflict=Airspace::none,
                                                                           conflict= Airspace::none, lastConflict= Airspace::none;

  bool warn = false; // warning flag

  // check if there are overlaps between the region around our current position and airspaces
  for( int loop = 0; loop < airspaceRegList.count(); loop++ )
    {
      pSpace = airspaceRegList.at(loop)->airspace;

      lastVConflict = pSpace->lastVConflict();
      lastHConflict = airspaceRegList.at(loop)->currentConflict();
      lastConflict = (lastHConflict < lastVConflict ? lastHConflict : lastVConflict);

      // check for vertical conflicts at first
      vConflict = pSpace->conflicts(alt, awd);

      needAirspaceRedraw |= (vConflict != lastVConflict);
      if ( vConflict == Airspace::none )
        {
          // No altitude conflict with airspace
          continue;
        }

      // check for horizontal conflicts
      hConflict = airspaceRegList.at(loop)->conflicts(pos, awd);


      // the resulting conflict is always the lesser of the two
      conflict = (hConflict < vConflict ? hConflict : vConflict);

      // qDebug("Conflict=%d, hConflict=%d, vConflict=%d, AS=%s",
      //        conflict, hConflict, vConflict, pSpace->getInfoString().latin1() );

      needAirspaceRedraw |= (conflict != lastConflict);
      if (conflict == Airspace::none)
        {
          // No conflict with airspace
          continue;
        }

      // fetch warning suppress time from config and compute it as milli seconds
      int warSupMS = GeneralConfig::instance()->getWarningSuppressTime() * 60 * 1000;

      // Only the last found match is reported per category
      switch (conflict)
        {
        case Airspace::inside:
          if ( ! inside.isEmpty() )
            continue;
          inside = tr("Inside") + " " + Airspace::getTypeName(pSpace->getTypeID());

          if ( inside != _insideAS && _lastInside.elapsed() >= warSupMS )
            {
              //qDebug( "inside AS: %s", pSpace->getInfoString().latin1() );
              warn = true;
              _insideAS = inside;
              _insideASInfo = pSpace->getInfoString();
              continue;
            }

          break;

        case Airspace::veryNear:

          if ( ! veryNear.isEmpty() )
            continue;

          veryNear = tr("Very near") + " " + Airspace::getTypeName(pSpace->getTypeID());

          if ( veryNear != _veryNearAS && _lastVeryNear.elapsed() >= warSupMS )
            {
              // qDebug( "very near AS: %s", pSpace->getInfoString().latin1() );
              warn = true;
              _veryNearAS = veryNear;
              _veryNearASInfo = pSpace->getInfoString();
              continue;
            }

          break;

        case Airspace::near:

          if ( ! near.isEmpty() )
            continue;

          near = tr("Near") + " " + Airspace::getTypeName(pSpace->getTypeID());

          if ( near != _nearAS && _lastNear.elapsed() >= warSupMS )
            {
              // qDebug( "near AS: %s", pSpace->getInfoString().latin1() );
              warn = true;
              _nearAS = near;
              _nearASInfo = pSpace->getInfoString();
              continue;
            }

          break;

        case Airspace::none:
        default:
          break;
        }

    } // End of For loop


  //redraw the airspaces if needed
  if (needAirspaceRedraw && fillingEnabled)
    sceduleRedraw(aeroLayer);


  if (!warningEnabled)
    return;

  _nearAS     = near;
  _veryNearAS = veryNear;
  _insideAS   = inside;

  QString text = "<html><table border=1><tr><th align=left>" +
                 tr("Airspace&nbsp;Warning") +
                 "</th></tr>";

  QString msg;

  // @AP: Only the highest priority will be displayed and updated!
  if ( !inside.isEmpty() )
    {
      msg += inside + " ";

      if ( warn == false )
        {
          if ( _lastASInfo != msg )
            {
              // update warning in capture only
              _lastASInfo = msg;
              emit airspaceWarning( msg, false );
            }
          return;
        }

      _lastInside.start(); // set last reporting time
      text += "<tr><td align=left>";
      text += inside + "</td></tr><tr><td align=left>" + _insideASInfo + "</td></tr>";
    }
  else if (!veryNear.isEmpty())
    {
      msg += veryNear + " ";

      if( warn == false )
        {
          if( _lastASInfo != msg )
            {
              // update warning in capture only
              _lastASInfo = msg;
              emit airspaceWarning( msg, false );
            }
          return;
        }

      _lastVeryNear.start(); // set last reporting time
      text += "<tr><td align=left>";
      text += veryNear + "</td></tr><tr><td align=left>" + _veryNearASInfo + "</td></tr>";
    }
  else if (!near.isEmpty())
    {
      msg += near + " ";

      if ( warn == false )
        {
          if ( _lastASInfo != msg )
            {
              _lastASInfo = msg;
              // update warning in capture only
              emit airspaceWarning( msg, false );
            }
          return;
        }

      _lastNear.start(); // set last reporting time
      text += "<tr><td align=left>";
      text += near + "</td></tr><tr><td align=left>" + _nearASInfo + "</td></tr>";
    }
  else
    {
      if( _lastASInfo != msg )
        {
          // reset warning in capture only
          _lastASInfo = msg;
          emit airspaceWarning( "", false );
        }

      return;
    }

  text +="</table></html>";

  // Pop up a warning window with all data to touched airspace
  if ( GeneralConfig::instance()->getAirspaceWarningEnabled(pSpace->getTypeID()) && warn == true )
    {
      emit airspaceWarning(msg);

      if( GeneralConfig::instance()->getPopupAirspaceWarnings() )
        {
          int showTime = GeneralConfig::instance()->getWarningDisplayTime() * 1000;

          // qDebug("airspace warning timer %dms, LoopLevel=%d",
          // showTime, qApp->loopLevel() );
          WhatsThat *box = new WhatsThat(this, text, showTime);
          box->show();
          return;
        }
    }
}


void Map::quickDraw()
{
  __redrawMap(navigationLayer);
}


bool Map::mutex()
{
  return _mutex;
}


void Map::setMutex(bool m)
{
  _mutex = m;
  emit isRedrawing(m);
}


bool Map::eventFilter(QObject * , QEvent * )
{
  if (_mutex)
    {
      qDebug("Map::eventFilter has blocked event.");
      return true; //stop all events while we're redrawing the map!
    }
  return false;
}
