/***********************************************************************
 **
 **   generalconfig.cpp
 **
 **   This file is part of Cumulus
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

#include <iostream>
using namespace std;

#include <stdlib.h>
#include <cmath>

#include <QtGui>

#include "generalconfig.h"
#include "hwinfo.h"
#include "gpsnmea.h"
#include "speed.h"
#include "altitude.h"
#include "distance.h"
#include "time_cu.h"

#ifdef MAEMO
#include "maemostyle.h"
#endif

// define NULL static instance
GeneralConfig* GeneralConfig::_theInstance = 0;

// @AP: We derive here from the QT settings as base class. The config
// file will be stored in the user home directory as $HOME/.config/Cumulus.conf
GeneralConfig::GeneralConfig() : QSettings( QSettings::UserScope, "Cumulus" )
{
  loadTerrainDefaultColors();
  load();

  cumulusTranslator = static_cast<QTranslator *> (0);
  qtTranslator      = static_cast<QTranslator *> (0);
}

GeneralConfig::~GeneralConfig()
{
  save();

  if( cumulusTranslator )
    {
      QCoreApplication::removeTranslator( cumulusTranslator );
      delete cumulusTranslator;
    }

  if( qtTranslator )
    {
      QCoreApplication::removeTranslator( qtTranslator );
      delete qtTranslator;
    }

  _theInstance = 0;
}

void GeneralConfig::load()
{
  // cumulus main data
  beginGroup("Main");
  _installRoot = value("InstallRoot", "./").toString();
  endGroup();

  // Main window properties
  beginGroup("MainWindow");

#ifdef TOMTOM
  _windowSize            = value("Geometry", QSize(480, 272)).toSize();
#else
  _windowSize            = value("Geometry", QSize(800, 480)).toSize();
#endif

  _mapSideFrameColor     = QColor( value("MapSideFrameColor", "#687ec6").toString() );

#ifndef MAEMO
  _guiStyle              = value("Style", "Plastique").toString();
#else
  _guiStyle              = value("Style", "Plastique").toString();
#endif

  _guiFont               = value("Font", "").toString();
  _guiMenuFont           = value("MenuFont", "").toString();
  _virtualKeyboard       = value("VirtualKeyboard", false).toBool();
  _screenSaverSpeedLimit = value("ScreenSaverSpeedLimit", 10).toDouble();
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  _awd.horClose          = value("HorizontalWarningDistance", 2000.0).toDouble();
  _awd.horVeryClose      = value("HorizontalWarningDistanceVC", 1000.0).toDouble();
  _awd.verAboveClose     = value("VerticalWarningDistanceAbove", 200.0).toDouble();
  _awd.verAboveVeryClose = value("VerticalWarningDistanceAboveVC", 100.0).toDouble();
  _awd.verBelowClose     = value("VerticalWarningDistanceBelow", 200.0).toDouble() ;
  _awd.verBelowVeryClose = value("VerticalWarningDistanceBelowVC", 100.0).toDouble();

  _lastAirspaceUrl      = value("LastAirspaceUrl", "").toString();
  _forceDrawing         = value("forceLowAirspaceDrawing", true ).toBool();
  _forceDrawingDistance = value("forceLowAirspaceDrawingDistance", 150.0).toDouble();

#ifndef MAEMO
  _airspaceLineWidth = value( "AirSpaceLineWidth", 5 ).toInt();
#else
  _airspaceLineWidth = value( "AirSpaceLineWidth", 7 ).toInt();
#endif

  // Airspace warning types
  _airspaceDrawingEnabled[BaseMapElement::AirA]         = value("checkAirspaceA", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::AirB]         = value("checkAirspaceB", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::AirC]         = value("checkAirspaceC", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::ControlC]     = value("checkControlC", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::AirD]         = value("checkAirspaceD", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::ControlD]     = value("checkControlD", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::AirE]         = value("checkAirspaceE", false).toBool();
  _airspaceDrawingEnabled[BaseMapElement::WaveWindow]   = value("checkWaveWindow", false).toBool();
  _airspaceDrawingEnabled[BaseMapElement::AirF]         = value("checkAirspaceF", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::Restricted]   = value("checkRestricted", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::Danger]       = value("checkDanger", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::Tmz]          = value("checkTMZ", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::LowFlight]    = value("checkLowFlight", true).toBool();
  _airspaceDrawingEnabled[BaseMapElement::GliderSector] = value("checkGliderSector", true).toBool();

  // Airspace border draw color
  _borderColorAirspaceA    = QColor( value("borderColorAirspaceA", AIRA_COLOR).toString() );
  _borderColorAirspaceB    = QColor( value("borderColorAirspaceB", AIRB_COLOR).toString() );
  _borderColorAirspaceC    = QColor( value("borderColorAirspaceC", AIRC_COLOR).toString() );
  _borderColorAirspaceD    = QColor( value("borderColorAirspaceD", AIRD_COLOR).toString() );
  _borderColorAirspaceE    = QColor( value("borderColorAirspaceE", AIRE_COLOR).toString() );
  _borderColorAirspaceF    = QColor( value("borderColorAirspaceF", AIRF_COLOR).toString() );
  _borderColorWaveWindow   = QColor( value("borderColorWaveWindow", WAVE_WINDOW_COLOR).toString() );
  _borderColorControlC     = QColor( value("borderColorControlC", CTRC_COLOR).toString() );
  _borderColorControlD     = QColor( value("borderColorControlD", CTRD_COLOR).toString() );
  _borderColorRestricted   = QColor( value("borderColorRestricted", RESTRICTED_COLOR).toString() );
  _borderColorDanger       = QColor( value("borderColorDanger", DANGER_COLOR).toString() );
  _borderColorTMZ          = QColor( value("borderColorTMZ", TMZ_COLOR).toString() );
  _borderColorLowFlight    = QColor( value("borderColorLowFlight", LOWF_COLOR).toString() );
  _borderColorGliderSector = QColor( value("borderColorGliderSector", GLIDER_SECTOR_COLOR).toString() );

  // Airspace fill (brush) draw color
  _fillColorAirspaceA    = QColor( value("fillColorAirspaceA", AIRA_BRUSH_COLOR).toString() );
  _fillColorAirspaceB    = QColor( value("fillColorAirspaceB", AIRB_BRUSH_COLOR).toString() );
  _fillColorAirspaceC    = QColor( value("fillColorAirspaceC", AIRC_BRUSH_COLOR).toString() );
  _fillColorAirspaceD    = QColor( value("fillColorAirspaceD", AIRD_BRUSH_COLOR).toString() );
  _fillColorAirspaceE    = QColor( value("fillColorAirspaceE", AIRE_BRUSH_COLOR).toString() );
  _fillColorAirspaceF    = QColor( value("fillColorAirspaceF", AIRF_BRUSH_COLOR).toString() );
  _fillColorWaveWindow   = QColor( value("fillColorWaveWindow", WAVE_WINDOW_BRUSH_COLOR).toString() );
  _fillColorControlC     = QColor( value("fillColorControlC", CTRC_BRUSH_COLOR).toString() );
  _fillColorControlD     = QColor( value("fillColorControlD", CTRD_BRUSH_COLOR).toString() );
  _fillColorRestricted   = QColor( value("fillColorRestricted", RESTRICTED_BRUSH_COLOR).toString() );
  _fillColorDanger       = QColor( value("fillColorDanger", DANGER_BRUSH_COLOR).toString() );
  _fillColorTMZ          = QColor( value("fillColorTMZ", TMZ_BRUSH_COLOR).toString() );
  _fillColorLowFlight    = QColor( value("fillColorLowFlight", LOWF_BRUSH_COLOR).toString() );
  _fillColorGliderSector = QColor( value("fillColorGliderSector", GLIDER_SECTOR_BRUSH_COLOR).toString() );

  _airspaceWarningGeneral = value("enableAirspaceWarning", true).toBool();

  // Airspace filling
  m_airspaceFillingEnabled = value("enableAirspaceFilling", true).toBool();
  _verticalAirspaceFillings[Airspace::none] =
    qMax(0, qMin(100, value("fillingNoneVertical", AS_FILL_NOT_NEAR).toInt()));
  _verticalAirspaceFillings[Airspace::near] =
    qMax(0, qMin(100, value("fillingNearVertical", AS_FILL_NEAR).toInt()));
  _verticalAirspaceFillings[Airspace::veryNear] =
    qMax(0, qMin(100, value("fillingVeryNearVertical", AS_FILL_VERY_NEAR).toInt()));
  _verticalAirspaceFillings[Airspace::inside] =
    qMax(0, qMin(100, value("fillingInsideVertical", AS_FILL_INSIDE).toInt()));

  _lateralAirspaceFillings[Airspace::none] =
    qMax(0, qMin(100, value("fillingNoneLateral", AS_FILL_NOT_NEAR).toInt()));
  _lateralAirspaceFillings[Airspace::near] =
    qMax(0, qMin(100, value("fillingNearLateral", AS_FILL_NEAR).toInt()));
  _lateralAirspaceFillings[Airspace::veryNear] =
    qMax(0, qMin(100, value("fillingVeryNearLateral", AS_FILL_VERY_NEAR).toInt()));
  _lateralAirspaceFillings[Airspace::inside] =
    qMax(0, qMin(100, value("fillingInsideLateral", AS_FILL_INSIDE).toInt()));
  endGroup();

  // Internet settings
  beginGroup("Internet");
  _proxy             = value("Proxy", getDefaultProxy() ).toString();
  endGroup();

  // Personal settings
  beginGroup("Personal Data");
  _disclaimerVersion = value( "Disclaimer", 0).toInt();
  _surname           = value("SurName", "").toString();
  _language          = value("Language", "en").toString();
  _userDataDirectory = value("UserDataDir", "").toString();
  endGroup();

  // Preflight settings
  beginGroup("Preflight Data");
  _safetyAltitude.setMeters(  value( "Arrival Altitude", 200.0 ).toDouble() );
  _qnh                 = value( "QNH", 1013 ).toInt();
  _bRecordInterval     = value( "B-RecordLoggerInterval", 10 ).toInt();
  _kRecordInterval     = value( "K-RecordLoggerInterval", 0 ).toInt();
  _loggerAutostartMode = value( "LoggerAutostartMode", false ).toBool();
  _tas                 = value( "TAS", 100 ).toInt();
  _windDirection       = value( "WindDirection", 0 ).toInt();
  _windSpeed           = value( "WindSpeed", 0 ).toInt();
  _currentTask         = value( "CurrentTask", "").toString();
  endGroup();

  // Task scheme settings for cylinder-sector and nearest-touched
  beginGroup("Task Scheme");
  _taskActiveCSScheme = (enum ActiveCSTaskScheme) value( "ActiveCSScheme",
                                                         GeneralConfig::Sector ).toInt();
  _taskActiveNTScheme = (enum ActiveNTTaskScheme) value( "ActiveNTScheme",
                                                         GeneralConfig::Touched ).toInt();
  _taskDrawShape  = value( "DrawShape", true ).toBool();
  _taskFillShape  = value( "FillShape", true ).toBool();
  _taskShapeAlpha = value( "ShapeAlpha", 20 ).toInt(); // transparency is in %
  endGroup();

  beginGroup("Task Scheme Cylinder");
  _taskCylinderRadius.setMeters( value( "Radius", 1000.0 ).toDouble() );
  endGroup();

  beginGroup("Task Scheme Sector");
  _taskSectorInnerRadius.setMeters( value( "InnerRadius", 0.0 ).toDouble() );
  _taskSectorOuterRadius.setMeters( value( "OuterRadius", 3000.0 ).toDouble() );
  _taskSectorAngle     = value( "Angle", 90).toInt();
  endGroup();

  beginGroup("Task");
#ifndef MAEMO
  _taskCourseLineWidth = value( "CourseLineWidth", 5 ).toInt();
#else
  _taskCourseLineWidth = value( "CourseLineWidth", 7 ).toInt();
#endif
  _taskCourseLineColor = QColor( value("CourseLineColor", QColor(Qt::darkMagenta).name()).toString() );
  endGroup();

  beginGroup("Map");
  _mapProjFollowsHome             = value( "ProjectionFollowsHome", true ).toBool();
  _mapUnload                      = value( "UnloadUnneededMap", true ).toBool();
  _downloadMissingMaps            = value( "DownloadMissingMaps", false ).toBool();
  _mapInstallRadius               = value( "MapInstallRadius", 500 ).toInt();
  _mapBearLine                    = value( "BearLine", true ).toBool();
  _mapLoadIsoLines                = value( "LoadIsoLines", true ).toBool();
  _mapShowIsoLineBorders          = value( "ShowIsoLineBorders", false ).toBool();
  _mapLoadRoads                   = value( "LoadRoads", true ).toBool();
  _mapLoadHighways                = value( "LoadHighways", true ).toBool();
  _mapLoadRailways                = value( "LoadRailways", true ).toBool();
  _mapLoadCities                  = value( "LoadCities", true ).toBool();
  _mapLoadWaterways               = value( "LoadWaterways", true ).toBool();
  _mapLoadForests                 = value( "LoadForests", false ).toBool();
  _drawTrail                      = (UseInMode) value( "DrawTrail", 0 ).toInt();
  _mapShowAirfieldLabels          = value( "ShowAirfieldLabels", false ).toBool();
  _mapShowTaskPointLabels         = value( "ShowTaskPointLabels", false ).toBool();
  _mapShowOutLandingLabels        = value( "ShowOutLandingLabels", false ).toBool();
  _mapShowWaypointLabels          = value( "ShowWaypointLabels", false ).toBool();
  _mapShowLabelsExtraInfo         = value( "ShowLabelsExtraInfo", false ).toBool();

  _wayPointScaleBorders[wayPoint::Low]    = value( "WpScaleBorderLow", 125 ).toInt();
  _wayPointScaleBorders[wayPoint::Normal] = value( "WpScaleBorderNormal", 250 ).toInt();
  _wayPointScaleBorders[wayPoint::High]   = value( "WpScaleBorderHigh", 500 ).toInt();
  endGroup();

  beginGroup("Map Data");
  _home.setX( value( "Homesite Latitude", HOME_DEFAULT_LAT).toInt() );
  _home.setY( value( "Homesite Longitude", HOME_DEFAULT_LON).toInt() );
  _homeElevation.setMeters( value("Homesite Elevation", 0.0).toDouble() );
  _mapRootDir        = value("Map Root", "").toString();
  _mapServerUrl      = value("Map Server Url", "http://www.kflog.org/data/landscape/").toString();
  _centerLat         = value("Center Latitude", HOME_DEFAULT_LAT).toInt();
  _centerLon         = value("Center Longitude", HOME_DEFAULT_LON).toInt();
  _mapScale          = value("Map Scale", 200).toDouble();
  _mapProjectionType = value("Projection Type", ProjectionBase::Cylindric ).toInt();

  _welt2000CountryFilter    = value("Welt2000CountryFilter", "").toString();
  _welt2000HomeRadius       = value("Welt2000HomeRadius", 500).toInt(); // km is assumed
  _welt2000LoadOutlandings  = value("Welt2000LoadOutlandings", false ).toBool();
  _welt2000Link             = value("Welt2000Link", "http://www.segelflug.de/vereine/welt2000/download").toString();
  _welt2000FileName         = value("Welt2000FileName", "WELT2000.TXT").toString();

  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QString name = "TerrainColor_" + QString::number(i);
      _terrainColors[i] = QColor( value(name, _terrainDefaultColors[i]).toString() );
    }

  _groundColor = QColor( value("GroundColor", COLOR_LEVEL_GROUND.name()).toString() );

  endGroup();

  beginGroup("List Display");
  _listDisplayPageSize = value("List Page Entries", 30).toInt();
  _listDisplayAFMargin = value("Airfield List Row Increase", 0).toInt();
  _listDisplayRPMargin = value("Emergency List Row Increase", 20).toInt();
  endGroup();

  beginGroup("Scale");
  _mapLowerLimit  = value("Lower Limit", VAL_BORDER_L).toInt();
  _mapUpperLimit  = value("Upper Limit", VAL_BORDER_U).toInt();
  _mapBorder1     = value("Border 1", VAL_BORDER_1).toInt();
  _mapBorder2     = value("Border 2", VAL_BORDER_2).toInt();
  _mapBorder3     = value("Border 3", VAL_BORDER_3).toInt();
  _mapSwitchScale = value("Switch Scale", VAL_BORDER_S).toInt();
  endGroup();

  beginGroup("Lambert Projection");
  _lambertParallel1 = value( "Parallel1", 32400000 ).toInt();
  _lambertParallel2 = value( "Parallel2", 30000000  ).toInt();
  _lambertOrign     = value( "Origin", 0 ).toInt();
  endGroup();

  beginGroup("Cylindrical Projection");
  _cylinderParallel = value( "Parallel", 27000000 ).toInt();
  endGroup();

  beginGroup("Waypoint Data");
  _waypointFile = value( "WaypointFile", "cumulus.kwp" ).toString();
  endGroup();

  beginGroup("Variometer");
  _varioIntegrationTime = value( "IntegrationTime", 5 ).toInt();
  _varioTekCompensation = value( "TekCompensation", false ).toBool();
  _varioTekAdjust       = value( "TekAdjust", 0 ).toInt();
  endGroup();

  beginGroup("Altimeter");
  _altimeterToggleMode = value( "Toggling_Mode", false ).toBool();
  _altimeterMode       = value( "Mode", 0 ).toInt();
  endGroup();

  beginGroup("Debug");
  _log2File  = value( "Log2File", false ).toBool();
  _useSysLog = value( "UseSystemLog", false ).toBool();
  endGroup();

  beginGroup("NearestSitesCalc");
  _nearestSiteCalculatorSwitch   = value( "NearestSiteCalculatorOn",
                                          NEAREST_SITE_CALCULATOR_DEFAULT ).toBool();
  _maxNearestSiteCalculatorSites = value("MaxNumberOfSitesInList", 50).toInt();
  endGroup();

  beginGroup("Information");
#ifdef MAEMO
  _soundPlayer           = value( "SoundPlayer", "/opt/cumulus/bin/aplay" ).toString();
#else
  _soundPlayer           = value( "SoundPlayer", "/usr/bin/aplay" ).toString();
#endif
  _airfieldDisplayTime   = value( "AirfieldDisplayTime",
                                  AIRFIELD_DISPLAY_TIME_DEFAULT ).toInt();
  _airspaceDisplayTime   = value( "AirspaceDisplayTime",
                                  AIRSPACE_DISPLAY_TIME_DEFAULT).toInt();
  _infoDisplayTime       = value( "InfoDisplayTime",
                                  INFO_DISPLAY_TIME_DEFAULT).toInt();
  _waypointDisplayTime   = value( "WaypointDisplayTime",
                                  WAYPOINT_DISPLAY_TIME_DEFAULT).toInt();
  _warningDisplayTime    = value( "WarningDisplayTime",
                                  WARNING_DISPLAY_TIME_DEFAULT).toInt();
  _warningSuppressTime   = value( "WarningSuppressTime",
                                  WARNING_SUPPRESS_TIME_DEFAULT).toInt();
  _alarmSound            = value( "AlarmSound", ALARM_SOUND_DEFAULT ).toBool();
  _popupAirspaceWarnings = value( "Popup Airspace Warnings", true ).toBool();
  endGroup();

  beginGroup("GPS");
  _gpsDevice          = value( "Device", getGpsDefaultDevice() ).toString();
  _gpsBtDevice        = value( "BT-Device", "" ).toString();
  _gpsSpeed           = value( "Speed", 4800 ).toInt();
  _gpsAltitudeType    = value( "AltitudeType", (int) GpsNmea::GPS ).toInt();
  _gpsAltitudeUserCorrection.setMeters(value( "AltitudeCorrection", 0 ).toInt());
  _gpsSoftStart       = value( "SoftStart", false ).toBool();
  _gpsHardStart       = value( "HardStart", false ).toBool();
  _gpsSyncSystemClock = value( "SyncSystemClock", false ).toBool();
  _gpsIpcPort         = value( "IpcPort", 0 ).toInt();
  _gpsStartClient     = value( "StartClient", true ).toBool();
  _gpsLastFixLat      = value( "LastFixLat", 0 ).toInt();
  _gpsLastFixLon      = value( "LastFixLon", 0 ).toInt();
  _gpsLastFixAlt      = value( "LastFixAlt", 0 ).toInt();
  _gpsLastFixClk      = value( "LastFixClk", 0 ).toInt();
  endGroup();

  beginGroup("Wind");
  _windMinSatCount   = value( "MinSatCount", 4 ).toInt();
  _windAltitudeRange = value( "AltitudeRange", 1000 ).toInt();
  _windTimeRange     = value( "TimeRange", 600 ).toInt();
  endGroup();

  beginGroup ("Calculator");
  _manualNavModeAltitude = value( "ManualNavModeAltitude", 1000 ).toInt();
  endGroup();

  beginGroup ("Flarm");
  _flarmAliasFileName = value("FlarmAliasFileName", "cumulus-flarm.txt").toString();
  endGroup();

  beginGroup("Units");
  _unitAlt   = value( "Altitude", Altitude::meters).toInt();
  _unitDist  = value( "Distance", Distance::kilometers).toInt();
  _unitSpeed = value( "Speed",    Speed::kilometersPerHour ).toInt();
  _unitVario = value( "Vario",    Speed::metersPerSecond ).toInt();
  _unitWind  = value( "Wind",     Speed::metersPerSecond ).toInt();
  _unitPos   = value( "Position", WGSPoint::DMS ).toInt();
  _unitTime  = value( "Time",     Time::utc ).toInt();
  endGroup();

  // configure static units
  Altitude::setUnit( Altitude::altitudeUnit( getUnitAlt() ) );
  Distance::setUnit( Distance::distanceUnit( getUnitDist() ) );
  Speed::setHorizontalUnit( Speed::speedUnit( getUnitSpeed() ) );
  Speed::setVerticalUnit( Speed::speedUnit( getUnitVario() ) );
  Speed::setWindUnit( Speed::speedUnit( getUnitWind() ) );
  Time::setUnit( Time::timeUnit( getUnitTime() ) );
  WGSPoint::setFormat( WGSPoint::Format( getUnitPos() ) );

  // Check if user data and user map directories do exist.
  // Try to create missing structures.
  setUserDataDirectory( _userDataDirectory );

  // Checks and guarantees existence of map directories.
  setMapRootDir( _mapRootDir );

  // Check proxy and try default setting again if it is an empty string.
  if( _proxy.isEmpty() )
    {
      _proxy = getDefaultProxy();
    }
}

void GeneralConfig::save()
{
  // cumulus main data
  beginGroup("Main");
  setValue("InstallRoot", _installRoot );
  endGroup();

  // Main window properties
  beginGroup("MainWindow");
  setValue("Geometry", _windowSize );
  setValue("MapSideFrameColor", _mapSideFrameColor);
  setValue("Style", _guiStyle);
  setValue("Font", _guiFont);
  setValue("MenuFont", _guiMenuFont);
  setValue("VirtualKeyboard", _virtualKeyboard);
  setValue("ScreenSaverSpeedLimit", _screenSaverSpeedLimit);
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  setValue("HorizontalWarningDistance", _awd.horClose.getMeters());
  setValue("HorizontalWarningDistanceVC", _awd.horVeryClose.getMeters());
  setValue("VerticalWarningDistanceAbove", _awd.verAboveClose.getMeters());
  setValue("VerticalWarningDistanceAboveVC", _awd.verAboveVeryClose.getMeters());
  setValue("VerticalWarningDistanceBelow", _awd.verBelowClose.getMeters());
  setValue("VerticalWarningDistanceBelowVC", _awd.verBelowVeryClose.getMeters());

  setValue("LastAirspaceUrl", _lastAirspaceUrl);
  setValue("forceLowAirspaceDrawing", _forceDrawing);
  setValue("forceLowAirspaceDrawingDistance", _forceDrawingDistance.getMeters());
  setValue( "AirSpaceLineWidth", _airspaceLineWidth );

  // Airspace warning types
  setValue("checkAirspaceA", _airspaceDrawingEnabled[BaseMapElement::AirA]);
  setValue("checkAirspaceB", _airspaceDrawingEnabled[BaseMapElement::AirB]);
  setValue("checkAirspaceC", _airspaceDrawingEnabled[BaseMapElement::AirC]);
  setValue("checkControlC", _airspaceDrawingEnabled[BaseMapElement::ControlC]);
  setValue("checkAirspaceD", _airspaceDrawingEnabled[BaseMapElement::AirD]);
  setValue("checkControlD", _airspaceDrawingEnabled[BaseMapElement::ControlD]);
  setValue("checkAirspaceE", _airspaceDrawingEnabled[BaseMapElement::AirE]);
  setValue("checkWaveWindow", _airspaceDrawingEnabled[BaseMapElement::WaveWindow]);
  setValue("checkAirspaceF", _airspaceDrawingEnabled[BaseMapElement::AirF]);
  setValue("checkRestricted", _airspaceDrawingEnabled[BaseMapElement::Restricted]);
  setValue("checkDanger", _airspaceDrawingEnabled[BaseMapElement::Danger]);
  setValue("checkTMZ", _airspaceDrawingEnabled[BaseMapElement::Tmz]);
  setValue("checkLowFlight", _airspaceDrawingEnabled[BaseMapElement::LowFlight]);
  setValue("checkGliderSector", _airspaceDrawingEnabled[BaseMapElement::GliderSector]);

  // Airspace border draw color
  setValue("borderColorAirspaceA",    _borderColorAirspaceA.name());
  setValue("borderColorAirspaceB",    _borderColorAirspaceB.name());
  setValue("borderColorAirspaceC",    _borderColorAirspaceC.name());
  setValue("borderColorAirspaceD",    _borderColorAirspaceD.name());
  setValue("borderColorAirspaceE",    _borderColorAirspaceE.name());
  setValue("borderColorAirspaceF",    _borderColorAirspaceF.name());
  setValue("borderColorWaveWindow",   _borderColorWaveWindow.name());
  setValue("borderColorControlC",     _borderColorControlC.name());
  setValue("borderColorControlD",     _borderColorControlD.name());
  setValue("borderColorRestricted",   _borderColorRestricted.name());
  setValue("borderColorDanger",       _borderColorDanger.name());
  setValue("borderColorTMZ",          _borderColorTMZ.name());
  setValue("borderColorLowFlight",    _borderColorLowFlight.name());
  setValue("borderColorGliderSector", _borderColorGliderSector.name());

  // Airspace fill (brush) draw color
  setValue("fillColorAirspaceA",    _fillColorAirspaceA.name());
  setValue("fillColorAirspaceB",    _fillColorAirspaceB.name());
  setValue("fillColorAirspaceC",    _fillColorAirspaceC.name());
  setValue("fillColorAirspaceD",    _fillColorAirspaceD.name());
  setValue("fillColorAirspaceE",    _fillColorAirspaceE.name());
  setValue("fillColorAirspaceF",    _fillColorAirspaceF.name());
  setValue("fillColorWaveWindow",   _fillColorWaveWindow.name());
  setValue("fillColorControlC",     _fillColorControlC.name());
  setValue("fillColorControlD",     _fillColorControlD.name());
  setValue("fillColorRestricted",   _fillColorRestricted.name());
  setValue("fillColorDanger",       _fillColorDanger.name());
  setValue("fillColorTMZ",          _fillColorTMZ.name());
  setValue("fillColorLowFlight",    _fillColorLowFlight.name());
  setValue("fillColorGliderSector", _fillColorGliderSector.name());

  setValue("enableAirspaceWarning", _airspaceWarningGeneral);

  // Airspace filling
  setValue("enableAirspaceFilling", m_airspaceFillingEnabled);

  setValue("fillingNoneVertical", _verticalAirspaceFillings[Airspace::none]);
  setValue("fillingNearVertical", _verticalAirspaceFillings[Airspace::near]);
  setValue("fillingVeryNearVertical", _verticalAirspaceFillings[Airspace::veryNear]);
  setValue("fillingInsideVertical", _verticalAirspaceFillings[Airspace::inside]);

  setValue("fillingNoneLateral", _lateralAirspaceFillings[Airspace::none]);
  setValue("fillingNearLateral", _lateralAirspaceFillings[Airspace::near]);
  setValue("fillingVeryNearLateral", _lateralAirspaceFillings[Airspace::veryNear]);
  setValue("fillingInsideLateral", _lateralAirspaceFillings[Airspace::inside]);
  endGroup();

  // Internet settings
  beginGroup("Internet");
  setValue("Proxy", _proxy);
  endGroup();

  // Personal settings
  beginGroup("Personal Data");
  setValue("Disclaimer", _disclaimerVersion);
  setValue("SurName", _surname);
  setValue("Language", _language);
  setValue( "UserDataDir", _userDataDirectory);
  setValue("Proxy", _proxy);
  endGroup();

  // Preflight data
  beginGroup("Preflight Data");
  setValue( "Arrival Altitude", _safetyAltitude.getMeters() );
  setValue( "QNH", _qnh );
  setValue( "B-RecordLoggerInterval", _bRecordInterval );
  setValue( "K-RecordLoggerInterval", _kRecordInterval );
  setValue( "LoggerAutostartMode", _loggerAutostartMode );
  setValue( "TAS", _tas );
  setValue( "WindDirection", _windDirection );
  setValue( "WindSpeed", _windSpeed );
  setValue( "CurrentTask", _currentTask);
  endGroup();

  // Task scheme settings for cylinder-sector and nearest touched
  beginGroup("Task Scheme");
  setValue( "ActiveCSScheme", _taskActiveCSScheme );
  setValue( "ActiveNTScheme", _taskActiveNTScheme );
  setValue( "DrawShape", _taskDrawShape );
  setValue( "FillShape", _taskFillShape );
  setValue( "ShapeAlpha", _taskShapeAlpha );
  endGroup();

  beginGroup("Task Scheme Cylinder");
  setValue( "Radius",    _taskCylinderRadius.getMeters() );
  endGroup();

  beginGroup("Task Scheme Sector");
  setValue( "InnerRadius", _taskSectorInnerRadius.getMeters() );
  setValue( "OuterRadius", _taskSectorOuterRadius.getMeters() );
  setValue( "Angle",       _taskSectorAngle );
  endGroup();

  beginGroup("Task");
  setValue( "CourseLineWidth", _taskCourseLineWidth );
  setValue( "CourseLineColor", _taskCourseLineColor.name() );
  endGroup();

  beginGroup("Map");
  setValue( "ProjectionFollowsHome", _mapProjFollowsHome );
  setValue( "UnloadUnneededMap", _mapUnload );
  setValue( "DownloadMissingMaps", _downloadMissingMaps );
  setValue( "MapInstallRadius", _mapInstallRadius );
  setValue( "BearLine", _mapBearLine );
  setValue( "LoadIsoLines", _mapLoadIsoLines );
  setValue( "ShowIsoLineBorders", _mapShowIsoLineBorders );
  setValue( "ShowWaypointLabels", _mapShowWaypointLabels );
  setValue( "ShowLabelsExtraInfo", _mapShowLabelsExtraInfo );
  setValue( "LoadRoads", _mapLoadRoads );
  setValue( "LoadHighways", _mapLoadHighways );
  setValue( "LoadRailways", _mapLoadRailways );
  setValue( "LoadCities", _mapLoadCities   );
  setValue( "LoadWaterways", _mapLoadWaterways );
  setValue( "LoadForests", _mapLoadForests );
  setValue( "DrawTrail", (int)_drawTrail );
  setValue( "ShowAirfieldLabels", _mapShowAirfieldLabels );
  setValue( "ShowTaskPointLabels", _mapShowTaskPointLabels );
  setValue( "ShowOutLandingLabels", _mapShowOutLandingLabels );
  setValue( "WpScaleBorderLow", _wayPointScaleBorders[wayPoint::Low] );
  setValue( "WpScaleBorderNormal", _wayPointScaleBorders[wayPoint::Normal] );
  setValue( "WpScaleBorderHigh", _wayPointScaleBorders[wayPoint::High] );
  endGroup();

  beginGroup("Map Data");

  setValue("Homesite Latitude", _home.x());
  setValue("Homesite Longitude", _home.y());
  setValue("Homesite Elevation", _homeElevation.getMeters() );
  setValue("Map Root", _mapRootDir);
  setValue("Map Server Url", _mapServerUrl);
  setValue("Center Latitude", _centerLat);
  setValue("Center Longitude", _centerLon);
  setValue("Map Scale", _mapScale);
  setValue("Projection Type", _mapProjectionType);
  setValue("Welt2000CountryFilter", _welt2000CountryFilter);
  setValue("Welt2000HomeRadius", _welt2000HomeRadius);
  setValue("Welt2000LoadOutlandings", _welt2000LoadOutlandings);

  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QString name = "TerrainColor_" + QString::number(i);
      setValue(name, _terrainColors[i].name());
    }

  setValue("GroundColor", _groundColor.name());

  endGroup();

  beginGroup("List Display");
  setValue("List Page Entries", _listDisplayPageSize);
  setValue("Airfield List Row Increase", _listDisplayAFMargin);
  setValue("Emergency List Row Increase", _listDisplayRPMargin);
  endGroup();

  beginGroup("Scale");
  setValue( "Lower Limit", _mapLowerLimit );
  setValue( "Upper Limit", _mapUpperLimit );
  setValue( "Border 1", _mapBorder1 );
  setValue( "Border 2", _mapBorder2 );
  setValue( "Border 3", _mapBorder3 );
  setValue( "Switch Scale", _mapSwitchScale );
  endGroup();

  beginGroup("Lambert Projection");
  setValue( "Parallel1", _lambertParallel1 );
  setValue( "Parallel2", _lambertParallel2 );
  setValue( "Origin", _lambertOrign );
  endGroup();

  beginGroup("Cylindrical Projection");
  setValue( "Parallel", _cylinderParallel );
  endGroup();

  beginGroup("Waypoint Data");
  endGroup();
  setValue( "WaypointFile", _waypointFile );

  beginGroup("Variometer");
  setValue( "IntegrationTime", _varioIntegrationTime );
  setValue( "TekCompensation", _varioTekCompensation );
  setValue( "TekAdjust", _varioTekAdjust );
  endGroup();

  beginGroup("Altimeter");
  setValue( "Toggling_Mode", _altimeterToggleMode );
  setValue( "Mode", _altimeterMode );
  endGroup();

  beginGroup("Debug");
  setValue( "Log2File", _log2File );
  setValue( "UseSystemLog", _useSysLog );
  endGroup();

  beginGroup("NearestSitesCalc");
  setValue( "NearestSiteCalculatorOn", _nearestSiteCalculatorSwitch );
  setValue( "MaxNumberOfSitesInList", _maxNearestSiteCalculatorSites );
  endGroup();

  beginGroup("Information");
  setValue( "SoundPlayer", _soundPlayer );
  setValue( "AirfieldDisplayTime", _airfieldDisplayTime );
  setValue( "AirspaceDisplayTime", _airspaceDisplayTime );
  setValue( "InfoDisplayTime", _infoDisplayTime );
  setValue( "WaypointDisplayTime", _waypointDisplayTime );
  setValue( "WarningDisplayTime", _warningDisplayTime );
  setValue( "WarningSuppressTime", _warningSuppressTime );
  setValue( "AlarmSound", _alarmSound );
  setValue( "Popup Airspace Warnings", _popupAirspaceWarnings );
  endGroup();

  beginGroup("GPS");
  setValue( "Device", _gpsDevice );
  setValue( "BT-Device", _gpsBtDevice );
  setValue( "Speed", _gpsSpeed );
  setValue( "AltitudeType", _gpsAltitudeType );
  setValue( "AltitudeCorrection", _gpsAltitudeUserCorrection.getMeters() );
  setValue( "HardStart", _gpsHardStart );
  setValue( "SoftStart", _gpsSoftStart );
  setValue( "SyncSystemClock", _gpsSyncSystemClock );
  setValue( "IpcPort", _gpsIpcPort );
  setValue( "StartClient", _gpsStartClient );
  setValue( "LastFixLat", _gpsLastFixLat );
  setValue( "LastFixLon", _gpsLastFixLon );
  setValue( "LastFixAlt", _gpsLastFixAlt );
  setValue( "LastFixClk", _gpsLastFixClk );
  endGroup();

  beginGroup("Wind");
  setValue( "MinSatCount", _windMinSatCount );
  setValue( "AltitudeRange", _windAltitudeRange );
  setValue( "TimeRange", _windTimeRange );
  endGroup();

  beginGroup("Calculator");
  setValue("ManualNavModeAltitude", _manualNavModeAltitude );
  endGroup();

  beginGroup("Units");
  setValue( "Altitude", _unitAlt );
  setValue( "Distance", _unitDist );
  setValue( "Speed", _unitSpeed );
  setValue( "Vario", _unitVario );
  setValue( "Wind", _unitWind );
  setValue( "Position", _unitPos );
  setValue( "Time", _unitTime );
  endGroup();
}

/** gets altimeter mode */
int GeneralConfig::getAltimeterMode() const
{
  return _altimeterMode;
}


