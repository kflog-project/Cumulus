/***********************************************************************
 **
 **   map.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 by Josua Dietze
 **                   2008-2020 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <ctype.h>
#include <cstdlib>
#include <cmath>

#include <QtGui>

#include "airfield.h"
#include "airspace.h"
#include "AirspaceFilters.h"
#include "calculator.h"
#include "mainwindow.h"
#include "distance.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "layout.h"
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

#ifdef FLARM
#include "flarm.h"
#include "flarmdisplay.h"
#endif

extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;
extern MapConfig   *_globalMapConfig;
extern MapView     *_globalMapView;

Map *Map::instance = static_cast<Map *>(0);

#ifdef MAEMO
#define TRAIL_LENGTH 7*60
#else
#define TRAIL_LENGTH 10*60
#endif

Map::Map(QWidget* parent) : QWidget(parent),
  TrailListLength( TRAIL_LENGTH )
{
//  qDebug( "Map::Map parent window size is %dx%d, width=%d, height=%d",
//          size().width(),
//          size().height(),
//          size().width(),
//          size().height() );

  setObjectName("Map");

  instance = this;
  m_isEnable = false;  // Disable map redrawing at startup
  m_isResizeEvent = false;
  m_isRedrawEvent = false;
  m_mouseMoveIsActive = false;
  m_ignoreMouseRelease = false;
  m_mapRot = 0;
  m_curMapRot = 0;
  m_heading = 0;
  m_bearing = 0;
  m_lastRelBearing = -999;
  m_mode = northUp;
  m_scheduledFromLayer = baseLayer;
  m_ShowGlider = false;
  setMutex(false);

  // Initiate loading of airspace filter data
  AirspaceFilters::loadFilterData();

  //setup progressive zooming values
  m_zoomProgressive = 0;
  m_zoomProgressiveVal[0] = 1.25;

  for( int i=1; i<8; i++ )
    {
      m_zoomProgressiveVal[i] = m_zoomProgressiveVal[i-1] * 1.25;
    }

  // Set pixmaps first to the size of the parent
  m_pixPaintBuffer = QPixmap( parent->size() );
  m_pixPaintBuffer.fill(Qt::white);

  QPalette palette;
  palette.setColor(backgroundRole(), Qt::white);
  setPalette(palette);

  m_redrawTimerShort = new QTimer(this);
  m_redrawTimerShort->setSingleShot(true);

  m_redrawTimerLong  = new QTimer(this);
  m_redrawTimerLong->setSingleShot(true);

  connect( m_redrawTimerShort, SIGNAL(timeout()),
            this, SLOT(slotRedrawMap()));

  connect( m_redrawTimerLong, SIGNAL(timeout()),
            this, SLOT(slotRedrawMap()));

  m_showASSTimer = new QTimer(this);
  m_showASSTimer->setSingleShot(true);

  connect( m_showASSTimer, SIGNAL(timeout()),
            this, SLOT(slotASSTimerExpired()));

  m_zoomFactor = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  m_curMANPos  = _globalMapMatrix->getMapCenter();
  m_curGPSPos  = _globalMapMatrix->getMapCenter();
  m_cross      = _globalMapConfig->getCross();

  for( int i = 0; i < 360/10; i++ )
    {
      m_glider[i] = _globalMapConfig->createGlider( i*10 );
    }
}

Map::~Map()
{
  qDeleteAll(m_airspaceRegionList);
}

/**
 * Display Info about Airspace items
*/
void Map::p_displayAirspaceInfo(const QPoint& current)
{
  static QPointer<WhatsThat> box;

  if( mutex() || ! isVisible() || ! box.isNull() )
    {
      //qDebug("Map::p_displayAirspaceInfo: Map drawing in progress: return");
      return;
    }

  int itemcount=0;

  QString text;

  bool show = false;

  text += "<html><table border=1 cellpadding=\"2\"><tr><th align=center>" +
          tr("Airspace&nbsp;Structure") +
          "</th></tr>";

  for( int loop = 0; loop < m_airspaceRegionList.count(); loop++ )
    {
      if(m_airspaceRegionList.at(loop)->m_region->contains(current))
        {
          Airspace* pSpace = m_airspaceRegionList.at(loop)->m_airspace;

          // qDebug ("name: %s", pSpace->getName().toLatin1().data());
          // qDebug ("lower limit: %d", pSpace->getLowerL());
          // qDebug ("upper limit: %d", pSpace->getUpperL());
          // qDebug ("lower limit type: %d", pSpace->getLowerT());
          // qDebug ("upper limit type: %d", pSpace->getUpperT());

          if( pSpace == 0 )
            {
              // Check, if a valid airspace is assigned.
              continue;
            }

          if( pSpace->getTypeID() == BaseMapElement::AirFlarm )
            {
              // Filter out invalid and inactive Flarm alert zones
              if( pSpace->getFlarmAlertZone().isValid() == false ||
                  pSpace->getFlarmAlertZone().isActive() == false )
                {
                  continue;
                }
            }

          //work around the phenomenon that airspaces tend to appear in the list twice -> this should be dealt with properly!
          if ( text.indexOf(pSpace->getInfoString()) == -1 )
            {
              // only add if the string has not been added before
              text += "<tr><td align=left>" + pSpace->getInfoString() + "</td></tr>";
              itemcount++;
            }

          show = true;
        }
    }

  if( show == false )
    {
      return;
    }

  text += "</table></html>";

  // @AP: replace long labels to small ones, otherwise the window is
  // sometimes to small for display whole line.
  // remove term Restricted, to display all info behind
  text.replace( QRegExp("Restricted "), "AR " );
  text.replace( QRegExp("Danger "), "AD " );
  text.replace( QRegExp("Prohibited "), "AP " );
  text.replace( QRegExp("AS-E low "), "AS-El " );
  text.replace( QRegExp("AS-E high "), "AS-Eh " );

  int showTime = GeneralConfig::instance()->getAirspaceDisplayTime() * 1000;
  box = new WhatsThat(this, text, showTime);
  box->show();
}

/**
 * Check, if a zoom button on the map was pressed. Handle zoom request and
 * return true in this case otherwise false.
 */
bool Map::p_zoomButtonPress(const QPoint& point)
{
  int plusWidth  = _globalMapConfig->getPlusButton().width();
  int minusWidth = _globalMapConfig->getMinusButton().width();

  QPainterPath plus;
  plus.addRect( width()-plusWidth, 0, plusWidth, plusWidth );

  QPainterPath minus;
  minus.addRect( width()-minusWidth, height()-minusWidth, minusWidth, minusWidth );

  if( plus.contains( point ) )
    {
      slotZoomIn();
      return true;
    }

  if( minus.contains( point ) )
    {
      slotZoomOut();
      return true;
    }

  return false;
}

/**
 * Display detailed Info about a task point, an airfield, a glider site,
 * a waypoint or an airspace. The first found shortest point is taken as
 * result.
 */
void Map::p_displayDetailedItemInfo(const QPoint& current)
{
  if( mutex() )
    {
      return;
    }

  bool found = false;

  // Radius for Mouse Snapping
  int delta = 0, dX = 0, dY = 0;

  // Define lists to be used for searching. Consider current drawing.
  QList<int> searchList;

  if( _globalMapMatrix->isBorder2() )
    {
      // These lists are drawn to this border
      searchList.append( MapContents::AirfieldList );
      searchList.append( MapContents::GliderfieldList );
      searchList.append( MapContents::OutLandingList );
      searchList.append( MapContents::RadioList );
    };

  if( _globalMapMatrix->isSwitchScale() )
    {
      // This list is drawn to this border only
      searchList.append( MapContents::HotspotList );
    }

  Waypoint *w = static_cast<Waypoint *> (0);

  // scale uses unit meter/pixel
  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  // snap distance
  delta = Layout::mouseSnapRadius();

  // Manhattan distance is used to found the point.
  int lastDist = 2 * delta + 1;

  // snapRect about the touched position
  QRect snapRect( current.x() - delta, current.y() - delta, 2 * delta, 2 * delta );

  //qDebug( "SnapRect %dx%d, delta=%d, w=%d, h=%d",
  //        current.x()-delta, current.y()-delta, delta, 2*delta, 2*delta );

  // As first search in the current task list, if waypoints can be found.
  FlightTask* task = (FlightTask*) _globalMapContents->getCurrentTask();

  if (task != static_cast<FlightTask *> (0) && cs < 1024.0)
    {
      QList<TaskPoint*>& tpList = task->getTpList ();

      for (int i = 0; i < tpList.size (); i++)
        {
          TaskPoint* tp = tpList[i];

          QPoint sitePos (_globalMapMatrix->map (tp->getPosition ()));

          if (!snapRect.contains (sitePos))
            {
              // @AP: Point lays outside of snap rectangle, we ignore it
              continue;
            }

          dX = abs (sitePos.x () - current.x ());
          dY = abs (sitePos.y () - current.y ());

          if (dX < delta && dY < delta)
            {
              if (found && ((dX + dY) >= lastDist))
                {
                  continue; // the point we found earlier was closer
                }

              found = true;
              w = tp->getWaypointObject ();
              lastDist = dX + dY;

              if (lastDist < (delta / 3))
                {
                  break;
                }
            }
        }
    }

  if( found == true )
    {
      emit showPoi( w );
      return;
    }

  // @AP: On map scale higher as 1024 we don't evaluate anything
  for( int l = 0; l < searchList.size() && cs < 1024.0; l++ )
    {
      for (unsigned int loop = 0;
          loop < _globalMapContents->getListLength (searchList.at (l)); loop++)
        {
          // Get specific site data from current list. We have to
          // distinguish between AirfieldList, GilderfieldList, OutlandingList
          // RadioList and HotspotList.
          SinglePoint* poi;

          QString siteName;
          QString siteDescription;
          short siteType;
          WGSPoint siteWgsPosition;
          QPoint sitePosition;
          float siteElevation;
          QPoint curPos;
          QString siteComment;
          QString siteCountry;

          if (searchList[l] == MapContents::AirfieldList)
            {
              // Fetch data from the airfield list
              poi = _globalMapContents->getAirfield (loop);
            }
          else if (searchList[l] == MapContents::GliderfieldList)
            {
              // fetch data from the gliderfield list
              poi = _globalMapContents->getGliderfield (loop);
            }
          else if (searchList[l] == MapContents::OutLandingList)
            {
              // fetch data from the outlanding list
              poi = _globalMapContents->getOutlanding (loop);
            }
          else if (searchList[l] == MapContents::RadioList)
            {
              // fetch data from the radio point list
              poi = _globalMapContents->getRadioPoint (loop);
            }
          else if (searchList[l] == MapContents::HotspotList)
            {
              // fetch data from the hotspot list
              poi = _globalMapContents->getHotspot (loop);
            }
          else
            {
              qWarning (
                  "Map::p_displayDetailedItemInfo: ListType %d is unknown",
                  searchList[l]);
              break;
            }

          curPos = poi->getMapPosition ();

          if (!snapRect.contains (curPos))
            {
              // @AP: Point lays outside of snap rectangle, we ignore it
              continue;
            }

          dX = abs (curPos.x () - current.x ());
          dY = abs (curPos.y () - current.y ());

          // qDebug( "pX=%d, pY=%d, cX=%d, cY=%d, delta=%d, dX=%d, dY=%d, lastDist=%d",
          //         curPos.x(), curPos.y(), current.x(), current.y(), delta, dX, dY, lastDist );

          // Abstand entspricht der Icon-Groesse
          if (dX < delta && dY < delta)
            {
              if (found && ((dX + dY) >= lastDist))
                {
                  // The point we found earlier was closer but a
                  // taskpoint can be overwritten by an better point.
                  continue;
                }

              siteName = poi->getWPName ();
              siteDescription = poi->getName ();
              siteType = poi->getTypeID ();
              siteWgsPosition = poi->getWGSPosition ();
              sitePosition = poi->getPosition ();
              siteElevation = poi->getElevation ();
              siteComment = poi->getComment ();
              siteCountry = poi->getCountry ();

              w = &m_wp;
              w->name = siteName;
              w->description = siteDescription;
              w->type = siteType;
              w->wgsPoint = siteWgsPosition;
              w->projPoint = sitePosition;
              w->elevation = siteElevation;
              w->comment = siteComment;
              w->country = siteCountry;
              w->icao.clear ();
              w->frequencyList.clear ();
              w->taskPointIndex = -1;
              w->wpListMember = false;
              w->rwyList.clear ();

              Airfield* af = dynamic_cast<Airfield *> (poi);
              RadioPoint* rp = dynamic_cast<RadioPoint *> (poi);

              if (af != static_cast<Airfield *> (0))
                {
                  // This is an airfield object
                  w->icao = af->getICAO ();
                  w->frequencyList = af->getFrequencyList ();
                  w->rwyList = af->getRunwayList ();
                }
              else if (rp != static_cast<RadioPoint *> (0))
                {
                  // This is a RadioPoint
                  w->icao = rp->getICAO ();
                  w->frequencyList = rp->getFrequencyList ();

                  // Workaround for declination a.s.o. These data are passed
                  // as comment.
                  if (m_wp.comment.isEmpty () == false)
                    {
                      m_wp.comment += ", ";
                    }

                  m_wp.comment += rp->getAdditionalText ();
                }

              found = true;
              lastDist = dX + dY;

              if (lastDist < (delta / 3))
                {
                  // if we're very near, stop searching the list
                  break;
                }
            }
        }
    }

  if( found == true )
    {
      emit showPoi( w );
      return;
    }

  // let's show waypoints
  // @AP: On map scale higher as 1024 we don't evaluate anything
  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  for( int i = 0; i < wpList.count() && cs < 1024.0; i++ )
    {
      Waypoint& wp = wpList[i];

      // consider only points, which are to draw on the map
      if( _globalMapMatrix->isWaypoint2Draw( wp.priority  ) )
        {
          continue;
        }

      QPoint sitePos( _globalMapMatrix->map( wp.projPoint ) );

      if( ! snapRect.contains(sitePos) )
        {
          // @AP: Point lays outside of snap rectangle, we ignore it
          continue;
        }

      dX = abs(sitePos.x() - current.x());
      dY = abs(sitePos.y() - current.y());

      // Abstand entspricht der Icon-Groesse
      if( dX < delta && dY < delta )
        {
          if( found && ((dX + dY) >= lastDist) )
            {
              continue; // the point we found earlier was closer
            }

          found = true;
          w = &wp;

          lastDist = dX+dY;

          if( lastDist < (delta / 3) )
            {
              // if we're very near, stop searching the list
              break;
            }
        }
    }

  if( found )
    {
      emit showPoi( w );
      return;
    }

  // @ee maybe we can show airspace info
  p_displayAirspaceInfo( current );
}

