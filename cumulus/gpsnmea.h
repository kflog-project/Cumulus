/***************************************************************************
    gpsnmea.h - Cumulus NMEA parser and decoder
                            -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers,
                               2008-2009 by Axel Pauli
    email                : axel@kflog.org

    $Id$

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GPSNMEA_H
#define GPSNMEA_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QPoint>
#include <QTimer>

#ifdef MAEMO
#include "gpsmaemo.h"
#endif

#include "speed.h"
#include "altitude.h"
#include "gpscon.h"
#include "wgspoint.h"

/**
 * This class parses and decodes the NMEA sentences and provides access
 * to the last know data. Furthermore it is managing the connection to a GPS
 * receiver connected by RS232, USB or to a GPS daemon process.
 */

struct SatInfo
  {
    int fixValidity;
    int fixAccuracy;
    int satCount;
    QString constellation;
    QTime constellationTime;
  };

struct SIVInfo
  {
    int id;        // Satellite id
    int db;        // signal to noise ratio (0-99), or negative for not tracking
    int azimuth;   // 0-359
    int elevation; // 0-90
  };

struct GPSInfo
  {
    double NMEAVersion;
    QString Brand;
    QString MapDatum;
    bool PressureAlt;
    bool Vario;
    bool Wind;
    bool AirSpeed;
  };

