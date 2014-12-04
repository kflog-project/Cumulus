/***********************************************************************
**
**   map.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2008-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class Map
 *
 * \author Heiner Lamprecht, Florian Ehinger, Andre Somers, Axel Pauli
 *
 * \brief This class provides the basic functions for the map display.
 *
 * \date 1999-2014
 *
 * \version $Id$
 *
 */

#ifndef MAP_H
#define MAP_H

#include <QMap>
#include <QMutableMapIterator>
#include <QPoint>
#include <QWidget>
#include <QBitmap>
#include <QTimer>
#include <QPainterPath>
#include <QPixmap>
#include <QString>
#include <QList>
#include <QLabel>
#include <QEvent>
#include <QResizeEvent>
#include <QRect>
#include <QTime>
#include <QWheelEvent>

#include "airspace.h"
#include "airregion.h"
#include "flighttask.h"
#include "speed.h"
#include "vector.h"
#include "waypoint.h"

#ifdef FLARM
#include "flarm.h"
#endif

class Map : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( Map )

public:

  /**
   * List of different modes for the map orientation
   * -headUp: the current heading is up, the glider symbol always points upwards.
   * -northUp: default. North is up.
   * -trackUp: the map is oriented towards the selected waypoint.
   * Currently, only northUp is supported.
   */
  enum mapMode {headUp=0, northUp=1, trackUp=2};

  /**
   * Layers of the map. Used for redrawing the map. Note that not all values are
   * actually in use. The values called xLayer can directly be used to trigger
   * drawing from this layer up.
   */
  enum mapLayer {baseLayer=0,
                 shores,
                 isoLines,
                 cities,
                 roads,
                 rails,
                 landmarks,
                 obstacles,
                 hydro,
                 motorways,
                 lakes,
                 aeroLayer,
                 airspaces,
                 grid,
                 navigationLayer,
		 hotspots,
                 navaids,
                 airfields,
                 outlandings,
                 gliderfields,
                 waypoints,
                 task,
                 scale,
                 informationLayer,
                 trail,
                 wind,
                 position,
                 topLayer
                };

  /**
   * The constructor creates a new Map object and
   * creates the icon used as a cursor in the map.
   * It is private because map is a singleton class.
   */
  Map( QWidget *parent );

  /**
   * Destroys the Map object.
   */
  virtual ~Map();

  /**
    * Lists the possible map orientations.
    */
  void setDrawing(bool isEnable);

  /**
   * Write property of int heading.
   */
  virtual void setHeading( const int& _newVal);

  /**
   * Read property of int heading.
   */
  virtual const int& getHeading();

  /**
   * Write property of int bearing.
   */
  virtual void setBearing( const int& _newVal);

  /**
   * Read property of int bearing.
   */
  virtual const int& getBearing();

  /**
   * Write property of map mode.
   */
  virtual void setMode( const mapMode& _newVal);

  /**
   * Read property of map mode.
   */
  virtual mapMode getMode() const;

  /**
   * Write property of bool ShowGlider.
   */
  virtual void setShowGlider( const bool& _newVal);

  /**
   * This function schedules a redraw of the map. It sets two timers:
   * The first timer is set for a small interval, and reset every time scheduleRedraw
   * is called. This allows for several modifications to the map being used for the
   * redraw at once.
   * The second timer is set for a larger interval, and is not reset. It makes sure
   * the redraw occurs once in a while, even if events modifying the map keep coming
   * in and would otherwise prevent the map from being redrawn.
   *
   * If either of the two timers times out, the status of redrawScheduled is reset
   * and the map is redrawn to reflect the current position and zoom factor.
   *
   * The argument @arg fromLayer indicates the level from which to start redrawing. The
   * default value is baseLayer, in effect redrawing the entire map from ground up.
   * If a redraw has already been scheduled, the map will be redrawn from the lowest
   * level indicated.
   */
  void scheduleRedraw(mapLayer fromLayer = baseLayer);

  /**
    * This function is used to check if there are airspaces in the proximity of the
    * current position. It shows a warning in that case.
    */
  void checkAirspace(const QPoint&);

  /**
   * Returns the instance of the map widget.
   */
  static Map *getInstance()
  {
    return instance;
  };

  /** clear airspace region list */
  void clearAirspaceRegionList()
    {
      qDeleteAll(m_airspaceRegionList);
      m_airspaceRegionList.clear();
      m_airspaceRegionList = QList<AirRegion *>();
    };