/** sets altimeter mode */
void GeneralConfig::setAltimeterMode(const int newValue)
{
  _altimeterMode = newValue;
}

/** gets altimeter toggle mode */
bool GeneralConfig::getAltimeterToggleMode() const
{
  return _altimeterToggleMode;
}

/** sets altimeter toggle mode */
void GeneralConfig::setAltimeterToggleMode(const bool newValue)
{
  _altimeterToggleMode = newValue;
}

/**
 * @returns Struct with warning distances for airspace warnings
 */
AirspaceWarningDistance GeneralConfig::getAirspaceWarningDistances()
{
  return _awd;
}

/**
 * Sets the warningdistances for airspaces
 */
void GeneralConfig::setAirspaceWarningDistances(const AirspaceWarningDistance& awd)
{
  _awd = awd;
}

/** gets log to file mode */
bool GeneralConfig::getLog2FileMode() const
{
  return _log2File;
}

/** sets log to file mode */
void GeneralConfig::setLog2FileMode(const bool newValue)
{
  _log2File = newValue;
}

/** gets system log mode */
bool GeneralConfig::getSystemLogMode() const
{
  return _useSysLog;
}

/** sets log to file mode */
void GeneralConfig::setSystemLogMode(const bool newValue)
{
  _useSysLog = newValue;
}

