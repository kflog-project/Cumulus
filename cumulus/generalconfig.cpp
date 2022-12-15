/***********************************************************************
 **
 **   generalconfig.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2004      by André Somers
 **                   2007-2022 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <cstdlib>
#include <cmath>
#include <iostream>

#include <QApplication>

#include "altitude.h"
#include "distance.h"
#include "generalconfig.h"
#include "gpsnmea.h"
#include "hwinfo.h"
#include "mapconfig.h"
#include "preflightwaypointpage.h"
#include "speed.h"
#include "time_cu.h"

#ifdef ANDROID
#include "androidstyle.h"
#endif

extern MapConfig* _globalMapConfig;

// define static instance as null pointer
GeneralConfig* GeneralConfig::_theInstance = 0;

// define LiveTrack server list
QStringList GeneralConfig::_liveTrackServerList =
    QStringList() << "http://www.livetrack24.com"
                  << "http://test.livetrack24.com"
                  << "http://livexc.dhv.de"
                  << "skylines.aero";

// define pressure devices list
QStringList GeneralConfig::_pressureDevicesList =
#ifdef ANDROID
    QStringList() << "Android"
                  << "Cambridge"
#else
    QStringList() << "Cambridge"
#endif
                  << "Flarm"
                  << "Garmin"
                  << "LX"
                  << "OpenVario"
                  << "Volkslogger"
                  << "XCVario";

// @AP: We derive here from QSettings as base class. The config
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
  _appRoot = value("InstallRoot", "./").toString();
  _builtDate = value("BuiltDate", __DATE__).toString();
  endGroup();

  // Main window properties
  beginGroup("MainWindow");

  _windowSize            = value("Geometry", QSize(800, 480)).toSize();
  _mapSideFrameColor     = QColor( value("MapSideFrameColor", INFOBOX_FRAME_COLOR).toString() );

#ifdef ANDROID
  _guiStyle              = value("Style", "Android").toString();
#else
#if QT_VERSION < 0x050000
  _guiStyle              = value("Style", "Plastique").toString();
#else
  _guiStyle              = value("Style", "fusion").toString();
#endif
#endif

  _guiFont               = value("Font", "").toString();
  _guiMenuFont           = value("MenuFont", "").toString();
  _virtualKeyboard       = value("VirtualKeyboard", false).toBool();
  _screenSaverSpeedLimit = value("ScreenSaverSpeedLimit", 10).toDouble();
  _resetConfiguration    = value("ResetConfiguration", 0).toInt();
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  _awd.horClose             = value("HorizontalWarningDistance", 2000.0).toDouble();
  _awd.horVeryClose         = value("HorizontalWarningDistanceVC", 1000.0).toDouble();
  _awd.verAboveClose        = value("VerticalWarningDistanceAbove", 200.0).toDouble();
  _awd.verAboveVeryClose    = value("VerticalWarningDistanceAboveVC", 100.0).toDouble();
  _awd.verBelowClose        = value("VerticalWarningDistanceBelow", 200.0).toDouble() ;
  _awd.verBelowVeryClose    = value("VerticalWarningDistanceBelowVC", 100.0).toDouble();

  _openAIPAirspaceCountries = value("OpenAIPCountries", "").toString();
  _forceDrawing             = value("forceLowAirspaceDrawing", true ).toBool();
  _forceDrawingDistance     = value("forceLowAirspaceDrawingDistance", 150.0).toDouble();
  _airspaceFileList         = value("FileList", QStringList(QString("All"))).toStringList();

  _asNoDrawing              = value("AirspaceNoDrawing", false ).toBool();;
  _asDrawingBorder          = value("AirspaceNoDrawingBorder", 100).toInt();

  _airspaceLineWidth        = value( "AirSpaceBorderLineWidth", AirSpaceBorderLineWidth ).toInt();
  _airspaceFiltersFileName  = value( "AirspaceFilters", "airspace_filters.txt").toString();

  // Airspace drawing types, set all to true as default
  for( int i=0; i < BaseMapElement::objectTypeSize; i++ )
    {
      _mapDrawingEnabled[i] = true;
    }

  _mapDrawingEnabled[BaseMapElement::AirA]         = value("checkAirspaceA", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirB]         = value("checkAirspaceB", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirC]         = value("checkAirspaceC", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirD]         = value("checkAirspaceD", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirE]         = value("checkAirspaceE", false).toBool();
  _mapDrawingEnabled[BaseMapElement::AirF]         = value("checkAirspaceF", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirFir]       = value("checkAirspaceFir", true).toBool();
  _mapDrawingEnabled[BaseMapElement::AirG]         = value("checkAirspaceG", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Ctr]          = value("checkControl", true).toBool();
  _mapDrawingEnabled[BaseMapElement::ControlC]     = value("checkControlC", true).toBool();
  _mapDrawingEnabled[BaseMapElement::ControlD]     = value("checkControlD", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Restricted]   = value("checkRestricted", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Danger]       = value("checkDanger", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Prohibited]   = value("checkProhibited", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Rmz]          = value("checkRMZ", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Tmz]          = value("checkTMZ", true).toBool();
  _mapDrawingEnabled[BaseMapElement::Sua]          = value("checkSUA", true).toBool();
  _mapDrawingEnabled[BaseMapElement::WaveWindow]   = value("checkWaveWindow", false).toBool();
  _mapDrawingEnabled[BaseMapElement::GliderSector] = value("checkGliderSector", true).toBool();

  // Airspace border draw color
  _borderColorAirspaceA      = QColor( value("borderColorAirspaceA", AIRA_COLOR).toString() );
  _borderColorAirspaceB      = QColor( value("borderColorAirspaceB", AIRB_COLOR).toString() );
  _borderColorAirspaceC      = QColor( value("borderColorAirspaceC", AIRC_COLOR).toString() );
  _borderColorAirspaceD      = QColor( value("borderColorAirspaceD", AIRD_COLOR).toString() );
  _borderColorAirspaceE      = QColor( value("borderColorAirspaceE", AIRE_COLOR).toString() );
  _borderColorAirspaceF      = QColor( value("borderColorAirspaceF", AIRF_COLOR).toString() );
  _borderColorAirspaceFlarm  = QColor( value("borderColorAirspaceFlarm", AIRFLARM_COLOR).toString() );
  _borderColorAirspaceFir    = QColor( value("borderColorAirspaceFir", AIRFIR_COLOR).toString() );
  _borderColorAirspaceG      = QColor( value("borderColorAirspaceG", AIRG_COLOR).toString() );
  _borderColorWaveWindow     = QColor( value("borderColorWaveWindow", WAVE_WINDOW_COLOR).toString() );
  _borderColorControl        = QColor( value("borderColorControl", CTR_COLOR).toString() );
  _borderColorControlC       = QColor( value("borderColorControlC", CTRC_COLOR).toString() );
  _borderColorControlD       = QColor( value("borderColorControlD", CTRD_COLOR).toString() );
  _borderColorRestricted     = QColor( value("borderColorRestricted", RESTRICTED_COLOR).toString() );
  _borderColorDanger         = QColor( value("borderColorDanger", DANGER_COLOR).toString() );
  _borderColorProhibited     = QColor( value("borderColorProhibited", PROHIBITED_COLOR).toString() );
  _borderColorRMZ            = QColor( value("borderColorRMZ", RMZ_COLOR).toString() );
  _borderColorTMZ            = QColor( value("borderColorTMZ", TMZ_COLOR).toString() );
  _borderColorSUA            = QColor( value("borderColorSUA", SUA_COLOR).toString() );
  _borderColorGliderSector   = QColor( value("borderColorGliderSector", GLIDER_SECTOR_COLOR).toString() );

  // Airspace fill (brush) draw color
  _fillColorAirspaceA     = QColor( value("fillColorAirspaceA", AIRA_BRUSH_COLOR).toString() );
  _fillColorAirspaceB     = QColor( value("fillColorAirspaceB", AIRB_BRUSH_COLOR).toString() );
  _fillColorAirspaceC     = QColor( value("fillColorAirspaceC", AIRC_BRUSH_COLOR).toString() );
  _fillColorAirspaceD     = QColor( value("fillColorAirspaceD", AIRD_BRUSH_COLOR).toString() );
  _fillColorAirspaceE     = QColor( value("fillColorAirspaceE", AIRE_BRUSH_COLOR).toString() );
  _fillColorAirspaceF     = QColor( value("fillColorAirspaceF", AIRF_BRUSH_COLOR).toString() );
  _fillColorAirspaceFlarm = QColor( value("fillColorAirspaceFlarm", AIRFLARM_BRUSH_COLOR).toString() );
  _fillColorAirspaceFir   = QColor( value("fillColorAirspaceFir", AIRFIR_BRUSH_COLOR).toString() );
  _fillColorAirspaceG     = QColor( value("fillColorAirspaceG", AIRG_BRUSH_COLOR).toString() );
  _fillColorWaveWindow    = QColor( value("fillColorWaveWindow", WAVE_WINDOW_BRUSH_COLOR).toString() );
  _fillColorControl       = QColor( value("fillColorControl", CTR_BRUSH_COLOR).toString() );
  _fillColorControlC      = QColor( value("fillColorControlC", CTRC_BRUSH_COLOR).toString() );
  _fillColorControlD      = QColor( value("fillColorControlD", CTRD_BRUSH_COLOR).toString() );
  _fillColorRestricted    = QColor( value("fillColorRestricted", RESTRICTED_BRUSH_COLOR).toString() );
  _fillColorDanger        = QColor( value("fillColorDanger", DANGER_BRUSH_COLOR).toString() );
  _fillColorProhibited    = QColor( value("fillColorProhibited", PROHIBITED_BRUSH_COLOR).toString() );
  _fillColorRMZ           = QColor( value("fillColorRMZ", RMZ_BRUSH_COLOR).toString() );
  _fillColorTMZ           = QColor( value("fillColorTMZ", TMZ_BRUSH_COLOR).toString() );
  _fillColorSUA           = QColor( value("fillColorSUA", SUA_BRUSH_COLOR).toString() );
  _fillColorGliderSector  = QColor( value("fillColorGliderSector", GLIDER_SECTOR_BRUSH_COLOR).toString() );

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
  _homeCountryCode   = value("HomeCountryCode", "").toString();
  _userDataDirectory = value("UserDataDir", "").toString();
  endGroup();

  // in Flight Settings
  beginGroup("Flight Settings");
  _useExternalMcAndBugs = value( "ExternalMcAndBugs", true ).toBool();
  endGroup();

  // Pre-flight settings
  beginGroup("Preflight Data");
  _safetyAltitude.setMeters(  value( "Arrival Altitude", 250.0 ).toDouble() );
  _arrivalAltitudeDisplay = (enum ArrivalAltitudeDisplay) value( "ArrivalAltitudeDisplay",
                                    GeneralConfig::landingTarget ).toInt();
  _qnh                    = value( "QNH", 1013 ).toInt();
  _bRecordInterval        = value( "B-RecordLoggerInterval", 3 ).toInt();
  _kRecordInterval        = value( "K-RecordLoggerInterval", 0 ).toInt();
  _loggerAutostartMode    = value( "LoggerAutostartMode", true ).toBool();
  _tas                    = Speed(value( "TAS", 100.0 / 3.6 ).toDouble());
  _currentTaskName        = value( "CurrentTask", "").toString();
  _flightLogbookFileName  = value( "FlightLogbookFileName", "cumulus-logbook.txt" ).toString();
  _autoLoggerStartSpeed   = value( "AutoLoggerStartSpeed", 35.0).toDouble();
  endGroup();

  beginGroup("Wind");
  _manualWindSpeed     = Speed(value( "Speed", 0 ).toDouble());
  _manualWindDirection = value( "Direction", 0 ).toInt();
  _manualWindIsEnabled = value( "ManualWindEnabled", false ).toBool();
  _externalWindIsEnabled = value( "ExternalWindEnabled", false ).toBool();
  _sfWCIsEnabled = value( "SfWindCalculationEnabled", false ).toBool();
  _startWindCalcInStraightFlight = value( "SfCalculationStart", 10 ).toInt();
  _minimumAirSpeed4WC = Speed(value( "SfMinimumAirSpeed4WC", 25.0 / 3.6 ).toDouble());
  _speedTolerance4WC = Speed(value( "SfSpeedTolerance4WC", 10.0 / 3.6).toDouble());
  _headingTolerance4WC = value( "SfHeadingTolerance4WC", 5 ).toInt();
  endGroup();

  beginGroup("Retriever");
  _retrieverMobileNumber   = value( "MobileNumber", "").toString();
  _retrieverPositionFormat = value( "PositionFormat", 0 ).toInt();
  endGroup();

  beginGroup("Preflight Window");
    _closePreFlightMenu = value( "CloseMenu", true ).toBool();
  endGroup();

  beginGroup("LiveTrack24");
  _liveTrackOnOff        = value( "OnOff", false ).toBool();
  _liveTrackInterval     = value( "Interval", 10 ).toInt();
  _liveTrackAirplaneType = value( "AirplaneType", 8 ).toInt();
  _liveTrackIndex        = value( "Index", 0 ).toInt();

  QStringList list;
  list << "" << "" << ""
       << "" << "" << ""
       << "" << "" << ""
       << "" << "" << "";

  list = value("Accounts", list).toStringList();

  for( int i = 0; i < list.size(); i++ )
    {
      if( i % 3 == 0 )
        {
          // Take server url from the current value.
          _liveTrackAccounts[i/3][0] = _liveTrackServerList.at(i/3);
        }
      else
        {
          // Take user name and password from the user configuration.
          _liveTrackAccounts[i/3][i%3] = rot47(list.at(i).toLatin1());
        }
    }

  endGroup();

  // Task scheme settings for circle-sector and nearest-touched
  beginGroup("Task Scheme");
  _taskActiveStartScheme = (enum ActiveTaskFigureScheme) value( "ActiveStartScheme",
                                                         GeneralConfig::Line ).toInt();
  _taskActiveFinishScheme = (enum ActiveTaskFigureScheme) value( "ActiveFinishScheme",
                                                         GeneralConfig::Line ).toInt();
  _taskActiveObsScheme = (enum ActiveTaskFigureScheme) value( "ActiveObserverScheme",
                                                         GeneralConfig::Sector ).toInt();
  _taskActiveSwitchScheme = (enum ActiveTaskSwitchScheme) value( "ActiveSwitchScheme",
                                                         GeneralConfig::Touched ).toInt();

  _reportTaskpointSwitch = value( "ReportTaskpointSwitch", true ).toBool();
  _taskDrawShape         = value( "DrawShape", true ).toBool();
  _taskFillShape         = value( "FillShape", true ).toBool();
  _taskPointAutoZoom     = value( "AutoZoom", true ).toBool();
  _taskShapeAlpha        = value( "ShapeAlpha", 20 ).toInt(); // transparency is in %
  endGroup();

  beginGroup("Task Scheme Start");
  _taskStartLineLength.setMeters( value( "Line", 1000.0 ).toDouble() );
  _taskStartRingRadius.setMeters( value( "Ring", 500.0 ).toDouble() );
  _taskStartSectorIRadius.setMeters( value( "InnerRadius", 0.0 ).toDouble() );
  _taskStartSectorORadius.setMeters( value( "OuterRadius", 3000.0 ).toDouble() );
  _taskStartSectorAngel = value( "Angle", 90).toInt();
  endGroup();

  beginGroup("Task Scheme Finish");
  _taskFinishLineLength.setMeters( value( "Line", 1000.0 ).toDouble() );
  // FAI Sporting code Annex A says, finish ring radius at least 3 km
  _taskFinishRingRadius.setMeters( value( "Ring", 3000.0 ).toDouble() );
  _taskFinishSectorIRadius.setMeters( value( "InnerRadius", 0.0 ).toDouble() );
  _taskFinishSectorORadius.setMeters( value( "OuterRadius", 3000.0 ).toDouble() );
  _taskFinishSectorAngel = value( "Angle", 90).toInt();
  endGroup();

  beginGroup("Task Scheme Observation Zone");
  _taskObsCircleRadius.setMeters( value( "CircleRadius", 500.0 ).toDouble() );
  _taskObsSectorInnerRadius.setMeters( value( "SectorInnerRadius", 0.0 ).toDouble() );
  _taskObsSectorOuterRadius.setMeters( value( "SectorOuterRadius", 3000.0 ).toDouble() );
  _taskObsSectorAngle = value( "SectorAngle", 90).toInt();
  endGroup();

  beginGroup("Task");
  _taskLineWidth = value( "TaskLineWidth", TaskLineWidth ).toInt();
  _taskLineColor = QColor( value("TaskLineColor", TaskLineColor).toString() );
  _targetLineDrawState = value( "TargetLineDrawing", true ).toBool();
  _targetLineWidth = value( "TargetLineWidth", TargetLineWidth ).toInt();

  _taskFiguresColor = QColor( value("TaskFiguresColor", TaskFiguresColor).toString() );
  _taskFiguresLineWidth = value( "TaskFiguresLineWidth", TaskFiguresLineWidth ).toInt();

  _headingLineWidth = value( "HeadingLineWidth", HeadingLineWidth ).toInt();
  _headingLineColor = QColor( value("HeadingLineColor", HeadingLineColor).toString() );
  _headingLineDrawState = value( "HeadingLineDrawing", true ).toBool();
  endGroup();

  beginGroup("Map");
  _mapProjFollowsHome             = value( "ProjectionFollowsHome", true ).toBool();
  _mapUnload                      = value( "UnloadUnneededMap", true ).toBool();
  _downloadMissingMaps            = value( "DownloadMissingMaps", false ).toBool();
  _mapInstallRadius               = value( "MapInstallRadius", 500 ).toInt();
  _mapLoadIsoLines                = value( "LoadIsoLines", true ).toBool();
  _mapShowIsoLineBorders          = value( "ShowIsoLineBorders", false ).toBool();
  _mapLoadRoads                   = value( "LoadRoads", true ).toBool();
  _mapLoadMotorways               = value( "LoadMotorways", true ).toBool();
  _mapLoadRailways                = value( "LoadRailways", true ).toBool();
  _mapLoadCities                  = value( "LoadCities", true ).toBool();
  _mapLoadWaterways               = value( "LoadWaterways", true ).toBool();
  _mapLoadForests                 = value( "LoadForests", false ).toBool();
  _drawTrail                      = value( "DrawTrail", true ).toBool();
  _mapTrailLineColor              = value( "TrailLineColor", TrailLineColor).toString();
  _mapTrailLineWidth              = value( "TrailLineWidth", TrailLineWidth ).toInt();
  _mapShowAirfieldLabels          = value( "ShowAirfieldLabels", false ).toBool();
  _mapShowNavAidsLabels           = value( "ShowNavAidsLabels", false ).toBool();
  _mapShowTaskPointLabels         = value( "ShowTaskPointLabels", false ).toBool();
  _mapShowOutLandingLabels        = value( "ShowOutLandingLabels", false ).toBool();
  _mapShowWaypointLabels          = value( "ShowWaypointLabels", false ).toBool();
  _mapShowLabelsExtraInfo         = value( "ShowLabelsExtraInfo", false ).toBool();
  _mapShowRelBearingInfo          = value( "ShowRelBearingInfo", true ).toBool();
  _mapCurrentTask                 = value( "CurrentTask", "" ).toString();

  _wayPointScaleBorders[Waypoint::Low]    = value( "WpScaleBorderLow", 125 ).toInt();
  _wayPointScaleBorders[Waypoint::Normal] = value( "WpScaleBorderNormal", 250 ).toInt();
  _wayPointScaleBorders[Waypoint::High]   = value( "WpScaleBorderHigh", 500 ).toInt();
  endGroup();

  beginGroup("Map Data");
  _home.setX( value( "Homesite Latitude", HOME_DEFAULT_LAT).toInt() );
  _home.setY( value( "Homesite Longitude", HOME_DEFAULT_LON).toInt() );
  _homeElevation.setMeters( value("Homesite Elevation", 0.0).toDouble() );
  _homeName          = value("Homesite Name", "HOME").toString();
  _mapRootDir        = value("Map Root", "").toString();
  _mapServerUrl      = value("Map Server Url", "http://www.kflog.org/data/landscape/").toString();
  _centerLat         = value("Center Latitude", HOME_DEFAULT_LAT).toInt();
  _centerLon         = value("Center Longitude", HOME_DEFAULT_LON).toInt();
  _mapScale          = value("Map Scale", 200).toDouble();
  _mapProjectionType = value("Projection Type", ProjectionBase::Cylindric ).toInt();

  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QString name = "TerrainColor_" + QString::number(i);
      _terrainColors[i] = QColor( value(name, _terrainDefaultColors[i]).toString() );
    }

  _groundColor = QColor( value("GroundColor", COLOR_LEVEL_GROUND.name()).toString() );
  _elevationColorOffset = value("ElevationColorOffset", 0).toInt();
  endGroup();

  beginGroup("Airfield Data");
  _airfieldSource             = value("Source", 0).toInt(); // openAIP is used
  _airfieldHomeRadius         = value("HomeRadius", 500000).toFloat(); // m is used
  _airfieldRunwayLengthFilter = value("RunwayLengthFilter", 0).toFloat(); // m is used
  _openAipPoiFileList         = value("OpenAipFileList", QStringList(QString("All"))).toStringList();
  _openAipAirfieldCountries   = value("OpenAipAirfieldCountries", "" ).toString();
  // See https://www.openaip.net/docs for more info.
  _openAipLink                = value("OpenAipLink", "https://storage.googleapis.com/download/storage/v1/b/29f98e10-a489-4c82-ae5e-489dbcd4912f/o/%1?alt=media").toByteArray();
  endGroup();

  beginGroup("List Display");
  _listDisplayPageSize = value("List Page Entries", 30).toInt();
  _listDisplayAFMargin = value("Airfield List Row Increase", 10).toInt();
  _listDisplayRPMargin = value("Emergency List Row Increase", 10).toInt();
  endGroup();

  beginGroup("Scale");
  _mapLowerLimit  = value("Lower Limit", VAL_BORDER_L).toInt();
  _mapUpperLimit  = value("Upper Limit", VAL_BORDER_U).toInt();
  _mapBorder1     = VAL_BORDER_1; // value("Border 1", VAL_BORDER_1).toInt();
  _mapBorder2     = VAL_BORDER_2; // value("Border 2", VAL_BORDER_2).toInt();
  _mapBorder3     = VAL_BORDER_3; // value("Border 3", VAL_BORDER_3).toInt();
  _mapSwitchScale = VAL_BORDER_S; // value("Switch Scale", VAL_BORDER_S).toInt();
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
  _waypointBinaryFileName    = value( "BinaryDefaultFileName", "cumulus.kwp" ).toString();
  _waypointXmlFileName       = value( "XmlDefaultFileName", "cumulus.kflogwp" ).toString();
  _waypointFileFormat        = (enum WpFileFormat) value( "FileFormat", Binary ).toInt();
  _waypointPriority          = value( "StoragePriority", 0 ).toInt();
  _waypointCenterReference   = value( "CenterReference", PreFlightWaypointPage::Home ).toInt();
  _waypointAirfieldReference = value( "AirfieldReference", "" ).toString();
  _waypointImportRadius      = value( "ImportRadius", "500" ).toString();
  endGroup();

  beginGroup("Variometer");
  _varioIntegrationTime = value( "IntegrationTime", 5 ).toInt();
  _varioTekCompensation = value( "TekCompensation", false ).toBool();
  _varioTekAdjust       = value( "TekAdjust", 0 ).toInt();
  endGroup();

  beginGroup("Altimeter");
  _altimeterToggleMode = value( "Toggling_Mode", false ).toBool();
  _altimeterMode       = value( "Mode", 0 ).toInt();
  _pressureDevice      = value( "PressureDevice", "Flarm" ).toString();
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
  _soundPlayer           = value( "SoundPlayer", SoundPlayer ).toString();
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
  _popupFlarmAlarms      = value( "PopupFlarmAlarms", true ).toBool();
  _blackBgInfoDisplay    = value( "BlackBgInfoDisplay", false ).toBool();
  endGroup();

  beginGroup("GPS");
  _gpsSource          = value( "Source", "$GP" ).toString();
  _gpsDevice          = value( "Device", getGpsDefaultDevice() ).toString();
  _gpsBtDevice        = value( "BT-Device", "" ).toString();
  _gpsSpeed           = value( "Speed", 4800 ).toInt();
  _gpsAltitudeType    = value( "AltitudeType", (int) GpsNmea::GPS ).toInt();
  _gpsAltitudeUserCorrection.setMeters(value( "AltitudeCorrection", 0 ).toDouble());
  _gpsSwitchState     = value( "SwitchState", true ).toBool();
  _gpsSyncSystemClock = value( "SyncSystemClock", false ).toBool();
  _gpsNmeaLogState    = value( "NmeaLogState", false ).toBool();
  _gpsIpcPort         = value( "IpcPort", 0 ).toInt();
  _gpsStartClient     = value( "StartClient", true ).toBool();
  _gpsLastFixLat      = value( "LastFixLat", 0 ).toInt();
  _gpsLastFixLon      = value( "LastFixLon", 0 ).toInt();
  _gpsLastFixAlt      = value( "LastFixAlt", 0 ).toInt();
  _gpsLastFixClk      = value( "LastFixClk", 0 ).toInt();

  _gpsWlanIp1         = value( "WlanIp1", "192.168.4.1" ).toString();
  _gpsWlanPort1       = value( "WlanPort1", "8880" ).toString();
  _gpsWlanPassword1   = value( "WlanPassword1", "" ).toString();
  _gpsWlanPassword1 = rot47( _gpsWlanPassword1.toLatin1() );

  _gpsWlanIp2         = value( "WlanIp2", "192.168.4.1" ).toString();
  _gpsWlanPort2       = value( "WlanPort2", "8881" ).toString();
  _gpsWlanPassword2   = value( "WlanPassword2", "" ).toString();
  _gpsWlanPassword2 = rot47( _gpsWlanPassword2.toLatin1() );
  endGroup();

  beginGroup( "GPS-Status" );
  _gpsFilterIndex = value( "FilterIndex", 0 ).toInt();
  endGroup();

  beginGroup("Wind");
  _windMinSatCount   = value( "MinSatCount", 5 ).toInt();
  endGroup();

  beginGroup ("Calculator");

  _manualNavModeAltitude = value( "ManualNavModeAltitude", 1000 ).toInt();
  _time4LDCalc           = value( "Time4LDCalculation", 30 ).toInt();

  double mc = value( "McCready", -1.0 ).toDouble();

  if( mc != -1.0 )
    {
      _mcCready = Speed( mc );
    }
  else
    {
      _mcCready.setInvalid();
    }

  mc = value( "ExternalMcCready", -1.0 ).toDouble();

    if( mc != -1.0 )
      {
        _mcCreadyExternal = Speed( mc );
      }
    else
      {
        _mcCreadyExternal.setInvalid();
      }

  endGroup();

  beginGroup("Flarm");
  _flarmAliasFileName      = value("AliasFileName", "cumulus-flarm.txt").toString();
  _flarmRadarDrawWindArrow = value("RadarDrawWindArrow", true).toBool();
  endGroup();

  beginGroup("Units");
  _unitAlt         = value( "Altitude", Altitude::meters).toInt();
  _unitDist        = value( "Distance", Distance::kilometers).toInt();
  _unitSpeed       = value( "Speed",    Speed::kilometersPerHour ).toInt();
  _unitVario       = value( "Vario",    Speed::metersPerSecond ).toInt();
  _unitWind        = value( "Wind",     Speed::metersPerSecond ).toInt();
  _unitPos         = value( "Position", WGSPoint::DMS ).toInt();
  _unitTime        = value( "Time",     Time::utc ).toInt();
  _unitTemperature = value( "Temperature", Celsius ).toInt();
  _unitAirPressure = value( "AirPressure", hPa ).toInt();
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
  setValue("InstallRoot", _appRoot );
  setValue("BuiltDate", _builtDate);
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
  setValue("ResetConfiguration", _resetConfiguration);
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  setValue("HorizontalWarningDistance", _awd.horClose.getMeters());
  setValue("HorizontalWarningDistanceVC", _awd.horVeryClose.getMeters());
  setValue("VerticalWarningDistanceAbove", _awd.verAboveClose.getMeters());
  setValue("VerticalWarningDistanceAboveVC", _awd.verAboveVeryClose.getMeters());
  setValue("VerticalWarningDistanceBelow", _awd.verBelowClose.getMeters());
  setValue("VerticalWarningDistanceBelowVC", _awd.verBelowVeryClose.getMeters());

  setValue("OpenAIPCountries", _openAIPAirspaceCountries);
  setValue("forceLowAirspaceDrawing", _forceDrawing);
  setValue("forceLowAirspaceDrawingDistance", _forceDrawingDistance.getMeters());
  setValue("AirspaceNoDrawing", _asNoDrawing);
  setValue("AirspaceNoDrawingBorder", _asDrawingBorder);

  setValue("AirSpaceBorderLineWidth", _airspaceLineWidth);
  setValue("FileList", _airspaceFileList);

  // Airspace warning types
  setValue("checkAirspaceA", _mapDrawingEnabled[BaseMapElement::AirA]);
  setValue("checkAirspaceB", _mapDrawingEnabled[BaseMapElement::AirB]);
  setValue("checkAirspaceC", _mapDrawingEnabled[BaseMapElement::AirC]);
  setValue("checkControl", _mapDrawingEnabled[BaseMapElement::Ctr]);
  setValue("checkControlC", _mapDrawingEnabled[BaseMapElement::ControlC]);
  setValue("checkAirspaceD", _mapDrawingEnabled[BaseMapElement::AirD]);
  setValue("checkControlD", _mapDrawingEnabled[BaseMapElement::ControlD]);
  setValue("checkAirspaceE", _mapDrawingEnabled[BaseMapElement::AirE]);
  setValue("checkWaveWindow", _mapDrawingEnabled[BaseMapElement::WaveWindow]);
  setValue("checkAirspaceF", _mapDrawingEnabled[BaseMapElement::AirF]);
  setValue("checkAirspaceFir", _mapDrawingEnabled[BaseMapElement::AirFir]);
  setValue("checkAirspaceG", _mapDrawingEnabled[BaseMapElement::AirG]);
  setValue("checkRestricted", _mapDrawingEnabled[BaseMapElement::Restricted]);
  setValue("checkDanger", _mapDrawingEnabled[BaseMapElement::Danger]);
  setValue("checkProhibited", _mapDrawingEnabled[BaseMapElement::Prohibited]);
  setValue("checkRMZ", _mapDrawingEnabled[BaseMapElement::Rmz]);
  setValue("checkTMZ", _mapDrawingEnabled[BaseMapElement::Tmz]);
  setValue("checkSUA", _mapDrawingEnabled[BaseMapElement::Sua]);
  setValue("checkGliderSector", _mapDrawingEnabled[BaseMapElement::GliderSector]);

  // Airspace border draw color
  setValue("borderColorAirspaceA",      _borderColorAirspaceA.name());
  setValue("borderColorAirspaceB",      _borderColorAirspaceB.name());
  setValue("borderColorAirspaceC",      _borderColorAirspaceC.name());
  setValue("borderColorAirspaceD",      _borderColorAirspaceD.name());
  setValue("borderColorAirspaceE",      _borderColorAirspaceE.name());
  setValue("borderColorAirspaceF",      _borderColorAirspaceF.name());
  setValue("borderColorAirspaceFlarm",  _borderColorAirspaceFlarm.name());
  setValue("borderColorAirspaceFir",    _borderColorAirspaceFir.name());
  setValue("borderColorAirspaceG",      _borderColorAirspaceG.name());
  setValue("borderColorWaveWindow",     _borderColorWaveWindow.name());
  setValue("borderColorControl",        _borderColorControl.name());
  setValue("borderColorControlC",       _borderColorControlC.name());
  setValue("borderColorControlD",       _borderColorControlD.name());
  setValue("borderColorRestricted",     _borderColorRestricted.name());
  setValue("borderColorDanger",         _borderColorDanger.name());
  setValue("borderColorProhibited",     _borderColorProhibited.name());
  setValue("borderColorRMZ",            _borderColorRMZ.name());
  setValue("borderColorTMZ",            _borderColorTMZ.name());
  setValue("borderColorSUA",            _borderColorSUA.name());
  setValue("borderColorGliderSector",   _borderColorGliderSector.name());

  // Airspace fill (brush) draw color
  setValue("fillColorAirspaceA",      _fillColorAirspaceA.name());
  setValue("fillColorAirspaceB",      _fillColorAirspaceB.name());
  setValue("fillColorAirspaceC",      _fillColorAirspaceC.name());
  setValue("fillColorAirspaceD",      _fillColorAirspaceD.name());
  setValue("fillColorAirspaceE",      _fillColorAirspaceE.name());
  setValue("fillColorAirspaceFlarm",  _fillColorAirspaceFlarm.name());
  setValue("fillColorAirspaceFir",    _fillColorAirspaceFir.name());
  setValue("fillColorAirspaceG",      _fillColorAirspaceG.name());
  setValue("fillColorWaveWindow",     _fillColorWaveWindow.name());
  setValue("fillColorControl",        _fillColorControl.name());
  setValue("fillColorControlC",       _fillColorControlC.name());
  setValue("fillColorControlD",       _fillColorControlD.name());
  setValue("fillColorRestricted",     _fillColorRestricted.name());
  setValue("fillColorDanger",         _fillColorDanger.name());
  setValue("fillColorProhibited",     _fillColorProhibited.name());
  setValue("fillColorRMZ",            _fillColorRMZ.name());
  setValue("fillColorTMZ",            _fillColorTMZ.name());
  setValue("fillColorSUA",            _fillColorSUA.name());
  setValue("fillColorGliderSector",   _fillColorGliderSector.name());

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
  setValue("HomeCountryCode", _homeCountryCode);
  setValue( "UserDataDir", _userDataDirectory);
  setValue("Proxy", _proxy);
  endGroup();

  // in Flight Settings
  beginGroup("Flight Settings");
  setValue( "ExternalMcAndBugs", _useExternalMcAndBugs );
  endGroup();

  // Preflight data
  beginGroup("Preflight Data");
  setValue( "Arrival Altitude", _safetyAltitude.getMeters() );
  setValue( "ArrivalAltitudeDisplay",  _arrivalAltitudeDisplay );
  setValue( "QNH", _qnh );
  setValue( "B-RecordLoggerInterval", _bRecordInterval );
  setValue( "K-RecordLoggerInterval", _kRecordInterval );
  setValue( "LoggerAutostartMode", _loggerAutostartMode );
  setValue( "TAS", _tas.getMps() );
  setValue( "CurrentTask", _currentTaskName);
  setValue( "FlightLogbookFileName", _flightLogbookFileName );
  setValue( "AutoLoggerStartSpeed", _autoLoggerStartSpeed );
  endGroup();

  beginGroup("Wind");
  setValue( "Speed", _manualWindSpeed.getMps() );
  setValue( "Direction", _manualWindDirection );
  setValue( "ManualWindEnabled", _manualWindIsEnabled );
  setValue( "ExternalWindEnabled", _externalWindIsEnabled );
  setValue( "SfWindCalculationEnabled", _sfWCIsEnabled );
  setValue( "SfCalculationStart", _startWindCalcInStraightFlight );
  setValue( "SfMinimumAirSpeed4WC", _minimumAirSpeed4WC.getMps() );
  setValue( "SfSpeedTolerance4WC", _speedTolerance4WC.getMps() );
  setValue( "SfHeadingTolerance4WC", _headingTolerance4WC );
  endGroup();

  beginGroup("Retriever");
  setValue( "MobileNumber", _retrieverMobileNumber );
  setValue( "PositionFormat", _retrieverPositionFormat );
  endGroup();

  beginGroup("Preflight Window");
  setValue( "CloseMenu", _closePreFlightMenu );
  endGroup();

  beginGroup("LiveTrack24");
  setValue( "OnOff", _liveTrackOnOff );
  setValue( "Interval", _liveTrackInterval );
  setValue( "AirplaneType", _liveTrackAirplaneType );
  setValue( "Index", _liveTrackIndex );

  QStringList list;

  for( int i = 0; i < 4; i++ )
    {
      for( int j = 0; j < 3; j++ )
        {
          list << rot47(_liveTrackAccounts[i][j].toLatin1());
        }
    }

  setValue( "Accounts", list );
  endGroup();

  // Task scheme settings for circle-sector and nearest touched
  beginGroup("Task Scheme");
  setValue( "ActiveStartScheme", _taskActiveStartScheme );
  setValue( "ActiveFinishScheme", _taskActiveFinishScheme );
  setValue( "ActiveObserverScheme", _taskActiveObsScheme );
  setValue( "ActiveSwitchScheme", _taskActiveSwitchScheme );
  setValue( "ReportTaskpointSwitch", _reportTaskpointSwitch );
  setValue( "DrawShape", _taskDrawShape );
  setValue( "FillShape", _taskFillShape );
  setValue( "AutoZoom", _taskPointAutoZoom );
  setValue( "ShapeAlpha", _taskShapeAlpha );
  endGroup();

  beginGroup("Task Scheme Start");
  setValue( "Line", _taskStartLineLength.getMeters() );
  setValue( "Ring", _taskStartRingRadius.getMeters() );
  setValue( "InnerRadius", _taskStartSectorIRadius.getMeters() );
  setValue( "OuterRadius", _taskStartSectorORadius.getMeters() );
  setValue( "Angle", _taskStartSectorAngel );
  endGroup();

  beginGroup("Task Scheme Finish");
  setValue( "Line", _taskFinishLineLength.getMeters() );
  setValue( "Ring", _taskFinishRingRadius.getMeters() );
  setValue( "InnerRadius", _taskFinishSectorIRadius.getMeters() );
  setValue( "OuterRadius", _taskFinishSectorORadius.getMeters() );
  setValue( "Angle", _taskFinishSectorAngel);
  endGroup();

  beginGroup("Task Scheme Observation Zone");
  setValue( "CircleRadius",    _taskObsCircleRadius.getMeters() );
  setValue( "SectorInnerRadius", _taskObsSectorInnerRadius.getMeters() );
  setValue( "SectorOuterRadius", _taskObsSectorOuterRadius.getMeters() );
  setValue( "SectorAngle",       _taskObsSectorAngle );
  endGroup();

  beginGroup("Task");
  setValue( "TaskLineWidth", _taskLineWidth );
  setValue( "TaskLineColor", _taskLineColor.name() );
  setValue( "TargetLineDrawing", _targetLineDrawState );
  setValue( "TargetLineWidth", _targetLineWidth );
  setValue( "TaskFiguresColor", _taskFiguresColor.name() );
  setValue( "TaskFiguresLineWidth", _taskFiguresLineWidth );
  setValue( "HeadingLineWidth", _headingLineWidth );
  setValue( "HeadingLineColor", _headingLineColor.name() );
  setValue( "HeadingLineDrawing", _headingLineDrawState );
  endGroup();

  beginGroup("Map");
  setValue( "ProjectionFollowsHome", _mapProjFollowsHome );
  setValue( "UnloadUnneededMap", _mapUnload );
  setValue( "DownloadMissingMaps", _downloadMissingMaps );
  setValue( "MapInstallRadius", _mapInstallRadius );
  setValue( "LoadIsoLines", _mapLoadIsoLines );
  setValue( "ShowIsoLineBorders", _mapShowIsoLineBorders );
  setValue( "ShowWaypointLabels", _mapShowWaypointLabels );
  setValue( "ShowLabelsExtraInfo", _mapShowLabelsExtraInfo );
  setValue( "ShowRelBearingInfo", _mapShowRelBearingInfo );
  setValue( "CurrentTask", _mapCurrentTask );

  setValue( "LoadRoads", _mapLoadRoads );
  setValue( "LoadMotorways", _mapLoadMotorways );
  setValue( "LoadRailways", _mapLoadRailways );
  setValue( "LoadCities", _mapLoadCities   );
  setValue( "LoadWaterways", _mapLoadWaterways );
  setValue( "LoadForests", _mapLoadForests );

  setValue( "DrawTrail", _drawTrail );
  setValue( "TrailColor", _mapTrailLineColor.name() );
  setValue( "TrailLineWidth", _mapTrailLineWidth);

  setValue( "ShowAirfieldLabels", _mapShowAirfieldLabels );
  setValue( "ShowNavAidsLabels", _mapShowNavAidsLabels );
  setValue( "ShowTaskPointLabels", _mapShowTaskPointLabels );
  setValue( "ShowOutLandingLabels", _mapShowOutLandingLabels );
  setValue( "WpScaleBorderLow", _wayPointScaleBorders[Waypoint::Low] );
  setValue( "WpScaleBorderNormal", _wayPointScaleBorders[Waypoint::Normal] );
  setValue( "WpScaleBorderHigh", _wayPointScaleBorders[Waypoint::High] );
  endGroup();

  beginGroup("Map Data");

  setValue("Homesite Latitude", _home.x());
  setValue("Homesite Longitude", _home.y());
  setValue("Homesite Elevation", _homeElevation.getMeters() );
  setValue("Homesite Name", _homeName);
  setValue("Map Root", _mapRootDir);
  setValue("Map Server Url", _mapServerUrl);
  setValue("Center Latitude", _centerLat);
  setValue("Center Longitude", _centerLon);
  setValue("Map Scale", _mapScale);
  setValue("Projection Type", _mapProjectionType);

  for( int i = 0; i < SIZEOF_TERRAIN_COLORS; i++ )
    {
      QString name = "TerrainColor_" + QString::number(i);
      setValue(name, _terrainColors[i].name());
    }

  setValue("GroundColor", _groundColor.name());
  setValue("ElevationColorOffset", _elevationColorOffset);
  endGroup();

  beginGroup("Airfield Data");
  setValue("Source", _airfieldSource);
  setValue("HomeRadius", _airfieldHomeRadius);
  setValue("RunwayLengthFilter", _airfieldRunwayLengthFilter);
  setValue("OpenAipFileList", _openAipPoiFileList);
  setValue("OpenAipAirfieldCountries", _openAipAirfieldCountries);
  // setValue("OpenAipLink", _openAipLink);
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
  setValue( "BinaryDefaultFileName", _waypointBinaryFileName );
  setValue( "XmlDefaultFileName", _waypointXmlFileName );
  setValue( "FileFormat", _waypointFileFormat );
  setValue( "StoragePriority", _waypointPriority );
  setValue( "CenterReference", _waypointCenterReference );
  setValue( "AirfieldReference", _waypointAirfieldReference );
  setValue( "ImportRadius", _waypointImportRadius );
  endGroup();

  beginGroup("Variometer");
  setValue( "IntegrationTime", _varioIntegrationTime );
  setValue( "TekCompensation", _varioTekCompensation );
  setValue( "TekAdjust", _varioTekAdjust );
  endGroup();

  beginGroup("Altimeter");
  setValue( "Toggling_Mode", _altimeterToggleMode );
  setValue( "Mode", _altimeterMode );
  setValue( "PressureDevice", _pressureDevice );
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
  setValue( "PopupFlarmAlarms", _popupFlarmAlarms );
  setValue( "BlackBgInfoDisplay", _blackBgInfoDisplay);
  endGroup();

  beginGroup("GPS");
  setValue( "Source", _gpsSource );
  setValue( "Device", _gpsDevice );
  setValue( "BT-Device", _gpsBtDevice );
  setValue( "Speed", _gpsSpeed );
  setValue( "AltitudeType", _gpsAltitudeType );
  setValue( "AltitudeCorrection", _gpsAltitudeUserCorrection.getMeters() );
  setValue( "SwitchState", _gpsSwitchState );
  setValue( "SyncSystemClock", _gpsSyncSystemClock );
  setValue( "NmeaLogState", _gpsNmeaLogState );
  setValue( "IpcPort", _gpsIpcPort );
  setValue( "StartClient", _gpsStartClient );
  setValue( "LastFixLat", _gpsLastFixLat );
  setValue( "LastFixLon", _gpsLastFixLon );
  setValue( "LastFixAlt", _gpsLastFixAlt );
  setValue( "LastFixClk", _gpsLastFixClk );

  setValue( "WlanIp1", _gpsWlanIp1 );
  setValue( "WlanPort1", _gpsWlanPort1 );
  setValue( "WlanPassword1", rot47( _gpsWlanPassword1.toLatin1() ) );

  setValue( "WlanIp2", _gpsWlanIp2 );
  setValue( "WlanPort2", _gpsWlanPort2 );
  setValue( "WlanPassword2", rot47( _gpsWlanPassword2.toLatin1() ) );
  endGroup();

  beginGroup( "GPS-Status" );
  setValue( "FilterIndex", _gpsFilterIndex );
  endGroup();

  beginGroup("Wind");
  setValue( "MinSatCount", _windMinSatCount );
  endGroup();

  beginGroup("Calculator");
  setValue( "ManualNavModeAltitude", _manualNavModeAltitude );
  setValue( "Time4LDCalculation", _time4LDCalc );

  if( _mcCready.isValid() )
    {
      setValue( "McCready", _mcCready.getMps() );
    }
  else
    {
      setValue( "McCready", -1.0 );
    }

  if( _mcCreadyExternal.isValid() )
    {
      setValue( "ExternalMcCready", _mcCreadyExternal.getMps() );
    }
  else
    {
      setValue( "ExternalMcCready", -1.0 );
    }

  endGroup();

  beginGroup ("Flarm");
  setValue( "RadarDrawWindArrow", _flarmRadarDrawWindArrow );
  endGroup();

  beginGroup("Units");
  setValue( "Altitude", _unitAlt );
  setValue( "Distance", _unitDist );
  setValue( "Speed", _unitSpeed );
  setValue( "Vario", _unitVario );
  setValue( "Wind", _unitWind );
  setValue( "Position", _unitPos );
  setValue( "Time", _unitTime );
  setValue( "Temperature", _unitTemperature );
  setValue( "AirPressure", _unitAirPressure );
  endGroup();

  // Save all to disk
  sync();
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
  return _appRoot + "/bin/";
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


/** gets minimum sat count for wind calculation */
int GeneralConfig::getWindMinSatCount() const
{
  return _windMinSatCount;
}