void Map::wheelEvent(QWheelEvent *event)
{
  int numDegrees = event->delta() / 8;
  int numSteps = numDegrees / 15;

  if( abs(numSteps) > 10 || numSteps == 0 )
    {
      event->ignore();
      return;
    }

  if( numSteps > 0 )
    {
      slotZoomIn();
    }
  else
    {
      slotZoomOut();
    }

  event->accept();
}

void Map::mousePressEvent(QMouseEvent* event)
{
  // qDebug() << "Map::mousePressEvent(): Pos=" << event->pos();

  if( mutex() )
    {
      // qDebug("Map::mousePressEvent(): mutex is locked, returning");
      return;
    }

  // Start a timer to recognize a long press for the airspace status display.
  m_showASSTimer->start(750);

  switch (event->button())
    {
      case Qt::RightButton: // press and hold generates mouse RightButton
        // qDebug("MP-RightButton");
        break;

      case Qt::LeftButton: // press generates mouse LeftButton immediately
        // qDebug("MP-LeftButton");
        m_beginMapMove = event->pos();
        event->accept();
        break;

      case Qt::MidButton:
        // qDebug("MP-MidButton");
        break;

      default:
        break;
    }
}

void Map::mouseMoveEvent( QMouseEvent* event )
{
  // qDebug() << "Map::mouseMoveEvent(): Pos=" << event->pos();

  if( m_mouseMoveIsActive == false )
    {
      QPoint dist = m_beginMapMove - event->pos();

      if( dist.manhattanLength() > 25 )
        {
          // On the N810/N900 we get a lot of move events also on a single
          // mouse press and release action. If we do not filter that,
          // all other mouse actions bound on release mouse are blocked.
          m_mouseMoveIsActive = true;
          QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
          m_showASSTimer->stop();
          event->accept();
        }
    }
}

void Map::mouseReleaseEvent( QMouseEvent* event )
{
  m_showASSTimer->stop();

  if( m_ignoreMouseRelease )
    {
      m_ignoreMouseRelease = false;
      event->accept();
      return;
    }

  if( m_mouseMoveIsActive )
    {
      // User has released the mouse button after a mouse move.
      m_mouseMoveIsActive = false;

      QApplication::restoreOverrideCursor();
      QPoint dist = m_beginMapMove - event->pos();

      if( GpsNmea::gps->getGpsStatus() == GpsNmea::validFix )
        {
          // We check, if the user has moved in east or west direction on the
          // map. If that is true the visibility of the map display boxes can
          // be changed.
          if( abs(dist.y()) < abs(dist.x() / 4) && abs(dist.x()) >= 75 )
            {
              if( dist.x() < 0 )
                {
                  // move to right
                  emit showInfoBoxes( true );
                }
              else
                {
                  // move to left
                  emit showInfoBoxes( false );
                }
            }
        }
      else
        {
          // We move the map to the new position if we are in manual mode
          // or the GPS is not connected
          QPoint center( width()/2, height()/2 );

          center += dist;

          // Center Map to new center point
          _globalMapMatrix->centerToPoint( center );
          QPoint newPos = _globalMapMatrix->mapToWgs( center );

          // Coordinates are toggled, don't know why
          m_curMANPos = QPoint(newPos.y(), newPos.x());
          emit newPosition( m_curMANPos );
          scheduleRedraw();
        }

      event->accept();
      return;
    }

  if( mutex() )
    {
      //qDebug("Map::mouseReleaseEvent(): mutex is locked, returning");
      return;
    }

  switch( event->button() )
    {
      case Qt::RightButton: // press and hold generates mouse RightButton
        // qDebug("MR-RightButton");
        break;

      case Qt::LeftButton: // press generates mouse LeftButton immediately
        // qDebug("MR-LeftButton");

        if( p_zoomButtonPress( event->pos() ) )
          {
            break;
          }

        p_displayDetailedItemInfo( event->pos() );
        break;

      case Qt::MidButton:

        // qDebug("MR-MidButton");
        break;

      default:
        break;
    }

  event->accept();
}

void Map::paintEvent(QPaintEvent* event)
{
//QDateTime dt = QDateTime::currentDateTime();
//QString dtStr = dt.toString(Qt::ISODate);
//
//   qDebug("%s: Map::paintEvent(): RecW=%d, RecH=%d, RecLeft(X)=%d, RecTop(Y)=%d",
//          dtStr.toAscii().data(),
//          event->rect().width(), event->rect().height(),
//          event->rect().left(), event->rect().top() );

  if( mutex() )
    {
      qDebug("Map::paintEvent(): mutex is locked");
    }

  // We copy always the content from the m_pixPaintBuffer to the paint device.
  QPainter p(this);

  p.drawPixmap( event->rect().left(), event->rect().top(), m_pixPaintBuffer,
                0, 0, event->rect().width(), event->rect().height() );

  // qDebug("Map.paintEvent(): return");
}

void Map::slotNewWind( Vector& wind )
{
  if( wind.isValid() && wind.getSpeed().getMps() > 0.0 )
    {
      int angle = wind.getAngleDeg();
      angle = (((angle + 5) / 10) * 10) % 360;  // Quantizes modulo 10

      QString resource;
      resource.sprintf("windarrows/wind-arrow-80px-%03d.png", angle );
      m_windArrow = GeneralConfig::instance()->loadPixmap( resource, true );
    }
  else
    {
      // Reset wind arrow pixmap, if vector is invalid.
      m_windArrow = QPixmap();
    }

  slotRedraw( Map::wind );
}

void Map::p_drawAirspaces( bool reset )
{
  QPainter cuAeroMapP;

  cuAeroMapP.begin(&m_pixAeroMap);

  // Get airspace filter data
  QHash<QString, QMultiHash<QString, QString> > asfs =
      AirspaceFilters::getAirspaceFilters();

  // AS type mapper from stored as-type to shown as-type
  QHash<QString, QString> astMapper;
  astMapper.insert( "Restricted", "AR" );
  astMapper.insert( "Danger", "AD" );
  astMapper.insert( "Prohibited", "AP" );
  astMapper.insert( "AS-E low", "AS-El" );
  astMapper.insert( "AS-E high", "AS-Eh" );

  QTime t;
  t.start();

  if( reset )
    {
      qDeleteAll(m_airspaceRegionList);
      m_airspaceRegionList.clear();
    }

  GeneralConfig* settings     = GeneralConfig::instance();
  bool fillAirspace           = settings->getAirspaceFillingEnabled();
  bool drawingBorder          = settings->getAirspaceDrawBorderEnabled();
  AirspaceWarningDistance awd = settings->getAirspaceWarningDistances();
  AltitudeCollection alt      = calculator->getAltitudeCollection();
  QPoint pos                  = calculator->getlastPosition();

  qreal airspaceOpacity;

  if( fillAirspace == true )
    {
      airspaceOpacity = 0.0; // full transparency in fill mode

      if( m_airspaceRegionList.size() == 0 )
        {
          // The airspace region list can be cleared by the reloading procedure,
          // if the projection has been changed. So setup a new list in such a case.
          reset = true;
        }
    }
  else
    {
      airspaceOpacity = 100.0; // no transparency
    }

  // The border is stored as FL
  uint asBorder = (uint) rint(settings->getAirspaceDrawingBorder() * 100.0 * Distance::mFromFeet );

  // Two airspace lists have to be processed.
  SortableAirspaceList* asl[2];

  asl[0] = _globalMapContents->getAirspaceList();
  asl[1] = _globalMapContents->getFlarmAlertZoneList();

  for( int i = 0; i < 2; i++ )
    {
      for( int loop = 0; loop < asl[i]->size(); loop++ )
        {
          AirRegion* region = 0;
          SortableAirspaceList* asList = asl[i];
          Airspace* currentAirS = dynamic_cast<Airspace *> (asList->value(loop));

          if( currentAirS == 0 || currentAirS->isDrawable() == false )
            {
              // Not of interest, step away
              continue;
            }

          // Check airspace against airspace filters
          // Key of AS Hash is country. Value is a multi hash where key is
          // AS-Type and value is AS-Name
          QString asCountry = currentAirS->getCountry();

          if( asCountry.isEmpty() == true )
            {
              // Country is not set. In this case we use * as country selector.
              // That is a workaround for openair files, which have no county
              // definitions inside.
              asCountry = "*";
            }

          if( asfs.contains( asCountry ) == true )
            {
              // Country filter is defined
              QMultiHash<QString, QString> countryFilter = asfs[asCountry];

              // get AS-Type as string
              QString asType = currentAirS->getTypeName(currentAirS->getTypeID() );

              // map back displayed airspace type to stored airspace type.
              if( astMapper.contains( asType ) == true )
                {
                  QString asTypeShown = astMapper.value( asType );
                  asType = asTypeShown;
                }

              // Look for Airspace type in country filters data
              if( countryFilter.contains( asType ) == true )
                {
                  QList<QString> asNames = countryFilter.values( asType );

                  // get airspace name
                  QString asName = currentAirS->getName();

                  bool ignore = false;

                  // Look, if airspace should be filtered out.
                  for( int i = 0; i < asNames.size(); i++ )
                    {
                      if( asName.startsWith( asNames.at( i )) == true )
                        {
                          // filter out this airspace
                          qDebug() << "Filter out AS " << asType << ": " << asName;
                          ignore = true;
                        }
                    }

                  if( ignore == true )
                    {
                      continue;
                    }
                }
            }

          if( drawingBorder == true )
            {
              // Ignore airspaces which lays with its lower border to high.
              if( currentAirS->getLowerL() > asBorder )
                {
                  continue;
                }
            }

          if( currentAirS->getTypeID() == BaseMapElement::AirFir )
            {
              // FIRs are always full transparent.
              airspaceOpacity = 0.0;
            }

          if( currentAirS->getTypeID() == BaseMapElement::AirFlarm )
            {
              // Filter out invalid and inactive Flarm alert zones
              if( currentAirS->getFlarmAlertZone().isValid() == false ||
                  currentAirS->getFlarmAlertZone().isActive() == false )
                {
                  continue;
                }
            }

          if( reset == true || currentAirS->getAirRegion() == 0 )
            {
              // We have to create a new region for that airspace and put
              // it in the airspace region list.
              region = new AirRegion( currentAirS->createRegion(), currentAirS );
              m_airspaceRegionList.append( region );
            }
          else
            {
              // try to reuse an existing region
              region = currentAirS->getAirRegion();
            }

          if( region && currentAirS->getTypeID() != BaseMapElement::AirFir )
            {
              // determine lateral conflict
              Airspace::ConflictType lConflict = region->conflicts( pos, awd );

              // determine vertical conflict
              Airspace::ConflictType vConflict = currentAirS->conflicts( alt, awd );

              if( fillAirspace == true )
                {
                  // load user settings for opacity
                  if( lConflict == Airspace::inside )
                    {
                      // We are inside from the lateral position out,
                      // vertical conflict has priority.
                      airspaceOpacity = (qreal) settings->getAirspaceFillingVertical( vConflict );
                    }
                  else
                    {
                      // We are not inside from the lateral position out,
                      // lateral conflict has priority.
                      airspaceOpacity = (qreal) settings->getAirspaceFillingLateral( lConflict );
                    }
                }
            }

          currentAirS->drawRegion( &cuAeroMapP, airspaceOpacity );
        }
    }

  cuAeroMapP.end();
  // qDebug("Airspace, drawTime=%d ms", t.elapsed());
}