/** gets nearest site calculator switch */
bool GeneralConfig::getNearestSiteCalculatorSwitch() const
{
  return _nearestSiteCalculatorSwitch;
}

/** sets nearest site calculator switch */
void GeneralConfig::setNearestSiteCalculatorSwitch(const bool newValue)
{
  _nearestSiteCalculatorSwitch = newValue;
}

/** gets max nearest site calculator sites */
int GeneralConfig::getMaxNearestSiteCalculatorSites() const
{
  return _maxNearestSiteCalculatorSites;
}

/** sets max nearest site calculator sites */
void GeneralConfig::setMaxNearestSiteCalculatorSites(const int newValue)
{
  _maxNearestSiteCalculatorSites = newValue;
}

/** gets AirfieldDisplayTime */
int GeneralConfig::getAirfieldDisplayTime() const
{
  return _airfieldDisplayTime;
}

/** sets AirfieldDisplayTime */
void GeneralConfig::setAirfieldDisplayTime(const int newValue)
{
  _airfieldDisplayTime = newValue;
}

/** gets AirspaceDisplayTime */
int GeneralConfig::getAirspaceDisplayTime() const
{
  return _airspaceDisplayTime;
}


/** sets AirspaceDisplayTime */
void GeneralConfig::setAirspaceDisplayTime(const int newValue)
{
  _airspaceDisplayTime = newValue;
}