/** sets minimum sat count for wind calculation */
void GeneralConfig::setMinSatCount(const int newValue)
{
  _windMinSatCount = newValue;
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


bool GeneralConfig::getItemDrawingEnabled (BaseMapElement::objectType type) const
{
  if (type > 0 && type < BaseMapElement::objectTypeSize)
    {
      return _mapDrawingEnabled[type];
    }
  else
    {
      qWarning() << "getItemDrawingEnabled(): unknown object" << type;
      return false;
    }
}

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

QPixmap GeneralConfig::loadPixmap( const QString& pixmapName, const bool doScale )
{
  static bool firstCall = true;
  static QPixmap smallEmptyPixmap( 16, 16 );
  static QPixmap emptyPixmap( 32, 32 );

  if( firstCall )
    {
      firstCall = false;
      smallEmptyPixmap.fill( Qt::transparent );
      emptyPixmap.fill( Qt::transparent );
    }

  float scale = Layout::getScaledDensity();

  // determine absolute path to pixmap directory
  QString path( ":/icons/" + pixmapName );

  // create key for cache access
  QString key(path);

  if( doScale == true )
    {
      key += QString::number(scale, 'f', 3);
    }

  //  qDebug() << "loadPixmap:" << pixmapName << "doScale" << doScale
  //           << "scale=" << scale << "key=" << key;

  QPixmap pm;

  if( QPixmapCache::find( key, pm ) )
    {
      return pm;
    }

  if( pm.load( path ) )
    {
      if( doScale == true && scale > 1.0 )
	{
	  pm = pm.scaled( pm.width() * scale, pm.height() * scale,
	                  Qt::KeepAspectRatio,
	                  Qt::SmoothTransformation );
	}

      QPixmapCache::insert( key, pm );

      // qDebug() << "loadPixmap:" << pixmapName << "w=" << pm.width() << "h=" << pm.height();

      return pm;
    }

  qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
             path.toLatin1().data() );

  // Return an empty transparent pixmap as default
  if( pixmapName.startsWith( "small/") )
    {
      return smallEmptyPixmap;
    }

  return emptyPixmap;
}