void Map::p_drawGrid()
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
  int lineWidth = 1 * Layout::getIntScaledDensity();

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
      break;
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
            gridP.setPen(QPen(Qt::black, lineWidth, Qt::DashLine));
          else
            gridP.setPen(QPen(Qt::black, lineWidth, Qt::DotLine));

          gridP.drawPolyline(_globalMapMatrix->map(pointArraySmall));
        }
      // Draw the main lines
      gridP.setPen(QPen(Qt::black, lineWidth));
      gridP.drawPolyline(_globalMapMatrix->map(pointArray));
    }

  // Now the longitudes:
  for(int loop = lon1; loop <= lon2; loop += gridStep)
    {
      cP = _globalMapMatrix->wgsToMap(lat1 * 600000, (loop * 600000));
      cP2 = _globalMapMatrix->wgsToMap(lat2 * 600000, (loop * 600000));

      // Draw the main longitudes:
      gridP.setPen(QPen(Qt::black, lineWidth));
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
            gridP.setPen(QPen(Qt::black, lineWidth, Qt::DashLine));
          else
            gridP.setPen(QPen(Qt::black, lineWidth, Qt::DotLine));

          gridP.drawLine(_globalMapMatrix->map(cP), _globalMapMatrix->map(cP2));
        }
    }

  gridP.end();
}

void Map::p_drawPlannedTask( QPainter *taskP, QList<TaskPoint*> &drawnTp )
{
  FlightTask* task = (FlightTask*) _globalMapContents->getCurrentTask();

  if( task == static_cast<FlightTask *> (0) )
    {
      // no active task available
      return;
    }

  // Draw active task
  task->drawTask( taskP, drawnTp );
}

/**
 * Draw a trail displaying the flight path if feature is turned on
 */
void Map::p_drawTrail()
{
  static uint counter = 0;

  // QTime t; t.start();

  int pointCnt = m_trailPoints.size();

  if( GeneralConfig::instance()->getMapDrawTrail() == false || pointCnt < 15 )
    {
      return;
    }

  int step = 2;

  double scale = _globalMapMatrix->getScale( MapMatrix::CurrentScale );

  // Set step width according to map scale. The values are nearly seconds.
  if( scale < 12 )
    {
      step = 2;
    }
  else if( scale < 15 )
    {
      step = 3;
    }
  else if( scale < 20 )
    {
      step = 4;
    }
  else if( scale < 30 )
    {
      step = 5;
    }
  else if( scale < 40 )
    {
      step = 8;
    }
  else if( scale < 100 )
    {
      step = 10;
    }
  else if( scale < 200 )
    {
      step = 20;
    }
  else
    {
      // draw nothing up to this scale
      return;
    }

  // view port rectangle
  QRect rect( QPoint(0, 0), size() );

  // define start conditions
  int startIdx = counter % step;
  QPoint startPos = m_trailPoints.at( startIdx );

  int loop = startIdx + step;

  counter++;

  if( startIdx == 0 || m_tpp.isEmpty() )
    {
      // Start a recalculation of the trail if start index is zero.
      if( ! m_tpp.isEmpty() )
        {
          // reset painter path
          m_tpp = QPainterPath();
        }

      m_tpp.moveTo( startPos );

      while( loop < pointCnt )
        {
          const QPoint& pos = m_trailPoints.at(loop);

          if( rect.contains( startPos ) || rect.contains( pos ) )
            {
              m_tpp.lineTo( pos );
            }

          startPos = pos;
          loop += step;
        }
    }

  qreal penWidth = GeneralConfig::instance()->getMapTrailLineWidth();
  QColor color = GeneralConfig::instance()->getMapTrailColor();

  if( m_tpp.isEmpty() == false )
    {
      // draw the trail
      QPainter p;
      p.begin( &m_pixInformationMap );
      QPen pen( color, penWidth );
      p.setPen(pen);
      p.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
      p.drawPath(m_tpp);
      p.end();
    }

  // qDebug("Trail, drawTime=%d ms", t.elapsed());
}

void Map::p_calculateTrailPoints()
{
  // clears the trail point list because map projection has been changed.
  m_trailPoints.clear();

  // reset trail point painter path
  if( ! m_tpp.isEmpty() )
    {
      m_tpp = QPainterPath();
    }

  if( GeneralConfig::instance()->getMapDrawTrail() == false )
    {
      return;
    }

  QDateTime minTime = calculator->getLastSampleTime().addSecs(- TrailListLength );

  int loop = 0;
  int sampleCnt = calculator->samplelist.count();

  while( loop < sampleCnt &&
          loop < TrailListLength &&
          calculator->samplelist.at(loop).time >= minTime )
    {
      // Map WGS84 position to map projection
      const QPoint& pos = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(calculator->samplelist.at(loop).position));

      // newest positions at first, oldest at last
      m_trailPoints.append( pos );
      loop++;
    }
}

void Map::setDrawing(bool isEnable)
{
  if (m_isEnable != isEnable)
    {
      if (isEnable)
        scheduleRedraw(baseLayer);
      m_isEnable = isEnable;
    }
}

void Map::resizeEvent(QResizeEvent* event)
{
  Q_UNUSED( event )

  // set resize flag
  m_isResizeEvent = true;
  slotDraw();
}

void Map::p_redrawMap(mapLayer fromLayer, bool queueRequest)
{
  static bool first = true; // mark first calling of method

  static QSize lastSize; // Save the last used window size

  // First call after creation of object can pass
  if( ! isVisible() && ! first )
    {
      // schedule requested layer
      m_scheduledFromLayer = qMin(m_scheduledFromLayer, fromLayer);

      // AP: ignore draw request when the window is hidden or not
      // visible to give the user all power of the device for interactions
      return;
    }

  if( ! m_isEnable )
    {
      // @AP: we reset also the redraw request flag.
      m_isRedrawEvent = false;
      return;
    }

  if( mutex() && queueRequest )
    {
      // @AP: we queue only the redraw request, timer will be started
      // again by p_redrawMap() method.
      m_isRedrawEvent = true;

      // schedule requested layer
      m_scheduledFromLayer = qMin(m_scheduledFromLayer, fromLayer);

      // qDebug("Map::p_redrawMap(): mutex is locked, returning");
      return;
    }

  // set mutex to block recursive entries and unwanted data modifications.
  setMutex(true);

  // Check, if a resize event is queued. In this case it must be
  // considered to create the right matrix size.
  if( m_isResizeEvent )
    {
      m_isResizeEvent = false;
      // reset layer to base layer, that all will be redrawn
      fromLayer = baseLayer;
    }

  //set the map rotation
  m_curMapRot = m_mapRot;

  // draw the layers we need to refresh
  if (fromLayer < aeroLayer)
    {
      // Draw the base layer, which contains the landscape elements.
      // First initialize map matrix
      _globalMapMatrix->slotSetScale(m_zoomFactor);

      if(calculator->isManualInFlight() || !m_ShowGlider)
        {
          _globalMapMatrix->slotCenterTo(m_curMANPos.x(), m_curMANPos.y());
        }
      else
        {
          _globalMapMatrix->slotCenterTo(m_curGPSPos.x(), m_curGPSPos.y());
        }

      m_zoomFactor = _globalMapMatrix->getScale(); //read back from matrix!

      // qDebug("MapMatrixSize: w=%d, h=%d", size().width(), size().height());

      _globalMapMatrix->createMatrix(this->size());

      if( lastSize.isValid() == false || lastSize != size() )
        {
          // save the last used size
          lastSize = size();
          // reinitialize the base pixmap
          m_pixBaseMap = QPixmap( size() );
        }

      //actually start doing our drawing
      p_drawBaseLayer();
    }

  if (fromLayer < navigationLayer)
    {
      p_drawAeroLayer(fromLayer < aeroLayer);
    }

  if (fromLayer < informationLayer)
    {
      p_drawNavigationLayer();
    }

  if (fromLayer < topLayer)
    {
      p_drawInformationLayer();
    }

  // copy the new map content into the paint buffer
  m_pixPaintBuffer = m_pixInformationMap;

  // unlock mutex
  setMutex(false);

#ifdef QSCROLLER1
  // Overshoot animation was stopped, reset it
  _globalMapView->resetScrolling();
#endif

  //QDateTime dt = QDateTime::currentDateTime();
  //QString dtStr = dt.toString(Qt::ISODate);
  // qDebug("%s: Map::p_redrawMap: repaint(%dx%d) is called",
  //       dtStr.toAscii().data(), this->rect().width(),this->rect().height() );

  if( first )
    {
      // suppress the first repaint call otherwise splash screen will disappear
      first = false;

      // Inform MainWindow about first drawing
      emit firstDrawingFinished();
    }
  else
    {
      repaint( m_pixPaintBuffer.rect() );
    }

  // @AP: check, if a pending redraw request is active. In this case
  // the scheduler timers will be restarted to handle it.
  if( m_isRedrawEvent )
    {
      qDebug("Map::p_redrawMap(): queued redraw event found, schedule Redraw");
      m_isRedrawEvent = false;
      scheduleRedraw( m_scheduledFromLayer );
    }

  // @AP: check, if a pending resize event exists. In this case the
  // scheduler timers will be restarted to handle it.
  if( m_isResizeEvent )
    {
      qDebug("Map::p_redrawMap(): queued resize event found, schedule Redraw");
      scheduleRedraw();
    }

  return;
}

/**
 * Draws the base layer of the map.
 * The base layer consists of the basic map, containing everything up
 * to the features of the landscape.
 * It is drawn on an empty pixmap.
 */