/** gets InfoDisplayTime */
int GeneralConfig::getInfoDisplayTime() const
{
  return _infoDisplayTime;
}


/** sets InfoDisplayTime */
void GeneralConfig::setInfoDisplayTime(const int newValue)
{
  _infoDisplayTime = newValue;
}


/** gets WaypointDisplayTime */
int GeneralConfig::getWaypointDisplayTime() const
{
  return _waypointDisplayTime;
}


/** sets WaypointDisplayTime */
void GeneralConfig::setWaypointDisplayTime(const int newValue)
{
  _waypointDisplayTime = newValue;
}


/** gets WarningDisplayTime */
int GeneralConfig::getWarningDisplayTime() const
{
  return _warningDisplayTime;
}


/** sets WarningDisplayTime */
void GeneralConfig::setWarningDisplayTime(const int newValue)
{
  _warningDisplayTime = newValue;
}


/** gets WarningDisplayTime */
int GeneralConfig::getWarningSuppressTime() const
{
  return _warningSuppressTime;
}


/** sets WarningSuppressTime */
void GeneralConfig::setWarningSuppressTime(const int newValue)
{
  _warningSuppressTime = newValue;
}


/** gets AlarmSound */
bool GeneralConfig::getAlarmSoundOn() const
{
  return _alarmSound;
}


