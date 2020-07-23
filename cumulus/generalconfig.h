/***********************************************************************
**
**   generalconfig.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2007-2020 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class GeneralConfig
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration manager class
 *
 * This class is used to store and retrieve all kinds of
 * configuration options. This class is a singleton class. Use the
 * static instance method to get a reference to the instance.
 *
 * \date 2004-2020
 *
 * \version 1.10
 */

#ifndef GENERAL_CONFIG_H
#define GENERAL_CONFIG_H

#include <QtGlobal>
#include <QByteArray>
#include <QPixmap>
#include <QSettings>
#include <QString>
#include <QSize>
#include <QStringList>

#include "airspace.h"
#include "altitude.h"
#include "basemapelement.h"
#include "layout.h"
#include "mapdefaults.h"
#include "speed.h"
#include "waypoint.h"

// default window display times in seconds
#define MIN_POPUP_DISPLAY_TIME 3
#define AIRFIELD_DISPLAY_TIME_DEFAULT 20
#define AIRSPACE_DISPLAY_TIME_DEFAULT 20
#define INFO_DISPLAY_TIME_DEFAULT     20
#define WAYPOINT_DISPLAY_TIME_DEFAULT 20
#define WARNING_DISPLAY_TIME_DEFAULT  20
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

// default line width options
#define AirSpaceBorderLineWidth 5 * Layout::getIntScaledDensity()
#define TaskLineWidth 5 * Layout::getIntScaledDensity()
#define TargetLineWidth 5 * Layout::getIntScaledDensity()
#define TaskFiguresLineWidth 5 * Layout::getIntScaledDensity()
#define HeadingLineWidth 5 * Layout::getIntScaledDensity()
#define TrailLineWidth 4 * Layout::getIntScaledDensity()

// default line color options
#define TaskLineColor QColor(Qt::darkMagenta).name()
#define TaskFiguresColor QColor(Qt::red).name()
#define HeadingLineColor QColor(Qt::gray).name()
#define TrailLineColor QColor(Qt::black).name()

#ifdef MAEMO4
#define SoundPlayer "/opt/cumulus/bin/aplay"
#else
#define SoundPlayer "/usr/bin/aplay"
#endif

class QTranslator;

extern const char* CumulusBuildDate;

// We do derive from the QT settings class as base class
class GeneralConfig : protected QSettings
{
 public:

  enum UseInMode
  {
    never, standstill, circling, cruising, wave, always
  };

  // Task figure scheme
  enum ActiveTaskFigureScheme
  {
    Undefined = -1, Circle = 0, Sector = 1, Line = 2, Keyhole = 3
  };

  // Task switch scheme
  enum ActiveTaskSwitchScheme
  {
    Nearst = 0, Touched = 1
  };

  /** Waypoint file storage formats. */
  enum WpFileFormat
  {
    Binary = 0, XML = 1
  };

  /** Arrival altitude display selection, if a flight task is active. */
  enum ArrivalAltitudeDisplay
  {
    landingTarget = 0, nextTarget = 1
  };

  /** Temperature units. */
  enum TemperatureUnit
  {
    Celsius = 0,
    Fahrenheit = 1
  };

  /** Air pressure units */
  enum AirPressureUnit
  {
    hPa = 0,
    inHg = 1
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

  /** gets the application root path of Cumulus */
  QString &getAppRoot()
    {
      return _appRoot;
    };

  /** sets the application root path of Cumulus */
  void setAppRoot( QString &newRoot )
  {
    _appRoot = newRoot;
  };

  /** gets the application root path of Cumulus */
  QString &getDataRoot()
    {
      return _dataRoot;
    };

  /** sets the application root path of Cumulus */
  void setDataRoot( QString &newRoot )
  {
    _dataRoot = newRoot;
  };

