/***********************************************************************
**
**   generalconfig.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2007-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration encapsulation class
 *
 * This class is used to store and retrieve all kinds of
 * configuration options. This class is a singleton class. Use the
 * static instance method to get a reference to the instance.
 *
*/

#ifndef GENERAL_CONFIG_H
#define GENERAL_CONFIG_H

#include <QtGlobal>
#include <QSettings>
#include <QPixmap>
#include <QString>
#include <QSize>
#include <QStringList>

#include "airspace.h"
#include "basemapelement.h"
#include "altitude.h"
#include "waypoint.h"
#include "mapdefaults.h"

// default window display times
#define MIN_POPUP_DISPLAY_TIME 3
#define AIRFIELD_DISPLAY_TIME_DEFAULT 10
#define AIRSPACE_DISPLAY_TIME_DEFAULT 10
#define INFO_DISPLAY_TIME_DEFAULT     10
#define WAYPOINT_DISPLAY_TIME_DEFAULT 10
#define WARNING_DISPLAY_TIME_DEFAULT  10
#define WARNING_SUPPRESS_TIME_DEFAULT 0 // time in minutes

// default for audible alarm switch
#define ALARM_SOUND_DEFAULT true
// default for calculator of nearest sites (true = ON)
#define NEAREST_SITE_CALCULATOR_DEFAULT true

// default airspace fillings
#define AS_FILL_NOT_NEAR   0
#define AS_FILL_NEAR      10
#define AS_FILL_VERY_NEAR 15
#define AS_FILL_INSIDE    20

class QTranslator;

// We do derive from the QT settings class as base class
class GeneralConfig : protected QSettings
{
 public:

  enum UseInMode {
    never,
    standstill,
    circling,
    cruising,
    wave,
    always
  };

  // CS Task scheme data
  enum ActiveCSTaskScheme {
    Cylinder=0,
    Sector=1
  };

  // CS Task scheme data
  enum ActiveNTTaskScheme {
    Nearst=0,
    Touched=1
  };

 private:
  /**
   * Constructor is private because this is a singleton class.
   * The relevant settings are loaded on construction.
   * @see load
   */
  GeneralConfig();

  /**
   * Because this is a singleton, don't allow copies and assignments.
   */
  GeneralConfig(const GeneralConfig& right);
  GeneralConfig& operator=(const GeneralConfig& right);

 public:
  /**
   * Destructor. The contained settings are stored before destruction.
   * @see save
   */
  ~GeneralConfig();

  /**
   * @returns The instance of GeneralConfig. Creates an instance if necessary.
   */
  static GeneralConfig *instance()
  {
    if ( ! _theInstance ) _theInstance = new GeneralConfig;

    return _theInstance;
  }

  /**
   * Saves the configuration settings.
   */
  void save();

  /**
   * Loads the configuration settings.
   */
  void load();

  //---------------------------
  // properties get/set methods
  //---------------------------

  /** gets the installation root of cumulus */
  QString &getInstallRoot()
    {
      return _installRoot;
    };

  /** sets the installation root of cumulus */
  void setInstallRoot( QString &newRoot )
  {
    _installRoot = newRoot;
  };

  /** gets the user data directory where waypoint file, task file,
      glider.pol, logger files are stored */
  QString getUserDataDirectory();

  /** sets the user data directory where waypoint file, task file,
      glider.pol,logger files are stored */
  void setUserDataDirectory( QString newDir );

  /** get main window size */
  QSize getWindowSize() const
  {
    return _windowSize;
  };

  /** set main window size */
  void setWindowSize( QSize size )
  {
    _windowSize = size;
  };

  /** Sets the GUI style, selected by the user.
   * Overwrites some GUI Style elements under Maemo to make them user friendly.
   */
  void setOurGuiStyle();

  /**
   * @returns requested pixmap
   */
  QPixmap loadPixmap( const QString& pixmapName );