class GpsNmea : public QObject
  {
    Q_OBJECT

  public:

    /**
     * defines altitude bases delivered by GPS unit
     */
    enum DeliveredAltitude {MSL=0, HAE=1, USER=2, PRESSURE=3};

    enum GpsStatus {notConnected=0, noFix=1, validFix=2};

  public:

    GpsNmea(QObject* parent);

    virtual ~GpsNmea();

    /**
     * @Starts the GPS receiver client process and activates the receiver.
     */
    void startGpsReceiver();

    /**
     * @Returns the current GPS connection status.
     */
    const GpsNmea::GpsStatus getGpsStatus() const
      {
        return( _status );
      }

    /**
     * @Returns the current GPS connection status. True if connected, false if not.
     */
    const bool getConnected() const
      {
        return( _status != notConnected );
      }

    /**
     * @Returns the last known speed.
     */
    Speed getLastSpeed() const
      {
        return _lastSpeed;
      };

    /**
     * @Returns the date of the last fix.
     */
    QDate getLastDate() const
      {
        return _lastDate;
      };

    /**
     * @Returns the time of the last fix.
     */
    QTime getLastTime() const
      {
        return _lastTime;
      };

    /**
     * @Returns the last known coordinate in KFLog format (x=lat, y=lon).
     */
    QPoint getLastCoord() const
      {
        return _lastCoord;
      };

    /**
     * @Returns the last know heading.
     */
    const double getLastHeading() const
      {
        return _lastHeading;
      };

    /**
     * @Returns the last know standard pressure altitude
     */
    Altitude getLastStdAltitude() const
      {
        return _lastStdAltitude;
      };

    /**
     * @Returns the last know pressure altitude above sea level
     */
    Altitude getLastPressureAltitude() const
      {
        return _lastPressureAltitude;
      };

    /**
     * @Returns the last know gps altitude depending on user
     * selection MSL or Pressure
     */
    Altitude getLastAltitude() const;

    /**
     * @Returns the last know altitude above the WGS84 ellipsoid
     */
    Altitude getLastGNSSAltitude() const
      {
        return _lastGNSSAltitude;
      };

    /**
     * @Returns the last known wind speed.
     */
    Speed getLastWindSpeed() const
      {
        return _lastWindSpeed;
      };

    /**
     * @Returns the last known wind direction.
     */
    const short getLastWindDirection() const
      {
        return _lastWindDirection;
      };

    /**
     * @Returns the last known wind age in seconds.
     */
    const int getLastWindAge() const
      {
        return _lastWindAge;
      };

    /**
     * @Returns the last known variometer speed.
     */
    Speed getLastVariometer() const
      {
        return _lastVariometer;
      };

    /**
     * @Returns the last know satellite constellation string.
     */
    SatInfo getLastSatInfo() const
      {
        return _lastSatInfo;
      };

    /**
     * force a reset of the serial connection after a resume
     */
    void forceReset();

    /**
     * set altitude reference delivered by the GPS unit
     */
    void setDeliveredAltitude( const GpsNmea::DeliveredAltitude newAltRef )
    {
      _deliveredAltitude = newAltRef;
    };

    /**
     * set altitude correction for the altitude by the GPS unit
     * (used in USER mode only)
     */
    void setDeliveredUserAltitude( const Altitude& userAlt  )
    {
      _userAltitudeCorrection = userAlt;
    };

    /**
     * Don't report once a connection lost
     */
    void ignoreConnectionLost()
    {
      _ignoreConnectionLost = true;
    };

    /**
     * @Returns selected altitude reference delivered by the GPS unit
     */
    const GpsNmea::DeliveredAltitude getDeliveredAltitude() const
      {
        return _deliveredAltitude;
      };

    QList<SIVInfo>& getSivInfo()
    {
      return sivInfo;
    };

    /**
     * @Returns the map datum of the GPS receiver.
     */
    QString getMapDatum() const
      {
        return _mapDatum;
      };

  public slots: // Public slots
    /**
     * This slot is called by the GPSCon object when a new
     * sentence has arrived on the serial port. The argument
     * contains the sentence to analyze.
     */
    void slot_sentence(const QString& sentence);

    /**
     * This slot is called if the object needs to reset. It is
     * used to destroy the serial connection and create a new
     * one, to adjust to new settings.
     */
    void slot_reset();

    /**
     * This slot is called to reset the gps device to factory
     * settings
     */
    void sendFactoryReset();

    /**
     * This slot is called to switch debugging mode on/off
     */
    void switchDebugging (bool on);

  private slots: // Private slots

    /** This slot is called by the external GPS receiver process to signal
     *  a connection lost to the GPS receiver or daemon. */
    void _slotGpsConnectionLost();

    /** This slot is called by the internal timer to signal a timeout.
     *  This timeout occurs if no valid position fix has been received since
     *  the last fix for the given time. The cause can be bad receiver conditions
     *  view to the sky is not clear a.s.o.
     */
    void _slotTimeoutFix();

  signals: // Signals
    /**
     * This signal signifies a new position fix.
     */
    void newPosition();

    /**
     * This signal is emitted if the altitude has been changed.
     */
    void newAltitude();

    /**
     * This signal is emitted if a new speed fix has been
     * established.
     */
    void newSpeed();

    /**
     * This signal is emitted if a new heading has been established.
     */
    void newHeading();

    /**
     * This signal is emitted if a new wind (speed, direction)
     * has been established.
     */
    void newWind( const Speed&, const short );

    /**
     * This signal is emitted if a new variometer value
     * has been established.
     */
    void newVario( const Speed& );

    /**
     * This signal is send if a new satellite constellation
     * has been detected (that is, the satellites used to
     * make a fix have changed).
     */
    void newSatConstellation();

    /**
     * This signal is send to indicate a change in the
     * connected status. The new status is send as boolean
     * (connected=true).
     */
    void connectedChange(bool);

    /**
     * This signal is send to indicate a change in status.
     * It supersedes the old connecedChange(boolean) signal.
     */
    void statusChange(GpsNmea::GpsStatus);

    /**
     * This signal is send to indicate that there is a new fix.
     * Data send after this fix belongs to the new fix!
     */
    void newFix();

    /**
     * This signal is send to indicate that new satellite in view
     * info is available.
     */
    void newSatInViewInfo();

    /**
     * Relay GPSCon newSentence signal
     */
    void newSentence(const QString&);

  private: // Private methods

    /** Resets all data objects to their initial values. This is called
     *  at startup, at restart and if the GPS fix has been lost. */
    void resetDataObjects();

    /** write configuration data to allow restore of last fix */
    void writeConfig();
    /** This function returns a QTime from the time encoded in a MNEA sentence. */
    QTime __ExtractTime(const QString& timestring);
    /** This function returns a QDate from the date encoded in a MNEA sentence. */
    QDate __ExtractDate(const QString& datestring);
    /** This function returns a Speed from the speed encoded in knots */
    Speed __ExtractKnotSpeed(const QString& speedstring);
    /** This function converts the coordinate data from the NMEA sentence to the internal QPoint coordinate format. */
    QPoint __ExtractCoord(const QString& slat, const QString& slatNS, const QString& slon, const QString& slonEW);
    /** Extract the heading from the NMEA sentence. */
    double __ExtractHeading(const QString& headingstring);
    /** Extracts the altitude from a NMEA GGA sentence */
    Altitude __ExtractAltitude(const QString& altitude, const QString& unitAlt,
                               const QString& heightOfGeoid, const QString& unitHeight);
    /** Extracts the altitude from a NMEA PGRMZ sentence */
    Altitude __ExtractAltitude(const QString& number, const QString& unit );
    /** Extracts the constellation from the NMEA sentence. */
    QString __ExtractConstellation(const QStringList& sentence);
    /** Extracts the satellites count from the NMEA sentence. */
    void __ExtractSatcount(const QString& satcount);
    /** Extracts satellites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QStringList& sentence);
    /** Extracts satellites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QString&, const QString&, const QString&, const QString&);
    /** Extracts wind, QNH and vario data from Cambridge's !w sentence. */
    void __ExtractCambridgeW(const QStringList& stringList);

    /** This function is called to indicate that good data has been received.
     *  It resets the TimeOut timer and if necessary changes the connected status. */
    void dataOK();
    /** This function is called to indicate that a good fix has been received. */
    void fixOK();

    /** This function calculates the checksum in the sentence. */
    static uint calcCheckSum (int pos, const QString& sentence);
    /** This function checks if the checksum in the sentence matches the sentence.
     *  It returns true if it matches, and false otherwise. */
    static bool checkCheckSum(int pos, const QString& sentence);

    /** This function calculates the STD altitude from the passed altitude. */
    void calcStdAltitude(const Altitude& altitude);

    /** This function sends the data of last valid fix to the gps receiver. */
    void sendLastFix (bool hard, bool soft);
    /** Set system date/time. Input is UTC related. */
    void setSystemClock( const QDateTime& utcDt );
    /** create a GPS connection */
    void createGpsConnection();

  private: // Private attributes

    /** contains the time of the last fix */
    QTime _lastTime;
    /** contains the date of the last fix. */
    QDate _lastDate;
    /** Contains the last known speed. */
    Speed _lastSpeed;
    /** Contains the last known coordinate in KFLog format */
    QPoint _lastCoord;
    /** Contains the last known STD pressure altitude */
    Altitude _lastStdAltitude;
    /** Contains the last known pressure altitude */
    Altitude _lastPressureAltitude;
    /** Contains the last known MSL altitude */
    Altitude _lastMslAltitude;
    /** Contains the last known HAE */
    Altitude _lastGNSSAltitude;
    /** Contains the last known heading */
    double _lastHeading;
    /** Contains the last known satellite information */
    SatInfo _lastSatInfo;
    /** Contains the last known clock offset of the gps receiver */
    int _lastClockOffset;

    /* --- some special items received from a logger e.g. cambridge device ----*/

    /** Contains the last received wind direction in degrees. */
    short _lastWindDirection;
    /** Contains the last received wind speed. */
    Speed _lastWindSpeed;
    /** Contains the last wind age in seconds */
    int _lastWindAge;
    /** Contains the last QNH value */
    ushort _lastQnh;
    /** Contains the last variometer speed */
    Speed _lastVariometer;

    /** This timer fires if a timeout on the data reception occurs.
     * The connection is then probably (temporary?) lost. */
    QTimer* timeOut;
    /** This timer fires if a timeout on the fix occurs.
     * The satellite reception is then probably (temporary?) lost. */
    QTimer* timeOutFix;
    /** Indicates the current GPS connection status */
    GpsStatus _status;
    /** Indicates the altitude delivered by the GPS unit */
    DeliveredAltitude _deliveredAltitude;
    /** The correction for the altitude when the altitude type is USER */
    Altitude _userAltitudeCorrection;
    /** Flag to ignore a lost connection, caused by a system clock update */
    bool _ignoreConnectionLost;
    /** SIV sentence count */
    uint cntSIVSentence;
    /** Published SIV list */
    QList<SIVInfo> sivInfo;
    /** Internal SIV list */
    QList<SIVInfo> sivInfoInternal;
    /** Map datum */
    QString _mapDatum;
    /** selected GPS device */
    QString gpsDevice;
    /** reference to the normal serial connection */
    GPSCon* serial;

#ifdef MAEMO
    /** reference to the Maemo GPS daemon connection */
    GpsMaemo* gpsdConnection;
#endif

    // number of created class instances
    static short instances;

  public:

    // make class object for all available
    static GpsNmea *gps;
  };

#endif