public slots:

  /** This slot is called, if a new wind value is available. */
  void slotNewWind( Vector& wind );

  /**
   *  Unscheduled immediate redraw of the map.
   */
  void slotDraw();

  /** Scheduled redraw of the map. */
  void slotRedraw();

  /** Scheduled redraw of the map starting up passed layer. */
  void slotRedraw( Map::mapLayer fromLayer );

  /**
   * This slot is called to set a new position. The map object
   * determines if it is necessary to recenter the map or if
   * the glider can just be drawn on a different position.
   */
  void slotPosition(const QPoint& newPos, const int source);

  /**
   * Used to zoom the map out. Will schedule a redraw.
   */
  void slotZoomOut();

  /**
   * Used to zoom in on the map. Will schedule a redraw.
   */
  void slotZoomIn();

  /**
   * Sets a new scale. Will schedule a redraw.
   */
  void slotSetScale(const double& newScale);

  /** Called to redraw on switch of manual mode */
  void slotSwitchManualInFlight();

  /**
   * Shows the current airspace status to the user.
   */
  void slotShowAirspaceStatus();

#ifdef FLARM

  /**
   * Called to show a Flarm traffic info.
   */
  void slotShowFlarmTrafficInfo( QString& info );

#endif

private slots:

  /** Called by timer expiration. */
  void slotRedrawMap();

  /** Called by timer expiration. */
  void slotASSTimerExpired();

signals:

  /**
   * Is emitted when left button is clicked on the map in the near of a POI.
   */
  void showPoi(Waypoint *);

  /**
   * Is emitted when an alarm has to be signaled to the user.
   */
  void alarm(const QString& text, const bool sound=true);

  /**
   * Is emitted when a notification has to be signaled to the user.
   */
  void notification(const QString& text, const bool sound=true);

  /**
   * Is emitted when the map starts and ends a redrawing sequence
   */
  void isRedrawing(bool);

  /**
   * Is emitted if the first drawing of the map is finished.
   */
  void firstDrawingFinished();

  /**
   * Is emitted, when the map was moved to a new position by using the mouse.
   */
  void newPosition( QPoint& newPosition );

  /**
   * Is emitted, if a zoom button is pressed by the user.
   */
  void userZoom();

  /**
   * Is emitted, to switch on/off the map display boxes, if the user moves
   * in left/right direction on the map.
   */
  void showInfoBoxes( bool show );

protected:
  /**
   * Redefinition of paintEvent.
   */
  virtual void paintEvent(QPaintEvent* event);

  /**
   * Redefinition of resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

  /**
   * Redefinition of mousePressEvent.
   */
  virtual void mousePressEvent(QMouseEvent* event);

  /**
   * Redefinition of mouseMoveEvent.
   */
  virtual void mouseMoveEvent( QMouseEvent* event );

  /**
   * Redefinition of mouseReleaseEvent.
   */
  virtual void mouseReleaseEvent(QMouseEvent* event);

  /**
   * Used for zoom action.
   */
  virtual void wheelEvent(QWheelEvent *event);