QPixmap GeneralConfig::loadPixmapAutoScaled( const QString& pixmapName )
{
  static bool firstCall = true;
  static QPixmap emptyPixmap( 32, 32 );

  if( firstCall )
    {
      firstCall = false;
      emptyPixmap.fill( Qt::transparent );
    }

  if( pixmapName == "empty.xpm" )
    {
      return emptyPixmap;
    }

  float scale = Layout::getScaledDensity();

  if( _globalMapConfig->useSmallIcons() == true )
    {
      scale /= 2.0;
    }

  // determine absolute path to pixmap directory
  QString path( ":/icons/" + pixmapName );

  // create key for cache access
  QString key( path + QString::number(scale, 'f', 3) );

  QPixmap pm;

  if( QPixmapCache::find( key, pm ) )
    {
      return pm;
    }

  if( pm.load( path ) )
    {
      if( scale != 1.0 )
	{
	  pm = pm.scaled( pm.width() * scale, pm.height() * scale,
	                  Qt::KeepAspectRatio,
	                  Qt::SmoothTransformation );
	}

      QPixmapCache::insert( key, pm );
      return pm;
    }

  qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
             path.toLatin1().data() );

  // Return an empty transparent pixmap as default
  return emptyPixmap;
}

QPixmap GeneralConfig::loadPixmap( const QString& pixmapName, int size )
{
  // determine absolute path to pixmap directory
  QString path( ":/icons/" + pixmapName );
  QString cacheKey( path + QString::number(size, 'f', 3) );

  QPixmap pm;

  if( QPixmapCache::find( cacheKey, pm ) )
    {
      return pm;
    }

  if( pm.load( path ) )
    {
      if( pm.width() != size || pm.height() != size )
	{
	  pm = pm.scaled( size, size,
	                  Qt::KeepAspectRatio,
	                  Qt::SmoothTransformation );
	}

      QPixmapCache::insert( cacheKey, pm );
      return pm;
    }

  qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
             path.toLatin1().data() );

  if( pm.load( "Empty" + QString::number(size) ) == false )
    {
      pm = QPixmap( size, size );
      pm.fill( Qt::transparent );
      QPixmapCache::insert( "Empty" + QString::number(size), pm );
    }

  return pm;
}