void Map::p_drawBaseLayer()
{
  if( !m_isEnable )
    {
      return;
    }

  m_drawnCityList.clear();
  QList<BaseMapElement *> drawnElements;

  // Erase the base layer and fill it with the subterrain color. If there
  // are no terrain map data available, this is the default map ground color.
  m_pixBaseMap.fill( GeneralConfig::instance()->getTerrainColor(0) );

  // make sure we have all the map files we need loaded
  _globalMapContents->proofeSection();

  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  // create a pixmap painter
  QPainter baseMapP;

  baseMapP.begin(&m_pixBaseMap);

  // first, draw the iso lines
  _globalMapContents->drawIsoList(&baseMapP);

  // next, draw the topographical elements and the cities
  _globalMapContents->drawList(&baseMapP, MapContents::TopoList, drawnElements);
  _globalMapContents->drawList(&baseMapP, MapContents::CityList, m_drawnCityList);
  _globalMapContents->drawList(&baseMapP, MapContents::LakeList, drawnElements);

  // draw the roads, the railroads, the hydro
  if( cs <= 130.0 )
    {
      _globalMapContents->drawList(&baseMapP, MapContents::RoadList, drawnElements);
      _globalMapContents->drawList(&baseMapP, MapContents::RailList, drawnElements);
      _globalMapContents->drawList(&baseMapP, MapContents::HydroList, drawnElements);
   }

  // draw the landmarks and the obstacles
  if( cs < 130.0 )
    {
      _globalMapContents->drawList(&baseMapP, MapContents::LandmarkList, drawnElements);
      _globalMapContents->drawList(&baseMapP, MapContents::ObstacleList, drawnElements);
      _globalMapContents->drawList(&baseMapP, MapContents::ReportList, drawnElements);
    }

  // draw the motorways
  _globalMapContents->drawList(&baseMapP, MapContents::MotorwayList, drawnElements);

  // end the painter
  baseMapP.end();

  // draw the city labels if scale is not to high
  if( cs <= 60.0 )
    {
      p_drawCityLabels( m_pixBaseMap );
    }

  // calculate the tail points because projection has been changed
  p_calculateTrailPoints();
}

/**
 * Draws the aero layer of the map.
 * The aero layer consists of the airspace structures and the navigation
 * grid.
 * It is drawn on top of the base layer
 */
void Map::p_drawAeroLayer(bool reset)
{
  // first, copy the base map to the aero map
  m_pixAeroMap = m_pixBaseMap;

  p_drawAirspaces(reset);
  p_drawGrid();
}

/**
 * Draws the navigation layer of the map.
 * The navigation layer consists of the navaids, airfields, glidersites,
 * outlanding sites and waypoints.
 * It is drawn on top of the aero layer.
 */
void Map::p_drawNavigationLayer()
{
  m_pixNavigationMap = m_pixAeroMap;

  double cs = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

  if( !m_isEnable || cs > 1024.0 )
    {
      return;
    }

  // Collect all drawn objects as reference for later label drawing:
  QList<RadioPoint*> drawnRp;
  QList<Airfield*> drawnAf;
  QList<Waypoint*> drawnWp;
  QList<TaskPoint*> drawnTp;
  QList<BaseMapElement*> drawnSp;
  QPainter navP;

  navP.begin(&m_pixNavigationMap);

  if( _globalMapMatrix->isBorder2() )
    {
      _globalMapContents->drawList(&navP, drawnRp);
      _globalMapContents->drawList(&navP, MapContents::OutLandingList, drawnAf);
      _globalMapContents->drawList(&navP, MapContents::GliderfieldList, drawnAf);
      _globalMapContents->drawList(&navP, MapContents::AirfieldList, drawnAf);
    }

  if( _globalMapMatrix->isSwitchScale() )
    {
      _globalMapContents->drawList(&navP, MapContents::HotspotList, drawnSp );
    }

  p_drawWaypoints(&navP, drawnWp);
  p_drawPlannedTask(&navP, drawnTp);

  // Now the labels of the drawn objects will be drawn, if activated via options.
  // Put all drawn labels into a set to avoid multiple drawing of them.
  QSet<QString> labelSet;

  // determine icon size
  const bool useSmallIcons = _globalMapConfig->useSmallIcons();

  int iconSize = 32 * Layout::getIntScaledDensity();

  if( useSmallIcons )
    {
      iconSize = 16 * Layout::getIntScaledDensity();
    }

  // qDebug("Af=%d, WP=%d", drawnAf.size(), drawnWp.size() );

  // 1. draw all navaids, ... collected labels
  for( int i = 0; i < drawnRp.size(); i++ )
    {
      QString corrString = WGSPoint::coordinateString( drawnRp[i]->getWGSPosition() );

      if( labelSet.contains( corrString ) )
        {
          // A label with the same coordinates was already drawn.
          // We do ignore the repeated drawing.
          continue;
        }

      // store label to be drawn
      labelSet.insert( corrString );

      p_drawLabel( &navP,
                   iconSize / 2 + 3,
                   drawnRp[i]->getWPName(),
                   drawnRp[i]->getMapPosition(),
                   drawnRp[i]->getWGSPosition(),
                   true );
    }

  // 2. draw all airfield, ... collected labels
  for( int i = 0; i < drawnAf.size(); i++ )
    {
      QString corrString = WGSPoint::coordinateString( drawnAf[i]->getWGSPosition() );

      if( labelSet.contains( corrString ) )
        {
          // A label with the same coordinates was already drawn.
          // We do ignore the repeated drawing.
          continue;
        }

      // store label to be drawn
      labelSet.insert( corrString );

      p_drawLabel( &navP,
                   iconSize / 2 + 3,
                   drawnAf[i]->getWPName(),
                   drawnAf[i]->getMapPosition(),
                   drawnAf[i]->getWGSPosition(),
                   true );
    }

  // 3. draw all collected waypoint point labels
  for( int i = 0; i < drawnWp.size(); i++ )
    {
      QString corrString = WGSPoint::coordinateString( drawnWp[i]->wgsPoint );

      if( labelSet.contains( corrString ) )
        {
          // A label with the same coordinates was already drawn
          // We do ignore the repeated drawing.
          continue;
        }

      // store label to be drawn
      labelSet.insert( corrString );

      bool isLandable = false;

      if( drawnWp[i]->rwyList.size() > 0 )
        {
          isLandable = drawnWp[i]->rwyList.at(0).m_isOpen;
        }

      p_drawLabel( &navP,
                   iconSize / 2 + 3,
                   drawnWp[i]->name,
                   _globalMapMatrix->map( drawnWp[i]->projPoint ),
                   drawnWp[i]->wgsPoint,
                   isLandable );
    }

  // Second draw all collected task point labels
  for( int i = 0; i < drawnTp.size(); i++ )
    {
      QString corrString = WGSPoint::coordinateString( drawnTp[i]->getWGSPosition() );

      if( labelSet.contains( corrString ) )
        {
          // A label with the same coordinates was already drawn
          // We do ignore the repeated drawing.
          continue;
        }

      // store label to be drawn
      labelSet.insert( corrString );

      p_drawLabel( &navP,
                   iconSize / 2 + 3,
                   drawnTp[i]->getWPName(),
                   _globalMapMatrix->map( drawnTp[i]->getPosition() ),
                   drawnTp[i]->getWGSPosition(),
                   false );
    }

  // and finally draw a scale indicator on top of this
  p_drawScale(navP);

  navP.end();
}

/**
 * Draws the information layer of the map.
 * The information layer consists of the wind arrow, the
 * trail and the position indicator.
 * It is drawn on top of the navigation layer.
 */
void Map::p_drawInformationLayer()
{
  m_pixInformationMap = m_pixNavigationMap;

  // Draw a glider symbol on the map if GPS has a fix and no manual mode
  // is selected by the user.
  if( m_ShowGlider && calculator->isManualInFlight() == false)
    {
      p_drawGlider();
      p_drawTrail();

#ifdef FLARM
      p_drawOtherAircraft();
#endif

    }

  // Draw an X at the map, if no GPS fix is available or if user has selected
  // the manual mode.
  if( m_ShowGlider == false || calculator->isManualInFlight() )
    {
      p_drawX();
    }

  // draw the wind arrow, if pixmap was initialized by slot slotNewWind
  if( ! m_windArrow.isNull() )
    {
      QPainter p(&m_pixInformationMap);
      p.drawPixmap( 10, 10, m_windArrow );
    }

  // Draw the zoom buttons at the map
  QPainter p(&m_pixInformationMap);

  QPixmap plus  = _globalMapConfig->getPlusButton();
  QPixmap minus = _globalMapConfig->getMinusButton();

  p.drawPixmap( width()-plus.width()-5, 5, plus );
  p.drawPixmap( width()-minus.width()-5, height()-minus.width()-5, minus);
}

// Performs an unscheduled, immediate redraw of the entire map.
// Any already pending redraws are canceled.
void Map::slotDraw()
{
  m_scheduledFromLayer = baseLayer;
  slotRedrawMap();
}

// This slot is called, if one of the timers redrawTimerShort or
// redrawTimerLong is expired. It starts the redrawing of the map, if
// the mutex is not locked. In the other case the new redraw request
// will be queued only.
void Map::slotRedrawMap()
{
  // qDebug("Map::slotRedrawMap(): LoopLevel=%d", qApp->loopLevel());

  // @AP: Don't allows changes on matrix data during drawing!!!

  if( mutex() )
    {
      // @AP: we ignore such events to get no infinite loops
      qDebug("Map::slotRedrawMap(): is locked by mutex, returning");
      return;
    }

  // stop timers
  m_redrawTimerShort->stop();
  m_redrawTimerLong->stop();

  // save requested layer
  mapLayer drawLayer = m_scheduledFromLayer;

  // reset m_scheduledFromLayer for new requests
  m_scheduledFromLayer = topLayer;

  // do drawing
  p_redrawMap(drawLayer);
}

/** Draws the waypoints of the waypoint catalog on the map */
void Map::p_drawWaypoints(QPainter* painter, QList<Waypoint*> &drawnWp)
{
  extern MapConfig* _globalMapConfig;

  // get map screen size
  int w = size().width();
  int h = size().height();

  QRect testRect(-15, -15, w + 30, h + 30);
  QString labelText;
  Altitude alt;
  Distance dist;

  QList<Waypoint>& wpList = _globalMapContents->getWaypointList();

  // load all configuration items once
  const bool showWpLabels   = GeneralConfig::instance()->getMapShowWaypointLabels();
  const bool useSmallIcons  = _globalMapConfig->useSmallIcons();

  // now step trough the waypoint list
  for( int i=0; i < wpList.count(); i++ )
    {
      Waypoint& wp = wpList[i];

      // qDebug("wp=%s", wp.name.toLatin1().data());

      // isSelected is used for the currently selected target point
      bool isSelected = false;

      if( calculator && calculator->getTargetWp() )
        {
          if( *calculator->getTargetWp() == wp )
            {
              isSelected = true;
            }
        }

      if( ! _globalMapMatrix->isBorder3() && ! isSelected )
        {
          // Don't draw any waypoints over border3 limit
          continue;
        }

      // Check if the waypoint is important enough for the current map scale.
      if( _globalMapMatrix->isWaypoint2Draw( wp.priority ) && isSelected == false )
        {
          // qDebug("Not important wp=%s", wp.name.toLatin1().data());
          continue;
        }

      // Project map point onto screen display.
      QPoint dispP = _globalMapMatrix->map(wp.projPoint);

      // Check, if point lays in the visible screen area
      if( ! testRect.contains(dispP) )
        {
          continue;
        }

    // load and draw the actual icons
    QPixmap pm;

    if( _globalMapConfig->isRotatable(wp.type) )
      {
        int rwyHeading = 0;

        if( wp.rwyList.size() > 0 )
          {
            rwyHeading = wp.rwyList.first().m_heading;
          }

        int heading = rwyHeading/256 >= 18 ? (rwyHeading/256)-18 : rwyHeading/256;

        if( useSmallIcons )
          {
            if( wp.type == BaseMapElement::UltraLight ||
                wp.type == BaseMapElement::Outlanding )
              {
                pm = Airfield::getSmallField( heading );
              }
            else
              {
                pm = Airfield::getSmallAirfield( heading );
              }
          }
        else
          {
            if( wp.type == BaseMapElement::UltraLight ||
                wp.type == BaseMapElement::Outlanding )
              {
                pm = Airfield::getBigField( heading );
              }
            else
              {
                pm = Airfield::getBigAirfield( heading );
              }
          }
      }
    else
      {
        pm = _globalMapConfig->getPixmap( wp.type, false );
      }

    int iconSize     = pm.width();
    int iconSizeHalf = iconSize / 2;
    int xOffset      = iconSizeHalf;
    int yOffset      = iconSizeHalf;
    int cxOffset     = iconSizeHalf;
    int cyOffset     = iconSizeHalf;

    if( wp.type == BaseMapElement::City ||
        wp.type == BaseMapElement::Turnpoint ||
        wp.type == BaseMapElement::Thermal )
      {
        // The lower end of the flag/beacon shall directly point to the
        // point at the map.
        xOffset  = iconSizeHalf;
        yOffset  = iconSize;
        cxOffset = iconSizeHalf;
        cyOffset = iconSizeHalf;
      }

    // Consider reachability during drawing. The circles must be drawn at first.
    enum ReachablePoint::reachable reachable = ReachableList::getReachable(wp.wgsPoint);

    if( reachable == ReachablePoint::yes )
      {
        // draw green circle, when safety
        painter->drawPixmap( dispP.x() - cxOffset, dispP.y() - cyOffset,
                             _globalMapConfig->getGreenCircle(iconSize) );

      }
    else if( reachable == ReachablePoint::belowSafety )
      {
        // draw magenta circle
        painter->drawPixmap( dispP.x() - cxOffset, dispP.y() - cyOffset,
                             _globalMapConfig->getMagentaCircle(iconSize));
      }

    painter->drawPixmap( dispP.x() - xOffset, dispP.y() - yOffset, pm );

    // Add the draw waypoint name to the list, if required by the user.
    if( showWpLabels )
      {
        drawnWp.append( &wpList[i] );
      }
    }
}

