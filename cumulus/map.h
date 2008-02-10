/***********************************************************************
**
**   map.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**                   2008 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_H
#define MAP_H

#include <QRegion>
#include <QPoint>
#include <QWidget>
#include <QBitmap>
#include <QTimer>
#include <QPixmap>
#include <QString>
#include <Q3PtrList>
#include <QEvent>
#include <QResizeEvent>

#include "waypoint.h"
#include "airspace.h"
#include "airregion.h"
#include "flighttask.h"
#include "waypointcatalog.h"

class Flight;
class WaypointCatalog;

/**
 * This class provides basic functions for displaying the map.
 */

class Map : public QWidget
  {
    Q_OBJECT

  public: //types
    /**
     * List of different modes for the maporientation
     * -headUp: the current heading is up, the glider symbol always points upwards.
     * -northUp: default. North is up.
     * -trackUp: the map is oriented so towards the selected waypoint.
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
                   highways,
                   lakes,
                   aeroLayer,
                   airspaces,
                   grid,
                   navigationLayer,
                   airports,
                   outlandingSites,
                   gliderSites,
                   waypoints,
                   informationLayer,
                   task,
                   trail,
                   wind,
                   position,
                   scale,
                   topLayer
                  };

  public:
    /**
     * The constructor creates a new Map-object and
     * creates the icon used as a cursor in the map.
     * It is private because map is a singleton class.
     */
    Map(QWidget * parent, const char* name);

    /**
     * Destroys the Map-object.
     */
    virtual ~Map();

    /**
      * Lists the possible map-orientations.
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
     * Write property of mapMode mode.
     */
    virtual void setMode( const mapMode& _newVal);

    /**
     * Read property of mapMode mode.
     */
    virtual mapMode getMode() const;

    /**
     * Write property of bool ShowGlider.
     */
    virtual void setShowGlider( const bool& _newVal);

    /**
     * This function scedules a redraw of the map. It sets two timers:
     * The first timer is set for a small interval, and reset every time sceduleRedraw
     * is called. This allows for several modifications to the map being used for the
     * redraw at once.
     * The second timer is set for a larger interval, and is not reset. It makes sure
     * the redraw occurs once in awhile, even if events modifying the map keep comming
     * in and would otherwise prevent the map from being redrawn.
     *
     * If either of the two timers times out, the status of redrawSceduled is reset
     * and the map is redrawn to reflect the current position and zoomfactor.
     *
     * The argument @arg fromLayer indicates the level from which to redraw. The default
     * value is the baseLayer, in effect redrawing the entire map from the ground up.
     * If a redraw has allready been sceduled, the map will be redrawn from the lowest
     * level indicated.
     */
    void sceduleRedraw(mapLayer fromLayer = baseLayer);

    /**
     * Only updates the glider symbol and the bearing line
     */
    void quickDraw();

    /**
      * This function is used to check if there are airspaces in the proximity of the
      * current position. It shows a warning if there are.
      */
    void checkAirspace(const QPoint&);

    /**
     * Undocumented
     */
    void readConfig();

    /**
     * Returns the instance of the map widget.
     */
    static Map *getInstance()
    {
      return instance;
    };

  public slots:
    /** */
    void slotNewWind();

    /**
     *  unsceduled immediate redraw
     */
    void slotDraw();

    /** */
    void slotRedraw();

    /** */
    void slotRedrawMap();

    /** */
    void slotCenterToFlight();

    /** */
    void slotCenterToTask();

    /** */
    void slotCenterToWaypoint(const unsigned int id);

    /**
     * Slot signalled when user selects another waypointcatalog.
     */
    void slotWaypointCatalogChanged(WaypointCatalog* c);

    /**
     * This slot is called to set a new position. The map object
     * determines if it is necessary to recenter the map, or if
     * the glider can just be drawn on a different position.
     */
    void slot_position(const QPoint& newPos, const int source);

    /**
     * Used to zoom the map out. Will scedule a redraw.
     */
    void slotZoomOut();

    /**
     * Used to zoom in on the map. Will scedule a redraw.
     */
    void slotZoomIn();

    /**
     * sets a new scale. Will scedule a redraw.
     */
    void slotSetScale(const double& newScale);

    /** called to redraw on switch of manual mode */
    void slot_switchManualInFlight();

  signals:
    /** */
    void changed(const QSize&);

    /**
     * is emited when left button click on the map
     */
    void waypointSelected(wayPoint *);

    /**
     * is emited when an airspace is entered or left behind
     */
    void airspaceWarning(const QString&, const bool sound=true);

    /**
     * is emitted when the map starts and ends a redrawing sequence
     */
    void isRedrawing(bool);

  protected:
    /**
     * Redefinition of the paintEvent.
     */
    virtual void paintEvent(QPaintEvent* event);

    /**
     * Redefinition of the resizeEvent.
     */
    virtual void resizeEvent(QResizeEvent* event);

    /**
     * Redefinition of the mousePressEvent.
     */
    virtual void mousePressEvent(QMouseEvent* event);

    /**
     * Redefinition of the mouseReleaseEvent.
     */
    virtual void mouseReleaseEvent(QMouseEvent* event);


  public: // Public attributes
    QPoint preSnapPoint;


  private: //methods
    /**
     * set mutex to avoid reentrance
     */
    void setMutex(bool);

    /**
     * Eventfilter to stop events if redrawing the map. Uses the mutex.
     */
    bool eventFilter(QObject *, QEvent *);

    /**
     * get mutex to avoid reentrance
     */
    bool mutex();

    /**
     * (Re)draws the map, starting at the indicated layer.
     * The actual drawing may start on a lower layer if needed. If a
     * redraw is allready in progress, a new redraw is sceduled so the
     * redraw can take place on a later time.
     */
    void __redrawMap(mapLayer fromLayer);

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
     * The information layer consists of the flight task, windarrow, the
     * trail, the position indicator and the scale.
     * It is drawn on top of the navigation layer.
     */
    void __drawInformationLayer();


    /**
     * Draws the Task which is currently planned
     */
    void __drawPlannedTask();

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
     * Puts the waypoints of the active waypoint catalog to the map
     * @arg wpPainter Painter for the waypoints themselves
     */
    void __drawWaypoints(QPainter * wpPainter);

    /**
     * Draws a trail indicating the flightpath taken, if that feature
     * is turned on. (CURRENTLY TURNED OFF)
     */
    void __drawTrail();

    /**
     * Display Info about Airspace items
     */
    void __displayMapInfo(const QPoint& current);

    /**
     * Display detailed Info about one MapItem
     */
    void __displayDetailedMapInfo(const QPoint& current);

    /**
     * This function sets the maprotation and redraws the map
     * if the maprotation differs too much from the current maprotation.
     */
    void setMapRot(int newRotation);

    /**
     * calculates the rotation of the glidersymbol based on the mapmode,
     * the heading and the bearing. In degrees counterclockwise.
     */
    int calcGliderRotation();

    /**
     * calculates the maprotation based on the mapmode, the heading and
     * the bearing. In degrees counterclockwise.
     */
    int calcMapRotation();

    /**
     *   search for a waypoint
     *   First look in task itself
     *   Second look in map contents
     */
    bool __getTaskWaypoint(QPoint current, struct wayPoint *wp,
                           Q3PtrList<wayPoint> &taskPointList);

    /**
     * Draws the glidersymbol on the pixmap
     */
    void __drawGlider();

    /**
     * Draws a scale on the pixmap.
     */
    void __drawScale();

    /**
     * Draws the Xsymbol on the pixmap
     */
    void __drawX();
    /**
     * This function draws a "directionline" on the map if a waypoint
     * has been selected. The QPoint is the projected & mapped
     * coordinate of the positionsymbol on the map, so we don't
     * have to calculate that all over again.
     */
    void __drawDirectionLine(const QPoint& from);


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
     * queued size of resize event
     */
    QSize resizeEventSize;

    /**
     * These pixmaps are used to store different layers the currently
     * displayed map. These pixmaps have the same size as the
     * map-widget, but are only used for internal buffering the
     * map. Whenever the widget is about to be drawn, these buffers
     * are used to get the content.
     */

    //the basic layer of the map
    QPixmap m_pixBaseMap;

    //the map, but now including the aeronautical elements
    QPixmap m_pixAeroMap;

    //the map, but now including the navigational elements
    QPixmap m_pixNavigationMap;

    //the map, but now including the informational elements
    QPixmap m_pixInformationMap;

    // this map is used as overall buffer, to make the last painted
    // map available for paint events even the map is redrawn and
    // new data are are requested by the window system.
    QPixmap m_pixPaintBuffer;

    //contains a strip with windarrows in different directions
    QPixmap windArrow;

    QPoint prePos;
    QPoint prePlanPos;
    QPoint preAnimationPos;
    QPoint preCur1;
    QPoint preCur2;
    int preIndex;

    int mapRot;
    int curMapRot;

    wayPoint wp;  // currently selected waypoint

    /**
     * Contains the regions of all visible airspaces. The list is needed to
     * find the airspace-data when the users selects a airspace in the map.
     */
    Q3PtrList<AirRegion> airspaceRegList;

    //contains the layer the next redraw should start from
    mapLayer m_sceduledFromLayer;

    // 0 keine Planung 1 Planung 2 Planung Aufgabe Abgeschlossen
    //      enum planning {NoPlanning = 0, Planning = 1, TaskFinished = 2};
    int planning;
    // Index des WP welcher verschoben wird bei planning == 3
    int moveWPindex;
    // indicates if a WP was Added in Snapping Mode (Planning)
    bool lastAdd;
    /** Contains the currently proposed zoomfactor. The actual factor
        used is stored in the MapMatrix. */
    double zoomFactor;
    // indicates if the mouse is in a snapping area
    bool isSnapping;

    /** reference to the short interval redrawtimer */
    QTimer * redrawTimerShort;
    /** reference to the long interval redrawtimer */
    QTimer * redrawTimerLong;
    /** Determines wether to draw the glidersymbol or not. */
    bool ShowGlider;
    unsigned int zoomProgressive;
    float zoomProgressiveVal[8];

  protected: // Protected attributes
    /**
     * Contains the current heading. Used for rotating the map in
     * headUp mapmode or rotating the glidersymbol in northUp and
     * trackUp modes. In degrees clockwise.
     */
    int heading;

    /**
     * Contains the current bearing. Used for rotating the map and
     * glider-symbol in trackUp mode. In degrees clockwise
     * (0=due north, 90 east, etc.)
     */
    int bearing;

    /**
     * Contains the mapmode.
     * northUp: the top of the map is north
     * headUp: the top of the map is the current flightdirection
     * trackUp: the top of the map is the current bearing to the selected waypoint
     */
    mapMode mode;

    /** Holds the current position. */
    QPoint curGPSPos, curMANPos;
    /** @ee: these pixmaps are pre-loaded to prevent runtime drawing */
    QPixmap _cross;
    QPixmap _glider;

    /** Airspace Warning Memos */
    QString _insideAS;
    QString _veryNearAS;
    QString _nearAS;
    QString _insideASInfo;
    QString _veryNearASInfo;
    QString _nearASInfo;
    QString _lastASInfo;

    /** save time of last touch of airspace */
    QTime _lastNear;
    QTime _lastVeryNear;
    QTime _lastInside;

    static Map *instance;
  };

#endif