/** sets AlarmSound */
void GeneralConfig::setAlarmSoundOn(const bool newValue)
{
  _alarmSound = newValue;
}


/** gets Popup Airspace Warnings */
bool GeneralConfig::getPopupAirspaceWarnings() const
{
  return _popupAirspaceWarnings;
}


/** sets Popup Airspace Warnings */
void GeneralConfig::setPopupAirspaceWarnings(const bool newValue)
{
  _popupAirspaceWarnings = newValue;
}


/** Gets the Gps Speed */
int GeneralConfig::getGpsSpeed() const
{
  return _gpsSpeed;
}


/** Sets the Gps Speed */
void GeneralConfig::setGpsSpeed( const int newValue )
{
  _gpsSpeed = newValue;
}


/** gets Gps altitude type */
int GeneralConfig::getGpsAltitude() const
{
  return _gpsAltitudeType;
}


/** sets Gps altitude type */
void GeneralConfig::setGpsAltitude(const int newValue)
{
  _gpsAltitudeType = newValue;
}


/** gets Gps altitude correction */
Altitude GeneralConfig::getGpsUserAltitudeCorrection() const
{
  return _gpsAltitudeUserCorrection;
}


/** sets Gps altitude correction */
void GeneralConfig::setGpsUserAltitudeCorrection(const Altitude newValue)
{
  _gpsAltitudeUserCorrection = newValue;
}