private:
  /**
   * set mutex to avoid reentrance
   */
  void setMutex(bool);

  /**
   * get mutex to avoid reentrance
   */
  bool mutex();

  /**
   * (Re)draws the map, starting at the indicated layer.
   * The actual drawing may start on a lower layer if needed. If a
   * redraw is already in progress, a new redraw is scheduled so the
   * redraw can take place on a later time.
   */
  void p_redrawMap(mapLayer fromLayer=baseLayer, bool queueRequest=true);

  /**
   * Draws the base layer of the map.
   * The base layer consists of the basic map, containing everything up
   * to the features of the landscape.
   * It is drawn on an empty canvas.
   */
  void p_drawBaseLayer();

  /**
   * Draws the aero layer of the map.
   * The aero layer consists of the airspace structures and the navigation
   * grid.
   * It is drawn on top of the base layer
   *
   * @arg reset If set to true, the registry of airspaces is reset. This only
   *      needs to be done if a change in which airspaces are drawn can be
   *      expected. Otherwise, it's better to re-use the current list.
   */
  void p_drawAeroLayer(bool reset = true);

  /**
   * Draws the navigation layer of the map.
   * The navigation layer consists of the airfields, outlanding sites and
   * waypoints.
   * It is drawn on top of the aero layer.
   */
  void p_drawNavigationLayer();

  /**
   * Draws the information layer of the map.
   * The information layer consists of the flight task, wind arrow, the
   * trail, the position indicator and the scale.
   * It is drawn on top of the navigation layer.
   */
  void p_drawInformationLayer();

  /**
   * Draws the task which is currently planned
   * @arg drawnTp List of drawn taskpoints, if taskpoint label drawing
   *      option is set.
   */
  void p_drawPlannedTask(QPainter *taskP, QList<TaskPoint*> &drawnTp);

  /**
   * Draws the grid on the map.
   */
  void p_drawGrid();

  /**
   * Draws the airspaces on the map
   * @arg reset If set to true, the registry of airspaces is reset.
   *            This only needs to be done if a change in which
   *            airspaces are drawn can be expected. Otherwise, it's
   *            better to re-use the current list.
   */
  void p_drawAirspaces(bool reset);

  /**
   * Draws the waypoints of the active waypoint catalog to the map.
   * @arg wpPainter Painter for the waypoints
   * @arg drawnWp List of drawn waypoints, if waypoint label drawing
   *      option is set.
   */
  void p_drawWaypoints(QPainter *wpPainter, QList<Waypoint*> &drawnWp);

  /**
   * Draws a trail indicating the flight path taken.
   */
  void p_drawTrail();

  /**
   * Calculates the trails points to be used for trail drawing. This method must
   * be always called after a projection change.
   */
  void p_calculateTrailPoints();

  /**
   * Draws a label with additional information on demand beside a map icon.
   */
  void p_drawLabel( QPainter* painter,          // painter to be used
                    const int xShift,           // x offset from the center point
                    const QString& name,        // name of point
                    const QPoint& dispP,        // projected point at the display
                    const WGSPoint& origP,      // WGS84 point
                    const bool isLandable );    // is landable?

  /**
   * Draws the city labels at the map.
   */
  void p_drawCityLabels( QPixmap& pixmap );

  /**
   * Display Info about Airspace items
   */
  void p_displayAirspaceInfo(const QPoint& current);

  /**
   * Display detailed info about a MapItem
   */
  void p_displayDetailedItemInfo(const QPoint& current);

  /**
   * Check, if a zoom button was pressed.
   * Return true in this case otherwise false.
   */
  bool p_zoomButtonPress(const QPoint& current);

  /**
   * This function sets the map rotation and redraws the map
   * if the new map rotation differs too much from the current one.
   */
  void setMapRot(int newRotation);

  /**
   * calculates the rotation of the glider symbol based on the map mode,
   * the heading and the bearing. In degrees counterclockwise.
   */
  int calcGliderRotation();

  /**
   * calculates the map rotation based on the map mode, the heading and
   * the bearing. In degrees counterclockwise.
   */
  int calcMapRotation();

  /**
   * Draws the glider symbol on the pixmap
   */
  void p_drawGlider();

  /**
   * Draws a scale on the pixmap.
   */
  void p_drawScale(QPainter& scaleP);

  /**
   * Draws the X symbol on the pixmap
   */
  void p_drawX();

  /**
   * This function draws a "direction line" on the map if a waypoint
   * has been selected. The QPoint is the projected & mapped
   * coordinate of the position symbol on the map, so we don't
   * have to calculate that all over again.
   */
  void p_drawDirectionLine(const QPoint& from);

  /**
   * This function draws a "heading line" beginning from the current position in
   * the moving direction.
   */
  void p_drawHeadingLine(const QPoint& from);

  /**
   * Draws a relative bearing indicator in the upper map area, if the flight
   * state is cruising or wave.
   */
  void p_drawRelBearingInfo();

  /**
   * Clears all entries from the map, where the suppress time has expired.
   *
   * \param it Mutable iterator of the map to be cleared
   *
   * \param suppressTime Elapsed suppress time for removing of map element.
   */
  void clearAirspaceMap( QMutableMapIterator<QString, QTime>& it,
                         int suppressTime );

#ifdef FLARM

  /**
   * Draws the Flarm most important reported object and
   * the user selected object.
   */
  void p_drawOtherAircraft();

  /**
   * Draws the most important object reported by Flarm.
   */
  void p_drawMostRelevantObject( const Flarm::FlarmStatus& status );

  /**
   * Draws the user selected Flarm object.
   */
  void p_drawSelectedFlarmObject( const Flarm::FlarmAcft& flarmAcft );

  /** Pixmaps used by Flarm for object drawing */
  QPixmap blackCircle;
  QPixmap redCircle;
  QPixmap blueCircle;
  QPixmap magentaCircle;

#endif

