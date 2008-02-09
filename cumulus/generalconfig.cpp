/***********************************************************************
 **
 **   generalconfig.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2004 by André Somers, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <iostream>
using namespace std;

#include <stdlib.h>
#include <cmath>

#include <QDir>
#include <QPixmapCache>

#include "generalconfig.h"
#include "hwinfo.h"
#include "mapdefaults.h"
#include "gpsnmea.h"
#include "speed.h"
#include "altitude.h"
#include "distance.h"

// set static values
GeneralConfig * GeneralConfig::_theInstance=0;

// @AP: We derive here from the QT settings as base class. The config
// file will be stored in the user home directory as
// $HOME/.config/Cumulus.conf
GeneralConfig::GeneralConfig() : QSettings( QSettings::UserScope, "Cumulus" )
{
  _homeWp = new wayPoint();
  load();

  // increase global pixmap cache to 2MB
  QPixmapCache::setCacheLimit ( 2*1024 );
}


GeneralConfig::~GeneralConfig()
{
  qDebug("GeneralConfig::~GeneralConfig(): is called");
  save();
  _theInstance=0;
  _airspaceWarningGeneral=true;
  delete _homeWp;
  sync();
}


GeneralConfig * GeneralConfig::instance()
{
  if (_theInstance==0)
    _theInstance=new GeneralConfig;

  return _theInstance;
}


void GeneralConfig::load()
{
  // cumulus main data
  beginGroup("Main");
  _installRoot = value("InstallRoot", "./").toString();
  endGroup();

  // Main window properties
  beginGroup("MainWindow");
  _windowSize = value("Geometrie", QSize(800, 480)).toSize();
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  _awd.horClose          = value("HorizontalWarningDistance", 1500.0).toDouble();
  _awd.horVeryClose      = value("HorizontalWarningDistanceVC", 200.0).toDouble();
  _awd.verAboveClose     = value("VerticalWarningDistanceAbove", 200.0).toDouble();
  _awd.verAboveVeryClose = value("VerticalWarningDistanceAboveVC", 50.0).toDouble();
  _awd.verBelowClose     = value("VerticalWarningDistanceBelow", 100.0).toDouble() ;
  _awd.verBelowVeryClose = value("VerticalWarningDistanceBelowVC", 25.0).toDouble();

  _forceDrawing         = value("forceLowAirspaceDrawing", true ).toBool();
  _forceDrawingDistance = value("forceLowAirspaceDrawingDistance", 300.0).toDouble();
    
  // Airspace warning types
  _airspaceWarning[BaseMapElement::AirA]       = value("checkAirspaceA", true).toBool();
  _airspaceWarning[BaseMapElement::AirB]       = value("checkAirspaceB", true).toBool();
  _airspaceWarning[BaseMapElement::AirC]       = value("checkAirspaceC", true).toBool();
  _airspaceWarning[BaseMapElement::ControlC]   = value("checkControlC", true).toBool();
  _airspaceWarning[BaseMapElement::AirD]       = value("checkAirspaceD", true).toBool();
  _airspaceWarning[BaseMapElement::ControlD]   = value("checkControlD", true).toBool();
  _airspaceWarning[BaseMapElement::AirElow]    = value("checkAirspaceElow", false).toBool();
  _airspaceWarning[BaseMapElement::AirEhigh]   = value("checkAirspaceEhigh", false).toBool();
  _airspaceWarning[BaseMapElement::AirF]       = value("checkAirspaceF", true).toBool();
  _airspaceWarning[BaseMapElement::Restricted] = value("checkRestricted", true).toBool();
  _airspaceWarning[BaseMapElement::Danger]     = value("checkDanger", true).toBool();
  _airspaceWarning[BaseMapElement::Tmz]        = value("checkTMZ", true).toBool();
  _airspaceWarning[BaseMapElement::LowFlight]  = value("checkLowFlight", true).toBool();
  _airspaceWarning[BaseMapElement::SuSector]   = value("checkSuSector", true).toBool();

  _airspaceWarningGeneral = value("enableAirspaceWarning", true).toBool();

  //airspace filling
  m_airspaceFillingEnabled = value("enableAirspaceFilling", true).toBool();
  _verticalAirspaceFillings[Airspace::none] =
    qMax(0, qMin(100, value("fillingNoneVertical", 0).toInt()));
  _verticalAirspaceFillings[Airspace::near] =
    qMax(0, qMin(100, value("fillingNearVertical", 15).toInt()));
  _verticalAirspaceFillings[Airspace::veryNear] =
    qMax(0, qMin(100, value("fillingVeryNearVertical", 20).toInt()));
  _verticalAirspaceFillings[Airspace::inside] =
    qMax(0, qMin(100, value("fillingInsideVertical", 25).toInt()));
         
  _totalAirspaceFillings[Airspace::none] =
    qMax(0, qMin(100, value("fillingNoneTotal", 0).toInt()));
  _totalAirspaceFillings[Airspace::near] =
    qMax(0, qMin(100, value("fillingNearTotal", 35).toInt()));
  _totalAirspaceFillings[Airspace::veryNear] =
    qMax(0, qMin(100, value("fillingVeryNearTotal", 45).toInt()));
  _totalAirspaceFillings[Airspace::inside] =
    qMax(0, qMin(100, value("fillingInsideTotal", 60).toInt()));
  endGroup();

  // Personal settings
  beginGroup("Personal Data");
  _disclaimerVersion = value( "Disclaimer", 0).toInt();
  _surname           = value("SurName", "").toString();
  _birthday          = value("Birthday", "").toString();
  endGroup();

  // Preflight settings
  beginGroup("Preflight Data");
  _safetyAltitude.setMeters(  value( "Arrival Altitude", 200.0 ).toDouble() );
  _qnh             = value( "QNH", 1013 ).toInt();
  _loggerInterval  = value( "LoggerInterval", 10 ).toInt();
  _cruisingSpeed   = value( "CruisingSpeed", 100 ).toInt();
  endGroup();

  // Task scheme settings for cylinder-sector and nearst-touched
  beginGroup("Task Scheme");
  _taskActiveCSScheme = (enum ActiveCSTaskScheme) value( "ActiveCSScheme",
                                                         GeneralConfig::Sector ).toInt();
  _taskActiveNTScheme = (enum ActiveNTTaskScheme) value( "ActiveNTScheme",
                                                         GeneralConfig::Touched ).toInt();
  _taskDrawShape  = value( "DrawShape", true ).toBool();
  _taskFillShape  = value( "FillShape", true ).toBool();
  _taskShapeAlpha = value( "ShapeAlpha", 64 ).toInt(); // transparency is in %
  endGroup();

  beginGroup("Task Scheme Cylinder");
  _taskCylinderRadius.setMeters( value( "Radius", 1000.0 ).toDouble() );
  endGroup();

  beginGroup("Task Scheme Sector");
  _taskSectorInnerRadius.setMeters( value( "InnerRadius", 0.0 ).toDouble() );
  _taskSectorOuterRadius.setMeters( value( "OuterRadius", 3000.0 ).toDouble() );
  _taskSectorAngle     = value( "Angle", 90).toInt();
  endGroup();

  beginGroup("Map");
  _mapDelete                      = value( "DeleteAfterMapCompile", false ).toBool();
  _mapUnload                      = value( "UnloadUnneededMap", false ).toBool();
  _mapBearLine                    = value( "BearLine", true ).toBool();
  _mapLoadIsoLines                = value( "LoadIsoLines", true ).toBool();
  _mapShowIsoLineBorders          = value( "ShowIsoLineBorders", false ).toBool();
  _mapShowWaypointLabels          = value( "ShowWaypointLabels", false ).toBool();
  _mapShowWaypointLabelsExtraInfo = value( "ShowWaypointLabelsExtraInfo", false ).toBool();
  _mapLoadRoads                   = value( "LoadRoads", true ).toBool();
  _mapLoadHighways                = value( "LoadHighways", true ).toBool();
  _mapLoadRailroads               = value( "LoadRailroads", true ).toBool();
  _mapLoadCities                  = value( "LoadCities", true ).toBool();
  _mapLoadWaterways               = value( "LoadWaterways", true ).toBool();
  _mapLoadForests                 = value( "LoadForests", true ).toBool();
  _drawTrail                      = (UseInMode)value( "DrawTrail", 0 ).toInt();
  endGroup();

  beginGroup("Map Data");
  resetHomeWp();
  _homeWp->origP.setLat( value( "Homesite Latitude", HOME_DEFAULT_LAT).toInt() );
  _homeWp->origP.setLon( value( "Homesite Longitude", HOME_DEFAULT_LON).toInt() );
  _centerLat  = value("Center Latitude", HOME_DEFAULT_LAT).toInt();
  _centerLon  = value("Center Longitude", HOME_DEFAULT_LON).toInt();
  _mapScale   = value("Map Scale", 200).toDouble();
  _mapProjectionType = value("Projection Type", ProjectionBase::Cylindric ).toInt();

  _welt2000CountryFilter = value("Welt2000CountryFilter", "").toString();
  _welt2000HomeRadius    = value("Welt2000HomeRadius", 0).toInt();
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
  _waypointFile = value( "WaypointFile",
                         QDir::homeDirPath() + "/cumulus/cumulus.kwp" ).toString();
  endGroup();

  beginGroup("Variometer");
  _varioIntegrationTime = value( "IntegrationTime", 20 ).toInt();
  _varioStepWidth       = value("StepWidth", 1).toInt();
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
  _gpsDevice          = value( "Device", "/dev/ttyS0" ).toString();
  _gpsSpeed           = value( "Speed", 4800 ).toInt();
  _gpsAltitude        = value( "Altitude", (int) GPSNMEA::MSL ).toInt();
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

  beginGroup("Units");
  _unitAlt   = value( "Altitude", Altitude::meters).toInt();
  _unitDist  = value( "Distance", Distance::kilometers).toInt();
  _unitSpeed = value( "Speed",    Speed::kilometersPerHour ).toInt();
  _unitVario = value( "Vario",    Speed::metersPerSecond ).toInt();
  _unitWind  = value( "Wind",     Speed::metersPerSecond ).toInt();
  _unitPos   = value( "Position", WGSPoint::DMS  ).toInt();
  endGroup();
}


void GeneralConfig::save()
{
  // cumulus main data
  beginGroup("Main");
  setValue("InstallRoot", _installRoot );
  endGroup();

  // Main window properties
  beginGroup("MainWindow");
  setValue("Geometrie", _windowSize );
  endGroup();

  // Airspace warning distances
  beginGroup("Airspace");
  setValue("HorizontalWarningDistance", _awd.horClose.getMeters());
  setValue("HorizontalWarningDistanceVC", _awd.horVeryClose.getMeters());
  setValue("VerticalWarningDistanceAbove", _awd.verAboveClose.getMeters());
  setValue("VerticalWarningDistanceAboveVC", _awd.verAboveVeryClose.getMeters());
  setValue("VerticalWarningDistanceBelow", _awd.verBelowClose.getMeters());
  setValue("VerticalWarningDistanceBelowVC", _awd.verBelowVeryClose.getMeters());

  setValue("forceLowAirspaceDrawing", _forceDrawing);
  setValue("forceLowAirspaceDrawingDistance", _forceDrawingDistance.getMeters());

  // Airspace warning types
  setValue("checkAirspaceA", _airspaceWarning[BaseMapElement::AirA]);
  setValue("checkAirspaceB", _airspaceWarning[BaseMapElement::AirB]);
  setValue("checkAirspaceC", _airspaceWarning[BaseMapElement::AirC]);
  setValue("checkControlC", _airspaceWarning[BaseMapElement::ControlC]);
  setValue("checkAirspaceD", _airspaceWarning[BaseMapElement::AirD]);
  setValue("checkControlD", _airspaceWarning[BaseMapElement::ControlD]);
  setValue("checkAirspaceElow", _airspaceWarning[BaseMapElement::AirElow]);
  setValue("checkAirspaceEhigh", _airspaceWarning[BaseMapElement::AirEhigh]);
  setValue("checkAirspaceF", _airspaceWarning[BaseMapElement::AirF]);
  setValue("checkRestricted", _airspaceWarning[BaseMapElement::Restricted]);
  setValue("checkDanger", _airspaceWarning[BaseMapElement::Danger]);
  setValue("checkTMZ", _airspaceWarning[BaseMapElement::Tmz]);
  setValue("checkLowFlight", _airspaceWarning[BaseMapElement::LowFlight]);
  setValue("checkSuSector", _airspaceWarning[BaseMapElement::SuSector]);

  setValue("enableAirspaceWarning", _airspaceWarningGeneral);

  // Airspace filling
  setValue("enableAirspaceFilling", m_airspaceFillingEnabled);

  setValue("fillingNoneVertical", _verticalAirspaceFillings[Airspace::none]);
  setValue("fillingNearVertical", _verticalAirspaceFillings[Airspace::near]);
  setValue("fillingVeryNearVertical", _verticalAirspaceFillings[Airspace::veryNear]);
  setValue("fillingInsideVertical", _verticalAirspaceFillings[Airspace::inside]);

  setValue("fillingNoneTotal", _totalAirspaceFillings[Airspace::none]);
  setValue("fillingNearTotal", _totalAirspaceFillings[Airspace::near]);
  setValue("fillingVeryNearTotal", _totalAirspaceFillings[Airspace::veryNear]);
  setValue("fillingInsideTotal", _totalAirspaceFillings[Airspace::inside]);
  endGroup();

  // Personal settings
  beginGroup("Personal Data");
  setValue("Disclaimer", _disclaimerVersion);
  setValue("SurName", _surname);
  setValue("Birthday", _birthday);
  endGroup();

  // Preflight data
  beginGroup("Preflight Data");
  setValue( "Arrival Altitude", _safetyAltitude.getMeters() );
  setValue( "QNH", _qnh );
  setValue( "LoggerInterval", _loggerInterval );
  setValue( "CruisingSpeed", _cruisingSpeed );
  endGroup();

  // Task scheme settings for cylinder-sector and nearst-touched
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
    
  beginGroup("Map");
  setValue( "DeleteAfterMapCompile", _mapDelete );
  setValue( "UnloadUnneededMap", _mapUnload );
  setValue( "BearLine", _mapBearLine );
  setValue( "LoadIsoLines", _mapLoadIsoLines );
  setValue( "ShowIsoLineBorders", _mapShowIsoLineBorders );
  setValue( "ShowWaypointLabels", _mapShowWaypointLabels );
  setValue( "ShowWaypointLabelsExtraInfo", _mapShowWaypointLabelsExtraInfo );
  setValue( "LoadRoads", _mapLoadRoads );
  setValue( "LoadHighways", _mapLoadHighways );
  setValue( "LoadRailroads", _mapLoadRailroads );
  setValue( "LoadCities", _mapLoadCities   );
  setValue( "LoadWaterways", _mapLoadWaterways  );
  setValue( "LoadForests", _mapLoadForests );
  setValue( "DrawTrail", (int)_drawTrail );
  endGroup();

  beginGroup("Map Data");
  setValue("Homesite Latitude", _homeWp->origP.lat());
  setValue("Homesite Longitude", _homeWp->origP.lon());
  setValue("Center Latitude", _centerLat);
  setValue("Center Longitude", _centerLon);
  setValue("Map Scale", _mapScale);
  setValue("Projection Type", _mapProjectionType);
  setValue("Welt2000CountryFilter", _welt2000CountryFilter);
  setValue("Welt2000HomeRadius", _welt2000HomeRadius);
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
  setValue( "StepWidth", _varioStepWidth );
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
  setValue( "Speed", _gpsSpeed );
  setValue( "Altitude", _gpsAltitude );
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
  endGroup();

  sync();
}


/** gets vario integration time */
int GeneralConfig::getVarioIntegrationTime() const
{
  return _varioIntegrationTime;
}