/** gets Gps soft start */
bool GeneralConfig::getGpsSoftStart() const
{
  return _gpsSoftStart;
}


/** sets Gps soft start */
void GeneralConfig::setGpsSoftStart(const bool newValue)
{
  _gpsSoftStart = newValue;
}


/** gets Gps hard start */
bool GeneralConfig::getGpsHardStart() const
{
  return _gpsHardStart;
}


/** sets Gps hard start */
void GeneralConfig::setGpsHardStart(const bool newValue)
{
  _gpsHardStart = newValue;
}


/** gets Gps sync system clock */
bool GeneralConfig::getGpsSyncSystemClock() const
{
  return _gpsSyncSystemClock;
}


/** sets Gps sync system clock */
void GeneralConfig::setGpsSyncSystemClock(const bool newValue)
{
  _gpsSyncSystemClock = newValue;
}


/** gets Gps Ipc port */
ushort GeneralConfig::getGpsIpcPort() const
{
  return _gpsIpcPort;
}


/** sets Gps Ipc port */
void GeneralConfig::setGpsIpcPort(const ushort newValue)
{
  _gpsIpcPort = newValue;
}


/** gets Gps client start option */
bool GeneralConfig::getGpsStartClientOption() const
{
  return _gpsStartClient;
}


/** sets Gps client start option */
void GeneralConfig::setGpsStartClientOption(const bool newValue)
{
  _gpsStartClient = newValue;
}


/** Gets the Gps Client path */
QString GeneralConfig::getGpsClientPath()
{
  return _installRoot + "/bin/";
}


/** gets Gps last fix latitude */
int GeneralConfig::getGpsLastFixLat() const
{
  return _gpsLastFixLat;
}


/** sets Gps last fix latitude */
void GeneralConfig::setGpsLastFixLat(const int newValue)
{
  _gpsLastFixLat = newValue;
}


/** gets Gps last fix logitude */
int GeneralConfig::getGpsLastFixLon() const
{
  return _gpsLastFixLon;
}


/** sets Gps last fix logitude */
void GeneralConfig::setGpsLastFixLon(const int newValue)
{
  _gpsLastFixLon = newValue;
}


/** gets Gps last fix altitude */
int GeneralConfig::getGpsLastFixAlt() const
{
  return _gpsLastFixAlt;
}


/** sets Gps last fix altitude */
void GeneralConfig::setGpsLastFixAlt(const int newValue)
{
  _gpsLastFixAlt = newValue;
}


/** gets Gps last fix clock */
int GeneralConfig::getGpsLastFixClk() const
{
  return _gpsLastFixClk;
}


/** sets Gps last fix clock */
void GeneralConfig::setGpsLastFixClk(const int newValue)
{
  _gpsLastFixClk = newValue;
}


/** gets minimum sat cout for wind calculation */
int GeneralConfig::getWindMinSatCount() const
{
  return _windMinSatCount;
}


/** sets minimum sat cout for wind calculation */
void GeneralConfig::setMinSatCount(const int newValue)
{
  _windMinSatCount = newValue;
}


/** gets wind altitude range */
int GeneralConfig::getWindAltitudeRange() const
{
  return _windAltitudeRange;
}


/** sets wind altitude range */
void GeneralConfig::setWindAltitudeRange(const int newValue)
{
  _windAltitudeRange = newValue;
}


/** gets wind time range */
int GeneralConfig::getWindTimeRange() const
{
  return _windTimeRange;
}


/** sets wind time range */
void GeneralConfig::setWindTimeRange(const int newValue)
{
  _windTimeRange = newValue;
}


/** gets manual navigation mode altitude */
int GeneralConfig::getManualNavModeAltitude() const
{
  return _manualNavModeAltitude;
}


/** sets manual navigation mode altitude */
void GeneralConfig::setManualNavModeAltitude(const int newValue)
{
  _manualNavModeAltitude = newValue;
}


/** Gets the waypoint file name with path */
QString &GeneralConfig::getWaypointFile()
{
  return _waypointFile;
}


/** Sets the waypoint file name with path */
void GeneralConfig::setWaypointFile( const QString newValue )
{
  _waypointFile = newValue;
}


/** Gets the unit for altitude*/
int GeneralConfig::getUnitAlt() const
{
  return _unitAlt;
}


/** Sets the unit for altitude */
void GeneralConfig::setUnitAlt(const int newValue)
{
  _unitAlt = newValue;
}


/** Gets the unit for distance*/
int GeneralConfig::getUnitDist() const
{
  return _unitDist;
}


/** Sets the unit for distance */
void GeneralConfig::setUnitDist(const int newValue)
{
  _unitDist = newValue;
}


/** Gets the unit for speed */
int GeneralConfig::getUnitSpeed() const
{
  return _unitSpeed;
}


/** Sets the unit for speed */
void GeneralConfig::setUnitSpeed(const int newValue)
{
  _unitSpeed = newValue;
}


/** Gets the unit for vario */
int GeneralConfig::getUnitVario() const
{
  return _unitVario;
}


/** Sets the unit for vario */
void GeneralConfig::setUnitVario(const int newValue)
{
  _unitVario = newValue;
}


/** Gets the unit for wind */
int GeneralConfig::getUnitWind() const
{
  return _unitWind;
}


/** Sets the unit for wind */
void GeneralConfig::setUnitWind(const int newValue)
{
  _unitWind = newValue;
}


/** Gets the unit for position */
int GeneralConfig::getUnitPos() const
{
  return _unitPos;
}


/** Sets the unit for position */
void GeneralConfig::setUnitPos(const int newValue)
{
  _unitPos = newValue;
}


bool GeneralConfig::getAirspaceDrawingEnabled (BaseMapElement::objectType type) const
{
  if( type >0 && type < BaseMapElement::objectTypeSize ) {
    // cout << "return=" << _airspaceDrawingEnabled [type] << endl;
    return _airspaceDrawingEnabled [type];
  } else {
    // cout << "objectTypeSize out of bounds type=" << type <<endl;
    return false;
  }
};


int GeneralConfig::getAirspaceFillingVertical(Airspace::ConflictType nearness)
{
  return _verticalAirspaceFillings[nearness];
}

int GeneralConfig::getAirspaceFillingLateral(Airspace::ConflictType nearness)
{
  return _lateralAirspaceFillings[nearness];
}

void GeneralConfig::setAirspaceFillingVertical(Airspace::ConflictType nearness, int filling)
{
  _verticalAirspaceFillings[nearness] = qMax(0, qMin(100, filling));
}

void GeneralConfig::setAirspaceFillingLateral(Airspace::ConflictType nearness, int filling)
{
  _lateralAirspaceFillings[nearness] = qMax(0, qMin(100, filling));
}