  /**
   * @removes a pixmap from the global cache
   */
  void removePixmap( const QString& pixmapName );

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceA()
    {
      return _borderColorAirspaceA;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceA( const QColor& newValue )
  {
    _borderColorAirspaceA = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceB()
    {
      return _borderColorAirspaceB;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceB( const QColor& newValue )
  {
    _borderColorAirspaceB = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceC()
    {
      return _borderColorAirspaceC;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceC( const QColor& newValue )
  {
    _borderColorAirspaceC = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceD()
    {
      return _borderColorAirspaceD;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceD( const QColor& newValue )
  {
    _borderColorAirspaceD = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceE()
    {
      return _borderColorAirspaceE;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceE( const QColor& newValue )
  {
    _borderColorAirspaceE = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceF()
    {
      return _borderColorAirspaceF;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceF( const QColor& newValue )
  {
    _borderColorAirspaceF = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorWaveWindow()
    {
      return _borderColorWaveWindow;
    };

  /** Sets the airspace border color */
  void setBorderColorWaveWindow( const QColor& newValue )
  {
    _borderColorWaveWindow = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorControlC()
    {
      return _borderColorControlC;
    };

  /** Sets the airspace border color */
  void setBorderColorControlC( const QColor& newValue )
  {
    _borderColorControlC = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorControlD()
    {
      return _borderColorControlD;
    };

  /** Sets the airspace border color */
  void setBorderColorControlD( const QColor& newValue )
  {
    _borderColorControlD = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorRestricted()
    {
      return _borderColorRestricted;
    };

  /** Sets the airspace border color */
  void setBorderColorRestricted( const QColor& newValue )
  {
    _borderColorRestricted = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorDanger()
    {
      return _borderColorDanger;
    };

  /** Sets the airspace border color */
  void setBorderColorDanger( const QColor& newValue )
  {
    _borderColorDanger = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorTMZ()
    {
      return _borderColorTMZ;
    };

  /** Sets the airspace border color */
  void setBorderColorTMZ( const QColor& newValue )
  {
    _borderColorTMZ = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorLowFlight()
    {
      return _borderColorLowFlight;
    };

  /** Sets the airspace border color */
  void setBorderColorLowFlight( const QColor& newValue )
  {
    _borderColorLowFlight = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorGliderSector()
    {
      return _borderColorGliderSector;
    };

  /** Sets the airspace border color */
  void setBorderColorGliderSector( const QColor& newValue )
  {
    _borderColorGliderSector = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceA()
    {
      return _fillColorAirspaceA;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceA( const QColor& newValue )
  {
    _fillColorAirspaceA = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceB()
    {
      return _fillColorAirspaceB;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceB( const QColor& newValue )
  {
    _fillColorAirspaceB = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceC()
    {
      return _fillColorAirspaceC;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceC( const QColor& newValue )
  {
    _fillColorAirspaceC = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceD()
    {
      return _fillColorAirspaceD;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceD( const QColor& newValue )
  {
    _fillColorAirspaceD = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceE()
    {
      return _fillColorAirspaceE;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceE( const QColor& newValue )
  {
    _fillColorAirspaceE = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceF()
    {
      return _fillColorAirspaceF;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceF( const QColor& newValue )
  {
    _fillColorAirspaceF = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorWaveWindow()
    {
      return _fillColorWaveWindow;
    };

  /** Sets the airspace fill color */
  void setFillColorWaveWindow( const QColor& newValue )
  {
    _fillColorWaveWindow = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorControlC()
    {
      return _fillColorControlC;
    };

  /** Sets the airspace fill color */
  void setFillColorControlC( const QColor& newValue )
  {
    _fillColorControlC = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorControlD()
    {
      return _fillColorControlD;
    };

  /** Sets the airspace fill color */
  void setFillColorControlD( const QColor& newValue )
  {
    _fillColorControlD = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorRestricted()
    {
      return _fillColorRestricted;
    };

  /** Sets the airspace fill color */
  void setFillColorRestricted( const QColor& newValue )
  {
    _fillColorRestricted = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorDanger()
    {
      return _fillColorDanger;
    };

  /** Sets the airspace fill color */
  void setFillColorDanger( const QColor& newValue )
  {
    _fillColorDanger = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorTMZ()
    {
      return _fillColorTMZ;
    };

  /** Sets the airspace fill color */
  void setFillColorTMZ( const QColor& newValue )
  {
    _fillColorTMZ = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorLowFlight()
    {
      return _fillColorLowFlight;
    };

  /** Sets the airspace fill color */
  void setFillColorLowFlight( const QColor& newValue )
  {
    _fillColorLowFlight = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorGliderSector()
    {
      return _fillColorGliderSector;
    };

  /** Sets the airspace fill color */
  void setFillColorGliderSector( const QColor& newValue )
  {
    _fillColorGliderSector = newValue;
  };

  /**
   * @returns Structure with warning distances for airspace warnings
   */
  AirspaceWarningDistance getAirspaceWarningDistances();

  /**
   * Sets the warning distances for airspaces
   */
  void setAirspaceWarningDistances(const AirspaceWarningDistance& awd);

  /**
   * @return True if drawing is enabled for the given base map type.
   * @param objectType The type of object (defined in @ref BaseMapElement) to query
   */
  bool getAirspaceDrawingEnabled (BaseMapElement::objectType type) const;

  /**
   * Enables or disables the airspace drawing for this type of airspace
   */
  void setAirspaceDrawingEnabled (BaseMapElement::objectType type, bool enable=true)
  {
    _airspaceDrawingEnabled[type] = enable;
  };

  /**
   * @return True if warnings are enabled in general
   */
  bool getAirspaceWarningEnabled () const
  {
    return _airspaceWarningGeneral;
  };

  /**
   * Enables or disables the airspace warnings in general
   */
  void setAirspaceWarningEnabled (bool enable=true)
  {
    _airspaceWarningGeneral=enable;
  };

  /**
   * @return True if forcing of airspace drawing for closed by
   * structures is enabled
   */
  bool getForceAirspaceDrawingEnabled () const
  {
    return _forceDrawing;
  };

  /**
   * Enables or disables the forcing of airspace drawing for closed by
   * structures
   */
  void setForceAirspaceDrawingEnabled (bool enable=true)
  {
    _forceDrawing=enable;
  };

  /**
   * Gets the distance for the forcing of airspace drawing for closed by
   * structures
   */
  Distance getForceAirspaceDrawingDistance() const
  {
    return _forceDrawingDistance;
  };

  /**
   * Sets the distance for the forcing of airspace drawing for closed by
   * structures
   */
  void setForceAirspaceDrawingDistance(const Distance dist)
  {
    _forceDrawingDistance = dist;
  };

  /** Gets the last airspace url */
  QString &getLastAirspaceUrl()
    {
      return _lastAirspaceUrl;
    };
  /** Sets the surname */
  void setLastAirspaceUrl( const QString newValue )
  {
    _lastAirspaceUrl = newValue;
  };

  /** gets disclaimer version */
  int getDisclaimerVersion() const
  {
    return _disclaimerVersion;
  };

  /** sets disclaimer version */
  void setDisclaimerVersion( const int newValue )
  {
    _disclaimerVersion = newValue;
  };

  /**
   * return the set safety altitude
   */
  Altitude &getSafetyAltitude ()
    {
      return _safetyAltitude;
    };

  /**
   * Sets the safety altitude
   */
  void setSafetyAltitude( const Altitude& alt )
  {
    _safetyAltitude=alt;
  };

  /** Gets the surname */
  QString &getSurname()
    {
      return _surname;
    };
  /** Sets the surname */
  void setSurname( const QString newValue )
  {
    _surname = newValue;
  };

  /** Gets the used language */
  QString &getLanguage()
    {
      return _language;
    };
  /** Sets the language to be used. */
  void setLanguage( const QString& newValue );

  /**
   * Tries to get the default proxy setting from the environment. If nothing
   * is defined an empty string is returned.
   *  */
  QString getDefaultProxy();

  /** Gets the proxy */
  QString &getProxy()
    {
      return _proxy;
    };
  /** Sets the proxy */
  void setProxy( const QString newValue )
  {
    _proxy = newValue;
  };

  /** Gets the map sidebar frame color */
  QColor &getMapFrameColor()
    {
      return _mapSideFrameColor;
    };
  /** Sets the map sidebar frame color */
  void setMapFrameColor( const QColor& newValue )
  {
    _mapSideFrameColor = newValue;
  };

  /** Gets the GUI style */
  QString &getGuiStyle()
    {
      return _guiStyle;
    };

  /** Sets the GUI style */
  void setGuiStyle( const QString newValue )
  {
    _guiStyle = newValue;
  };

  /** gets GUI font */
  QString &getGuiFont()
  {
    return _guiFont;
  };
  /** sets GUI font  */
  void setGuiFont( const QString newValue )
  {
    _guiFont = newValue;
  };

  /** gets GUI menu font */
  QString &getGuiMenuFont()
  {
    return _guiMenuFont;
  };
  /** sets GUI menu font  */
  void setGuiMenuFont( const QString newValue )
  {
    _guiMenuFont = newValue;
  };

  /** gets virtual keyboard usage */
  bool getVirtualKeyboard() const
  {
    return _virtualKeyboard;
  };

  /** sets virtual keyboard usage */
  void setVirtualKeyboard(const bool newValue)
  {
    _virtualKeyboard = newValue;
  };

  /** gets screen saver speed limit in Km/h */
  double getScreenSaverSpeedLimit() const
  {
    return _screenSaverSpeedLimit;
  };

  /** sets screen saver speed limit in Km/h */
  void setScreenSaverSpeedLimit( const double newValue )
  {
    _screenSaverSpeedLimit = newValue;
  };

  /** gets QNH */
  int getQNH() const
  {
    return _qnh;
  };
  /** sets QNH */
  void setQNH( const int newValue )
  {
    _qnh = newValue;
  };

  /** gets B-Record logger interval */
  int getBRecordInterval() const
  {
    return _bRecordInterval;
  };
  /** sets B-Record logger interval */
  void setBRecordInterval( const int newValue )
  {
    _bRecordInterval = newValue;
  };

  /** gets K-Record logger interval */
  int getKRecordInterval() const
  {
    return _kRecordInterval;
  };
  /** sets K-Record logger interval */
  void setKRecordInterval( const int newValue )
  {
    _kRecordInterval = newValue;
  };

  /** gets logger autostart mode */
  bool getLoggerAutostartMode() const
  {
    return _loggerAutostartMode;
  };
  /** sets logger autostart mode */
  void setLoggerAutostartMode( const bool newValue )
  {
    _loggerAutostartMode = newValue;
  };

  /** gets TAS */
  int getTas() const
  {
    return _tas;
  };
  /** sets TAS */
  void setTas( const int newValue )
  {
    _tas = newValue;
  };

  /** gets wind speed */
  int getWindSpeed() const
  {
    return _windSpeed;
  };

  /** sets wind speed */
  void setWindSpeed( const int newValue )
  {
    _windSpeed = newValue;
  };

  /** gets wind direction */
  int getWindDirection() const
  {
    return _windDirection;
  };

  /** sets wind direction */
  void setWindDirection( const int newValue )
  {
    _windDirection = newValue;
  };

  /** gets current task */
  QString &getCurrentTask()
  {
    return _currentTask;
  };

  /** sets current task  */
  void setCurrentTask( const QString newValue )
  {
    _currentTask = newValue;
  };

  /** gets homesite Latitude */
  int getHomeLat() const
  {
    return _home.x();
  };
  /** sets homesite Latitude */
  void setHomeLat( const int newValue )
  {
    _home.setX( newValue );
  };

  /** gets Homesite  Longitude */
  int getHomeLon() const
  {
    return _home.y();
  };
  /** sets Homesite  Longitude */
  void setHomeLon( const int newValue )
  {
    _home.setY( newValue );
  };

  /** gets homesite coordinates */
  QPoint getHomeCoord() const
    {
      return _home;
    };

  /** sets homesite coordinates */
  void setHomeCoord( const QPoint& newValue )
  {
    _home = newValue;
  };

  /** Gets homesite elevation. */
  Distance &getHomeElevation()
  {
    return _homeElevation;
  };

  /** Sets homesite elevation. */
  void setHomeElevation( const Distance &newValue )
  {
    _homeElevation = newValue;
  };

  /** gets Center Latitude */
  int getCenterLat()  const
  {
    return _centerLat;
  };
  /** sets Center Latitude */
  void setCenterLat( const int newValue )
  {
    _centerLat = newValue;
  };

  /** gets Center  Longitude */
  int getCenterLon()  const
  {
    return _centerLon;
  };
  /** sets Center  Longitude */
  void setCenterLon( const int newValue )
  {
    _centerLon = newValue;
  };

  /** gets the expected places of map directories */
  QStringList getMapDirectories();

  /** gets map root directory */
  QString getMapRootDir() const
  {
    return _mapRootDir;
  };
  /** Sets map root directory. All needed subdirectories are created if
   *  they are missing.
   */
  void setMapRootDir( QString newValue );

  /** Gets the user's default root directory. This is the root for the data
   * and map directories.
   */
  QString getUserDefaultRootDir();

  /** gets map server url */
  QString getMapServerUrl() const
  {
    return _mapServerUrl;
  };

  /** sets map server url */
  void setMapServerUrl( QString newValue )
  {
    _mapServerUrl = newValue;
  };

  /** gets map scale */
  double getMapScale() const
  {
    return _mapScale;
  };
  /** sets map scale */
  void setMapScale( const double newValue )
  {
    _mapScale = newValue;
  };

  /** gets map Lower Limit */
  int getMapLowerLimit() const
  {
    return _mapLowerLimit;
  };
  /** sets map  */
  void setMapLowerLimit( const int newValue )
  {
    _mapLowerLimit = newValue;
  };

  /** gets map Upper Limit */
  int getMapUpperLimit() const
  {
    return _mapUpperLimit;
  };
  /** sets map Upper Limit */
  void setMapUpperLimit( const int newValue )
  {
    _mapUpperLimit = newValue;
  };

  /** gets map Border 1 */
  int getMapBorder1() const
  {
    return _mapBorder1;
  };
  /** sets map Border 1 */
  void setMapBorder1( const int newValue )
  {
    _mapBorder1 = newValue;
  };

  /** gets map Border 2 */
  int getMapBorder2() const
  {
    return _mapBorder2;
  };
  /** sets map Border 2 */
  void setMapBorder2( const int newValue )
  {
    _mapBorder2 = newValue;
  };

  /** gets map Border 3 */
  int getMapBorder3() const
  {
    return _mapBorder3;
  };
  /** sets map Border 3 */
  void setMapBorder3( const int newValue )
  {
    _mapBorder3 = newValue;
  };

  /** gets map Switch Scale */
  int getMapSwitchScale() const
  {
    return _mapSwitchScale;
  };
  /** sets map Switch Scale */
  void setMapSwitchScale( const int newValue )
  {
    _mapSwitchScale = newValue;
  };

  /** gets Map BearLine */
  bool getMapBearLine() const
  {
    return _mapBearLine;
  };
  /** sets Map BearLine */
  void setMapBearLine(const bool newValue)
  {
    _mapBearLine = newValue;
  };

  /** gets Map LoadIsoLines */
  bool getMapLoadIsoLines() const
  {
    return _mapLoadIsoLines;
  };
  /** sets Map LoadIsoLines */
  void setMapLoadIsoLines(const bool newValue)
  {
    _mapLoadIsoLines = newValue;
  };

  /** gets Map ShowIsoLineBorders */
  bool getMapShowIsoLineBorders() const
  {
    return _mapShowIsoLineBorders;
  };
  /** sets Map ShowIsoLineBorders */
  void setMapShowIsoLineBorders(const bool newValue)
  {
    _mapShowIsoLineBorders = newValue;
  };

  /** gets Map ShowWaypointLabels */
  bool getMapShowWaypointLabels() const
  {
    return _mapShowWaypointLabels;
  };
  /** sets Map ShowWaypointLabels */
  void setMapShowWaypointLabels(const bool newValue)
  {
    _mapShowWaypointLabels = newValue;
  };

  /** gets Map ShowAirfieldLabels */
  bool getMapShowAirfieldLabels() const
  {
    return _mapShowAirfieldLabels;
  };
  /** sets Map ShowAirfieldLabels */
  void setMapShowAirfieldLabels(const bool newValue)
  {
    _mapShowAirfieldLabels = newValue;
  };

  /** gets Map ShowTaskPointLabels */
  bool getMapShowTaskPointLabels() const
  {
    return _mapShowTaskPointLabels;
  };
  /** sets Map ShowTaskPointLabels */
  void setMapShowTaskPointLabels(const bool newValue)
  {
    _mapShowTaskPointLabels = newValue;
  };

  /** gets Map ShowOutLandingLabels */
  bool getMapShowOutLandingLabels() const
  {
    return _mapShowOutLandingLabels;
  };
  /** sets Map ShowOutLandingLabels */
  void setMapShowOutLandingLabels(const bool newValue)
  {
    _mapShowOutLandingLabels = newValue;
  };

  /** gets Map ShowLabelsExtraInfo */
  bool getMapShowLabelsExtraInfo() const
  {
    return _mapShowLabelsExtraInfo;
  };
  /** sets Map ShowLabelsExtraInfo */
  void setMapShowLabelsExtraInfo(const bool newValue)
  {
    _mapShowLabelsExtraInfo = newValue;
  };

  /** gets Map LoadRoads */
  bool getMapLoadRoads() const
  {
    return _mapLoadRoads;
  };
  /** sets Map LoadRoads */
  void setMapLoadRoads(const bool newValue)
  {
    _mapLoadRoads = newValue;
  };

  /** gets Map LoadHighways */
  bool getMapLoadHighways() const
  {
    return _mapLoadHighways;
  };
  /** sets Map LoadHighways */
  void setMapLoadHighways(const bool newValue)
  {
    _mapLoadHighways = newValue;
  };

  /** gets Map LoadRailways */
  bool getMapLoadRailways() const
  {
    return _mapLoadRailways;
  };
  /** sets Map LoadRailways */
  void setMapLoadRailways(const bool newValue)
  {
    _mapLoadRailways = newValue;
  };

  /** gets Map LoadCities */
  bool getMapLoadCities() const
  {
    return _mapLoadCities;
  };
  /** sets Map LoadCities */
  void setMapLoadCities(const bool newValue)
  {
    _mapLoadCities = newValue;
  };

  /** gets Map LoadWaterways */
  bool getMapLoadWaterways() const
  {
    return _mapLoadWaterways;
  };
  /** sets Map LoadWaterways */
  void setMapLoadWaterways(const bool newValue)
  {
    _mapLoadWaterways = newValue;
  };

  /** gets Map LoadForests */
  bool getMapLoadForests() const
  {
    return _mapLoadForests;
  };
  /** sets Map LoadForests */
  void setMapLoadForests(const bool newValue)
  {
    _mapLoadForests = newValue;
  };

  /** gets Map projection settings follows home position */
  bool getMapProjectionFollowsHome() const
  {
    return _mapProjFollowsHome;
  };
  /** sets Map projection settings follows home position */
  void setMapProjectionFollowsHome(const bool newValue)
  {
    _mapProjFollowsHome = newValue;
  };

  /** gets Map unload unneeded */
  bool getMapUnload() const
  {
    return _mapUnload;
  };
  /** sets Map unload unneeded */
  void setMapUnload(const bool newValue)
  {
    _mapUnload = newValue;
  };

  /** gets download missing map files */
  bool getDownloadMissingMaps() const
  {
    return _downloadMissingMaps;
  };
  /** sets download missing map files */
  void setDownloadMissingMaps(const bool newValue)
  {
    _downloadMissingMaps = newValue;
  };

  /** gets map install radius */
  int getMapInstallRadius() const
  {
    return _mapInstallRadius;
  };
  /** sets map install radius */
  void setMapInstallRadius(const int newValue)
  {
    _mapInstallRadius = newValue;
  };

  /** Gets the welt2000 country filter */
  QString &getWelt2000CountryFilter()
    {
      return _welt2000CountryFilter;
    };
  /** Sets the welt2000 country filter */
  void setWelt2000CountryFilter( const QString newValue )
  {
    _welt2000CountryFilter = newValue;
  };

  /** Gets the welt2000 link name. */
  QString &getWelt2000Link()
    {
      return _welt2000Link;
    };

  /** Gets the welt2000 file name used on web page. */
  QString &getWelt2000FileName()
    {
      return _welt2000FileName;
    };

  /** gets the welt2000 home radius */
  int getWelt2000HomeRadius() const
  {
    return _welt2000HomeRadius;
  };
  /** sets the welt2000 home radius */
  void setWelt2000HomeRadius( const int newValue )
  {
    _welt2000HomeRadius = newValue;
  };

  /** gets the welt2000 outlanding load flag */
  bool getWelt2000LoadOutlandings() const
  {
    return _welt2000LoadOutlandings;
  };
  /** sets the welt2000 outlanding load flag */
  void setWelt2000LoadOutlandings(const bool newValue)
  {
    _welt2000LoadOutlandings = newValue;
  };

  /** gets the af/wp list page size */
  int getListDisplayPageSize() const
  {
    return _listDisplayPageSize;
  };
  /** sets the af/wp list page size */
  void setListDisplayPageSize( const int newValue )
  {
    _listDisplayPageSize = newValue;
  };

  /** gets the additional af list row margin */
  int getListDisplayAFMargin() const
  {
    return _listDisplayAFMargin;
  };
  /** sets the additional af list row margin */
  void setListDisplayAFMargin( const int newValue )
  {
    _listDisplayAFMargin = newValue;
  };

  /** gets the additional rp list row margin */
  int getListDisplayRPMargin() const
  {
    return _listDisplayRPMargin;
  };
  /** sets the additional rp list row margin */
  void setListDisplayRPMargin( const int newValue )
  {
    _listDisplayRPMargin = newValue;
  };

  /** gets map Projection Type */
  int getMapProjectionType()  const
  {
    return _mapProjectionType;
  };
  /** sets map Projection Type */
  void setMapProjectionType( const int newValue )
  {
    _mapProjectionType = newValue;
  };

  /** gets Lambert Parallel1 */
  int getLambertParallel1()  const
  {
    return _lambertParallel1;
  };
  /** sets map Projection Type */
  void setLambertParallel1( const int newValue )
  {
    _lambertParallel1 = newValue;
  };

  /** gets Lambert Parallel2 */
  int getLambertParallel2()  const
  {
    return _lambertParallel2;
  };
  /** sets map Projection Type */
  void setLambertParallel2( const int newValue )
  {
    _lambertParallel2 = newValue;
  };

  /** gets Lambert Orign */
  int getLambertOrign()  const
  {
    return _lambertOrign;
  };
  /** sets Lambert Orign */
  void setLambertOrign( const int newValue )
  {
    _lambertOrign = newValue;
  };

  /** gets Cylinder Parallel */
  int getCylinderParallel()  const
  {
    return _cylinderParallel;
  };
  /** sets Cylinder Parallel */
  void setCylinderParallel( const int newValue )
  {
    _cylinderParallel = newValue;
  };

  /** gets variometer integration time */
  int getVarioIntegrationTime() const
  {
    return _varioIntegrationTime;
  };
  /** sets variometer integration time */
  void setVarioIntegrationTime(const int newValue)
  {
    _varioIntegrationTime = newValue;
  };

  /** gets variometer tek adjust */
  int getVarioTekAdjust() const
  {
    return _varioTekAdjust;
  };
  /** sets variometer tek adjust */
  void setVarioTekAdjust(const int newValue)
  {
    _varioTekAdjust = newValue;
  };

  /** gets variometer tek compensation */
  bool getVarioTekCompensation() const
  {
    return _varioTekCompensation;
  };
  /** sets variometer tek compensation */
  void setVarioTekCompensation(const bool newValue)
  {
    _varioTekCompensation = newValue;
  };

  /** gets altimeter mode */
  int getAltimeterMode() const;
  /** sets altimeter mode */
  void setAltimeterMode(const int newValue);

  /** gets altimeter toggle mode */
  bool getAltimeterToggleMode() const;
  /** sets altimeter toggle mode */
  void setAltimeterToggleMode(const bool newValue);

  /** gets log to file mode */
  bool getLog2FileMode() const;
  /** sets log to file mode */
  void setLog2FileMode(const bool newValue);

  /** gets sys log mode flag */
  bool getSystemLogMode() const;
  /** sets sys log mode flag */
  void setSystemLogMode(const bool newValue);

  /** gets nearest site calculator switch */
  bool getNearestSiteCalculatorSwitch() const;
  /** sets nearest site calculator switch */
  void setNearestSiteCalculatorSwitch(const bool newValue);

  /** gets max nearest site calculator sites */
  int getMaxNearestSiteCalculatorSites() const;
  /** sets max nearest site calculator sites */
  void setMaxNearestSiteCalculatorSites(const int newValue);

  /** Gets the user sound player */
  QString &getSoundPlayer()
    {
      return _soundPlayer;
    };

  /** Sets the user sound player */
  void setSoundPlayer( const QString newValue )
  {
    _soundPlayer = newValue;
  };

  /** gets AirfieldDisplayTime */
  int getAirfieldDisplayTime() const;
  /** sets AirfieldDisplayTime */
  void setAirfieldDisplayTime(const int newValue);

  /** gets AirspaceDisplayTime */
  int getAirspaceDisplayTime() const;
  /** sets AirspaceDisplayTime */
  void setAirspaceDisplayTime(const int newValue);

  /** gets InfoDisplayTime */
  int getInfoDisplayTime() const;
  /** sets InfoDisplayTime */
  void setInfoDisplayTime(const int newValue);

  /** gets WaypointDisplayTime */
  int getWaypointDisplayTime() const;
  /** sets WaypointDisplayTime */
  void setWaypointDisplayTime(const int newValue);

  /** gets WarningDisplayTime */
  int getWarningDisplayTime() const;
  /** sets WarningDisplayTime */
  void setWarningDisplayTime(const int newValue);

  /** gets WarningSuppressTime */
  int getWarningSuppressTime() const;
  /** sets WarningSuppressTime */
  void setWarningSuppressTime(const int newValue);

  /** gets AlarmSound */
  bool getAlarmSoundOn() const;
  /** sets AlarmSound */
  void setAlarmSoundOn(const bool newValue);

  /** gets Popup Airspace Warnings */
  bool getPopupAirspaceWarnings() const;
  /** sets Popup Airspace Warnings */
  void setPopupAirspaceWarnings(const bool newValue);

  /** Gets the Gps Device */
  QString &getGpsDevice()
  {
    return _gpsDevice;
  };
  /** Sets the Gps Device */
  void setGpsDevice( const QString newValue )
  {
    _gpsDevice = newValue;
  };

  /** Gets the Gps BT Device */
  QString &getGpsBtDevice()
  {
    return _gpsBtDevice;
  };
  /** Sets the Gps BT Device */
  void setGpsBtDevice( const QString newValue )
  {
    _gpsBtDevice = newValue;
  };

  /** Gets the Gps Speed */
  int getGpsSpeed() const;
  /** Sets the Gps Speed */
  void setGpsSpeed( const int newValue );

  /** gets Gps altitude */
  int getGpsAltitude() const;
  /** sets Gps altitude */
  void setGpsAltitude(const int newValue);
  /** gets Gps altitude correction */
  Altitude getGpsUserAltitudeCorrection() const;
  /** sets Gps altitude correction */
  void setGpsUserAltitudeCorrection(const Altitude newValue);

  /** gets Gps soft start */
  bool getGpsSoftStart() const;
  /** sets Gps soft start */
  void setGpsSoftStart(const bool newValue);

  /** gets Gps hard start */
  bool getGpsHardStart() const;
  /** sets Gps hard start */
  void setGpsHardStart(const bool newValue);

  /** gets Gps sync system clock */
  bool getGpsSyncSystemClock() const;
  /** sets Gps sync system clock */
  void setGpsSyncSystemClock(const bool newValue);

  /** gets Gps Ipc port */
  ushort getGpsIpcPort() const;
  /** sets Gps Ipc port */
  void setGpsIpcPort(const ushort newValue);

  /** gets Gps client start option */
  bool getGpsStartClientOption() const;
  /** sets Gps client start option */
  void setGpsStartClientOption(const bool newValue);

  /** Gets the Gps Client path */
  QString getGpsClientPath();

  /** gets Gps last fix latitude */
  int getGpsLastFixLat() const;
  /** sets Gps last fix latitude */
  void setGpsLastFixLat(const int newValue);

  /** gets Gps last fix longitude */
  int getGpsLastFixLon() const;
  /** sets Gps last fix longitude */
  void setGpsLastFixLon(const int newValue);

  /** gets Gps last fix altitude */
  int getGpsLastFixAlt() const;
  /** sets Gps last fix altitude */
  void setGpsLastFixAlt(const int newValue);

  /** gets Gps last fix clock */
  int getGpsLastFixClk() const;
  /** sets Gps last fix clock */
  void setGpsLastFixClk(const int newValue);

  /** gets minimum sat cout for wind calculation */
  int getWindMinSatCount() const;
  /** sets minimum sat cout for wind calculation */
  void setMinSatCount(const int newValue);

  /** gets wind altitude range */
  int getWindAltitudeRange() const;
  /** sets wind altitude range */
  void setWindAltitudeRange(const int newValue);

  /** gets wind time range */
  int getWindTimeRange() const;
  /** sets wind time range */
  void setWindTimeRange(const int newValue);

  /** gets manual navigation mode altitude */
  int getManualNavModeAltitude() const;
  /** sets manual navigation mode altitude */
  void setManualNavModeAltitude(const int newValue);

  /** Gets the waypoint file name with path */
  QString &getWaypointFile();
  /** Sets the waypoint file name with path */
  void setWaypointFile( const QString newValue );

  /** Gets the unit for altitude*/
  int getUnitAlt() const;
  /** Sets the unit for altitude */
  void setUnitAlt(const int newValue);

  /** Gets the unit for distance*/
  int getUnitDist() const;
  /** Sets the unit for distance */
  void setUnitDist(const int newValue);

  /** Gets the unit for speed */
  int getUnitSpeed() const;
  /** Sets the unit for speed */
  void setUnitSpeed(const int newValue);

  /** Gets the unit for variometer */
  int getUnitVario() const;
  /** Sets the unit for variometer */
  void setUnitVario(const int newValue);

  /** Gets the unit for wind */
  int getUnitWind() const;
  /** Sets the unit for wind */
  void setUnitWind(const int newValue);

  /** Gets the unit for position */
  int getUnitPos() const;
  /** Sets the unit for position */
  void setUnitPos(const int newValue);

  /** Gets the unit for time */
  int getUnitTime() const
  {
    return _unitTime;
  };
  /** Sets the unit for time */
  void setUnitTime(const int newValue)
  {
    _unitTime = newValue;
  };

  /** Gets the airspace line width */
  int getAirspaceLineWidth() const
  {
    return _airspaceLineWidth;
  };
  /** Sets the airspace line width */
  void setAirspaceLineWidth(const int newValue)
  {
    _airspaceLineWidth = newValue;
  };

  /**
   * Get whether airspace filling is enabled at all
   */
  bool getAirspaceFillingEnabled() {return m_airspaceFillingEnabled;};
  /**
   * Set whether airspace filling is enabled at all
   */
  void setAirspaceFillingEnabled(bool enabled) {m_airspaceFillingEnabled = enabled;};

  /**
   * Get the filling percentages for the indicated "nearness"
   * in the vertical direction only.
   * @param nearness Which filling are you looking for?
   * @returns a percentage filling
   */
  int getAirspaceFillingVertical(Airspace::ConflictType nearness);
  /**
   * Set the filling percentages for the indicated "nearness"
   * in the vertical direction only.
   * @param nearness Which filling are you looking for?
   * @param filling a percentage filling (0-100 inclusive)
   */
  void setAirspaceFillingVertical(Airspace::ConflictType nearness, int filling);

  /**
   * Get the filling percentages for the indicated "nearness"
   * in both directions.
   * @param nearness Which filling are you looking for?
   * @returns a percentage filling
   */
  int getAirspaceFillingLateral(Airspace::ConflictType nearness);
  /**
   * Set the filling percentages for the indicated "nearness"
   * in both directions.
   * @param nearness Which filling are you looking for?
   * @param filling a percentage filling (0-100 inclusive)
   */
  void setAirspaceFillingLateral(Airspace::ConflictType nearness, int filling);

  /**
   * @returns the filling for the given nearnesses.
   * This is a convenience function that combines the results of
   * getAirspaceFillingVertical and getAirspaceFillingTotal. It returns
   * 0 if airspace fillings are not enabled.
   *
   * @param vertical nearness in vertical direction only
   * @param total total nearness in both directions
   */
  int airspaceFilling(Airspace::ConflictType vertical,
                      Airspace::ConflictType total)
  {
    if (! m_airspaceFillingEnabled) return 0;

    return qMax(getAirspaceFillingVertical(vertical),
                getAirspaceFillingLateral(total) );
  }

  /**
   * Draw a trail?
   */
  UseInMode getDrawTrail() {return _drawTrail;};

  /**
   * Sets whether to draw a trail or not.
   */
  void setDrawTrail(UseInMode draw) {_drawTrail = draw;};

  /** Gets the active cs task scheme */
  enum ActiveCSTaskScheme getActiveCSTaskScheme() const
  {
    return _taskActiveCSScheme;
  };

  /** Sets the active cs task scheme */
  void setActiveCSTaskScheme( const enum ActiveCSTaskScheme newValue )
  {
    _taskActiveCSScheme = newValue;
  };

  /** Gets the active nt task scheme */
  enum ActiveNTTaskScheme getActiveNTTaskScheme() const
  {
    return _taskActiveNTScheme;
  };

  /** Sets the active nt task scheme */
  void setActiveNTTaskScheme( const enum ActiveNTTaskScheme newValue )
  {
    _taskActiveNTScheme = newValue;
  };

  /** Gets task shape alpha transparency. 0 represents a fully
      transparent color, while 255 represents a fully opaque
      color. See also
      http://doc.trolltech.com/4.2/qcolor.html#alpha-blended-drawing
      for more info.
  */
  int getTaskShapeAlpha()
  {
    return _taskShapeAlpha;
  };

  /** Sets task shape alpha transparency. */
  void setTaskShapeAlpha( const int newValue )
  {
    _taskShapeAlpha = newValue;
  };

  /** Gets task cylinder radius in meters */
  Distance &getTaskCylinderRadius()
    {
      return _taskCylinderRadius;
    };

  /** Sets task cylinder radius. Unit must be meters. */
  void setTaskCylinderRadius( const Distance &newValue )
  {
    _taskCylinderRadius = newValue;
  };

  /** gets cylinder/sector draw option */
  bool getTaskDrawShape() const
  {
    return _taskDrawShape;
  };

  /** sets cylinder/sector draw option */
  void setTaskDrawShape( const bool newValue )
  {
    _taskDrawShape = newValue;
  };

  /** gets cylinder/sector fill option */
  bool getTaskFillShape() const
  {
    return _taskFillShape;
  };
  /** sets cylinder/sector fill option */
  void setTaskFillShape( const bool newValue )
  {
    _taskFillShape = newValue;
  };

  /** Gets task sector inner radius in meters */
  Distance &getTaskSectorInnerRadius()
  {
    return _taskSectorInnerRadius;
  };

  /** Sets task sector inner radius. Unit must be meters. */
  void setTaskSectorInnerRadius( const Distance &newValue )
  {
    _taskSectorInnerRadius = newValue;
  };

  /** Gets task sector outer radius in meters */
  Distance &getTaskSectorOuterRadius()
    {
      return _taskSectorOuterRadius;
    };

  /** Sets task sector outer radius. Unit must be meters. */
  void setTaskSectorOuterRadius( const Distance &newValue )
  {
    _taskSectorOuterRadius = newValue;
  };

  /** Gets task sector angle 90-180 degrees.  */
  int getTaskSectorAngle() const
  {
    return _taskSectorAngle;
  };

  /** Sets task sector angle 90-180 degrees.  */
  void setTaskSectorAngle( const int newValue )
  {
    _taskSectorAngle = newValue;
  };

  /** Gets the GPS default device depending on the hardware type */
  QString getGpsDefaultDevice();

  /** Sets the terrain color at position index */
  void setTerrainColor( const QColor& newValue, const ushort index );

  /** Gets the terrain color at position index */
  QColor& getTerrainColor( const ushort index );

  /** Sets the common ground color */
  void setGroundColor( const QColor& newValue )
  {
    _groundColor = newValue;
  };

  /** Gets the common ground color */
  QColor& getGroundColor()
  {
    return _groundColor;
  };

  /** Sets the task course line color */
  void setTaskCourseLineColor( const QColor& newValue )
  {
    _taskCourseLineColor = newValue;
  };

  /** Gets the task course line color */
  QColor& getTaskCourseLineColor()
  {
    return _taskCourseLineColor;
  };

  /** gets task course line width */
  int getTaskCourseLineWidth() const
  {
    return _taskCourseLineWidth;
  };
  /** sets task course line width */
  void setTaskCourseLineWidth(const int newValue)
  {
    _taskCourseLineWidth = newValue;
  };

  /** Gets waypoint scale border. */
  int getWaypointScaleBorder( const wayPoint::Importance importance) const;

  /** Sets waypoint scale border. */
  void setWaypointScaleBorder( const wayPoint::Importance importance,
                               const int newScale );

  /** Gets the Flarm alias file name. */
  const QString &getFlarmAliasFileName() const
    {
      return _flarmAliasFileName;
    };

 private:

  /** loads the terrain default colors */
  void loadTerrainDefaultColors();

  static GeneralConfig *_theInstance;

  // Root path of cumulus installation
  QString _installRoot;

  // Main window size
  QSize _windowSize;

  // user data directory
  QString _userDataDirectory;

  // Internet proxy
  QString _proxy;

  // Task course line color
  QColor _taskCourseLineColor;

  // Task course line width
  int _taskCourseLineWidth;

  // terrain colors
  QColor _terrainColors[SIZEOF_TERRAIN_COLORS];

  // terrain default colors
  QString _terrainDefaultColors[SIZEOF_TERRAIN_COLORS];

  // terrain ground color, used when the iso line drawing is disabled
  QColor _groundColor;

  //properties
  //used to store the distances for airspace warnings
  AirspaceWarningDistance _awd;
  //draw airspace type and display a warning for this type
  bool _airspaceDrawingEnabled[BaseMapElement::objectTypeSize];

  // border colors of airspaces
  QColor _borderColorAirspaceA;
  QColor _borderColorAirspaceB;
  QColor _borderColorAirspaceC;
  QColor _borderColorAirspaceD;
  QColor _borderColorAirspaceE;
  QColor _borderColorAirspaceF;
  QColor _borderColorWaveWindow;
  QColor _borderColorControlC;
  QColor _borderColorControlD;
  QColor _borderColorRestricted;
  QColor _borderColorDanger;
  QColor _borderColorTMZ;
  QColor _borderColorLowFlight;
  QColor _borderColorGliderSector;

  // fill (brush) colors of airspaces
  QColor _fillColorAirspaceA;
  QColor _fillColorAirspaceB;
  QColor _fillColorAirspaceC;
  QColor _fillColorAirspaceD;
  QColor _fillColorAirspaceE;
  QColor _fillColorAirspaceF;
  QColor _fillColorWaveWindow;
  QColor _fillColorControlC;
  QColor _fillColorControlD;
  QColor _fillColorRestricted;
  QColor _fillColorDanger;
  QColor _fillColorTMZ;
  QColor _fillColorLowFlight;
  QColor _fillColorGliderSector;

  //display airspace warnings at all?
  bool _airspaceWarningGeneral;
  // vertical fillings for airspaces
  int _verticalAirspaceFillings[4];
  // lateral fillings for airspaces
  int _lateralAirspaceFillings[4];
  //enable airspace filling?
  bool m_airspaceFillingEnabled;

  //force drawing for airspaces close by?
  bool _forceDrawing;
  //vertical distance below airspace
  Distance _forceDrawingDistance;

  // airspace line width
  int _airspaceLineWidth;

  // last used airspace url
  QString _lastAirspaceUrl;

  // disclaimer version
  int _disclaimerVersion;
  Altitude _safetyAltitude;
  // surname
  QString _surname;
  // user language
  QString _language;
  // map side frame color
  QColor _mapSideFrameColor;
  // GUI style
  QString _guiStyle;
  // GUI font
  QString _guiFont;
  // GUI menu font
  QString _guiMenuFont;
  // Virtual keyboard usage
  bool _virtualKeyboard;
  // Screen saver speed limit
  double _screenSaverSpeedLimit;
  // QNH
  int _qnh;
  // B-Record logger interval
  int _bRecordInterval;
  // K-Record logger interval
  int _kRecordInterval;
  // auto logger start mode
  bool _loggerAutostartMode;
  // true air speed
  int _tas;
  // wind direction
  int _windDirection;
  // wind speed
  int _windSpeed;
  // current selected task
  QString _currentTask;

  // Homesite elevation
  Distance _homeElevation;
  // Homesite coordinates
  QPoint _home;
  // Center Latitude
  int _centerLat;
  // Center Longitude
  int _centerLon;

  // Map directory, defined by user
  QString _mapRootDir;

  // KFLog map room server Url
  QString _mapServerUrl;

  // Map Scale
  double _mapScale;
  // Map Data Projection Type
  int _mapProjectionType;
  // Map projection settings follows home position
  bool _mapProjFollowsHome;
  // Map unload unneeded
  bool _mapUnload;
  // Download missing map files
  bool _downloadMissingMaps;
  // Map install radius for download
  int _mapInstallRadius;

  // Welt2000 country filter
  QString _welt2000CountryFilter;
  // Welt2000 home radius
  int _welt2000HomeRadius;
  // Welt2000 outlanding load flag
  bool _welt2000LoadOutlandings;
  // Welt2000 download link
  QString _welt2000Link;
  // Welt2000 filename on web page
  QString _welt2000FileName;

  // Airfield/WP lists page size (entries)
  int _listDisplayPageSize;
  // Airfield/WP lists row height increase (px)
  int _listDisplayAFMargin;
  // Reachable points list row height increase (px)
  int _listDisplayRPMargin;

  // Map BearLine
  bool _mapBearLine;
  // Map LoadIsoLines
  bool _mapLoadIsoLines;
  // Map ShowIsoLineBorders
  bool _mapShowIsoLineBorders;
  // Map ShowWaypointLabels
  bool _mapShowWaypointLabels;
  // Map ShowLabelsExtraInfo
  bool _mapShowLabelsExtraInfo;
  // Map ShowAirfieldLabels
  bool _mapShowAirfieldLabels;
  // Map ShowTaskPointLabels
  bool _mapShowTaskPointLabels;
  // Map ShowOutLandingLabels
  bool _mapShowOutLandingLabels;
  // Map LoadRoads
  bool _mapLoadRoads;
  // Map LoadHighways
  bool _mapLoadHighways;
  // Map LoadRailways
  bool _mapLoadRailways;
  // Map LoadCities
  bool _mapLoadCities;
  // Map LoadWaterways
  bool _mapLoadWaterways;
  // Map LoadForests
  bool _mapLoadForests;
  // Map trail
  UseInMode _drawTrail;

  // Map Lower Limit
  int _mapLowerLimit;
  // Map Upper Limit
  int _mapUpperLimit;
  // Map Border 1
  int _mapBorder1;
  // Map Border 2
  int _mapBorder2;
  // Map Border 3
  int _mapBorder3;
  // Map Switch Scale
  int _mapSwitchScale;

  // Lambert Projection
  int _lambertParallel1;
  // Lambert Projection
  int _lambertParallel2;
  // Lambert Projection
  int _lambertOrign;
  // Cylindrical Projection
  int _cylinderParallel;

  // variometer integration time
  int _varioIntegrationTime;
  // variometer tek compensation
  bool _varioTekCompensation;
  // variometer tek adjust
  int _varioTekAdjust;

  // altimeter mode
  int _altimeterMode;
  // altimeter toggle mode
  bool _altimeterToggleMode;

  // logging into a fixed file for debugging purposes
  bool _log2File;
  // use syslog facility instead of stderr
  bool _useSysLog;

  // nearest site calculator switch
  bool _nearestSiteCalculatorSwitch;
  // maximum sites considered by nearest site calculator
  int _maxNearestSiteCalculatorSites;

  // sound player selected by user
  QString _soundPlayer;
  // AirfieldDisplayTime
  int _airfieldDisplayTime;
  // AirspaceDisplayTime
  int _airspaceDisplayTime;
  // InfoDisplayTime
  int _infoDisplayTime;
  // WaypointDisplayTime
  int _waypointDisplayTime;
  // WarningDisplayTime
  int _warningDisplayTime;
  // WarningSuppressTime
  int _warningSuppressTime;
  // AlarmSound
  bool _alarmSound;
  // Popup Airspace Warnings
  bool _popupAirspaceWarnings;

  // Gps device
  QString _gpsDevice;
  // Gps BT device
  QString _gpsBtDevice;
  // Gps speed
  int _gpsSpeed;
  // Gps delivered altitude
  int _gpsAltitudeType;
  // Gps delivered altitude user correction
  Altitude _gpsAltitudeUserCorrection;
  // Gps soft start
  bool _gpsSoftStart;
  // Gps hard start
  bool _gpsHardStart;
  // Gps synchronize system clock
  bool _gpsSyncSystemClock;
  // Gps IPC port
  ushort _gpsIpcPort;
  // Gps client start option
  bool _gpsStartClient;
  // Gps last fix latitude
  int _gpsLastFixLat;
  // Gps last fix longitude
  int _gpsLastFixLon;
  // Gps last fix altitude
  int _gpsLastFixAlt;
  // Gps last fix clock
  int _gpsLastFixClk;

  // minimum sat count for wind calculation
  int _windMinSatCount;
  // wind altitude range
  int _windAltitudeRange;
  // wind time range
  int _windTimeRange;
  // manual navigation mode altitude
  int _manualNavModeAltitude;

  // Waypoint data file name
  QString _waypointFile;

  /** Flarm alias file name. */
  QString _flarmAliasFileName;

  // In the unit items is stored the label index of the related combo box.
  // The indexes are used in the unit setting tabulator.

  // unit altitude
  int _unitAlt;
  // unit distance
  int _unitDist;
  // unit speed
  int _unitSpeed;
  // unit vario
  int _unitVario;
  // unit wind
  int _unitWind;
  // unit position
  int _unitPos;
  // unit time
  int _unitTime;

  // aktive cs task scheme
  enum ActiveCSTaskScheme _taskActiveCSScheme;
  enum ActiveNTTaskScheme _taskActiveNTScheme;

  // Task shape alpha transparency
  int _taskShapeAlpha;

  // Cylinder scheme
  Distance _taskCylinderRadius;

  // Sector scheme
  Distance _taskSectorInnerRadius;
  Distance _taskSectorOuterRadius;
  int _taskSectorAngle;

  // Cylinder/Sector draw options
  bool _taskDrawShape;
  bool _taskFillShape;

  /** Waypoint drawing scale borders. Addressed by waypoint importance
   *  (Low=0, Normal=1, High=2). It contains the scale borders defined
   *  user.
   */
  int _wayPointScaleBorders[3];

  /** Translator for other GUI languages as English. */
  QTranslator *cumulusTranslator;

  /** Translator for other Qt library languages as English. */
  QTranslator *qtTranslator;
};

#endif