/** sets vario integration time */
void GeneralConfig::setVarioIntegrationTime(const int newValue)
{
  _varioIntegrationTime = newValue;
}


/** gets vario vario step width */
int GeneralConfig::getVarioStepWidth() const
{
  return _varioStepWidth;
}


/** sets vario vario step width */
void GeneralConfig::setVarioStepWidth(const int newValue)
{
  _varioStepWidth = newValue;
}


/** gets vario tek adjust */
int GeneralConfig::getVarioTekAdjust() const
{
  return _varioTekAdjust;
}


/** sets vario tek adjust */
void GeneralConfig::setVarioTekAdjust(const int newValue)
{
  _varioTekAdjust = newValue;
};


/** gets vario tek compensation */
bool GeneralConfig::getVarioTekCompensation() const
{
  return _varioTekCompensation;
}


/** sets vario tek compensation */
void GeneralConfig::setVarioTekCompensation(const bool newValue)
{
  _varioTekCompensation = newValue;
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


/** Gets the Gps Device */
QString &GeneralConfig::getGpsDevice()
{
  return _gpsDevice;
}


/** Sets the Gps Device */
void GeneralConfig::setGpsDevice( const QString newValue )
{
  _gpsDevice = newValue;
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


/** gets Gps altitude */
int GeneralConfig::getGpsAltitude() const
{
  return _gpsAltitude;
}


/** sets Gps altitude */
void GeneralConfig::setGpsAltitude(const int newValue)
{
  _gpsAltitude = newValue;
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


void GeneralConfig::resetHomeWp()
{
  // qDebug("resetting Home Wp %d", int(_homeWp));
  _homeWp->name = "Home";
  _homeWp->description = "Home airfield";
  _homeWp->icao = "";
  _homeWp->elevation = 0;
  _homeWp->comment = "The information on this field is incomplete. Trust only the coordinates!";
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


bool GeneralConfig::getAirspaceWarningEnabled (BaseMapElement::objectType type) const
{
  if( type >0 && type < BaseMapElement::objectTypeSize ) {
    // cout << "return=" << _airspaceWarning [type] << endl;
    return _airspaceWarning [type];
  } else {
    // cout << "objectTypeSize out of bounds type=" << type <<endl;
    return false;
  }
};


int GeneralConfig::getAirspaceFillingVertical(Airspace::ConflictType nearness)
{
  return _verticalAirspaceFillings[nearness];
}

int GeneralConfig::getAirspaceFillingTotal(Airspace::ConflictType nearness)
{
  return _totalAirspaceFillings[nearness];
}

void GeneralConfig::setAirspaceFillingVertical(Airspace::ConflictType nearness, int filling)
{
  _verticalAirspaceFillings[nearness] = qMax(0, qMin(100, filling));
}

void GeneralConfig::setAirspaceFillingTotal(Airspace::ConflictType nearness, int filling)
{
  _totalAirspaceFillings[nearness] = qMax(0, qMin(100, filling));
}

/* Load a pixmap from the cache. If not contained there, insert it. */
QPixmap GeneralConfig::loadPixmap( const char* pixmapName )
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
