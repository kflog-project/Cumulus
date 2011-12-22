/***********************************************************************
**
**   map.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2008-2011 by Axel Pauli
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
 * \date 1999-2011
 *
 * \version $Id$
 *
 */

#ifndef MAP_H
#define MAP_H

#include <QMap>
#include <QPoint>
#include <QWidget>
#include <QBitmap>
#include <QTimer>
#include <QPixmap>
#include <QString>
#include <QList>
#include <QLabel>
#include <QEvent>
#include <QResizeEvent>
#include <QRect>
#include <QWheelEvent>

#include "waypoint.h"
#include "airspace.h"
#include "airregion.h"
#include "flighttask.h"

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
  Map(QWidget *parent);

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
      qDeleteAll(airspaceRegionList);
      airspaceRegionList.clear();
      airspaceRegionList = QList<AirRegion *>();
    };

public slots:

  /** This slot is called, if a new wind value is available. */
  void slotNewWind();

  /**
   *  Unscheduled immediate redraw of the map.
   */
  void slotDraw();

  /** Scheduled redraw of the map. */
  void slotRedraw();

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
   * sets a new scale. Will schedule a redraw.
   */
  void slotSetScale(const double& newScale);

  /** called to redraw on switch of manual mode */
  void slotSwitchManualInFlight();

private slots:

  /** Called by timer expiration. */
  void slotRedrawMap();

signals:

  /**
   * Is emitted when left button click on the map
   */
  void waypointSelected(Waypoint *);

  /**
   * Is emitted when an airspace is entered or left behind
   */
  void airspaceWarning(const QString&, const bool sound=true);

  /**
   * Is emitted when the map starts and ends a redrawing sequence
   */
  void isRedrawing(bool);

  /**
   * Is emitted, when the map was moved to a new position by using the mouse.
   */
  void newPosition( QPoint& newPosition );

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
  void __redrawMap(mapLayer fromLayer=baseLayer, bool queueRequest=true);

  /**
   * Draws the base layer of the map.
   * The base layer consists of the basic map, containing everything up
   * to the features of the landscape.
   * It is drawn on an empty canvas.
   */
  void __drawBaseLayer();

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
  void __drawAeroLayer(bool reset = true);

  /**
   * Draws the navigation layer of the map.
   * The navigation layer consists of the airfields, outlanding sites and
   * waypoints.
   * It is drawn on top of the aero layer.
   */
  void __drawNavigationLayer();

  /**
   * Draws the information layer of the map.
   * The information layer consists of the flight task, wind arrow, the
   * trail, the position indicator and the scale.
   * It is drawn on top of the navigation layer.
   */
  void __drawInformationLayer();

  /**
   * Draws the task which is currently planned
   * @arg drawnWp List of drawn waypoints, if taskpoint label drawing
   *      option is set.
   */
  void __drawPlannedTask(QPainter *taskP, QList<Waypoint*> &drawnWp);

  /**
   * Draws the grid on the map.
   */
  void __drawGrid();

  /**
   * Draws the airspaces on the map
   * @arg reset If set to true, the registry of airspaces is reset.
   *            This only needs to be done if a change in which
   *            airspaces are drawn can be expected. Otherwise, it's
   *            better to re-use the current list.
   */
  void __drawAirspaces(bool reset);

  /**
   * Draws the waypoints of the active waypoint catalog to the map.
   * @arg wpPainter Painter for the waypoints
   * @arg drawnWp List of drawn waypoints, if waypoint label drawing
   *      option is set.
   */
  void __drawWaypoints(QPainter *wpPainter, QList<Waypoint*> &drawnWp);

  /**
   * Draws a trail indicating the flight path taken, if that feature
   * is turned on. (CURRENTLY TURNED OFF)
   */
  void __drawTrail();

  /**
   * Draws a label with additional information on demand beside a map icon.
   */
  void __drawLabel( QPainter* painter,          // painter to be used
                    const int xShift,           // x offset from the center point
                    const QString& name,        // name of point
                    const QPoint& dispP,        // projected point at the display
                    const WGSPoint& origP,      // WGS84 point
                    const bool isLandable );    // is landable?

  /**
   * Draws the city labels at the map.
   */
  void _drawCityLabels( QPixmap& pixmap );

  /**
   * Display Info about Airspace items
   */
  void __displayAirspaceInfo(const QPoint& current);

  /**
   * Display detailed info about a MapItem
   */
  void __displayDetailedItemInfo(const QPoint& current);

  /**
   * Check, if a zoom button was pressed.
   * Return true in this case otherwise false.
   */
  bool __zoomButtonPress(const QPoint& current);

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
  void __drawGlider();

  /**
   * Draws a scale on the pixmap.
   */
  void __drawScale(QPainter& scaleP);

  /**
   * Draws the Xsymbol on the pixmap
   */
  void __drawX();

  /**
   * This function draws a "direction line" on the map if a waypoint
   * has been selected. The QPoint is the projected & mapped
   * coordinate of the position symbol on the map, so we don't
   * have to calculate that all over again.
   */
  void __drawDirectionLine(const QPoint& from);

  /**
   * This function draws a "track line" beginning from the current position in
   * the moving direction.
   */
  void __drawTrackLine(const QPoint& from);

  /**
   * Draws a relative bearing indicator in the upper map area, if the flight
   * state is cruising or wave.
   */
  void __drawRelBearingInfo();