private: //members
  /**
   * Mutex to avoid reentrance
   */
  bool m_mutex;

  /**
   * Map drawing can be switched off during startup
   */
  bool m_isEnable;

  /**
   * Flag is set, if a redraw event has to be queued because map
   * drawing is already running.
   */
  bool m_isRedrawEvent;

  /**
   * queued resize event is available
   */
  bool m_isResizeEvent;

  /**
   * set is mouse move is active.
   */
  bool m_mouseMoveIsActive;

  /**
   * Begin point of map move.
   */
  QPoint m_beginMapMove;

  /**
   * queued size of resize event
   */
  QSize m_resizeEventSize;

  /**
   * These pixmaps are used to store different layers of the currently
   * displayed map. These pixmaps have the same size as the
   * map-widget, but are only used for internal buffering the
   * map. Whenever the widget is about to be drawn, these buffers
   * are used to get the content.
   */

  //the basic layer of the map
  QPixmap m_pixBaseMap;

  //the map, but now including the aeronautical elements
  QPixmap m_pixAeroMap;

  //the map, but now including the navigation elements
  QPixmap m_pixNavigationMap;

  //the map, but now including the informational elements
  QPixmap m_pixInformationMap;

  // this map is used as overall buffer, to make the last painted
  // map available for paint events even the map is redrawn and
  // new data are are requested by the window system.
  QPixmap m_pixPaintBuffer;

  // Pixmap containing relative bearing display.
  QPixmap m_pixRelBearingDisplay;

  //contains a strip with wind arrows in different directions
  QPixmap m_windArrow;

  int m_mapRot;
  int m_curMapRot;

  // currently selected waypoint
  Waypoint m_wp;

  /**
   * Contains the regions of all visible airspaces. The list is needed to
   * find the airspace data when the user selects an airspace in the map.
   */
  QList<AirRegion*> m_airspaceRegionList;

  //contains the layer the next redraw should start from
  mapLayer m_scheduledFromLayer;

  /** Contains the currently proposed zoom factor. The actual factor
      used is stored in the map matrix */
  double m_zoomFactor;

  /** reference to the short interval redraw timer */
  QTimer *m_redrawTimerShort;
  /** reference to the long interval redraw timer */
  QTimer *m_redrawTimerLong;
  /** Determines weather to draw the glider symbol. */
  bool m_ShowGlider;

  unsigned int m_zoomProgressive;
  float m_zoomProgressiveVal[8];

  QRect m_relBearingTextBox;

protected:
  /**
   * Contains the current heading. Used for rotating the map in
   * headUp map mode or rotating the glider symbol in northUp and
   * trackUp modes. In degrees clockwise.
   */
  int m_heading;

  /**
   * Contains the current bearing. Used for rotating the map and
   * glider symbol in trackUp mode. In degrees clockwise
   * (0=due north, 90 east, etc.)
   */
  int m_bearing;

  /**
   * Contains the last calculated relative bearing.
   */
  int m_lastRelBearing;

  /**
   * Contains the map mode.
   * northUp: the top of the map is north
   * headUp: the top of the map is the current flight direction
   * trackUp: the top of the map is the current bearing to the selected waypoint
   */
  mapMode m_mode;

  /** Holds the current position. */
  QPoint m_curGPSPos, m_curMANPos;

  /** these pixmaps are preloaded to improve runtime drawing */
  QPixmap m_cross;
  QPixmap m_glider;

  /** Airspace conflicts */
  QMap<QString, int> m_insideAsMap;   // AS Text and AS type
  QMap<QString, int> m_veryNearAsMap; // AS Text and AS type
  QMap<QString, int> m_nearAsMap;     // AS Text and AS type

  /* Airspace conflicts touch times */
  QMap<QString, QTime> m_insideAsMapTouchTime;   // AS Text and touch time
  QMap<QString, QTime> m_veryNearAsMapTouchTime; // AS Text and touch time
  QMap<QString, QTime> m_nearAsMapTouchTime;     // AS Text and touch time

  /** List of drawn cities. */
  QList<BaseMapElement *> m_drawnCityList;

  /** List of mapped positions for trail drawing */
  QList<QPoint> m_trailPoints;

  /** trail point painter path. */
  QPainterPath m_tpp;

  /** maximum length of trail list */
  const int TrailListLength;

  /** Timer which activates the airspace status display. */
  QTimer* m_showASSTimer;

  /** Flag to ignore mouse release event. */
  bool m_ignoreMouseRelease;

public:

  static Map *instance;
};

#endif
