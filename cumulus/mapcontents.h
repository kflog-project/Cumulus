/***********************************************************************
 **
 **   mapcontents.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef MAPCONTENTS_H
#define MAPCONTENTS_H

#include <QBitArray>
#include <QFile>
#include <Q3PtrList>
#include <QStringList>
#include <QPointer>
#include <QDateTime>
#include <QMap>

#include "airspace.h"
#include "flighttask.h"
#include "mapelementlist.h"
#include "waitscreen.h"
#include "isolist.h"

class Airport;
class Flight;
class GliderSite;
class Isohypse;
class LineElement;
class RadioPoint;
class SinglePoint;
class Distance;


/**
 * This class provides functions for accessing the contents of the map.
 * It takes control over loading all needed map-files.
 * The class contains several Q3PtrLists holding the mapelements.
 */

class MapContents : public QObject
{
  Q_OBJECT

  public:

  /**
   * The index of Mapelement-Lists.
   */
  enum MapContentsListID {NotSet = 0, AirportList, GliderList,
                          AddSitesList, OutList, NavList, AirspaceList,
                          ObstacleList, ReportList, CityList, VillageList,
                          LandmarkList, HighwayList, HighwayEntryList,
                          RoadList, RailList, StationList, HydroList,
                          LakeList, TopoList, IsohypseList,
                          WaypointList, DigitList, FlightList};

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
  unsigned int getListLength(int listIndex) const;

  /**
   * Proofes, which mapsections are needed to draw the map and loads
   * the missing sections.
   */
  void proofeSection();

  /**
   * @return a pointer to the BaseMapElement of the given mapelement in
   *         the list.
   *
   * @param  listIndex  the index of the list containing the element
   * @param  index  the index of the element in the list
   */
  BaseMapElement* getElement(int listIndex, unsigned int index);

  /**
   * @return a pointer to the given airspace
   *
   * @param  index  the list-index of the airspace
   */
  Airspace* getAirspace(unsigned int index);

  /**
   * @returns a pointer to the given glidersite
   *
   * @param  index  the list-index of the glidersite
   */
  GliderSite* getGlidersite(unsigned int index);

  /**
   * @return a pointer to the given airport
   *
   * @param  index  the list-index of the airport
   */
  Airport* getAirport(unsigned int index);

  /**
   * @return a pointer to the SinglePoint of the given mapelement
   *
   * @param  listIndex  the index of the list containing the element
   * @param  index  the index of the element in the list
   */
  SinglePoint* getSinglePoint(int listIndex, unsigned int index);

  /**
   * Draws all elements of a list into the painter.
   *
   * @param  targetP  The painter to draw the elements into
   * @param  listID  The index of the list to be drawn
   */
  void drawList(QPainter* targetPainter,
                unsigned int listID);

  /**
   * Draws all isohypses into the given painter
   *
   * @param  targetP  The painter to draw the elements into
   */
  void drawIsoList(QPainter* targetP);

  /**
   * Prints the whole content of the map into the given painter.
   *
   * @param  targetP  The painter to draw the elements into
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  void printContents(QPainter* targetP, bool isText);

  /**
   * @returns the waypoint list
   */
  Q3PtrList<wayPoint>* getWaypointList()
  {
    return &wpList;
  };

  /**
   * Write property of FlightTask * currentTask.
   */
  void setCurrentTask( FlightTask * _newVal);

  /**
   * Read property of FlightTask *currentTask.
   */
  FlightTask *getCurrentTask();

  /**
   * @Returns true if the coordinates of the waypoint in the argument
   * matches one of the waypoints in the list.
   */
  bool getIsInWaypointList(const wayPoint * wp);

  /**
   * Add a point to a rectangle, so the rectangle will be the bounding box
   * of all points added to it. If the point allready lies within the borders
   * of the QRect, the QRect is unchanged. If the point is outside the
   * defined QRect, the QRox will be modified so the point lies inside the
   * new QRect. If the QRect is empty, the QRect will be set to a rect of
   * size (1,1) at the location of the point.
   */
  void AddPointToRect(QRect& rect, const QPoint& point);

  /** Returns list of IsoHypse Regions */
  IsoList* getIsohypseRegions()
  {
    return &regIsoLines;
  };

  /** returns ground elevation in meters
   * If the error argument is given, it will be set to the errormargin for the
   * returned value.
   */
  int findElevation(const QPoint& coord, Distance * errorDist=0);

  /**
   * Deletes all currently not-needed mapsections from memory
   */
  void unloadMaps(unsigned int=0);

  /**
   * Deletes all map items that are not in the sectionArray from the given list.
   * Used by @ref unloadMaps to do the actual deleting.
   */
  void unloadMapObjects(Q3PtrList<LineElement> * list);

  void unloadMapObjects(Q3PtrList<SinglePoint> * list);

  void unloadMapObjects(Q3PtrList<RadioPoint> * list);

  void unloadMapObjects(Q3PtrList< Q3PtrList<Isohypse> > * list);

  /**
   * This function checks all possible mapdirectories for the
   * mapfile. If found, it returns true and returns the complete
   * path in pathName.
   */
  static bool locateFile(const QString& fileName, QString & pathName);

  /**
   * this function serves as a substitute for the not existing
   * QDir::entryInfoList with complete path information
   */
  static void addDir(QStringList& list, const QString& path, const QString& filter);

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

  /** Locations of maps */
  static const QString mapDir1;
  static const QString mapDir2;
  static const QString mapDir3;

  public slots:
  /** */
  void slotReloadMapData();
  /** reload welt 2000 data file */
  void slotReloadWelt2000Data();