/** Draws a label beside the map icon. It is assumed, that the icon is to see
 *  at the screen.
 */
void Map::p_drawLabel( QPainter* painter,
                       const int xShift,       // x offset from the center point
                       const QString& name,    // name of point
                       const QPoint& dispP,    // projected point at the display
                       const WGSPoint& origP,  // WGS84 point
                       const bool isLandable ) // is landable?
{
  // qDebug("LabelName=%s, xShift=%d", name.toLatin1().data(), xShift );
  if( _globalMapMatrix->getScale(MapMatrix::CurrentScale) >= 120.0 )
    {
      return;
    }

  // save the current painter, must be restored before return!!!
  painter->save();

  // Set font size used for text painting a little bit bigger, that
  // the labels are good to see at the map.
  QFont font = painter->font();

  // We use always the same point size independently from the screen size
  font.setPointSize( MapLabelFontPointSize );

  QString labelText = name;
  Altitude alt = ReachableList::getArrivalAltitude( origP );
  QColor reachColor = ReachableList::getReachColor( origP );

  const bool drawLabelInfo = GeneralConfig::instance()->getMapShowLabelsExtraInfo();

  if( drawLabelInfo )
    {
      // draw the name together with the additional information
      if( isLandable )
        {
          Distance dist = ReachableList::getDistance( origP );

          if( dist.isValid() )
            { // check if the distance is valid...
              labelText += "\n" +
              dist.getText( false, uint(0), uint(0) ) +
              " / " +
              alt.getText( false, 0 );
            }
        }
    }

  // Consider reachability during drawing.
  enum ReachablePoint::reachable reachable = ReachableList::getReachable( origP );

  if( isLandable && reachable == ReachablePoint::yes)
    { // land and reachable? then the label will become bold
      font.setBold(true);
    }
  else
    {
      font.setBold(false);
    }

  // Check, if our point has a selection. In this case inverse drawing is used.
  bool isSelected = false;

  if( calculator && calculator->getTargetWp() )
    {
      if( calculator->getTargetWp()->name == name &&
          calculator->getTargetWp()->wgsPoint == origP )
        {
          isSelected = true;
        }
    }

  int pw = 2 * Layout::getIntScaledDensity();;

  if( ! isSelected )
    {
      painter->setPen(QPen(Qt::black, pw, Qt::SolidLine));
      painter->setBrush( Qt::white );
    }
  else
    {
      // draw selected waypoint label inverse
      painter->setPen(QPen(Qt::white, pw, Qt::SolidLine));
      painter->setBrush( Qt::black );
    }

  painter->setFont( font );

  // calculate text bounding box
  QRect dRec( 0, 0, 400, 400 );
  QRect textBox;

  textBox = painter->fontMetrics().boundingRect( dRec, Qt::AlignCenter, labelText );

  // test, if rectangle is right calculated, when newline is in string
  /* qDebug( "FontText=%s, w=%d, h=%d",
          labelText.toLatin1().data(),
          textBox.width(),
          textBox.height() ); */

  // add a little bit more space in the width and in the height
  textBox.setRect( 0, 0, textBox.width() + 8, textBox.height() + 4 );

  int xOffset = xShift;
  int yOffset = 0;

  if( origP.lon() < _globalMapMatrix->getMapCenter(false).y() )
    {
      // The point is on the left side of the map,
      // so draw the text label on the right side.
      yOffset = -textBox.height() / 2;
    }
  else
    {
      // The point is on the right side of the map,
      // so draw the text label on the left side.
      xOffset = -textBox.width() - xShift;
      yOffset = -textBox.height() / 2;
    }

  // move the textbox at the right position on the display
  textBox.setRect( dispP.x() + xOffset,
                   dispP.y() + yOffset,
                   textBox.width(), textBox.height() );

  QPen cPen = painter->pen();
  painter->setPen(QPen(reachColor, pw, Qt::SolidLine));
  painter->drawRect( textBox );
  painter->setPen( cPen );
  painter->drawText( textBox, Qt::AlignCenter, labelText );
  painter->restore();
}

void Map::p_drawCityLabels( QPixmap& pixmap )
{
  if( m_drawnCityList.size() == 0 )
    {
      return;
    }

  QString labelText;

  QPainter painter(&pixmap);
  QFont font = painter.font();

  // Uses on all screens the same font point size
  font.setPointSize( MapCityLabelFontPointSize );

  painter.setFont( font );
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));

  QSet<QString> set;

  for( int i = 0; i < m_drawnCityList.size(); i++ )
    {
      LineElement* city = static_cast<LineElement *> (m_drawnCityList.at(i));

      // Get the screen bounding box of the city
      QRect sbRect = city->getScreenBoundingBox();

      // A city can consist of several segments at a border edge but we want to
      // draw the name only once.
      if( sbRect.isValid() && set.contains( city->getName() ) == false )
        {
          painter.drawText( sbRect.x() + sbRect.width() / 2,
                            sbRect.y() + sbRect.height() / 2 + 3,
                            city->getName() );

          set.insert( city->getName() );
        }
    }
}

/** This function sets the map rotation and redraws the map if the
    map rotation differs too much from the current map rotation. */
void Map::setMapRot(int newRotation)
{
  // qDebug("Map::setMapRot");
  m_mapRot=newRotation;

  if (abs(m_mapRot-m_curMapRot) >= 2)
    {
      scheduleRedraw(baseLayer);
    }
}


/** Read property of int heading. */
const int& Map::getHeading()
{
  return m_heading;
}


/** Write property of int heading. */
void Map::setHeading( const int& _newVal)
{
  m_heading = _newVal;
  setMapRot(calcMapRotation());
}


/** Read property of int bearing. */
const int& Map::getBearing()
{
  return m_bearing;
}


/** Write property of int bearing. */
void Map::setBearing( const int& _newVal)
{
  m_bearing = _newVal;
  setMapRot(calcMapRotation());
}


/** calculates the maprotation based on the map mode, the heading and
    the bearing. In degrees counter clockwise. */
int Map::calcMapRotation()
{
  switch (m_mode)
    {
    case northUp:
      return 0;
    case headUp:
      return -m_heading;
    case trackUp:
      return -m_bearing;
    default:
      return 0;
    }
}


/** calculates the rotation of the glider symbol based on the map mode,
    the heading and the bearing. In degrees counter clockwise. */
int Map::calcGliderRotation()
{
  switch (m_mode)
    {
    case northUp:
      return m_heading;
    case headUp:
      return 0;
    case trackUp:
      return m_heading-m_bearing;
    default:
      return m_heading;
    }
}


/** Read property of mapMode mode. */
Map::mapMode Map::getMode() const
  {
    return m_mode;
  }


/** Write property of mapMode mode. */
void Map::setMode( const mapMode& )
{ //_newVal){
  //  mode = _newVal;
  m_mode = Map::northUp; //other modes are not supported yet
  setMapRot(calcMapRotation());
}


/** Write property of bool ShowGlider. */
void Map::setShowGlider( const bool& _newVal)
{
  if (m_ShowGlider!=_newVal)
    {
      m_ShowGlider = _newVal;
      scheduleRedraw(informationLayer);
    }
}


/** Draws a scale indicator on the pixmap. */
void Map::p_drawScale(QPainter& scaleP)
{
  const int isd = Layout::getIntScaledDensity();

  QPen pen;
  QBrush brush(Qt::white);

  pen.setColor(Qt::black);
  pen.setWidth(3 * isd);
  pen.setCapStyle(Qt::RoundCap);
  scaleP.setPen(pen);

  QFont font = scaleP.font();
  Layout::adaptFont( font, MapScalebarFontHeight );
  scaleP.setFont(font);

  double scale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  Distance barLen;

  /** select appropriate length of bar. This needs to be done for each unit
   * separately, because else you'd get weird, broken bar length.
   * Note: not all possible units are taken into account (meters, feet),
   * because we are not going to use these for horizontal distances (at least
   * not externally.) */
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
      barLen.setKilometers(len * isd);
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
      barLen.setMiles(len * isd);
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
      barLen.setNautMiles(len * isd);
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
      barLen.setKilometers(len * isd);
      break;
    };

  //determine how long the bar should be in pixels
  int drawLength = (int)rint(barLen.getMeters()/scale);

  //...and where to start drawing. Now at the left lower side ...
  scaleP.translate( QPoint( -width() + drawLength + 10*isd, 0) );

  int leftXPos = width()-drawLength-5*isd;

  // Now, draw the bar
  scaleP.drawLine(leftXPos, height()-5*isd, width()-5*isd, height()-5*isd); // main bar
  pen.setWidth(3*isd);
  scaleP.setPen(pen);
  scaleP.drawLine(leftXPos, height()-9*isd,leftXPos,height()-1*isd); // left endbar
  scaleP.drawLine(width()-5*isd, height()-9*isd, width()-5*isd, height()-1*isd); // right endbar

  // get the string to draw
  QString scaleText = barLen.getText(true, 0);

  // get the metrics for this string
  QRect txtRect = scaleP.fontMetrics().boundingRect(scaleText);

  // Adapt the text length to the bar length
  if( (txtRect.width() + 4*isd) > drawLength )
    {
      QFont pf = scaleP.font();

      // The text font size must be narrowed
      while( pf.pointSize() > 6 )
        {
          pf.setPointSize( pf.pointSize() - 1 );
          scaleP.setFont( pf );
          txtRect = scaleP.fontMetrics().boundingRect(scaleText);

          if( (txtRect.width() + 4*isd) > drawLength )
            {
              continue;
            }

          break;
        }
    }

  int leftTPos = width()+int((drawLength-txtRect.width())/2)-drawLength-5*isd;

  // draw white box to draw text in it
  scaleP.setBrush(brush);
  scaleP.setPen(Qt::NoPen);
  scaleP.drawRect( leftTPos, height()-txtRect.height()-8*isd,
                   txtRect.width()+4*isd, txtRect.height() );

  // draw text itself
  scaleP.setPen(pen);
  scaleP.drawText( leftTPos, height()-txtRect.height()-8*isd,
                   txtRect.width()+4*isd, txtRect.height(), Qt::AlignCenter,
                   scaleText );
}

