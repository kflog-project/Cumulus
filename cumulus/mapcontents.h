/***********************************************************************
 **
 **   mapcontents.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008-2015 by Axel Pauli <kflog.cumulus@gmail.com>
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class MapContents
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map content management class.
 *
 * This class provides methods for accessing the contents of the map.
 * It takes control over loading all needed map files as value lists.
 *
 * \date 2000-2015
 *
 * \version 1.3
 */

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
#include <QMutex>
#include <QString>

#include "airfield.h"
#include "airspace.h"
#include "distance.h"
#include "flighttask.h"
#include "isolist.h"
#include "map.h"
#include "radiopoint.h"
#include "singlepoint.h"
#include "waitscreen.h"

#ifdef INTERNET
#include "DownloadManager.h"
#endif

class Isohypse;
class LineElement;
class SinglePoint;

// number of isoline levels
#define ISO_LINE_LEVELS 51

class MapContents : public QObject
  {
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( MapContents )

  public:

    /**
     * The identifiers for the map element types.
     */
    enum ListID { NotSet = 0,
                  AirfieldList,
                  GliderfieldList,
                  AddSitesList,
                  OutLandingList,
                  RadioList,
                  AirspaceList,
                  ObstacleList,
                  ReportList,
                  CityList,
                  VillageList,
                  LandmarkList,
                  MotorwayList,
                  RoadList,
                  RailList,
                  HotspotList,
                  HydroList,
                  LakeList,
                  TopoList,
                  IsohypseList,
                  WaypointList,
                  FlightList
                };

    /**
     * Creates a new MapContents-object.
     */
    MapContents(QObject*, WaitScreen *waitscreen);

    /**
     * Destructor, deletes all lists.
     */
    virtual ~MapContents();

    /**
     * @return the current length of the given list.
     *
     * @param  listSelector A selector for list addressing.
     */
    unsigned int getListLength( const int listSelector ) const;

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
     * @param  listType The type of the list containing the element.
     * @param  index The index of the element in the list.
     */
    BaseMapElement* getElement(int listType, unsigned int index);

    /**
     * @return a pointer to the given airspace
     *
     * @param index The list index of the  airspace.
     */
    Airspace* getAirspace(unsigned int index)
      {
	return static_cast<Airspace *> (airspaceList[index]);
      };

    /**
     * @return a pointer to the given glider site
     *
     * @param index The list index of the gliderfield.
     */
    Airfield* getGliderfield(unsigned int index)
      {
	return &gliderfieldList[index];
      };

    /**
     * @return a pointer to the given airfield
     *
     * @param index The list index of the airfield.
     */
    Airfield* getAirfield(unsigned int index)
      {
	return &airfieldList[index];
      };

    /**
     * @return a pointer to the given outlanding
     *
     * @param  index The list index of the outlanding
     */
    Airfield* getOutlanding(unsigned int index)
      {
	return &outLandingList[index];
      };

    /**
     * @return a pointer to the given RadioPoint
     *
     * @param  index The list index of the radio point
     */
    RadioPoint* getRadioPoint(unsigned int index)
      {
	return &radioList[index];
      };

    /**
     * @return a pointer to the given hotspot point.
     *
     * @param  index  The list index of the hotspot point
     */
    SinglePoint* getHotspot(unsigned int index)
      {
	return &hotspotList[index];
      };

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
     * @param  drawnAfList Add all drawn objects to this list
     */
    void drawList( QPainter* targetP,
                   unsigned int listID,
                   QList<Airfield*> &drawnAfList );

    /**
     * Draws all elements of a list into the painter.
     *
     * @param  targetP  The painter to draw the elements into
     * @param  drawnNaList Add all drawn objects to this list
     */
    void drawList( QPainter* targetP,
                   QList<RadioPoint*> &drawnNaList );

    /**
     * Draws all elements of a list into the painter.
     *
     * @param  targetP  The painter to draw the elements into
     * @param  listID  The index of the list to be drawn
     * @param  drawnElements A list of drawn elements
     */
    void drawList( QPainter* targetP,
                   unsigned int listID,
                   QList<BaseMapElement *>& drawnElements );

    /**
     * Draws all isohypses into the given painter
     *
     * @param  targetP  The painter to draw the elements into
     */
    void drawIsoList(QPainter* targetP);

    /**
     * @return the waypoint list
     */
    QList<Waypoint>& getWaypointList()
    {
      return wpList;
    };

    /**
     * Saves the current waypoint list into a file.
     */
    void saveWaypointList();

    /**
     * Sets the current flight task.
     */
    void setCurrentTask( FlightTask* newTask );

    /**
     * Returns the current flight task.
     *
     * \return The current flight task.
     */
    FlightTask *getCurrentTask()
    {
      return currentTask;
    };

    /**
     * Restores the last set flight task. Can be used after a reboot.
     *
     * \return True in case of success otherwise false.
     */
    bool restoreFlightTask();

    /**
     * @return true if the coordinates of the waypoint in the argument
     * matches one of the waypoints in the list.
     */
    bool isInWaypointList( const QPoint& wgsCoord );

    /**
     * @return true if the name of the waypoint in the argument
     * matches one of the waypoints in the list.
     */
    bool isInWaypointList( const QString& name );

    /**
     * Gets the pointer of the waypoint from the waypoint list which is equal
     * to the passed waypoint.
     *
     * \param wp waypoint to be serached in waypoint list
     *
     * \return Null if no waypoint match was found otherwise a pointer to the
     * waypoint object.
     */
    Waypoint* getWaypointFromList( const Waypoint* wp );

    /**
     * @return how often the name of the waypoint in the argument
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
      return &pathIsoLines;
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
     * Compares two projection objects for equality.
     *
     * @return true if equal; otherwise false
     */
    static bool compareProjections(ProjectionBase* p1, ProjectionBase* p2);

  public slots:

    /**
     * This slot is called to do a first load of all map data or to do a
     * reload of certain map data after a position move or projection change.
     */
    void slotReloadMapData();

    /**
     * Reloads the Welt2000 data file. Can be called after a configuration
     * change or a file download.
     */
    void slotReloadWelt2000Data();

    /**
     * Reloads the airspace data files. Can be called after a configuration
     * change or a download.
     */
    void slotReloadAirspaceData();

    /**
     * Reloads the OpenAIP POI data files. Can be called after a
     * configuration change.
     */
    void slotReloadOpenAipPoi();

    /**
     * This slot is called by the OpenAip load thread to signal, that the
     * requested airfield data have been loaded.
     */
    void slotOpenAipAirfieldLoadFinished( int noOfLists,
                                          QList<Airfield>* airfieldListIn );

    /**
     * This slot is called by the OpenAip load thread to signal, that the
     * requested navAids data have been loaded.
     */
    void slotOpenAipNavAidLoadFinished( int noOfLists,
                                        QList<RadioPoint>* radioListIn );

    /**
     * This slot is called by the OpenAip load thread to signal, that the
     * requested hotspot data have been loaded.
     */
    void slotOpenAipHotspotLoadFinished( int noOfLists,
                                         QList<SinglePoint>* hotspotListIn );

    /**
     * This slot is called by the AirspaceHelper load thread to signal, that the
     * requested airspace data have been loaded.
     */
    void slotAirspaceLoadFinished( int noOfLists,
                                   SortableAirspaceList* airspaceListIn );

#ifdef INTERNET

    /**
     * Ask the user once for download of missing map files. The answer
     * is stored permanently to have it for further request.
     *
     * \return True, if download is desired otherwise false.
     */
    bool askUserForDownload();

    /**
     * This slot is called to download the Welt2000 file from the internet.
     *
     * @param welt2000FileName The Welt2000 filename as written at the web page
     * without any path prefixes.
     */
    void slotDownloadWelt2000( const QString& welt2000FileName );

    /**
     * This slot is called to download openAip POI files from the Internet.
     *
     * @param openAipCountryList The list of countries to be downloaded.
     */
    void slotDownloadOpenAipPois( const QStringList& openAipCountryList );

    /**
     * Downloads all map tiles enclosed by the square with the center point. The
     * square edges are in parallel with the sky directions N, S, W, E. Inside
     * the square you can place a circle with radius length.
     *
     * @param center The center coordinates (Lat/lon) in KFLog format
     * @param length The half length of the square edge in meters.
     */
    void slotDownloadMapArea( const QPoint &center, const Distance& length );

    /**
     * This slot is called to download openAIP airspace files from the Internet.
     *
     * @param openAipCountryList The list of countries to be downloaded.
     */
    void slotDownloadAirspaces( const QStringList& openAipCountryList );

  private slots:

    /** Called, if all map downloads are finished. */
    void slotDownloadMapsFinished( int requests, int errors );

    /** Called, if a welt2000 download is finished. */
    void slotDownloadWelt2000Finished( int requests, int errors );

    /** Called, if all openAIP point downloads are finished. */
    void slotDownloadOpenAipPoisFinished( int requests, int errors );

    /** Called, if all openAIP airspace downloads are finished. */
    void slotDownloadOpenAipAsFinished( int requests, int errors );

    /** Called, if a network error occurred during the download. */
    void slotNetworkError();

#endif

    /**
     * This slot is called by the Welt2000 load thread to signal, that the
     * requested point data have been loaded.
     */
    void slotWelt2000LoadFinished( bool ok,
                                   QList<Airfield>* airfieldListIn,
                                   QList<Airfield>* gliderfieldListIn,
                                   QList<Airfield>* outlandingListIn );
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
     * Emitted after a reload of point data. That is the trigger for the update
     * of the point list views.
     */
    void mapDataReloaded();

    /**
     * Emitted after a reload of map data.
     *
     * \param layer The map layer to be reloaded.
     */
    void mapDataReloaded( Map::mapLayer layer );

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
    bool readBinaryFile(const int fileSecID, const char fileTypeID);

    /**
     * Reads a binary ground/terrain file.
     *
     * @param  fileSecID  The sectionID of the map file
     * @param  fileTypeID  The typeID of the map file ("G" for ground-data,
     *                     and "T" for terrain data)
     *
     * @return "true", when the file has successfully been loaded
     */
    bool readTerrainFile( const int fileSecID, const int fileTypeID );

    /**
     * Starts a thread, which is loading the requested Welt2000 data.
     */
    void loadWelt2000DataViaThread();

    /**
     * Starts a thread, which is loading the requested OpenAIP airfield data.
     */
    void loadOpenAipAirfieldsViaThread();

    /**
     * Starts a thread, which is loading the requested OpenAIP navAids data.
     */
    void loadOpenAipNavAidsViaThread();

    /**
     * Starts a thread, which is loading the requested OpenAIP hotspot data.
     */
    void loadOpenAipHotspotsViaThread();

    /**
     * Starts a thread, which is loading the requested airspace data.
     */
    void loadAirspacesViaThread();

#ifdef INTERNET

    /**
     * Try to download a missing ground/terrain/map file.
     *
     * @param file The name of the file without any path prefixes.
     * @param directory The destination directory.
     *
     */
    bool downloadMapFile( QString &file, QString &directory );

#endif

    /**
     * shows a progress message at the wait screen
     */
    void showProgress2WaitScreen( QString message );

    /**
     * airfieldList contains airports, airfields, ultralight sites
     */
    QList<Airfield> airfieldList;

    /**
     * gliderfieldList contains all glider sites.
     */
    QList<Airfield> gliderfieldList;

    /**
     * outLandingList contains all outlanding fields.
     */
    QList<Airfield> outLandingList;

    /**
     * radioList contains all radio navigation facilities.
     */
    QList<RadioPoint> radioList;

    /**
     * hotspotList contains all thermal hotspots.
     */
    QList<SinglePoint> hotspotList;

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
     * motorwayList contains all motorways.
     */
    QList<LineElement> motorwayList;

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

    /**
     * Flag to signal a reloading of map data
     */
    bool isReload;

    QPointer<WaitScreen> ws;

    /**
     * List of all drawn isohypses.
     */
    IsoList pathIsoLines;

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
    QList<Waypoint> wpList;

#ifdef INTERNET

    /** Manager to handle downloads of missing map file. */
    DownloadManager *m_downloadMangerMaps;

    /** Manager to handle downloads of missing welt2000.txt file. */
    DownloadManager *m_downloadMangerWelt2000;

    /** Manager to handle downloads of openAIP point data. */
    DownloadManager *m_downloadMangerOpenAipPois;

    /** Manager to handle downloads of missing openAIP airspace file. */
    DownloadManager *m_downloadMangerOpenAipAs;

    /** Store user decision to download missing data files. */
    bool m_shallDownloadData;

    /** Store that user has asked once for download of missing data file. */
    bool m_hasAskForDownload;

#endif

    /** Mutex to protect airfield loading actions. */
    QMutex m_airfieldLoadMutex;

    /** Mutex to protect radio point loading actions. */
    QMutex m_radioPointLoadMutex;

    /** Mutex to protect hotspot loading actions. */
    QMutex m_hotspotLoadMutex;

    /** Mutex to protect airspace loading actions. */
    QMutex m_airspaceLoadMutex;
  };

#endif