/* Load a pixmap from the cache. If not contained there, insert it. */
QPixmap GeneralConfig::loadPixmap( const QString& pixmapName )
{
  // determine absolute path to pixmap directory
  QString path( _installRoot + "/icons/" + pixmapName );
  QString emptyPath( _installRoot + "/icons/empty.xpm" );

  QPixmap pm;

  if( !QPixmapCache::find( path, pm ) )
    {
      if( ! pm.load( path ) )
        {
          qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
                    path.toLatin1().data() );
        }

      QPixmapCache::insert( path, pm );
    }
  else if( !QPixmapCache::find( emptyPath, pm ) )
    {
      if( ! pm.load( path ) )
        {
          qWarning( "Could not load fallback Pixmap file '%s'. Maybe it was not installed?",
                    emptyPath.toLatin1().data() );
        }

      QPixmapCache::insert( path, pm );
    }


  return pm;
}

/**
 * @removes a pixmap from the global cache
 */
void GeneralConfig::removePixmap( const QString& pixmapName )
{
  // determine absolute path to pixmap directory and remove pixmap
  QPixmapCache::remove( _installRoot + "/icons/" + pixmapName );
}

/**
 * Sets map root directory. All needed subdirectories are created
 * if they are missing.
 */
void GeneralConfig::setMapRootDir( QString newValue )
{
  if( newValue.isEmpty() )
    {
      // New directory name is empty, fall back to a default one.
      newValue = getUserDefaultRootDir() + "/maps";
    }
  else
    {
      QDir testMapDir( newValue );

      if( ! testMapDir.exists() )
        {
          qWarning( "Cannot create map root directory %s! Fall back to default.",
                      _mapRootDir.toLatin1().data() );

          if( ! testMapDir.mkpath( newValue ) )
            {
              // New directory not creatable, fall back to a default one.
              newValue = getUserDefaultRootDir() + "/maps";
            }
        }
    }

  _mapRootDir = newValue;

  QDir mapDir = QDir( _mapRootDir );

  if( ! mapDir.exists() )
    {
      // try to create it
      if( ! mapDir.mkpath( _mapRootDir ) )
        {
          qWarning( "Cannot create user map root directory %s!",
                      _mapRootDir.toLatin1().data() );
          return;
        }
    }

  // Check existence of subdirectories and create all missing one.
  QStringList subDirs;
  subDirs << _mapRootDir + "/airfields"
          << _mapRootDir + "/airspaces"
          << _mapRootDir + "/landscape";

  for( int i = 0; i < subDirs.size(); i++ )
    {
      mapDir.setPath( subDirs.at(i) );

      if( mapDir.exists() == false )
        {
          mapDir.mkpath( subDirs.at(i) );
        }
    }
}

/** Gets the user's default root directory. This is the root for the data
 * and map directories.
 */
QString GeneralConfig::getUserDefaultRootDir()
{
  QString root = QDir::homePath();

#ifdef MAEMO

  QStringList paths;

  // Look if MMCs are mounted
  paths << "/media/mmc1"
        << "/media/mmc2";

  // Check, if root path is mounted.
  for( int i = 0; i < paths.size(); i++ )
    {
      if( ! HwInfo::isMounted( paths.at(i)) )
        {
          continue;
        }

      root = paths.at(i);

      break;
    }

  // That is the fall back solution but normally unfit for map files on a
  // N800 or N810 due to the limited space on that file system.
  // For N900 it is a good location because the file system lays there
  // on the internal MMC.
  if( root == QDir::homePath() )
    {
      root += "/MyDocs"; // Maemo user default directory
    }

#endif

  root += "/Cumulus"; // Cumulus default root directory

  return root;
}

/** Returns the expected places of map directories
    There are: 1. Map directory defined by user
               2. $HOME/cumulus/maps (desktop) or
                  /media/mmc.../Cumulus/maps resp. $HOME/MyDocs/Cumulus/maps (Maemo)
*/
QStringList GeneralConfig::getMapDirectories()
{
  QStringList mapDirs;
  QString     mapDefault = getUserDefaultRootDir() + "/maps";

  // First takes defined map directory
  if(  ! _mapRootDir.isEmpty() )
    {
      mapDirs << _mapRootDir;
    }

  // Next follows default map directory
  if( _mapRootDir != mapDefault )
    {
      // add only if different
      mapDirs << mapDefault;
    }

  return mapDirs;
}

/** gets the user data directory where waypoint file, task file,
    logger files are stored */
QString GeneralConfig::getUserDataDirectory()
{
  // Check, if directory exists
  QDir userDir = QDir( _userDataDirectory );

  if( ! userDir.exists() )
    {
      // try to create it
      if( ! userDir.mkpath( _userDataDirectory ) )
        {
          // user data directory is not createable, fall back to the default one
          _userDataDirectory = getUserDefaultRootDir();
          userDir.mkpath( _userDataDirectory );
          qWarning( "Cannot create user data directory %s",
                    _userDataDirectory.toLatin1().data() );
        }
    }

  return _userDataDirectory;
}

/** Sets the user data directory where waypoint file, task file,
    glider.pol,logger files are stored */
void GeneralConfig::setUserDataDirectory( QString newDir )
{
  if( newDir.isEmpty() )
    {
      // No directory defined, we do fall back to the default one.
      _userDataDirectory = getUserDefaultRootDir();
    }
  else
    {
      _userDataDirectory = newDir;
    }

  // Check, if directory exists
  QDir userDir = QDir( newDir );

  if( ! userDir.exists() )
    {
      // try to create it
      if( ! userDir.mkpath( newDir ) )
        {
          qWarning( "Cannot create user data directory %s", newDir.toLatin1().data() );
          // user data directory not creatable, fallback to the default one
          _userDataDirectory = getUserDefaultRootDir();
          userDir.mkpath( _userDataDirectory );
        }
    }
}

/** Get the GPS default device depending on the hardware type. */
QString GeneralConfig::getGpsDefaultDevice()
{
  if( HwInfo::instance()->getSubType() == HwInfo::n800 )
    {
      return MAEMO_LOCATION_SERVICE;
    }

  if( HwInfo::instance()->getSubType() == HwInfo::n810 )
    {
      return MAEMO_LOCATION_SERVICE;
    }

  if( HwInfo::instance()->getSubType() == HwInfo::n900 )
    {
      return MAEMO_LOCATION_SERVICE;
    }

  // Default in unknown case is the serial device
  return "/dev/ttyS0";
}

/**
 * Tries to get the default proxy setting from the environment. If nothing
 * is defined an empty string is returned.
 *  */
QString GeneralConfig::getDefaultProxy()
{
  char* proxy = getenv("http_proxy");

  if( proxy )
    {
      QString qProxy(proxy);

      // remove an existing http prefix
      return qProxy.remove("http://");
    }

  return "";
}

/**
 * Sets the GUI style, selected by the user.
 * Overwrites some GUI Style elements under Maemo to make them user friendly.
 */
void GeneralConfig::setOurGuiStyle()
{
  qDebug() << "Setting GuiStyle:" << _guiStyle;

#ifdef MAEMO

  QStyle* style = QApplication::setStyle( _guiStyle );
  QApplication::setStyle( new MaemoProxyStyle( style ) );

#else

  QApplication::setStyle( _guiStyle );

#endif
}

/** Sets the terrain color at position index */
void GeneralConfig::setTerrainColor( const QColor& newValue, const ushort index )
{
  if( index < SIZEOF_TERRAIN_COLORS )
    {
      _terrainColors[index] = newValue;
    }
  else
    {
      qWarning( "GeneralConfig::getTerrainColor(): Index %d out of range", index );
    }
}

/** Gets the terrain color at position index */
QColor& GeneralConfig::getTerrainColor( const ushort index )
{
  static QColor defaultColor;

  if( index < SIZEOF_TERRAIN_COLORS )
    {
      return _terrainColors[index];
    }

  qWarning( "GeneralConfig::getTerrainColor(): Index %d out of range", index );
  return defaultColor;
}