/**
 * This slot is called to set a new position. The map object determines if it
 * is necessary to recenter the map, or if the glider can just be drawn on a
 * different position.
 */
void Map::slotPosition(const QPoint& newPos, const int source)
{
  //qDebug("Map::slot_position x=%d y=%d", newPos.x(), newPos.y() );

  if( !m_isEnable )
    {
      return;
    }

  if( source == Calculator::GPS )
    {
      if( m_curGPSPos != newPos )
        {
          m_curGPSPos = newPos;

          if( !calculator->isManualInFlight() )
            {
              // let cross be at the GPS position so that if we switch to manual mode
              // show up at the same location
              m_curMANPos = m_curGPSPos;

              if( !_globalMapMatrix->isInCenterArea( newPos ) )
                {
                  // qDebug("Map::slot_position:scheduleRedraw()");
                  // this is the slow redraw
                  scheduleRedraw();
                }
              else
                {
                  static QTime lastDisplay = QTime::currentTime();

                  // The display is updated every 1 seconds only.
                  // That will reduce the X-Server load.
                  if( lastDisplay.elapsed() < 750 )
                    {
                      scheduleRedraw( informationLayer );
                    }
                  else
                    {
                      // this is the faster redraw
                      p_redrawMap( informationLayer, false );

                      lastDisplay = QTime::currentTime();
                    }
                }
            }
          else
            {
              // if we are in manual mode, the real center is the cross, not the glider
              p_redrawMap( aeroLayer );
            }
        }
    }
  else
    { // source is CuCalc::MAN
      if( m_curMANPos != newPos )
        {
          m_curMANPos = newPos;

          if( !_globalMapMatrix->isInCenterArea( newPos ) || mutex() )
            {
              // qDebug("Map::slot_position:scheduleRedraw()");
              scheduleRedraw();
            }
          else
            {
              p_redrawMap( aeroLayer );
            }
        }
    }
}

void Map::slotSwitchManualInFlight()
{
  // show or hide the cross; for switchOn == true, positions are identical
  // the cross is drawn in the scale layer

  // if positions are the same, calculator->setPosition(curGPSPos)
  // would not redraw which is required to show the cross
  if(m_curMANPos != m_curGPSPos)
    {
      // if switch off, the map is redrawn which takes some time
      // and in this the the cross is still visible. So the user may think
      // the switch off was not successfully; To hide the cross immediately,
      // call redraw of scale layer
      scheduleRedraw(scale);
      calculator->setPosition(m_curGPSPos);
    }
  else
    {
      scheduleRedraw(scale);
    }
}

#ifdef FLARM

/** Draws the most important aircraft reported by Flarm. */
void Map::p_drawOtherAircraft()
{

#ifndef MAEMO
  const int diameter = 22 * Layout::getIntScaledDensity();
#else
  const int diameter = 30;
#endif

  if( _globalMapMatrix->getScale(MapMatrix::CurrentScale) > 150.0 )
    {
      // scale to large
      return;
    }

  const Flarm::FlarmStatus& status = Flarm::getFlarmStatus();

  if( status.valid == false )
    {
      // No valid Flarm data available.
      return;
    }

  if( blackCircle.isNull() )
    {
      // do create all needed pixmaps once.
      extern MapConfig* _globalMapConfig;

      _globalMapConfig->createCircle( blackCircle, diameter, QColor(Qt::black), 1.0 );
      _globalMapConfig->createCircle( redCircle, diameter, QColor(Qt::red), 1.0 );
      _globalMapConfig->createCircle( blueCircle, diameter, QColor(Qt::blue), 1.0 );
      _globalMapConfig->createCircle( magentaCircle, diameter, QColor(Qt::magenta), 1.0 );
    }

  // Load selected Flarm object. It is empty in case of no selection.
  QString& selectedObject = FlarmDisplay::getSelectedObject();

  bool selectedObjectFound = false;

  if( ! selectedObject.isEmpty() && Flarm::getPflaaHash().contains(selectedObject) )
    {
      selectedObjectFound = true;
    }

  // Check, if Flarm most relevant object is identical to selected object
  if( selectedObjectFound )
    {
      const Flarm::FlarmAcft& flarmAcft = Flarm::getPflaaHash().value( selectedObject );

      if( status.ID == flarmAcft.ID )
        {
          // Draw only selected object because both objects are identical
          p_drawSelectedFlarmObject( flarmAcft );
        }
      else
        {
          // Draw both most relevant object and selected object
          p_drawSelectedFlarmObject( flarmAcft );
          p_drawMostRelevantObject( status );
        }
    }
  else
    {
      // Draw most relevant object.
      p_drawMostRelevantObject( status );
    }
}

/**
 * Draws the most important object reported by Flarm.
 */
void Map::p_drawMostRelevantObject( const Flarm::FlarmStatus& status )
{
  if( status.RelativeBearing.isEmpty() ||
      status.RelativeVertical.isEmpty() ||
      status.RelativeDistance.isEmpty() ||
      status.Alarm == Flarm::No )
    {
      // no valid data available resp. no alarm
      return;
    }
  // compute true bearing to other aircraft
  bool ok1, ok2, ok3;

  int relBearing  = status.RelativeBearing.toInt( &ok1 );
  int relVertical = status.RelativeVertical.toInt( &ok2 );
  int relDistance = status.RelativeDistance.toInt( &ok3 );

  if( ! (ok1 & ok2 & ok3) )
    {
      // conversion error
      return;
    }

  // calculate true heading to the other object
  int th = MapCalc::normalize( static_cast<int> ( GpsNmea::gps->getLastHeading()) + relBearing );

  // calculate coordinates of other object
  QPoint other;
  WGSPoint::calcFlarmPos( relDistance, th, m_curGPSPos, other );

  // get the projected coordinates of the other position
  QPoint projPos = _globalMapMatrix->wgsToMap( other );

  // map them to a coordinate on the pixmap
  QPoint mapPos = _globalMapMatrix->map( projPos );

  int Rx = mapPos.x();
  int Ry = mapPos.y();

  QRect rect( QPoint(0, 0), this->size() );

  // don't continue if position is outside of window's view port
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  // Set font size used for text painting a little bit bigger, that
  // the labels are good to see at the map.
  QFont font = this->font();

  // We use always the same point size independently from the screen size
  font.setPointSize( MapFlarmLabelFontPointSize );

  // Check, which circle we do need
  QPainter painter( &m_pixInformationMap );
  painter.setFont( font );

  // Get the font's height in pixels.
  int diameter = painter.fontMetrics().height();

  if( status.Alarm != Flarm::No )
    {
      // alarm is active
      painter.drawPixmap(  Rx-diameter/2, Ry-diameter/2, redCircle );
    }
  else if( relVertical < 0 )
    {
      painter.drawPixmap(  Rx-diameter/2, Ry-diameter/2, blackCircle );
    }
  else
    {
      painter.drawPixmap(  Rx-diameter/2, Ry-diameter/2, blueCircle );
    }

  // additional info can be drawn here, like horizontal and vertical distance
  QString text = Distance::getText( relDistance, false, -1 ) + "/";

  if( relVertical > 0 )
    {
      // prefix positive value with a plus sign
      text += "+";
    }

  text += Altitude::getText( relVertical, false, -1 );

  QRect textRect = painter.fontMetrics().boundingRect( text );

  int xOffset = 0;
  int yOffset = 0;

  if( th >= 0 && th <= 180 )
    {
      // draw text at the right side of the circle
      xOffset = Rx + diameter / 2 + 5;
      yOffset = Ry - textRect.height() / 2;
    }
  else
    {
      // draw text at the left side of the circle
      xOffset = Rx - diameter / 2 - 5 - textRect.width();
      yOffset = Ry - textRect.height() / 2;
    }

  painter.setPen( QPen( Qt::NoPen ) );
  painter.setBrush( Qt::white );
  textRect.setRect( xOffset, yOffset, textRect.width(), textRect.height() );
  painter.drawRect( textRect );

  painter.setPen( QPen( Qt::black ) );
  painter.drawText( textRect, Qt::AlignCenter, text );
}

/**
 * Draws the user selected Flarm object.
 */
void Map::p_drawSelectedFlarmObject( const Flarm::FlarmAcft& flarmAcft )
{
  QPoint other;
  double distance = 0.0;
  int usedObjectSize;

  bool result = WGSPoint::calcFlarmPos( m_curGPSPos,
                                        flarmAcft.RelativeNorth,
                                        flarmAcft.RelativeEast,
                                        other,
                                        distance );

  if( ! result )
    {
      // Other Flarm position calculation failed
      return;
    }

  // get the projected coordinates of the other position
  QPoint projPos = _globalMapMatrix->wgsToMap( other );

  // map them to a coordinate on the pixmap
  QPoint mapPos = _globalMapMatrix->map( projPos );

  int Rx = mapPos.x();
  int Ry = mapPos.y();

  QRect rect( QPoint(0, 0), this->size() );

  // don't continue if position is outside of window's view port
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  // Set font size used for text painting a little bit bigger, that
  // the labels are good to see at the map.
  QFont font = this->font();

  // We use always the same point size independently from the screen size
  font.setPointSize( MapFlarmLabelFontPointSize );

  // Check, which circle we do need
  QPainter painter( &m_pixInformationMap );
  painter.setFont( font );

  // Get the font's height in pixels.
  const int diameter = painter.fontMetrics().height();

  font.setPointSize( MapFlarmLabelFontPointSize + 4 );
  const int triangle = QFontMetrics(font).height();

  if( flarmAcft.Track == INT_MIN )
    {
      // Stealth mode is active, no additional information are available.
      // We draw only a circle.
      painter.drawPixmap(  Rx-diameter/2, Ry-diameter/2, magentaCircle );
      usedObjectSize = diameter;
    }
  else
    {
      // Additional Information are available. We draw a triangle.
      QPixmap object;

      QPen pen(Qt::magenta);
      pen.setWidth( 3 * Layout::getIntScaledDensity() );
      QColor liftColor = FlarmDisplay::getLiftColor( flarmAcft.ClimbRate );

      MapConfig::createTriangle( object, triangle, liftColor,
                                 flarmAcft.Track, 1.0, Qt::transparent, pen );

      painter.drawPixmap(  Rx-triangle/2, Ry-triangle/2, object );
      usedObjectSize = triangle;
    }

  // additional info can be drawn here, like horizontal arelDistancend vertical distance
  QString text = Distance::getText( distance, false, -1 );

  if( flarmAcft.ClimbRate != INT_MIN )
    {
      text +=  "/";

      if( flarmAcft.ClimbRate > 0 )
        {
          // prefix positive value with a plus sign
          text +=  "+";
        }

      Speed climb(flarmAcft.ClimbRate);

      text += climb.getVerticalText( false, 1);
    }

  QRect textRect = painter.fontMetrics().boundingRect( text );

  int xOffset = 0;
  int yOffset = 0;

  if( flarmAcft.RelativeEast >= 0 )
    {
      // draw text at the right side of the circle
      xOffset = Rx + usedObjectSize / 2 + 5;
      yOffset = Ry - textRect.height() / 2;
    }
  else
    {
      // draw text at the left side of the circle
      xOffset = Rx - usedObjectSize / 2 - 5 - textRect.width();
      yOffset = Ry - textRect.height() / 2;
    }

  painter.setPen( QPen( Qt::NoPen ) );
  painter.setBrush( Qt::white );
  textRect.setRect( xOffset, yOffset, textRect.width(), textRect.height() );
  painter.drawRect( textRect );

  painter.setPen( QPen( Qt::darkMagenta ) );
  painter.drawText( textRect, Qt::AlignCenter, text );
}

#endif