#ifdef FLARM

  /**
   * Draws the Flarm most important reported object and
   * the user selected object.
   */
  void __drawOtherAircraft();

  /**
   * Draws the most important object reported by Flarm.
   */
  void __drawMostRelevantObject( const Flarm::FlarmStatus& status );

  /**
   * Draws the user selected Flarm object.
   */
  void __drawSelectedFlarmObject( const Flarm::FlarmAcft& flarmAcft );

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
  bool _mutex;

  /**
   * Map drawing can be switched off during startup
   */
  bool _isEnable;

  /**
   * Flag is set, if a redraw event has to be queued because map
   * drawing is already running.
   */
  bool _isRedrawEvent;

  /**
   * queued resize event is available
   */
  bool _isResizeEvent;

  /**
   * set is mouse move is active.
   */
  bool _mouseMoveIsActive;

  /**
   * Begin point of map move.
   */
  QPoint _beginMapMove;

  /**
   * queued size of resize event
   */
  QSize resizeEventSize;

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
  QPixmap windArrow;

  int mapRot;
  int curMapRot;

  // currently selected waypoint
  Waypoint wp;

  /**
   * Contains the regions of all visible airspaces. The list is needed to
   * find the airspace data when the user selects an airspace in the map.
   */
  QList<AirRegion*> airspaceRegionList;

  //contains the layer the next redraw should start from
  mapLayer m_scheduledFromLayer;

  /** Contains the currently proposed zoom factor. The actual factor
      used is stored in the map matrix */
  double zoomFactor;

  /** reference to the short interval redraw timer */
  QTimer *redrawTimerShort;
  /** reference to the long interval redraw timer */
  QTimer *redrawTimerLong;
  /** Determines weather to draw the glider symbol. */
  bool ShowGlider;

  unsigned int zoomProgressive;
  float zoomProgressiveVal[8];

  QRect relBearingTextBox;

protected:
  /**
   * Contains the current heading. Used for rotating the map in
   * headUp map mode or rotating the glider symbol in northUp and
   * trackUp modes. In degrees clockwise.
   */
  int heading;

  /**
   * Contains the current bearing. Used for rotating the map and
   * glider symbol in trackUp mode. In degrees clockwise
   * (0=due north, 90 east, etc.)
   */
  int bearing;

  /**
   * Contains the last calculated relative bearing.
   */
  int lastRelBearing;

  /**
   * Contains the map mode.
   * northUp: the top of the map is north
   * headUp: the top of the map is the current flight direction
   * trackUp: the top of the map is the current bearing to the selected waypoint
   */
  mapMode mode;

  /** Holds the current position. */
  QPoint curGPSPos, curMANPos;

  /** these pixmaps are preloaded to improve runtime drawing */
  QPixmap _cross;
  QPixmap _glider;

  /** Airspace conflicts */
  QMap<QString, int> _insideAsMap;   // AS Text and AS type
  QMap<QString, int> _veryNearAsMap; // AS Text and AS type
  QMap<QString, int> _nearAsMap;     // AS Text and AS type

  /** last emitted airspace warning strings */
  QString _lastInsideAsInfo;
  QString _lastVeryNearAsInfo;
  QString _lastNearAsInfo;

  /** last emitted airspace type string */
  QString _lastAsType;

  /** save time of last touch of airspace */
  QTime _lastNearTime;
  QTime _lastVeryNearTime;
  QTime _lastInsideTime;

  /** List of drawn cities. */
  QList<BaseMapElement *> m_drawnCityList;

public:

  static Map *instance;
};

#endif