/**
 * @removes a pixmap from the global cache
 */
void GeneralConfig::removePixmap( const QString& pixmapName )
{
  // determine absolute path to pixmap directory and remove pixmap
  QPixmapCache::remove( ":/icons/" + pixmapName );
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
  subDirs << _mapRootDir + "/points"
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
  return QDir::homePath();
}

/** Returns the expected places of map directories
    There are: 1. Map directory defined by user
               2. $HOME/Cumulus/maps (desktop) or
                  /media/mmc.../Cumulus/maps
*/
QStringList GeneralConfig::getMapDirectories()
{
  QStringList mapDirs;
  QString     mapDefault = getUserDefaultRootDir() + "/maps";

#ifdef ANDROID
  // Under Android only the default map directory on sdcard exists
  mapDirs << mapDefault;
  return mapDirs;
#endif

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

  if( proxy == 0 )
    {
      proxy = getenv("HTTP_PROXY");
    }

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

#ifdef ANDROID

  // QStyle* style = QApplication::setStyle( _guiStyle );

#warning "GUI Style for Android is hard coded to Plastique"

  // The Android style makes trouble, therefore we use only Plastique atm.
  QStyle* style = QApplication::setStyle( "Plastique" );


  QApplication::setStyle( new AndroidProxyStyle( style ) );

#else
#ifdef MAEMO

  QStyle* style = QApplication::setStyle( _guiStyle );
  QApplication::setStyle( new MaemoProxyStyle( style ) );

#else

  QApplication::setStyle( _guiStyle );

#endif
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
int GeneralConfig::getWaypointScaleBorder( const Waypoint::Priority importance) const
{
  switch( importance )
  {
    case Waypoint::Low:
    case Waypoint::Normal:
    case Waypoint::High:

      // qDebug("Priority=%d, value=%d", priority, _wayPointScaleBorders[priority] );
      return _wayPointScaleBorders[importance];

    case Waypoint::Top:
      // Cumulus does not support Top priority. All is mapped to High.
      return _wayPointScaleBorders[Waypoint::High];

    default:
      qWarning("getWaypointScaleBorder(): Undefined Importance=%d passed!", importance );
      return _wayPointScaleBorders[Waypoint::Low];
  }
}

/** Sets waypoint scale border. */
void GeneralConfig::setWaypointScaleBorder( const Waypoint::Priority importance,
                                            const int newScale )
{
  switch( importance )
  {
    case Waypoint::Low:
    case Waypoint::Normal:
    case Waypoint::High:
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
      QString langDir = ":/locale/" + _language;

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
          qWarning() << "No GUI translation file" << langFile
                     << "found in" << langDir;
        }

      // Load Qt library translation file, e.g. qt_de.qm
      langFile = "qt_" + _language + ".qm";

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
          qWarning() << "No Library translation file" << langFile
                     << "found in" << langDir;
        }
    }
}

QByteArray GeneralConfig::rot47( const QByteArray& input )
{
  // const char* a0 = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
  const char* rotA  = "PQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNO";

  QByteArray out;

  if( input.isEmpty() )
    {
      return out;
    }

  for( int i = 0; i < input.size(); i++ )
    {
      unsigned char c = input.at(i);

      if( c < '!' || c > '~' )
        {
          // let it as it is
          out.append(c);
        }
      else
        {
          // translate character
          out.append( rotA[c - 0x21] );
        }
    }

  return out;
}