/** Draws the glider symbol on the pixmap */
void Map::p_drawGlider()
{
  // get the projected coordinates of the current position
  QPoint projPos = _globalMapMatrix->wgsToMap(m_curGPSPos);

  // map them to a coordinate on the pixmap
  QPoint mapPos = _globalMapMatrix->map(projPos);

  int Rx = mapPos.x();
  int Ry = mapPos.y();

  QRect rect( QPoint(0, 0), this->size() );

  // don't continue if position is outside of window's view port
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  if( GeneralConfig::instance()->getMapDrawTrail() == true )
    {
      // Add the mapped point at the beginning of the tail point list.
      m_trailPoints.prepend( mapPos );

      if( m_trailPoints.length() > TrailListLength )
        {
          m_trailPoints.removeLast();
        }
    }

  int rot = calcGliderRotation();

  //we only want to rotate in steps of 10 degrees. Finer is not useful.
  rot = ((rot+5)/10) % 36;

  // now, draw the line from the glider symbol to the waypoint
  p_drawDirectionLine(QPoint(Rx,Ry));

  // draws a line from the current position into the movement direction.
  p_drawHeadingLine( QPoint(Rx,Ry) );

  p_drawRelBearingInfo();

  QPainter p(&m_pixInformationMap);

  QPixmap& gl = m_glider[rot];
  p.drawPixmap( Rx - gl.width()/2, Ry - gl.height()/2, gl );
}

/** Draws the X symbol on the pixmap */
void Map::p_drawX()
{
  // get the projected coordinates of the current position
  QPoint projPos=_globalMapMatrix->wgsToMap(m_curMANPos);

  // map them to a coordinate on the pixmap
  QPoint mapPos = _globalMapMatrix->map(projPos);

  int Rx = mapPos.x();
  int Ry = mapPos.y();

  QRect rect( QPoint(0, 0), this->size() );

  // don't continue if position is outside of window's viewport
  if( ! rect.contains( Rx, Ry, false ) )
    {
      return;
    }

  if(!m_ShowGlider)
    {
      // now, draw the line from the X symbol to the waypoint
      p_drawDirectionLine(QPoint(Rx,Ry));
    }

  // @ee draw preloaded pixmap
  QPainter p(&m_pixInformationMap);
  p.drawPixmap(  Rx-m_cross.width() / 2, Ry-m_cross.height() / 2, m_cross );
}

/** Used to zoom into the map. Will schedule a redraw. */
void Map::slotZoomIn()
{
  emit userZoom();

  // @AP: block zoom events, if map is redrawn
  if( mutex() )
    {
      //qDebug("Map::slotZoomIn(): mutex is locked, returning");
      return;
    }

  if( _globalMapMatrix->getScale() <= _globalMapMatrix->getScale(MapMatrix::LowerLimit) )
    {
      // @AP: lower limit reached, ignore event
      return;
    }

  if( m_zoomProgressive < 7 && m_redrawTimerShort->isActive() )
    {
      m_zoomProgressive++;
    }
  else if( m_zoomProgressive == 7 && m_redrawTimerShort->isActive() )
    {
      return;
    }
  else
    {
      m_zoomProgressive=0;
    }

  m_zoomFactor /= m_zoomProgressiveVal[m_zoomProgressive];

  if( m_zoomFactor < GeneralConfig::instance()->getMapLowerLimit() )
    {
      m_zoomFactor = GeneralConfig::instance()->getMapLowerLimit();
    }

  scheduleRedraw();
  QString msg = QString(tr("Zoom scale 1:%1")).arg(m_zoomFactor, 0, 'f', 0);
  _globalMapView->slot_message( msg, 1000 );
}

/** Used . Will schedule a redraw. */
void Map::slotRedraw()
{
  // qDebug("Map::slotRedraw");
  scheduleRedraw();
}

void Map::slotRedraw( Map::mapLayer fromLayer )
{
  scheduleRedraw(fromLayer);
}

/** Used to zoom the map out. Will schedule a redraw. */
void Map::slotZoomOut()
{
  emit userZoom();

  // @AP: block zoom events, if map is redrawn
  if( mutex() )
    {
      //qDebug("Map::slotZoomOut(): mutex is locked, returning");
      return;
    }

  if( _globalMapMatrix->getScale() >= _globalMapMatrix->getScale(MapMatrix::UpperLimit) )
    {
      // @AP: upper limit reached, ignore event
      return;
    }

  if( m_zoomProgressive < 7 && m_redrawTimerShort->isActive() )
    {
      m_zoomProgressive++;
    }
  else if( m_zoomProgressive == 7 && m_redrawTimerShort->isActive() )
    {
      return;
    }
  else
    {
      m_zoomProgressive=0;
    }

  m_zoomFactor *= m_zoomProgressiveVal[m_zoomProgressive];

  if( m_zoomFactor > GeneralConfig::instance()->getMapUpperLimit() )
    {
      m_zoomFactor = GeneralConfig::instance()->getMapUpperLimit();
    }

  scheduleRedraw();
  QString msg = QString(tr("Zoom scale 1:%1")).arg(m_zoomFactor, 0, 'f', 0);
  _globalMapView->slot_message( msg, 2000 );
}

/**
 * This function schedules a redraw of the map. It sets two timers:
 * The first timer is set for a small interval, and reset every time scheduleRedraw
 * is called. This allows for several modifications to the map being used for the
 * redraw at once.
 * The second timer is set for a larger interval, and is not reset. It makes sure
 * the redraw occurs once in awhile, even if events modifying the map keep comming
 * in and would otherwise prevent the map from being redrawn.
 *
 * If either of the two timers expires, the status of redrawScheduled is reset
 * and the map is redrawn to reflect the current position and zoom factor.
 */
void Map::scheduleRedraw(mapLayer fromLayer)
{
  // qDebug("Map::scheduleRedraw(): mapLayer=%d, loopLevel=%d", fromLayer, qApp->loopLevel() );

  if( !m_isEnable )
    {
      m_isRedrawEvent = false;
      return;
    }

  // schedule requested layer
  m_scheduledFromLayer = qMin(m_scheduledFromLayer, fromLayer);

  if( mutex() )
    {
      // Map drawing is running therefore queue redraw request
      // only. The timers will be restarted at the end of the
      // drawing routine, if the _isRedrawEvent flag is set.
      m_isRedrawEvent = true;
      return;
    }

  // start resp. restart short timer to combine several draw requests to one
#ifndef MAEMO
  m_redrawTimerShort->start(500);
#else
  m_redrawTimerShort->start(750);
#endif

  if (!m_redrawTimerLong->isActive() && m_ShowGlider)
    {
      // Long timer shall ensure, that a map drawing is executed on expiration
      // in every case. Will be activated only in GPS mode and not in manually
      // mode.
      m_redrawTimerLong->start(2000);
    }
}


/** sets a new scale */
void Map::slotSetScale(const double& newScale)
{
  // qDebug("Map::slotSetScale");
  if (newScale != m_zoomFactor)
    {
      m_zoomFactor = newScale;
      scheduleRedraw();
    }
}

/**
 * This function draws a "direction line" on the map if a waypoint has been
 * selected. The QPoint is the projected & mapped coordinate of the position symbol
 * on the map, so we don't have to calculate that all over again.
 */
void Map::p_drawDirectionLine(const QPoint& from)
{
  if ( ! GeneralConfig::instance()->getTargetLineDrawState() )
    {
      return;
    }

  if (calculator && calculator->getTargetWp())
    {
      QPoint to  = _globalMapMatrix->map(calculator->getTargetWp()->projPoint);

      if( from == to )
        {
          // no point in drawing - a line without length
          return;
        }

      QColor col = ReachableList::getReachColor(calculator->getTargetWp()->wgsPoint);

      // we do take the task course line width
      qreal penWidth = GeneralConfig::instance()->getTargetLineWidth();

      QPainter lineP;
      lineP.begin(&m_pixInformationMap);
      lineP.setClipping(true);
      lineP.setPen(QPen(col, penWidth, Qt::DashLine));
      lineP.drawLine(from, to);
      lineP.end();
    }
}

/**
 * This function draws a "heading line" beginning from the current position in
 * the moving direction.
 */
void Map::p_drawHeadingLine(const QPoint& from)
{
  if( ! GeneralConfig::instance()->getHeadingLineDrawState() ||
      ! calculator || ! calculator->getTargetWp() )
    {
      return;
    }

  // define a radius length
  int radius = qMax( width(), height() );

  static const double rad = M_PI / 180.;

  // correct angle because the different coordinate systems.
  int heading = (360 - calculator->getLastHeading()) + 90;

  // Note, that the Cartesian coordinate system must be mirrored at the
  // the X-axis to get the painter's coordinate system. That means all
  // angles must be multiplied by -1.
  double angle = -rad * MapCalc::normalize(heading);

  // Calculate the second point by using polar coordinates.
  int toX = static_cast<int> (rint(cos(angle) * radius)) + from.x();
  int toY = static_cast<int> (rint(sin(angle) * radius)) + from.y();

  QPoint to( toX, toY );

  // we do take the task course line width
  qreal penWidth = GeneralConfig::instance()->getHeadingLineWidth();
  QColor color = GeneralConfig::instance()->getHeadingLineColor();

  QPainter lineP;
  lineP.begin(&m_pixInformationMap);
  lineP.setClipping(true);
  lineP.setPen(QPen(color, penWidth, Qt::SolidLine));
  lineP.drawLine(from, to);
  lineP.end();
}

/**
 * Draws a relative bearing indicator in the upper map area, if the flight
 * state is cruising or wave.
 */
void Map::p_drawRelBearingInfo()
{
  if( ! GeneralConfig::instance()->getMapShowRelBearingInfo() ||
      ! calculator || ! calculator->getTargetWp() ||
        calculator->currentFlightMode() != Calculator::cruising )
    {
      return;
    }

  int heading = calculator->getLastHeading();
  int bearing = calculator->getlastBearing();

  int relBearing = bearing - heading;

  if( relBearing < -180 )
    {
      relBearing += 360;
    }
  else if( relBearing > 180 )
    {
      relBearing -= 360;
    }

  if( m_lastRelBearing != relBearing )
    {
      m_lastRelBearing = relBearing;

      QFont font = this->font();

      font.setPointSize( MapBearingIndicatorFontPointSize );
      font.setBold(true);

      QString text = "";

      if( relBearing == 0 )
        {
          //text = QString("0") + QChar(Qt::Key_degree);
          text = QString("A");
        }
      else if( relBearing > 0 )
        {
          text = QString("%1").arg(relBearing) + QChar(Qt::Key_degree) + ">>";
        }
      else if( relBearing < 0 )
        {
          text = QString("<<%1").arg(-relBearing) + QChar(Qt::Key_degree);
        }

      // calculate text bounding box, if necessary
      if( m_relBearingTextBox.isNull() )
        {
          QFontMetrics fm(font);
          m_relBearingTextBox = fm.boundingRect( "<000°>" );
        }

      m_pixRelBearingDisplay = QPixmap( m_relBearingTextBox.width() + 2, m_relBearingTextBox.height() );
      m_pixRelBearingDisplay.fill( Qt::black );

      QPainter painter;
      painter.begin(&m_pixRelBearingDisplay);
      painter.setFont(font);
      painter.setPen(QPen(Qt::white));
      painter.drawText( 0, 0,
                        m_relBearingTextBox.width(), m_relBearingTextBox.height(),
                        Qt::AlignCenter,
                        text );
      painter.end();
    }

  if( m_pixRelBearingDisplay.isNull() )
    {
      return;
    }

  QPainter painter;
  painter.begin(&m_pixInformationMap);
  painter.drawPixmap( width() / 2 - m_pixRelBearingDisplay.width() / 2,
                      0,
                      m_pixRelBearingDisplay );
  painter.end();
}

/**
 * Check if the new position is near to or inside of an airspace. A warning message
 * will be generated and shown as pop up window. Due to resize problems caused
 * by long texts, the message is not more displayed in the status bar.
 */