/** loads the terrain default colors */
void GeneralConfig::loadTerrainDefaultColors()
{
  _terrainDefaultColors[0] = COLOR_LEVEL_SUB.name();
  _terrainDefaultColors[1] = COLOR_LEVEL_0.name();
  _terrainDefaultColors[2] = COLOR_LEVEL_10.name();
  _terrainDefaultColors[3] = COLOR_LEVEL_25.name();
  _terrainDefaultColors[4] = COLOR_LEVEL_50.name();
  _terrainDefaultColors[5] = COLOR_LEVEL_75.name();
  _terrainDefaultColors[6] = COLOR_LEVEL_100.name();
  _terrainDefaultColors[7] = COLOR_LEVEL_150.name();
  _terrainDefaultColors[8] = COLOR_LEVEL_200.name();
  _terrainDefaultColors[9] = COLOR_LEVEL_250.name();
  _terrainDefaultColors[10] = COLOR_LEVEL_300.name();
  _terrainDefaultColors[11] = COLOR_LEVEL_350.name();
  _terrainDefaultColors[12] = COLOR_LEVEL_400.name();
  _terrainDefaultColors[13] = COLOR_LEVEL_450.name();
  _terrainDefaultColors[14] = COLOR_LEVEL_500.name();
  _terrainDefaultColors[15] = COLOR_LEVEL_600.name();
  _terrainDefaultColors[16] = COLOR_LEVEL_700.name();
  _terrainDefaultColors[17] = COLOR_LEVEL_800.name();
  _terrainDefaultColors[18] = COLOR_LEVEL_900.name();
  _terrainDefaultColors[19] = COLOR_LEVEL_1000.name();
  _terrainDefaultColors[20] = COLOR_LEVEL_1250.name();
  _terrainDefaultColors[21] = COLOR_LEVEL_1500.name();
  _terrainDefaultColors[22] = COLOR_LEVEL_1750.name();
  _terrainDefaultColors[23] = COLOR_LEVEL_2000.name();
  _terrainDefaultColors[24] = COLOR_LEVEL_2250.name();
  _terrainDefaultColors[25] = COLOR_LEVEL_2500.name();
  _terrainDefaultColors[26] = COLOR_LEVEL_2750.name();
  _terrainDefaultColors[27] = COLOR_LEVEL_3000.name();
  _terrainDefaultColors[28] = COLOR_LEVEL_3250.name();
  _terrainDefaultColors[29] = COLOR_LEVEL_3500.name();
  _terrainDefaultColors[30] = COLOR_LEVEL_3750.name();
  _terrainDefaultColors[31] = COLOR_LEVEL_4000.name();
  _terrainDefaultColors[32] = COLOR_LEVEL_4250.name();
  _terrainDefaultColors[33] = COLOR_LEVEL_4500.name();
  _terrainDefaultColors[34] = COLOR_LEVEL_4750.name();
  _terrainDefaultColors[35] = COLOR_LEVEL_5000.name();
  _terrainDefaultColors[36] = COLOR_LEVEL_5250.name();
  _terrainDefaultColors[37] = COLOR_LEVEL_5500.name();
  _terrainDefaultColors[38] = COLOR_LEVEL_5750.name();
  _terrainDefaultColors[39] = COLOR_LEVEL_6000.name();
  _terrainDefaultColors[40] = COLOR_LEVEL_6250.name();
  _terrainDefaultColors[41] = COLOR_LEVEL_6500.name();
  _terrainDefaultColors[42] = COLOR_LEVEL_6750.name();
  _terrainDefaultColors[43] = COLOR_LEVEL_7000.name();
  _terrainDefaultColors[44] = COLOR_LEVEL_7250.name();
  _terrainDefaultColors[45] = COLOR_LEVEL_7500.name();
  _terrainDefaultColors[46] = COLOR_LEVEL_7750.name();
  _terrainDefaultColors[47] = COLOR_LEVEL_8000.name();
  _terrainDefaultColors[48] = COLOR_LEVEL_8250.name();
  _terrainDefaultColors[49] = COLOR_LEVEL_8500.name();
  _terrainDefaultColors[50] = COLOR_LEVEL_8750.name();
}

#if 0
/** helper method to create the color definitions */
void GeneralConfig::printIsoColorDefinitions()
{
  int isoLines[] =
  {
    0, 10, 25, 50, 75, 100, 150, 200, 250,
    300, 350, 400, 450, 500, 600, 700, 800, 900, 1000, 1250, 1500, 1750,
    2000, 2250, 2500, 2750, 3000, 3250, 3500, 3750, 4000, 4250, 4500,
    4750, 5000, 5250, 5500, 5750, 6000, 6250, 6500, 6750, 7000, 7250,
    7500, 7750, 8000, 8250, 8500, 8750
  };

  bool ok;
  QString cn;
  QString rot;
  QString gruen;
  QString blau;

  cn = _terrainColors[0].name(); // color name as '#RRGGBB'

  rot = cn.mid(1, 2);
  gruen = cn.mid(3, 2);
  blau = cn.mid(5, 2);

  printf("\n#define COLOR_LEVEL_SUB QColor(%d, %d, %d)\n",
          rot.toInt(&ok, 16),
          gruen.toInt(&ok, 16),
          blau.toInt(&ok, 16) );

  for( int i=0; i < 50; i++ )
    {
      cn = _terrainColors[i+1].name(); // color name as '#RRGGBB'

      rot = cn.mid(1, 2);
      gruen = cn.mid(3, 2);
      blau = cn.mid(5, 2);

      printf("#define COLOR_LEVEL_%d QColor(%d, %d, %d)\n",
              isoLines[i],
              rot.toInt(&ok, 16),
              gruen.toInt(&ok, 16),
              blau.toInt(&ok, 16) );
    }
}
#endif

/** Gets waypoint scale border. */
int GeneralConfig::getWaypointScaleBorder( const wayPoint::Importance importance) const
{
  switch( importance )
  {
    case wayPoint::Low:
    case wayPoint::Normal:
    case wayPoint::High:

      // qDebug("Importance=%d, value=%d", importance, _wayPointScaleBorders[importance] );

      return _wayPointScaleBorders[importance];
      break;

    default:
      qWarning("getWaypointScaleBorder(): Undefined Importance=%d passed!", importance );
      return 0;
  }
}

/** Sets waypoint scale border. */
void GeneralConfig::setWaypointScaleBorder( const wayPoint::Importance importance,
                                            const int newScale )
{
  switch( importance )
  {
    case wayPoint::Low:
    case wayPoint::Normal:
    case wayPoint::High:
      _wayPointScaleBorders[importance] = newScale;
      break;

    default:
      qWarning("setWaypointScaleBorder(): Undefined Importance=%d passed!", importance );
      break;
  }
}

/**
 * Sets the language for the GUI and for the Qt libraries.
 * This was a helpful link :-) concerning that:
 *
 * http://flylib.com/books/en/2.18.1.93/1/
 *
 */
void GeneralConfig::setLanguage( const QString& newValue )
{
  static bool first = true; // first call flag

  qDebug() << "GeneralConfig::setLanguage" << newValue;

  if( first == false && _language == newValue )
    {
      // No change of language, ignore this call.
      return;
    }

  first     = false;
  _language = newValue;

  if( _language == "en" )
    {
      // That is a reset to the default language English.
      if( cumulusTranslator )
        {
          QCoreApplication::removeTranslator( cumulusTranslator );
          delete cumulusTranslator;
          cumulusTranslator = static_cast<QTranslator *> (0);
        }

      if( qtTranslator )
        {
          QCoreApplication::removeTranslator( qtTranslator );
          delete qtTranslator;
          qtTranslator = static_cast<QTranslator *> (0);
        }

      return;
    }

  if( ! _language.isEmpty() )
    {
      QString langFile = "cumulus_" + _language + ".qm";
      QString langDir = _installRoot + "/locale/" + _language;

      // Load GUI translation file
      if( ! cumulusTranslator )
        {
          cumulusTranslator = new QTranslator;
        }

      if( cumulusTranslator->load( langFile, langDir ) )
        {
          QCoreApplication::installTranslator( cumulusTranslator );
          qDebug() << "Using GUI translation file"
                   << langFile
                   << "for language"
                   << _language;
        }
      else
        {
          qWarning() << "No GUI translation file found in" << langDir;
        }

      // Load Qt library translation file, e.g. qt_de.qm
      langFile = "qt_" + _language + ".qm";

#ifdef MAEMO5
      // MAEMO5 stores here the language files of Qt.
      langDir = "/usr/share/qt4/translations";
#endif

      // Load library translation file
      if( ! qtTranslator )
        {
          qtTranslator = new QTranslator;
        }

      if( qtTranslator->load( langFile, langDir ) )
        {
          QCoreApplication::installTranslator( qtTranslator );
          qDebug() << "Using Library translation file"
                   << langFile
                   << "for language"
                   << _language;
        }
      else
        {
          qWarning() << "No Library translation file found in" << langDir;
        }
    }
}