  /** gets the built date of Cumulus */
  static QString getBuiltDate()
    {
      return QString(CumulusBuildDate);
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
   * Load a required pixmap. The loaded pixmap is put into a cache for faster
   * loading on next request.
   *
   * @param pixmapName The pixmap to be loaded.
   *
   * @param doScale Do scale the pixmap with the current set scale.
   *
   * @return The requested pixmap.
   */
  QPixmap loadPixmap( const QString& pixmapName, const bool doScale=true );

  /**
   * Load a required pixmap and scale it to the needed size. The loaded pixmap
   * is put into a cache for faster loading on next request.
   *
   * @param pixmapName The pixmap file to be loaded.
   *
   * @return The requested pixmap.
   */
  QPixmap loadPixmapAutoScaled( const QString& pixmapName );

  /**
   * Load a required pixmap. The loaded pixmap is put into a cache for faster
   * loading on next request.
   *
   * @param pixmapName The pixmap to be loaded.
   *
   * @param size The size of the pixmap to be scaled to.
   *
   * @return The requested pixmap.
   */
  QPixmap loadPixmap( const QString& pixmapName, int size );

  /**
   * @param pixmapName Removes the pixmap from the global cache.
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
  QColor &getBorderColorAirspaceFlarm()
    {
      return _borderColorAirspaceFlarm;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceFlarm( const QColor& newValue )
  {
    _borderColorAirspaceFlarm = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceFir()
    {
      return _borderColorAirspaceFir;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceFir( const QColor& newValue )
  {
    _borderColorAirspaceFir = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorAirspaceG()
    {
      return _borderColorAirspaceG;
    };

  /** Sets the airspace border color */
  void setBorderColorAirspaceG( const QColor& newValue )
  {
    _borderColorAirspaceG = newValue;
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
  QColor &getBorderColorControl()
    {
      return _borderColorControl;
    };

  /** Sets the airspace border color */
  void setBorderColorControl( const QColor& newValue )
  {
    _borderColorControl = newValue;
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
  QColor &getBorderColorProhibited()
    {
      return _borderColorProhibited;
    };

  /** Sets the airspace border color */
  void setBorderColorProhibited( const QColor& newValue )
  {
    _borderColorProhibited = newValue;
  };

  /** Gets the airspace border color */
  QColor &getBorderColorRMZ()
    {
      return _borderColorRMZ;
    };

  /** Sets the airspace border color */
  void setBorderColorRMZ( const QColor& newValue )
  {
    _borderColorRMZ = newValue;
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
  QColor &getFillColorAirspaceFlarm()
    {
      return _fillColorAirspaceFlarm;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceFlarm( const QColor& newValue )
  {
    _fillColorAirspaceFlarm = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceFir()
    {
      return _fillColorAirspaceFir;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceFir( const QColor& newValue )
  {
    _fillColorAirspaceFir = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorAirspaceG()
    {
      return _fillColorAirspaceG;
    };

  /** Sets the airspace fill color */
  void setFillColorAirspaceG( const QColor& newValue )
  {
    _fillColorAirspaceG = newValue;
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
  QColor &getFillColorControl()
    {
      return _fillColorControl;
    };

  /** Sets the airspace fill color */
  void setFillColorControl( const QColor& newValue )
  {
    _fillColorControl = newValue;
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
  QColor &getFillColorProhibited()
    {
      return _fillColorProhibited;
    };

  /** Sets the airspace fill color */
  void setFillColorProhibited( const QColor& newValue )
  {
    _fillColorProhibited = newValue;
  };

  /** Gets the airspace fill color */
  QColor &getFillColorRMZ()
    {
      return _fillColorRMZ;
    };

  /** Sets the airspace fill color */
  void setFillColorRMZ( const QColor& newValue )
  {
    _fillColorRMZ = newValue;
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
  AirspaceWarningDistance getAirspaceWarningDistances()
  {
    return _awd;
  };

  /**
   * Sets the warning distances for airspaces
   */
  void setAirspaceWarningDistances(const AirspaceWarningDistance& awd)
  {
    _awd = awd;
  };

  /**
   * @return True if drawing is enabled for the given base map type.
   * @param type The type of object (defined in @ref BaseMapElement) to query.
   */
  bool getItemDrawingEnabled (BaseMapElement::objectType type) const;

  /**
   * Enables or disables the map item drawing
   *
   * \param type The type of object (defined in @ref BaseMapElement) to set.
   * \param enable A flag to enable/disable drawing of map item.
   */
  void setItemDrawingEnabled (BaseMapElement::objectType type, bool enable=true)
  {
    _mapDrawingEnabled[type] = enable;
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

  /**
   * @return True if airspace ignore border should be active.
   */
  bool getAirspaceDrawBorderEnabled() const
  {
    return _asNoDrawing;
  };

  /**
   * Enables or disables the drawing of airspaces at a certain vertical border.
   */
  void setAirspaceDrawBorderEnabled(bool enable=false)
  {
    _asNoDrawing = enable;
  };

  /**
   * Gets the lower vertical distance border for airspace drawing. The lower
   * airspace border must lay under that limit otherwise the airspace is not
   * drawn. The border is returned as Flight Level.
   */
  int getAirspaceDrawingBorder() const
  {
    return _asDrawingBorder;
  };

  /**
   * Sets the lower vertical distance border for airspace drawing. The lower
   * airspace border must lay under that limit otherwise the airspace is not
   * drawn. The border is stored as Flight Level.
   */
  void setAirspaceDrawingBorder(const int alt)
  {
    _asDrawingBorder = alt;
  };

  /** Gets the openAIP airspace countries */
  QString &getOpenAipAirspaceCountries()
    {
      return _openAIPAirspaceCountries;
    };

  /** Sets the openAIP airspace countries */
  void setOpenAipAirspaceCountries( const QString newValue )
  {
    _openAIPAirspaceCountries = newValue;
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

  /** gets auto logger start speed in Km/h */
  double getAutoLoggerStartSpeed() const
  {
    return _autoLoggerStartSpeed;
  };

  /** sets auto logger start speed in Km/h */
  void setAutoLoggerStartSpeed( const double newValue )
  {
    _autoLoggerStartSpeed = newValue;
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
  const Speed&  getTas() const
  {
    return _tas;
  };

  /** sets TAS */
  void setTas( const Speed& newValue )
  {
    _tas = newValue;
  };

  /** gets manual wind speed */
  const Speed& getManualWindSpeed() const
  {
    return _manualWindSpeed;
  };

  /** sets manual wind speed */
  void setManualWindSpeed( const Speed& newValue )
  {
    _manualWindSpeed = newValue;
  };

  /** gets manual wind direction */
  int getManualWindDirection() const
  {
    return _manualWindDirection;
  };

  /** sets manual wind direction */
  void setManualWindDirection( const int newValue )
  {
    _manualWindDirection = newValue;
  };

  /** gets the manual wind flag. */
  bool isManualWindEnabled() const
  {
    return _manualWindIsEnabled;
  };

  /** sets the manual wind flag. */
  void setManualWindEnabled( bool newValue )
  {
    _manualWindIsEnabled = newValue;
  };

  /** gets the McCready value */
  const Speed& getMcCready() const
  {
    return _mcCready;
  };

  /** sets the McCready value */
  void setMcCready( const Speed& newValue )
  {
	_mcCready = newValue;
  };

  /** sets current map task  */
  void setMapCurrentTask( const QString newValue )
  {
    _mapCurrentTask = newValue;
  };

  /** gets current map task */
  QString &getMapCurrentTask()
  {
    return _mapCurrentTask;
  };

  /** gets the current task name */
  QString &getCurrentTaskName()
  {
    return _currentTaskName;
  };

  /** sets the current task name */
  void setCurrentTaskName( const QString newValue )
  {
    _currentTaskName = newValue;
  };

  /** Gets the homesite country code */
  QString &getHomeCountryCode()
    {
      return _homeCountryCode;
    };
  /** Sets the homesite country code */
  void setHomeCountryCode( const QString newValue )
  {
    _homeCountryCode = newValue;
  };

  /** Gets the homesite name */
  QString &getHomeName()
    {
      return _homeName;
    };
  /** Sets the homesite name */
  void setHomeName( const QString newValue )
  {
    _homeName = newValue;
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

  /** gets Map ShowNavAidsLabels */
  bool getMapShowNavAidsLabels() const
  {
    return _mapShowNavAidsLabels;
  };
  /** sets Map ShowAirfieldLabels */
  void setMapShowNavAidsLabels(const bool newValue)
  {
    _mapShowNavAidsLabels = newValue;
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

  /** gets Map ShowRelBearingInfo */
  bool getMapShowRelBearingInfo() const
  {
    return _mapShowRelBearingInfo;
  };
  /** sets Map ShowRelBearingInfo */
  void setMapShowRelBearingInfo(const bool newValue)
  {
    _mapShowRelBearingInfo = newValue;
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

  /** gets Map LoadMotorways */
  bool getMapLoadMotorways() const
  {
    return _mapLoadMotorways;
  };
  /** sets Map LoadMotorways */
  void setMapLoadMotorways(const bool newValue)
  {
    _mapLoadMotorways = newValue;
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

  /** gets the airfield source */
  int getAirfieldSource() const
  {
    return _airfieldSource;
  };
  /** sets the airfield source */
  void setAirfieldSource( const int newValue )
  {
    _airfieldSource = newValue;
  };

  /** gets the airfield home radius */
  float getAirfieldHomeRadius() const
  {
    return _airfieldHomeRadius;
  };
  /** sets the airfield home radius */
  void setAirfieldHomeRadius( const float newValue )
  {
    _airfieldHomeRadius = newValue;
  };

  /** gets the airfield runway length filter radius */
  float getAirfieldRunwayLengthFilter() const
  {
    return _airfieldRunwayLengthFilter;
  };
  /** sets the airfield runway length filter radius */
  void setAirfieldRunwayLengthFilter( const float newValue )
  {
    _airfieldRunwayLengthFilter = newValue;
  };

  /** Gets the openAIP POI file list to be loaded. */
  QStringList& getOpenAipPoiFileList()
  {
    return _openAipPoiFileList;
  };

  /** Sets the openAIP POI file list to be loaded. */
  void setOpenAipPoiFileList( const QStringList& newValue )
  {
    _openAipPoiFileList = newValue;
  };

  /** Gets the openAip airfield countries to be downloaded */
  QString &getOpenAipAirfieldCountries()
    {
      return _openAipAirfieldCountries;
    };

  /** Sets the openAip airfield countries to be downloaded */
  void setOpenAipAirfieldCountries( const QString newValue )
  {
    _openAipAirfieldCountries = newValue;
  };

  QString getOpenAipLink()
  {
    return QString(rot47(_openAipLink));
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

  /** gets Circle Parallel */
  int getCylinderParallel()  const
  {
    return _cylinderParallel;
  };
  /** sets Circle Parallel */
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
  int getAltimeterMode() const
  {
    return _altimeterMode;
  };

  /** sets altimeter mode */
  void setAltimeterMode(const int newValue)
  {
    _altimeterMode = newValue;
  };

  /** gets altimeter toggle mode */
  bool getAltimeterToggleMode() const
  {
    return _altimeterToggleMode;
  };

  /** sets altimeter toggle mode */
  void setAltimeterToggleMode(const bool newValue)
  {
    _altimeterToggleMode = newValue;
  };

  /** gets log to file mode */
  bool getLog2FileMode() const
  {
    return _log2File;
  };

  /** sets log to file mode */
  void setLog2FileMode(const bool newValue)
  {
    _log2File = newValue;
  };

  /** gets sys log mode flag */
  bool getSystemLogMode() const
  {
    return _useSysLog;
  };

  /** sets sys log mode flag */
  void setSystemLogMode(const bool newValue)
  {
    _useSysLog = newValue;
  };

  /** gets nearest site calculator switch */
  bool getNearestSiteCalculatorSwitch() const
  {
    return _nearestSiteCalculatorSwitch;
  };

  /** sets nearest site calculator switch */
  void setNearestSiteCalculatorSwitch(const bool newValue)
  {
    _nearestSiteCalculatorSwitch = newValue;
  };

  /** gets max nearest site calculator sites */
  int getMaxNearestSiteCalculatorSites() const
  {
    return _maxNearestSiteCalculatorSites;
  };

  /** sets max nearest site calculator sites */
  void setMaxNearestSiteCalculatorSites(const int newValue)
  {
    _maxNearestSiteCalculatorSites = newValue;
  };

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

  /** gets Popup Flarm alarms */
  bool getPopupFlarmAlarms() const
  {
    return _popupFlarmAlarms;
  };

  /** sets Popup Flarm alarms */
  void setPopupFlarmAlarms(const bool newValue)
  {
    _popupFlarmAlarms = newValue;
  };

  /** Gets flag for white or black info display background. */
  bool getBlackBgInfoDisplay() const
  {
    return _blackBgInfoDisplay;
  };

  /** sets Popup Flarm alarms */
  void setBlackBgInfoDisplay(const bool newValue)
  {
    _blackBgInfoDisplay = newValue;
  };

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

  /** Gets the Gps Source */
  QString &getGpsSource()
  {
    return _gpsSource;
  };

  /** Sets the Gps Source */
  void setGpsSource( const QString newValue )
  {
    _gpsSource = newValue;
  }

  /** Gets the Gps WLAN IP */
  const QString &getGpsWlanIp()
  {
    return _gpsWlanIp;
  };

  /** Sets the Gps WLAN IP */
  void setGpsWlanIp( const QString newValue )
  {
    _gpsWlanIp = newValue;
  }

  /** Gets the Gps WLAN IP port */
  const QString &getGpsWlanPort()
  {
    return _gpsWlanPort;
  };

  /** Sets the Gps WLAN IP port*/
  void setGpsWlanPort( const QString newValue )
  {
    _gpsWlanPort = newValue;
  }

  /** Gets the Gps WLAN password */
  const QString &getGpsWlanPassword()
  {
    return _gpsWlanPassword;
  };

  /** Sets the Gps WLAN password*/
  void setGpsWlanPassword( const QString newValue )
  {
    _gpsWlanPassword = newValue;
  }

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

  /** gets Gps NMEA log state */
  bool getGpsNmeaLogState() const
  {
    return _gpsNmeaLogState;
  }
  /** sets Gps NMEA log state */
  void setGpsNmeaLogState(const bool newValue)
  {
    _gpsNmeaLogState = newValue;
  }

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

  /** gets the average time for the LD calculation */
  int getLDCalculationTime() const
  {
    return _time4LDCalc;
  };

  /** sets the average time for the LD calculation */
  void setLDCalculationTime(const int newValue)
  {
    _time4LDCalc = newValue;
  }

  /** Gets the default binary waypoint file name with path. */
  QString &getBinaryWaypointFileName()
  {
    return _waypointBinaryFileName;
  };
  /** Sets the default binary waypoint file name with path. */
  void setBinaryWaypointFileName( const QString newValue )
  {
    _waypointBinaryFileName = newValue;
  };

  /** Gets the default XML waypoint file name with path. */
  QString &getXmlWaypointFileName()
  {
    return _waypointXmlFileName;
  };
  /** Sets the default XML waypoint file name with path. */
  void setXmlWaypointFileName( const QString newValue )
  {
    _waypointXmlFileName = newValue;
  };

  /** Gets the waypoint file format */
  enum WpFileFormat getWaypointFileFormat() const
  {
    return _waypointFileFormat;
  };
  /** Sets the waypoint file format */
  void setWaypointFileFormat(const enum WpFileFormat newValue)
  {
    _waypointFileFormat = newValue;
  };

  /** Gets the waypoint priority */
  int getWaypointPriority() const
  {
    return _waypointPriority;
  };
  /** Sets the waypoint priority */
  void setWaypointPriority(const int newValue)
  {
    _waypointPriority = newValue;
  };

  /** Gets the waypoint import center reference. */
  int getWaypointCenterReference() const
  {
    return _waypointCenterReference;
  };
  /** Sets the waypoint import center reference. */
  void setWaypointCenterReference(const int newValue)
  {
    _waypointCenterReference = newValue;
  };

  /** Gets the waypoint import airfield reference. */
  QString &getWaypointAirfieldReference()
  {
    return _waypointAirfieldReference;
  };
  /** Sets the waypoint import airfield reference. */
  void setWaypointAirfieldReference( const QString newValue )
  {
    _waypointAirfieldReference = newValue;
  };

  /** Gets the waypoint import radius value. */
  QString& getWaypointImportRadius()
  {
    return _waypointImportRadius;
  };
  /** Sets the waypoint import radius value. */
  void setWaypointImportRadius(const QString newValue)
  {
    _waypointImportRadius = newValue;
  };

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

  /** Gets the unit for the temperature */
  int getUnitTemperature() const
  {
    return _unitTemperature;
  };

  /** Sets the unit for the temperature */
  void setUnitTemperature(const int newValue)
  {
    _unitTemperature = newValue;
  };

  /** Gets the unit for the air pressure */
  int getUnitAirPressure() const
  {
    return _unitAirPressure;
  };

  /** Sets the unit for the air pressure  */
  void setUnitAirPressure(const int newValue)
  {
    _unitAirPressure = newValue;
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

  /** Gets the airspace file list to be loaded. */
  QStringList& getAirspaceFileList()
  {
    return _airspaceFileList;
  };

  /** Sets the airspace file list to be loaded. */
  void setAirspaceFileList( const QStringList& newValue )
  {
    _airspaceFileList = newValue;
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
   * Gets map trail drawing.
   */
  bool getMapDrawTrail() const
  {
    return _drawTrail;
  };

  /**
   * Sets map trail drawing.
   */
  void setMapDrawTrail( bool draw )
  {
    _drawTrail = draw;
  };

  /** Gets the arrival altitude display selection. */
  enum ArrivalAltitudeDisplay getArrivalAltitudeDisplay() const
  {
    return _arrivalAltitudeDisplay;
  };

  /** Sets the arrival altitude display selection. */
  void setArrivalAltitudeDisplay( const enum ArrivalAltitudeDisplay newValue )
  {
    _arrivalAltitudeDisplay = newValue;
  };

  /** Gets the active task start scheme */
  enum ActiveTaskFigureScheme getActiveTaskStartScheme() const
  {
    return _taskActiveStartScheme;
  };

  /** Sets the active task start scheme */
  void setActiveTaskStartScheme( const enum ActiveTaskFigureScheme newValue )
  {
    _taskActiveStartScheme = newValue;
  };

  /** Gets the active task finish scheme */
  enum ActiveTaskFigureScheme getActiveTaskFinishScheme() const
  {
    return _taskActiveFinishScheme;
  };

  /** Sets the active task finish scheme */
  void setActiveTaskFinishScheme( const enum ActiveTaskFigureScheme newValue )
  {
    _taskActiveFinishScheme = newValue;
  };

  /** Gets the active task observer scheme */
  enum ActiveTaskFigureScheme getActiveTaskObsScheme() const
  {
    return _taskActiveObsScheme;
  };

  /** Sets the active task observer scheme */
  void setActiveTaskObsScheme( const enum ActiveTaskFigureScheme newValue )
  {
    _taskActiveObsScheme = newValue;
  };

  /** Gets the active task switch scheme */
  enum ActiveTaskSwitchScheme getActiveTaskSwitchScheme() const
  {
    return _taskActiveSwitchScheme;
  };

  /** Sets the active nt task scheme */
  void setActiveTaskSwitchScheme( const enum ActiveTaskSwitchScheme newValue )
  {
    _taskActiveSwitchScheme = newValue;
  };

  /**
   * Gets the report task point switch flag.
   */
  bool getReportTpSwitch() const
  {
    return _reportTaskpointSwitch;
  };

  /**
   * Sets the report task point switch flag.
   */
  void setReportTpSwitch( const bool value )
  {
    _reportTaskpointSwitch = value;
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

  /** gets taskpoint auto zoom option */
  bool getTaskPointAutoZoom() const
  {
    return _taskPointAutoZoom;
  };

  /** sets taskpoint auto zoom option */
  void setTaskPointAutoZoom( const bool newValue )
  {
    _taskPointAutoZoom = newValue;
  };

  /** Gets the length of the finish line. */
  Distance getTaskFinishLineLength() const
  {
    return _taskFinishLineLength;
  };

  /** Sets the length of the finish line. */
  void setTaskFinishLineLength( const Distance& taskFinishLineLength )
  {
    _taskFinishLineLength = taskFinishLineLength;
  };

  /** Gets the radius of the finish ring. */
  Distance getTaskFinishRingRadius() const
  {
    return _taskFinishRingRadius;
  };

  /** Sets the radius of the finish ring. */
  void setTaskFinishRingRadius( const Distance& taskFinishRingRadius )
  {
    _taskFinishRingRadius = taskFinishRingRadius;
  };

  /** Gets the angle of the task finish sector. */
  int getTaskFinishSectorAngel() const
  {
    return _taskFinishSectorAngel;
  };

  /** Sets the angle of the task finish sector. */
  void setTaskFinishSectorAngel( const int taskFinishSectorAngel )
  {
    _taskFinishSectorAngel = taskFinishSectorAngel;
  };

  /** Gets the inner radius of the task finish sector. */
  Distance getTaskFinishSectorIRadius() const
  {
    return _taskFinishSectorIRadius;
  };

  /** Sets the inner radius of the task finish sector. */
  void setTaskFinishSectorIRadius( const Distance& taskFinishSectorIRadius )
  {
    _taskFinishSectorIRadius = taskFinishSectorIRadius;
  };

  /** Gets the outer radius of the task finish sector. */
  Distance getTaskFinishSectorORadius() const
  {
    return _taskFinishSectorORadius;
  };

  /** Sets the outer radius of the task finish sector. */
  void setTaskFinishSectorORadius( const Distance& taskFinishSectorORadius )
  {
    _taskFinishSectorORadius = taskFinishSectorORadius;
  };

  /** Gets the length of the start line. */
  Distance getTaskStartLineLength() const
  {
    return _taskStartLineLength;
  };

  /** Sets the length of the start line. */
  void setTaskStartLineLength( const Distance& taskStartLineLength )
  {
    _taskStartLineLength = taskStartLineLength;
  };

  /** Gets the radius of the start ring. */
  Distance getTaskStartRingRadius() const
  {
    return _taskStartRingRadius;
  };

  /** Sets the radius of the start ring. */
  void setTaskStartRingRadius( const Distance& taskStartRingRadius )
  {
    _taskStartRingRadius = taskStartRingRadius;
  };

  /** Gets the angle of the task start sector. */
  int getTaskStartSectorAngel() const
  {
    return _taskStartSectorAngel;
  };

  /** Sets the angle of the task start sector. */
  void setTaskStartSectorAngel( const int taskStartSectorAngel )
  {
    _taskStartSectorAngel = taskStartSectorAngel;
  };

  /** Gets the inner radius of the task start sector. */
  Distance getTaskStartSectorIRadius() const
  {
    return _taskStartSectorIRadius;
  };

  /** Sets the inner radius of the task start sector. */
  void setTaskStartSectorIRadius( const Distance& taskStartSectorIRadius )
  {
    _taskStartSectorIRadius = taskStartSectorIRadius;
  };

  /** Gets the outer radius of the task start sector. */
  Distance getTaskStartSectorORadius() const
  {
    return _taskStartSectorORadius;
  };

  /** Sets the outer radius of the task start sector. */
  void setTaskStartSectorORadius( const Distance& taskStartSectorORadius )
  {
    _taskStartSectorORadius = taskStartSectorORadius;
  };

  /** Gets task observation zone circle radius in meters */
  Distance &getTaskObsCircleRadius()
  {
    return _taskObsCircleRadius;
  };

  /** Sets task observation zone circle radius. Unit must be meters. */
  void setTaskObsCircleRadius( const Distance &newValue )
  {
    _taskObsCircleRadius = newValue;
  };

  /** Gets task observation zone sector inner radius in meters */
  Distance &getTaskObsSectorInnerRadius()
  {
    return _taskObsSectorInnerRadius;
  };

  /** Sets task observation zone sector inner radius. Unit must be meters. */
  void setTaskObsSectorInnerRadius( const Distance &newValue )
  {
    _taskObsSectorInnerRadius = newValue;
  };

  /** Gets task observation zone sector outer radius in meters */
  Distance &getTaskObsSectorOuterRadius()
  {
    return _taskObsSectorOuterRadius;
  };

  /** Sets task observation zone sector outer radius. Unit must be meters. */
  void setTaskObsSectorOuterRadius( const Distance &newValue )
  {
    _taskObsSectorOuterRadius = newValue;
  };

  /** Gets task observation zone sector angle 1-360 degrees.  */
  int getTaskObsSectorAngle() const
  {
    return _taskObsSectorAngle;
  };

  /** Sets task sector angle 1-360 degrees.  */
  void setTaskObsSectorAngle( const int newValue )
  {
    _taskObsSectorAngle = newValue;
  };

  /** gets the target line draw state */
  bool getTargetLineDrawState() const
  {
    return _targetLineDrawState;
  };
  /** sets the target line draw state */
  void setTargetLineDrawState(const bool newValue)
  {
    _targetLineDrawState = newValue;
  };

  /** gets target line width */
  int getTargetLineWidth() const
  {
    return _targetLineWidth;
  };

  /** sets target line width */
  void setTargetLineWidth(const int newValue)
  {
    _targetLineWidth = newValue;
  };

  /** gets the heading line draw state */
  bool getHeadingLineDrawState() const
  {
    return _headingLineDrawState;
  };
  /** sets the track line draw state */
  void setHeadingLineDrawState(const bool newValue)
  {
    _headingLineDrawState = newValue;
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

  /** Gets the elevation color offset index. */
  int getElevationColorOffset()
  {
    return _elevationColorOffset;
  };

  /** Sets the elevation color offset index. */
  void setElevationColorOffset( const int newValue )
  {
    _elevationColorOffset = newValue;
  };

  /** Sets the task line color */
  void setTaskLineColor( const QColor& newValue )
  {
    _taskLineColor = newValue;
  };

  /** Gets the task line color */
  QColor& getTaskLineColor()
  {
    return _taskLineColor;
  };

  /** gets task line width */
  int getTaskLineWidth() const
  {
    return _taskLineWidth;
  };

  /** sets task line width */
  void setTaskLineWidth(const int newValue)
  {
    _taskLineWidth = newValue;
  };

  /** Sets the heading line color */
  void setHeadingLineColor( const QColor& newValue )
  {
    _headingLineColor = newValue;
  };

  /** Gets the heading line color */
  QColor& getHeadingLineColor()
  {
    return _headingLineColor;
  };

  /** gets the heading line width */
  int getHeadingLineWidth() const
  {
    return _headingLineWidth;
  };

  /** sets the heading line width */
  void setHeadingLineWidth(const int newValue)
  {
    _headingLineWidth = newValue;
  };

  /** Gets waypoint scale border. */
  int getWaypointScaleBorder( const Waypoint::Priority importance) const;

  /** Sets waypoint scale border. */
  void setWaypointScaleBorder( const Waypoint::Priority importance,
                               const int newScale );


  /** Gets the map trail color. */
  QColor& getMapTrailColor()
  {
    return _mapTrailLineColor;
  };

  /** Sets the map trail color. */
  void setMapTrailColor(QColor mapTrailColor)
  {
    _mapTrailLineColor = mapTrailColor;
  };

  /** Gets the map trail line width. */
  int getMapTrailLineWidth() const
  {
    return _mapTrailLineWidth;
  };

  /** Sets the map trail line width. */
  void setMapTrailLineWidth(int mapTrailLineWidth)
  {
    _mapTrailLineWidth = mapTrailLineWidth;
  };

  /** Gets the task figures color. */
  QColor& getTaskFiguresColor()
  {
    return _taskFiguresColor;
  };

  /** Sets the task figures color. */
  void setTaskFiguresColor(const QColor& taskFigureColor)
  {
    _taskFiguresColor = taskFigureColor;
  };

  /** Gets the task figures line width. */
  int getTaskFiguresLineWidth() const
  {
    return _taskFiguresLineWidth;
  };

  /** Sets the task figures line width. */
  void setTaskFiguresLineWidth(int taskFigureLineWidth)
  {
    _taskFiguresLineWidth = taskFigureLineWidth;
  };

  /** Gets the Flarm alias file name. */
  const QString &getFlarmAliasFileName() const
    {
      return _flarmAliasFileName;
    };

  /** Gets the Airspace filters file name. */
  const QString &getAirspaceFlitersFileName() const
    {
      return _airspaceFiltersFileName;
    };

  /** Gets the flight logbook file name. */
  const QString &getFlightLogbookFileName() const
    {
      return _flightLogbookFileName;
    };

  /** gets the pre-flight menu close flag */
  bool getClosePreFlightMenu() const
  {
    return _closePreFlightMenu;
  };
  /** sets the the pre-flight menu close flag */
  void setClosePreFlightMenu(const bool newValue)
  {
    _closePreFlightMenu = newValue;
  };

  /** Gets reset configuration value. */
  int getResetConfiguration() const
  {
    return _resetConfiguration;
  };

  /** Sets reset configuration value. */
  void setResetConfiguration( const int newValue )
  {
    _resetConfiguration = newValue;
  };

  /** Gets the returner's mobile number. */
  QString getReturnerMobileNumber()
  {
    return _retrieverMobileNumber;
  };

  /** Sets the returner's mobile number. */
  void setReturnerMobileNumber( const QString newValue )
  {
    _retrieverMobileNumber = newValue;
  };

  /** Gets the returner's position format. */
  int getReturnerPositionFormat() const
  {
    return _retrieverPositionFormat;
  };

  /** Sets the returner's position format. */
  void setReturnerPositionFormat( const int newValue )
  {
    _retrieverPositionFormat = newValue;
  };

  /**
   * Encode/decode a string using rot47 algorithm.
   *
   * \param input String to be executed
   *
   * \return Rotated String
   */
  static QByteArray rot47( const QByteArray& input );

  /** Gets the Livetrack airplane type. */
  int getLiveTrackAirplaneType() const
  {
    return _liveTrackAirplaneType;
  };

  /** Sets the Livetrack airplane type. */
  void setLiveTrackAirplaneType(int liveTrackAirplaneType)
  {
    _liveTrackAirplaneType = liveTrackAirplaneType;
  };

  /** Gets the Livetrack tracking interval. */
  int getLiveTrackInterval() const
  {
    return _liveTrackInterval;
  };

  /** Sets the Livetrack tracking interval. */
  void setLiveTrackInterval(int liveTrackInterval)
  {
    _liveTrackInterval = liveTrackInterval;
  };

  /** Gets the Livetrack state. */
  bool isLiveTrackOnOff() const
  {
    return _liveTrackOnOff;
  };

  /** Sets the Livetrack state. */
  void setLiveTrackOnOff(bool liveTrackOnOff)
  {
    _liveTrackOnOff = liveTrackOnOff;
  };

  /** Gets the Livetrack password. */
  QString getLiveTrackPassword() const
  {
    return _liveTrackAccounts[_liveTrackIndex][2];
  };

  /** Gets the Livetrack server url with http prefix. */
  const QString& getLiveTrackServer() const
  {
    if( _liveTrackIndex < _liveTrackServerList.size() )
      {
        return _liveTrackServerList.at(_liveTrackIndex);
      }
    else
      {
        static QString empty;
        return empty;
      }
  };

  /** Gets the LiveTrack server list as URLs with http prefix. */
  static const QStringList& getLiveTrackServerList()
  {
    return _liveTrackServerList;
  };

  /** Gets the Livetrack user name. */
  const QString& getLiveTrackUserName() const
  {
    return _liveTrackAccounts[_liveTrackIndex][1];
  };

  /** Gets the Livetrack selection index. */
  int getLiveTrackIndex() const
  {
    return _liveTrackIndex;
  };

  /** Sets the Livetrack selection index. */
  void setLiveTrackIndex(int liveTrackindex)
  {
    _liveTrackIndex = liveTrackindex;
  };

  /**
   * Gets the live track user account data, server url with http prefix,
   * user name and password.
   */
  void getLiveTrackAccountData( int index, QString data[3] )
  {
    data[0] = _liveTrackAccounts[index][0];
    data[1] = _liveTrackAccounts[index][1];
    data[2] = _liveTrackAccounts[index][2];
  };

  /** Sets the live track user account data. */
  void setLiveTrackAccountData( int index,
                                const QString& user,
                                const QString& password )
  {
    _liveTrackAccounts[index][1] = user;
    _liveTrackAccounts[index][2] = password;
  };

  bool getFlarmRadarDrawWindArrow() const
  {
    return _flarmRadarDrawWindArrow;
  };

  void setFlarmRadarDrawWindArrow( bool flarmRadarDrawWindArrow )
  {
    _flarmRadarDrawWindArrow = flarmRadarDrawWindArrow;
  };

 private:

  /** loads the terrain default colors */
  void loadTerrainDefaultColors();

  static GeneralConfig *_theInstance;

  // Application root path of Cumulus
  QString _appRoot;

  // Application data path of Cumulus
  QString _dataRoot;

  // Date of built
  QString _builtDate;

  // Main window size
  QSize _windowSize;

  // user data directory
  QString _userDataDirectory;

  // Internet proxy
  QString _proxy;

  // Task line color
  QColor _taskLineColor;

  // Task line width
  int _taskLineWidth;

  // Task figure color
  QColor _taskFiguresColor;

  // Task figure line width
  int _taskFiguresLineWidth;

  // Heading line color
  QColor _headingLineColor;

  // Heading line width
  int _headingLineWidth;

  // terrain colors
  QColor _terrainColors[SIZEOF_TERRAIN_COLORS];

  // terrain default colors
  QString _terrainDefaultColors[SIZEOF_TERRAIN_COLORS];

  // terrain ground color, used when the ISO line drawing is disabled
  QColor _groundColor;

  // Offset value to be added or subtracted to current elevation
  // color index value.
  int _elevationColorOffset;

  // draw map element
  bool _mapDrawingEnabled[BaseMapElement::objectTypeSize];

  //used to store the distances for airspace warnings
  AirspaceWarningDistance _awd;

  // border colors of airspaces
  QColor _borderColorAirspaceA;
  QColor _borderColorAirspaceB;
  QColor _borderColorAirspaceC;
  QColor _borderColorAirspaceD;
  QColor _borderColorAirspaceE;
  QColor _borderColorAirspaceF;
  QColor _borderColorAirspaceFlarm;
  QColor _borderColorAirspaceFir;
  QColor _borderColorAirspaceG;
  QColor _borderColorWaveWindow;
  QColor _borderColorControl;
  QColor _borderColorControlC;
  QColor _borderColorControlD;
  QColor _borderColorRestricted;
  QColor _borderColorDanger;
  QColor _borderColorProhibited;
  QColor _borderColorRMZ;
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
  QColor _fillColorAirspaceFlarm;
  QColor _fillColorAirspaceFir;
  QColor _fillColorAirspaceG;
  QColor _fillColorWaveWindow;
  QColor _fillColorControl;
  QColor _fillColorControlC;
  QColor _fillColorControlD;
  QColor _fillColorRestricted;
  QColor _fillColorDanger;
  QColor _fillColorProhibited;
  QColor _fillColorRMZ;
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

  // no airspace drawing flag.
  bool _asNoDrawing;
  // Vertical lower distance up to which the airspace drawing is ignored. The
  // value is stored as flight level.
  int _asDrawingBorder;

  // airspace line width
  int _airspaceLineWidth;

  // openAIP airspace countries to be loaded
  QString _openAIPAirspaceCountries;

  // Airspace files to be loaded
  QStringList _airspaceFileList;

  // OpenAIP POI list to be loaded
  QStringList _openAipPoiFileList;

  // openAIP airfield countries to be downloaded as ISO two letter code.
  QString _openAipAirfieldCountries;

  // openAIP link
  QByteArray _openAipLink;

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
  // Auto logger start speed
  double _autoLoggerStartSpeed;
  // true air speed
  Speed _tas;

  // The name of the currently selected task in the preflight task list.
  QString _currentTaskName;

  // The name of the current task at the map.
  QString _mapCurrentTask;

  // The current used McCready value
  Speed _mcCready;

  // manual wind speed
  Speed _manualWindSpeed;
  // manual wind direction
  int _manualWindDirection;
  // manual wind enable state
  bool _manualWindIsEnabled;

  // Homesite country code
  QString _homeCountryCode;
  // Homesite elevation
  Distance _homeElevation;
  // Homesite name
  QString _homeName;
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

  // Airfield source. 0 = OpenAip
  int _airfieldSource;

  // airfield home radius in meters
  float _airfieldHomeRadius;

  // Airfield runway length filter. 0 means filter is off.
  float _airfieldRunwayLengthFilter;

  // Airfield/WP lists page size (entries)
  int _listDisplayPageSize;
  // Airfield/WP lists row height increase (px)
  int _listDisplayAFMargin;
  // Reachable points list row height increase (px)
  int _listDisplayRPMargin;
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
  // Map ShowNavAidsLabels
  bool _mapShowNavAidsLabels;
  // Map ShowTaskPointLabels
  bool _mapShowTaskPointLabels;
  // Map ShowOutLandingLabels
  bool _mapShowOutLandingLabels;
  // relative bearing info
  bool _mapShowRelBearingInfo;
  // Map LoadRoads
  bool _mapLoadRoads;
  // Map LoadMotorways
  bool _mapLoadMotorways;
  // Map LoadRailways
  bool _mapLoadRailways;
  // Map LoadCities
  bool _mapLoadCities;
  // Map LoadWaterways
  bool _mapLoadWaterways;
  // Map LoadForests
  bool _mapLoadForests;
  // Map trail
  bool _drawTrail;
  // Map trail line color
  QColor _mapTrailLineColor;
  // Map trail line width
  int _mapTrailLineWidth;

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
  // Popup Flarm Alarms
  bool _popupFlarmAlarms;
  // White or black background for info displays
  bool _blackBgInfoDisplay;

  // GPS source
  QString _gpsSource;
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
  // Gps NMEA log state
  bool _gpsNmeaLogState;
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
  // WLAN IP address
  QString _gpsWlanIp;
  // WLAN IP port
  QString _gpsWlanPort;
  // WLAN password
  QString _gpsWlanPassword;

  // minimum sat count for wind calculation
  int _windMinSatCount;
  // wind altitude range
  int _windAltitudeRange;
  // wind time range
  int _windTimeRange;
  // manual navigation mode altitude
  int _manualNavModeAltitude;

  // time for average LD calculation
  int _time4LDCalc;

  // Default binary waypoint file name
  QString _waypointBinaryFileName;

  // Default XML waypoint file name
  QString _waypointXmlFileName;

  /** Waypoint file format. 0=binary, 1=xml */
  enum WpFileFormat _waypointFileFormat;

  /** Waypoint import storage priority. */
  int _waypointPriority;

  /** Waypoint import center reference */
  int _waypointCenterReference;

  /** Waypoint import airfield reference. */
  QString _waypointAirfieldReference;

  /** Waypoint import radius. */
  QString _waypointImportRadius;

  /** Flarm alias file name. */
  QString _flarmAliasFileName;

  /** Airspace filters file name. */
  QString _airspaceFiltersFileName;

  /** Flarm Radar wind arrow drawing. */
  bool _flarmRadarDrawWindArrow;

  /** Flight logbook file name */
  QString _flightLogbookFileName;

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
  // unit temperature
  int _unitTemperature;
  // unit air pressure
  int _unitAirPressure;

  // aktive task figure scheme
  enum ActiveTaskFigureScheme _taskActiveStartScheme;
  enum ActiveTaskFigureScheme _taskActiveFinishScheme;
  enum ActiveTaskFigureScheme _taskActiveObsScheme;
  enum ActiveTaskSwitchScheme _taskActiveSwitchScheme;

  bool _reportTaskpointSwitch;

  // arrival altitude display selection
  enum ArrivalAltitudeDisplay _arrivalAltitudeDisplay;

  // Task start options
  Distance _taskStartLineLength;
  Distance _taskStartRingRadius;
  Distance _taskStartSectorIRadius;
  Distance _taskStartSectorORadius;
  int      _taskStartSectorAngel;

  // Task finish options
  Distance _taskFinishLineLength;
  Distance _taskFinishRingRadius;
  Distance _taskFinishSectorIRadius;
  Distance _taskFinishSectorORadius;
  int      _taskFinishSectorAngel;

  // Task observation Circle
  Distance _taskObsCircleRadius;

  // Task observation Sector scheme
  Distance _taskObsSectorInnerRadius;
  Distance _taskObsSectorOuterRadius;
  int      _taskObsSectorAngle;

  // Circle/Sector draw options
  bool _taskDrawShape;
  bool _taskFillShape;

  // Task point auto zoom
  bool _taskPointAutoZoom;

  // Task shape alpha transparency, used during filling of the shape
  int _taskShapeAlpha;

  // Target line draw state
  bool _targetLineDrawState;

  // Target line width
  int _targetLineWidth;

  // Heading line draw state
  bool _headingLineDrawState;

  /** Waypoint drawing scale borders. Addressed by waypoint priority
   *  (Low=0, Normal=1, High=2). It contains the scale borders defined
   *  user.
   */
  int _wayPointScaleBorders[3];

  /** Translator for other GUI languages as English. */
  QTranslator *cumulusTranslator;

  /** Translator for other Qt library languages as English. */
  QTranslator *qtTranslator;

  // Flag to handle preflight menu close
  bool _closePreFlightMenu;

  // variable to handle configuration resets
  int _resetConfiguration;

  // Mobile number of retriever
  QString _retrieverMobileNumber;

  // Coordinate format used for retriever position.
  int _retrieverPositionFormat;

  // LiveTrack on/off
  bool _liveTrackOnOff;

  // LiveTrack interval in seconds
  int _liveTrackInterval;

  // LiveTrack airplane type
  int _liveTrackAirplaneType;

  // LiveTrack selection index
  int _liveTrackIndex;

  // LiveTrack account array
  QString _liveTrackAccounts[4][3];

  // LiveTrack server list
  static QStringList _liveTrackServerList;
};

#endif
