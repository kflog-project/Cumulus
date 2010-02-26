/***********************************************************************
 **
 **   mapcontents.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008-2010 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef MAP_CONTENTS_H
#define MAP_CONTENTS_H

#include <QSet>
#include <QFile>
#include <QList>
#include <QStringList>
#include <QPointer>
#include <QDateTime>
#include <QHash>
#include <QMap>
#include <QString>

#include "airfield.h"
#include "airspace.h"
#include "distance.h"
#include "flighttask.h"
#include "waitscreen.h"
#include "isolist.h"
#include "downloadmanager.h"

class Isohypse;
class LineElement;
class RadioPoint;
class SinglePoint;

// number of isoline levels
#define ISO_LINE_LEVELS 51

/**
 * This class provides functions for accessing the contents of the map.
 * It takes control over loading all needed map-files as value lists.
 */

class MapContents : public QObject
  {
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( MapContents )

  public:

    /**
     * The identifiers for the map element types.
     */
    enum MapContentsListID {NotSet = 0, AirfieldList, GliderSiteList,
                            AddSitesList, OutLandingList, RadioList, AirspaceList,
                            ObstacleList, ReportList, CityList, VillageList,
                            LandmarkList, HighwayList,
                            RoadList, RailList, HydroList,
                            LakeList, TopoList, IsohypseList,
                            WaypointList, FlightList
                           };

    /**
     * Creates a new MapContents-object.
     */
    MapContents(QObject*, WaitScreen * waitscreen);

    /**
     * Destructor, deletes all lists.
     */
    virtual ~MapContents();

    /**
     * @return the current length of the given list.
     *
     * @param  listIndex  the index of the list.
     */
    unsigned int getListLength( const int listIndex ) const;

    /**
     * clears the content of the given list.
     *
     * @param  listIndex  the index of the list.
     */
    void clearList( const int listIndex );

    /**
     * Proofs, which map sections are needed to draw the map and loads
     * the missing sections.
     */
    void proofeSection();

    /**
     * @return a pointer to the BaseMapElement of the given map element in
     * the list.
     *
     * @param  listType the type of the list containing the element
     * @param  index  the index of the element in the list
     */
    BaseMapElement* getElement(int listType, unsigned int index);

    /**
     * @return a pointer to the given airspace
     *
     * @param  index  the list-index of the airspace
     */
    Airspace* getAirspace(unsigned int index);

    /**
     * @returns a pointer to the given glider site
     *
     * @param  index  the list-index of the glider site
     */
    Airfield* getGlidersite(unsigned int index);

    /**
     * @return a pointer to the given airport
     *
     * @param  index  the list-index of the airport
     */
    Airfield* getAirport(unsigned int index);

    /**
     * @return a pointer to the given outlanding
     *
     * @param  index  the list-index of the outlanding
     */
    Airfield* getOutlanding(unsigned int index);

    /**
     * @return a pointer to the SinglePoint of the given map element
     *
     * @param  listIndex  the index of the list containing the element
     * @param  index  the index of the element in the list
     */
    SinglePoint* getSinglePoint(int listIndex, unsigned int index);

    /**
     * Draws all elements of a list into the painter.
     *
     * @param  targetP  The painter to draw the elements into
     * @param  listID   The index of the list to be drawn
     * @param  drawnAf  Add all drawn objects to this list
     */
    void drawList( QPainter* targetP,
                   unsigned int listID,
                   QList<Airfield*> &drawnAfList );
    /**
     * Draws all elements of a list into the painter.
     *
     * @param  targetP  The painter to draw the elements into
     * @param  listID  The index of the list to be drawn
     */
    void drawList(QPainter* targetP, unsigned int listID);

    /**
     * Draws all isohypses into the given painter
     *
     * @param  targetP  The painter to draw the elements into
     */
    void drawIsoList(QPainter* targetP);

    /**
     * @returns the waypoint list
     */
    QList<wayPoint>& getWaypointList()
    {
      return wpList;
    };

    /**
     * @saves the current waypoint list
     */
    void saveWaypointList();

    /**
     * Write property of FlightTask * currentTask.
     */
    void setCurrentTask( FlightTask * _newVal);

    /**
     * Return the current flight task.
     */
    FlightTask *getCurrentTask();

    /**
     * @Returns true if the coordinates of the waypoint in the argument
     * matches one of the waypoints in the list.
     */
    bool isInWaypointList( const QPoint& wgsCoord );

    /**
     * @Returns true if the name of the waypoint in the argument
     * matches one of the waypoints in the list.
     */
    bool isInWaypointList( const QString& name );

    /**
     * @Returns how often the name of the waypoint in the argument
     * matches one of the waypoints in the list.
     */
    unsigned short countNameInWaypointList( const QString& name );

    /**
       * Add a point to a rectangle, so the rectangle will be the bounding box
       * of all points added to it. If the point already lies within the borders
       * of the QRect, the QRect is unchanged. If the point is outside the
       * defined QRect, the QRox will be modified so the point lies inside the
       * new QRect. If the QRect is empty, the QRect will be set to a rectangle of
       * size (1,1) at the location of the point.
       */
    void AddPointToRect(QRect& rect, const QPoint& point);

    /** Returns list of IsoHypse Regions */
    IsoList* getIsohypseRegions()
    {
      return &regIsoLines;
    };

    /** Returns the elevation index for an elevation step in meters
     */
    uchar getElevationIndex(const ushort elevation ) const;

    /** returns ground elevation in meters
     * If the error argument is given, it will be set to the error margin for the
     * returned value.
     */
    int findElevation(const QPoint& coord, Distance* errorDist=0);

    /** Updates the projected coordinates of this map object type */
    void updateProjectedCoordinates( QList<SinglePoint>& list );
    /**
     * Deletes all currently not-needed map sections from memory
     */
    void unloadMaps(unsigned int=0);

    /**
     * Deletes all map items that are not contained in the tile section set
     * of the passed list.
     * Used by @ref unloadMaps to do the actual deleting.
     */
    void unloadMapObjects(QList<LineElement>& list);

    void unloadMapObjects(QList<SinglePoint>& list);

    void unloadMapObjects(QList<RadioPoint>& list);

    void unloadMapObjects(QMap<int, QList<Isohypse> > isoMap);

    /**
     * This function checks all possible map directories for the
     * map file. If found, it returns true and returns the complete
     * path in pathName.
     */
    static bool locateFile(const QString& fileName, QString& pathName);

    /**
     * this function serves as a substitute for the not existing
     * QDir::entryInfoList with complete path information
     */
    static void addDir( QStringList& list, const QString& path,
                        const QString& filter);

    /**
     * JD This function extracts the QDateTime entry from a
     * kfl/kfc file.  It doesn't check anything.  Need this to
     * identify new kfl files and to 'recompile' them.
     */
    static QDateTime getDateFromMapFile( const QString& path );

    /**
     * @AP: Compares two projection objects for equality.
     * @returns true if equal; otherwise false
     */
    static bool compareProjections(ProjectionBase* p1, ProjectionBase* p2);

  public slots:

    /**
     * This slot is called to do a first load of all map data or to do a
     * reload of certain map data after a position move or projection change.
     */
    void slotReloadMapData();

    /** Reload Welt2000 data file */
    void slotReloadWelt2000Data();

    /**
     * This slot is called to download the Welt2000 file from the internet.
     * @param welt2000FileName The Welt2000 filename as written at the web page
     * without any path prefixes.
     */
    void slotDownloadWelt2000( const QString& welt2000FileName );

    /**
     * Downloads all map tiles enclosed by the square with the center point. The
     * square edges are in parallel with the sky directions N, S, W, E. Inside
     * the square you can place a circle with radius length.
     *
     * @param center The center coordinates (Lat/lon) in KFLog format
     * @param length The half length of the square edge in meters.
     */
    void slotDownloadMapArea( const QPoint &center, const Distance& length );

  private slots:

    /** Called, if all downloads are finished. */
    void slotDownloadsFinished( int requests, int errors );

  signals:

    /**
     * Emitted if a new file is being loaded.
     */
    void loadingFile(const QString&);

    /**
     * Emitted if an object has been loaded.
     */
    void progress(int);

    /**
     * Emitted after reload of map data
     */
    void mapDataReloaded();

  private:

    /**
     * Reads a binary map file.
     *
     * @param  fileSecID  The sectionID of the map file
     * @param  fileTypeID  The typeID of the map file ("M" for additional
     *                     map data)
     *
     * @return "true", when the file has successfully been loaded
     */
    bool __readBinaryFile(const int fileSecID, const char fileTypeID);

    /**
     * Reads a binary ground/terrain file.
     *
     * @param  fileSecID  The sectionID of the map file
     * @param  fileTypeID  The typeID of the map file ("G" for ground-data,
     *                     and "T" for terrain data)
     *
     * @return "true", when the file has successfully been loaded
     */
    bool __readTerrainFile( const int fileSecID,
                            const int fileTypeID );

    /**
     * Try to download a missing ground/terrain file.
     *
     * @param file The name of the file without any path prefixes.
     * @param directory The destination directory.
     *
     */
    bool __downloadMapFile( QString &file, QString &directory );

    /**
     * Ask the user once for download of missing map files. The answer
     * is stored permanently to have it for further request.
     * Returns true, if download is desired otherwise false.
     */
    bool __askUserForDownload();

    /**
     * shows a progress message at the wait screen
     */
    void showProgress2WaitScreen( QString message );

    /**
     * airfieldList contains airports, airfields, ultralight sites
     */
    QList<Airfield> airfieldList;

    /**
     * gliderSiteList contains all glider sites.
     */
    QList<Airfield> gliderSiteList;

    /**
     * addSitesList contains all, ultralight sites,
     * hang glider sites, free balloon sites, parachute jumping sites.
     *
     * NOT used atm
     */
    // QList<SinglePoint> addSitesList;

    /**
     * outLandingList contains all outlanding fields.
     */
    QList<Airfield> outLandingList;

    /**
     * radioList contains all radio navigation facilities.
     */
    QList<RadioPoint> radioList;

    /**
     * airspaceList contains all airspaces. The sort function on this
     * list will sort the airspaces from top to bottom. This list must be stay
     * a pointer list because the cross reference to the airspace region.
     */
    SortableAirspaceList airspaceList;
    //  there are different airspaces with same name ! Don't use MapElementList,
    //  it would sort them out.

    /**
     * obstacleList contains all obstacles and groups, as well
     * as the spots and passes.
     */
    QList<SinglePoint> obstacleList;

    /**
     * reportList contains all reporting points.
     */
    QList<SinglePoint> reportList;

    /**
     * cityList contains all cities.
     */
    QList<LineElement> cityList;

    /**
     * villageList contains all villages.
     */
    QList<SinglePoint> villageList;

    /**
     * landmarkList contains all landmarks.
     */
    QList<SinglePoint> landmarkList;

    /**
     * highwayList contains all highways.
     */
    QList<LineElement> highwayList;

    /**
     * roadList contains all roads.
     */
    QList<LineElement> roadList;
    /**
     * railList contains all railways and aerial railways.
     */
    QList<LineElement> railList;
    /**
     * hydroList contains all shore lines, rivers, ...
     */
    QList<LineElement> hydroList;
    /**
     * hydroList contains all lakes, ...
     */
    QList<LineElement> lakeList;
    /**
     * topoList contains all topographical objects.
     */
    QList<LineElement> topoList;
    /**
     * Isohypse map contains all isohypses above the ground of a tile in a list.
     */
    QMap<int, QList<Isohypse> > terrainMap;
    /**
     * Isohypse map contains all ground isohypses of a tile in a list.
     */
    QMap<int, QList<Isohypse> > groundMap;

    /**
     * Set over map tiles. Contains the sectionId for all fully loaded
     * section files otherwise nothing.
     */
    QSet<int> tileSectionSet;

    /**
     * QMap of all partially loaded map tiles. These map tiles are
     * marked as not loaded in the sectionArray above. Partially
     * loaded tiles (each tile currently consists of a maximum of
     * three files) can occur when Cumulus runs out of free
     * memory. The proofeSection routine uses this QMap to determine
     * if it really needs to load a specific file for that tile.
     */
    typedef QMap<int, char> TilePartMap;
    TilePartMap tilePartMap;

    /**
     * True if an unload call has already been made this 'round' of map drawing.
     */
    bool unloadDone;

    /**
     * True if even after the unload, there is still not enough free memory, so new maps
     * may not be loaded.
     */
    bool memoryFull;

    /**
     * Flag to signal first loading of map data
     */
    bool isFirst;

    QPointer<WaitScreen> ws;

    /**
     * List of all drawn isohypses.
     */
    IsoList regIsoLines;

    /**
     * Elevation where the next search for the current elevation will start.
     * Set to one level higher than the current level by findElevation().
     */
    int _nextIsoLevel;
    int _lastIsoLevel;
    bool _isoLevelReset;
    const IsoListEntry* _lastIsoEntry;

    /**
     * Array containing the used elevation levels in meters. Is used as help
     * for reverse mapping elevation to array index.
     */
    static const short isoLevels[ISO_LINE_LEVELS];

    /** Hash table with elevation in meters as key and related elevation
     * index as value
     */
    QHash<short, uchar> isoHash;

    /**
     * Contains a reference to the currently selected flight task
     */
    FlightTask *currentTask;

    /*
     * This list is reset every time the current WaypointCatalog is changed.
     */
    QList<wayPoint> wpList;

    /** Manager to handle downloads of missing map file. */
    DownloadManager *downloadManger;

    /** Store user decision to download missing data files. */
    bool shallDownloadData;

    /** Store that user has asked once for download of missing data file. */
    bool hasAskForDownload;
  };

#endif