 signals:
  /**
   * emitted during map loading to display a message f.e. in the
   * splash-screen of the mainwindow.
   */
  void loadingMessage(const QString& message);

  /**
   * signal that a new task has been created
   * FIXME: remove planning mode
   */
  void newTaskAdded(FlightTask *);

  /**
   * Emitted, when no mapfiles are found, or the when the map-directories
   * do not exists.
   */
  void errorOnMapLoading();

  /**
   * Emitted if a new file is being loaded.
   */
  void loadingFile(const QString&);

  /**
   * Emitted if an object has been loaded.
   */
  void progress(int);

  /**
   * Emitted before loading maps
   */
  void majorAction(const QString&);

  /**
   * Emitted after reload of map data
   */
  void mapDataReloaded();

 private:
  /**
   * Reads a binary map file containing airfields.
   *
   * @param  fileName  The path and name of the airfield-file.
   */
  bool __readAirfieldFile(const QString& pathName);

  /**
   * Reads a binary map file containing airspaces.
   *
   * @param  fileName  The path and name of the airspace-file.
   */
  // Not more supported by cumulus X11, AP 02.02.2008
  // bool __readAirspaceFile(const QString& pathName);

  /**
   * Reads a new binary map file.
   *
   * @param  fileSecID  The sectionID of the mapfile
   * @param  fileTypeID  The typeID of the mapfile ("G" for ground-data,
   *                     "M" for additional mapdata and "T" for
   *                     terraindata)
   *
   * @return "true", when the file has successfully been loaded
   */
  bool __readBinaryFile(const int fileSecID, const char fileTypeID);

  /**
   * Reads a new binary terrain-map file.
   *
   * @param  fileSecID  The sectionID of the mapfile
   * @param  fileTypeID  The typeID of the mapfile ("G" for ground-data,
   *                     "M" for additional mapdata and "T" for
   *                     terraindata)
   *
   * @return "true", when the file has successfully been loaded
   */
  bool __readTerrainFile(const int fileSecID, const int fileTypeID);

  /**
   * airportList contains all airports.
   */
  MapElementList airportList;

  /**
   * gliderList contains all glider-sites.
   */
  MapElementList gliderList;

  /**
   * addSitesList contains all, ultra-light,
   * hang-glider-sites, free-ballon-sites, parachute-jumping-sites.
   */
  Q3PtrList<SinglePoint> addSitesList;

  /**
   * outList contains all outlanding-fields.
   */
  MapElementList outList;

  /**
   * navList contains all radio navigation facilities.
   */
  Q3PtrList<RadioPoint> navList;

  /**
   * airspaceList contails all airspaces. The sort funtion on this
   * list will sort the airspaces from top to bottom.
   */
  SortableAirspaceList airspaceList;
  //  there are different airspaces with same name ! Don't use MapElementList,
  //  it would sort them out.
  //  MapElementList airspaceList;

  /**
   * obstacleList contains all obstacles and -groups, as well
   * as the spots and passes.
   */
  Q3PtrList<SinglePoint> obstacleList;

  /**
   * reportList contains all reporting points.
   */
  Q3PtrList<SinglePoint> reportList;

  /**
   * cityList contails all cities.
   */
  Q3PtrList<LineElement> cityList;

  /**
   * villageList contains all villages.
   */
  Q3PtrList<SinglePoint> villageList;

  /**
   * landmarkList contains all landmarks.
   */
  Q3PtrList<SinglePoint> landmarkList;

  /**
   * highwayList contails all highways.
   */
  Q3PtrList<LineElement> highwayList;

  /**
   * roadList contails all roads.
   */
  Q3PtrList<LineElement> roadList;
  /**
   * railList contains all railways and aerial railways.
   */
  Q3PtrList<LineElement> railList;
  /**
   * stationList contains all stations.
   */
  //    Q3PtrList<SinglePoint> stationList;
  /**
   * hydroList contains all shorelines, rivers, ...
   */
  Q3PtrList<LineElement> hydroList;
  /**
   * hydroList contains all lakes, ...
   */
  Q3PtrList<LineElement> lakeList;
  /**
   * topoList contains all topographical objects.
   */
  Q3PtrList<LineElement> topoList;
  /**
   * isohypseList contains all isohypses.
   */
  Q3PtrList< Q3PtrList<Isohypse> > isoList;
  /**
   * List of all map-sections. Contains a "1" for all fully loaded section-files,
   * otherwise "0".
   */
  QBitArray sectionArray;
  /**
   * QMap of all partially loaded map tiles. These maptiles are
   * marked as not loaded in the sectionArray above. Partially
   * loaded tiles (each tile currently consists of a maximum of
   * three files) can occur when Cumulus runs out of free
   * memory. The proofeSection routine uses this QMap to determine
   * if it really needs to load a specific file for that tile.
   */
  typedef QMap<int, char> TilePartMap;
  TilePartMap tilePartMap;

  /**
   * True if an unload call has allready been made this 'round' of mapdrawing.
   */
  bool unloadDone;

  /**
   * True if even after the unload, there is still not enough free memory, so new maps
   * may not be loaded.
   */
  bool memoryFull;

  /**
   * Array containing the evevations of all possible isohypses.
   */
  static const int isoLines[];

  /**
   * Should be deleted somtime ...
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
  IsoListEntry* _lastIsoEntry;


 protected: // Protected attributes
  /**
   * Contains a reference to the currently selected flighttask
   */
  FlightTask *currentTask;

 private:
  /*
   * This list is reset every time the current WaypointCatalog is changed.
   */
  Q3PtrList<wayPoint> wpList;

};

#endif