void Map::checkAirspace(const QPoint& pos)
{
  if ( mutex() )
    {
      return;
    }

  bool warningEnabled = GeneralConfig::instance()->getAirspaceWarningEnabled();
  bool fillingEnabled = GeneralConfig::instance()->getAirspaceFillingEnabled();
  bool needAirspaceRedraw = false;

  // fetch warning suppress time from configuration and compute it as milli seconds
  int warSupMS = GeneralConfig::instance()->getWarningSuppressTime() * 60 * 1000;

  if( warSupMS > 0 )
    {
      // Tidy up suppress maps with stored conflicts, if warning suppression time
      // is set.
      QMutableMapIterator<QString, QTime> it(m_insideAsMapTouchTime);
      QMutableMapIterator<QString, QTime> vt(m_veryNearAsMapTouchTime);
      QMutableMapIterator<QString, QTime> nt(m_nearAsMapTouchTime);

      clearAirspaceMap( it, warSupMS );
      clearAirspaceMap( vt, warSupMS );
      clearAirspaceMap( nt, warSupMS );
    }

  // fetch warning show time and compute it as milli seconds
  int showTime = GeneralConfig::instance()->getWarningDisplayTime() * 1000;

  // maps to collect the new and the old conflicting airspaces.
  QMap<QString, int> newInsideAsMap;
  QMap<QString, int> allInsideAsMap;
  QMap<QString, int> newVeryNearAsMap;
  QMap<QString, int> allVeryNearAsMap;
  QMap<QString, int> newNearAsMap;
  QMap<QString, int> allNearAsMap;

  AltitudeCollection alt = calculator->getAltitudeCollection();
  AirspaceWarningDistance awd = GeneralConfig::instance()->getAirspaceWarningDistances();

  Airspace::ConflictType hConflict=Airspace::none, lastHConflict=Airspace::none;
  Airspace::ConflictType vConflict=Airspace::none, lastVConflict=Airspace::none;
  Airspace::ConflictType conflict= Airspace::none, lastConflict= Airspace::none;

  bool warn = false; // warning flag

  // check if there are overlaps between the region around our current position and airspaces
  for( int loop = 0; loop < m_airspaceRegionList.count(); loop++ )
    {
      Airspace* pSpace = m_airspaceRegionList.at(loop)->m_airspace;

      if( pSpace->getTypeID() == BaseMapElement::AirFir )
        {
          // FIRs are not included in the conflict checks.
          continue;
        }

      if( pSpace->getTypeID() == BaseMapElement::AirFlarm )
        {
          // Filter out invalid and inactive Flarm alert zones
          if( pSpace->getFlarmAlertZone().isValid() == false ||
              pSpace->getFlarmAlertZone().isActive() == false )
            {
              continue;
            }
        }

      lastVConflict = pSpace->lastVConflict();
      lastHConflict = m_airspaceRegionList.at(loop)->currentConflict();
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
      hConflict = m_airspaceRegionList.at(loop)->conflicts(pos, awd);

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

      if( ! GeneralConfig::instance()->getItemDrawingEnabled(pSpace->getTypeID()) )
        {
           // warning for airspace type disabled by user
          continue;
        }

      // process conflicts
      switch (conflict)
        {
        case Airspace::inside:

          // collect all conflicting airspaces
          allInsideAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );

          // Check, if airspace is to suppress
          if( warSupMS > 0 )
            {
              if( m_insideAsMapTouchTime.contains(pSpace->getInfoString()) )
                {
                  // Yes suppress airspace
                  continue;
                }

              // Add airspace to suppression control map
              m_insideAsMapTouchTime.insert( pSpace->getInfoString(), QTime::currentTime() );
            }

          // Check, if airspace is already known as conflict
          if( ! m_insideAsMap.contains( pSpace->getInfoString() ) )
            {
              // insert new airspace text and airspace type into the map
              newInsideAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );
              warn = true;
            }

          continue;

        case Airspace::veryNear:

          // collect all conflicting airspaces
          allVeryNearAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );

          // Check, if airspace is to suppress
          if( warSupMS > 0 )
            {
              if( m_veryNearAsMapTouchTime.contains(pSpace->getInfoString()) )
                {
                  // Yes suppress airspace
                  continue;
                }

              // Add airspace to suppression control map
              m_veryNearAsMapTouchTime.insert( pSpace->getInfoString(), QTime::currentTime() );
            }

          // Check, if airspace is already known as conflict. A warning is setup
          // only, if the previous state was not inside to avoid senseless alarms.
          if( ! m_veryNearAsMap.contains( pSpace->getInfoString() ) &&
              ! m_insideAsMap.contains( pSpace->getInfoString() ) )
            {
              // insert new airspace text and airspace type into the hash
              newVeryNearAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );
              warn = true;
            }

          continue;

        case Airspace::near:

          // collect all conflicting airspaces
          allNearAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );

          // Check, if airspace is to suppress
          if( warSupMS > 0 )
            {
              if( m_nearAsMapTouchTime.contains(pSpace->getInfoString()) )
                {
                  // Yes suppress airspace
                  continue;
                }

              // Add airspace to suppression control map
              m_nearAsMapTouchTime.insert( pSpace->getInfoString(), QTime::currentTime() );
            }

          // Check, if airspace is already known as conflict. A warning is setup
          // only, if the previous state was not inside or very near to avoid
          // senseless alarms.
          if( ! m_nearAsMap.contains( pSpace->getInfoString() ) &&
              ! m_veryNearAsMap.contains( pSpace->getInfoString() ) &&
              ! m_insideAsMap.contains( pSpace->getInfoString() ) )
            {
              // insert new airspace text and airspace type into the hash
              newNearAsMap.insert( pSpace->getInfoString(), pSpace->getTypeID() );
              warn = true;
            }

          continue;

        case Airspace::none:
        default:

          continue;
        }

    } // End of For loop

  // save all conflicting airspaces for the next round
  m_insideAsMap   = allInsideAsMap;
  m_veryNearAsMap = allVeryNearAsMap;
  m_nearAsMap     = allNearAsMap;

  // redraw the airspaces if needed
  if (needAirspaceRedraw && fillingEnabled)
    {
      scheduleRedraw(aeroLayer);
    }

  if ( ! warningEnabled )
    {
      return;
    }

  QString severity = tr("Warning");

  if( ! newInsideAsMap.isEmpty() )
    {
      severity = tr("Alarm");
    }

  // warning text, contains only conflict changes
  QString text = "<html><table border=1 cellpadding=\"2\"><tr><th align=center>" +
                 tr("Airspace") + "&nbsp;" + severity +
                 "</th></tr>";

  QString msg; // status message containing all conflicting airspaces

  // Only the airspace with the highest priority will be displayed and updated.
  // First we do look step by step for new results according to our predefined
  // priority.
  if ( ! newInsideAsMap.isEmpty() )
    {
      // new inside has been found
      msg += tr("Inside") + " ";
      QMapIterator<QString, int> i(allInsideAsMap);
      bool first = true;

      while ( i.hasNext()  )
        {
           i.next();

           if( ! first )
            {
              msg += ", ";
            }
          else
            {
              first = false;
            }

          msg += Airspace::getTypeName( (BaseMapElement::objectType) i.value() );
        }

      QMapIterator<QString, int> j(newInsideAsMap);

      while ( j.hasNext()  )
        {
           j.next();

           text += "<tr><td align=left>"
                + tr("Inside") + " "
                + "</td></tr><tr><td align=left>"
                + j.key()
                + "</td></tr>";
          }
    }
  else if( ! newVeryNearAsMap.isEmpty() )
    {
      // new very near has been found
      msg += tr("Very Near") + " ";

      QMapIterator<QString, int> i(allVeryNearAsMap);
      bool first = true;

      while ( i.hasNext()  )
        {
          i.next();

          if( ! first )
            {
              msg += ", ";
            }
          else
            {
              first = false;
            }

          msg += Airspace::getTypeName( (BaseMapElement::objectType) i.value() );
        }

      QMapIterator<QString, int> j(newVeryNearAsMap);

      while ( j.hasNext()  )
        {
           j.next();

           text += "<tr><td align=left>"
                + tr("Very Near") + " "
                + "</td></tr><tr><td align=left>"
                + j.key()
                + "</td></tr>";
          }
     }
  else if ( ! newNearAsMap.isEmpty() )
    {
      // new near has been found
      msg += tr("Near") + " ";
      QMapIterator<QString, int> i(allNearAsMap);
      bool first = true;

      while ( i.hasNext()  )
        {
          i.next();

          if( ! first )
            {
              msg += ", ";
            }
          else
            {
              first = false;
            }

          msg += Airspace::getTypeName( (BaseMapElement::objectType) i.value() );
       }

      QMapIterator<QString, int> j(newNearAsMap);

      while ( j.hasNext()  )
        {
           j.next();

           text += "<tr><td align=left>"
                + tr("Near") + " "
                + "</td></tr><tr><td align=left>"
                + j.key()
                + "</td></tr>";
          }
    }

  // Pop up a warning window with all data to touched airspace
  if ( warn == true )
    {
      text += "</table></html>";

      if( GeneralConfig::instance()->getPopupAirspaceWarnings() )
        {
          emit alarm( "", true );
          WhatsThat *box = new WhatsThat(this, text, showTime);
          box->show();
          return;
        }
    }
}

void Map::clearAirspaceMap( QMutableMapIterator<QString, QTime>& it,
                            int suppressTime )
{
  while( it.hasNext () )
    {
      it.next();

      if( it.value().elapsed() > suppressTime )
        {
          it.remove();
        }
    }
}

void Map::slotASSTimerExpired()
{
  // User has pressed long the mouse button. We show the airspace status and
  // ignore the mouse release event.
  m_ignoreMouseRelease = true;
  slotShowAirspaceStatus();
}

void Map::slotShowAirspaceStatus()
{
  static QPointer<WhatsThat> box;

  if( ! box.isNull() )
    {
      // A status display is yet active.
      return;
    }

  // fetch warning show time and compute it as milli seconds
  int showTime = GeneralConfig::instance()->getAirspaceDisplayTime() * 1000;

  QString text = "<html><table border=1 cellpadding=\"2\"><tr><th align=center>" +
                 tr("Airspace") + "&nbsp;" + tr("Status") +
                 "</th></tr>";

  QString endTable = "</table></html>";

  if( m_insideAsMap.size() == 0 &&
       m_veryNearAsMap.size() == 0 &&
       m_nearAsMap.size() == 0 )
    {
      text += "<tr><td align=center>" +
              tr("No Airspace violation") + " " +
              "</td></tr>" +
              endTable;

      box = new WhatsThat( this, text, showTime );
      box->show();
      return;
    }

  if( m_insideAsMap.size() )
    {
      text += "<tr><td align=center><b>" +
              tr("Inside") + "</b></td></tr>";

      QMapIterator<QString, int> it(m_insideAsMap);

      while (it.hasNext())
        {
          it.next();
          text += "<tr><td>" + it.key() + "</td></tr>";
        }
    }

  if( m_veryNearAsMap.size() )
    {
      text += "<tr><td align=center><b>" +
              tr("Very Near") + "</b></td></tr>";

      QMapIterator<QString, int> it(m_veryNearAsMap);

      while (it.hasNext())
        {
          it.next();
          text += "<tr><td>" + it.key() + "</td></tr>";
        }
    }

  if( m_nearAsMap.size() )
    {
      text += "<tr><td align=center><b>" +
              tr("Near") + "</b></td></tr>";

      QMapIterator<QString, int> it(m_nearAsMap);

      while (it.hasNext())
        {
          it.next();
          text += "<tr><td>" + it.key() + "</td></tr>";
        }
    }

  box = new WhatsThat( this, text, showTime );
  box->show();
}

#ifdef FLARM

void Map::slotShowFlarmTrafficInfo( QString& info )
{
  static QPointer<WhatsThat> box;

  if( ! box.isNull() )
    {
      // This alarm is obsolete. Close the popup.
      box->close();
    }

  int showTime = GeneralConfig::instance()->getWarningDisplayTime() * 1000;
  box = new WhatsThat( MainWindow::mainWindow(), info, showTime);
  box->show();

  emit notification( "", true );
}

#endif

bool Map::mutex()
{
  return m_mutex;
}

void Map::setMutex(bool m)
{
  m_mutex = m;
  emit isRedrawing(m);
}
