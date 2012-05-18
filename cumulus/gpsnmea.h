/***************************************************************************
    gpsnmea.h - Cumulus NMEA parser and decoder
                            -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002      by André Somers,
                               2008-2012 by Axel Pauli
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

/**
 * \class GpsNmea
 *
 * \author André Somers, Axel Pauli
 *
 * \brief NMEA parser, decoder and GPS connection handler.
 *
 * This class parses and decodes the NMEA sentences and provides access
 * to the last know data. Furthermore it is managing the connection to a GPS
 * receiver connected by RS232, USB or to a Maemo GPS daemon process.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef GPS_NMEA_H
#define GPS_NMEA_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QPoint>
#include <QTimer>
#include <QHash>
#include <QFile>
#include <QSet>
#include <QMutex>

#include "speed.h"
#include "altitude.h"
#include "wgspoint.h"

#ifndef ANDROID
#include "gpscon.h"
#else
#include <QEvent>
#endif

struct SatInfo
  {
    int fixValidity;
    int fixAccuracy;
    int satsInView;
    int satsInUse;
    QString constellation;
    QTime constellationTime;
  };

struct SIVInfo
  {
    int id;        // Satellite identifier
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

#ifdef MAEMO

/**
 * The following two enumerations are used by Maemo's Location Service for
 * status and mode encoding. They are reused here again for decoding purposes.
 *
 * See here for more information:
 *
 * http://maemo.org/api_refs/5.0/5.0-final/liblocation/LocationGPSDevice.html
 */

/**
Enumeration representing the various states that a Maemo GPS device can be in.

LOCATION_GPS_DEVICE_STATUS_NO_FIX   The device does not have a fix.
LOCATION_GPS_DEVICE_STATUS_FIX      The device has a fix.
LOCATION_GPS_DEVICE_STATUS_DGPS_FIX The device has a DGPS fix.
                                    Deprecated: this constant is not used anymore.
*/

typedef enum
  {
    LOCATION_GPS_DEVICE_STATUS_NO_FIX,
    LOCATION_GPS_DEVICE_STATUS_FIX,
    LOCATION_GPS_DEVICE_STATUS_DGPS_FIX,
  } LocationGPSDeviceStatus;

/**
Enumeration representing the modes that a Maemo GPS device can operate in.

LOCATION_GPS_DEVICE_MODE_NOT_SEEN The device has not seen a satellite yet.
LOCATION_GPS_DEVICE_MODE_NO_FIX   The device has no fix.
LOCATION_GPS_DEVICE_MODE_2D       The device has latitude and longitude fix.
LOCATION_GPS_DEVICE_MODE_3D       The device has latitude, longitude, and altitude.
*/
typedef enum
  {
    LOCATION_GPS_DEVICE_MODE_NOT_SEEN,
    LOCATION_GPS_DEVICE_MODE_NO_FIX,
    LOCATION_GPS_DEVICE_MODE_2D,
    LOCATION_GPS_DEVICE_MODE_3D
  } LocationGPSDeviceMode;

#endif

class GpsNmea : public QObject
  {
    Q_OBJECT

  private:

    Q_DISABLE_COPY ( GpsNmea )

  public:

    /**
     * defines altitude bases delivered by GPS unit
     */
    enum DeliveredAltitude { GPS=0, PRESSURE=1 };

    /**
     * defines the states of the GPS unit
     */
    enum GpsStatus { notConnected=0, noFix=1, validFix=2 };

  public:

    GpsNmea(QObject* parent);

    virtual ~GpsNmea();

    /**
     * Enables or disables the notifications from the GPS receiver socket. Can
     * be used to stop GPS data receiving for a certain time to prevent data loss.
     * But be careful to prevent a receiver socket buffer overflow.
     */
    void enableReceiving( bool enable );

    /**
     * Starts the GPS client process and activates the GPS receiver.
     */
    void startGpsReceiver();

    /**
     * @return the current GPS connection status.
     */
    GpsNmea::GpsStatus getGpsStatus() const
      {
        return( _status );
      }

    /**
     * @return the current GPS connection status. True if connected, false if not.
     */
    bool getConnected() const
      {
        return( _status != notConnected );
      }

    /**
     * @return the last known speed.
     */
    Speed getLastSpeed() const
      {
        return _lastSpeed;
      };

    /**
     * @return the last known TAS.
     */
    Speed getLastTas() const
      {
        return _lastTas;
      };

    /**
     * @return the date of the last fix.
     */
    QDate getLastDate() const
      {
        return _lastDate;
      };

    /**
     * @return the time of the last fix.
     */
    QTime getLastTime() const
      {
        return _lastTime;
      };

    /**
     * @return the date time as UTC of the last fix.
     */
    QDateTime getLastUtc() const
      {
        return _lastUtc;
      };

    /**
     * @return the last known coordinate in KFLog format (x=lat, y=lon).
     */
    QPoint getLastCoord() const
      {
        return _lastCoord;
      };

    /**
     * @return the last know heading.
     */
    double getLastHeading() const
      {
        return _lastHeading;
      };

    /**
     * @return the last know standard pressure altitude
     */
    Altitude getLastStdAltitude() const
      {
        return _lastStdAltitude;
      };

    /**
     * @return the last know pressure altitude above sea level
     */
    Altitude getLastPressureAltitude() const
      {
        return _lastPressureAltitude;
      };

    /**
     * @return the last know gps altitude depending on user
     * selection MSL or Pressure
     */
    Altitude getLastAltitude() const;

    /**
     * @return the last know altitude above the WGS84 ellipsoid
     */
    Altitude getLastGNSSAltitude() const
      {
        return _lastGNSSAltitude;
      };

    /**
     * @return the last known wind speed.
     */
    Speed getLastWindSpeed() const
      {
        return _lastWindSpeed;
      };

    /**
     * @return the last known wind direction.
     */
    short getLastWindDirection() const
      {
        return _lastWindDirection;
      };

    /**
     * @return the last known wind age in seconds.
     */
    int getLastWindAge() const
      {
        return _lastWindAge;
      };

    /**
     * @return the last known variometer speed.
     */
    Speed getLastVariometer() const
      {
        return _lastVariometer;
      };

    /**
     * @return the last know satellite constellation string.
     */
    SatInfo& getLastSatInfo()
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
     * @return selected altitude reference delivered by the GPS unit
     */
    GpsNmea::DeliveredAltitude getDeliveredAltitude() const
      {
        return _deliveredAltitude;
      };

    /**
     * @return the satellites in view.
     */
    QList<SIVInfo>& getSivInfo()
    {
      return sivInfo;
    };

    /**
     * @return the map datum of the GPS receiver.
     */
    QString getMapDatum() const
      {
        return _mapDatum;
      };

    /**
     * Puts all desired GPS message keys into the passed hash. This method is
     * thread safe.
     *
     * @param gpsKeys hash table where the results are stored.
     *
     */
    static void getGpsMessageKeys( QHash<QString, short>& gpsKeys );

  public slots: // Public slots
    /**
     * This slot is called by the GpsCon object when a new
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
     * called to open the NMEA log file.
     */
    void slot_openNmeaLogFile();

    /**
     * called to close the NMEA log file.
     */
    void slot_closeNmeaLogFile();

#if 0
    /**
     * This slot is called to reset the gps device to factory
     * settings
     */
    void sendFactoryReset();

    /**
     * This slot is called to switch debugging mode on/off
     */
    void switchDebugging (bool on);

    /** This function sends the data of last valid fix to the gps receiver. */
    void sendLastFix (bool hard, bool soft);
#endif

#ifdef ANDROID

  protected:

    /** Add an event receiver, used by Android only. */
    bool event(QEvent *event);

#endif

  private slots: // Private slots

    /** This slot is called by the external GPS receiver process to signal
     *  a connection lost to the GPS receiver or daemon.
     */
    void _slotGpsConnectionOff();

    /** This slot is called by the external GPS receiver process to signal
     *  a established connection to the GPS receiver or daemon.
     */
    void _slotGpsConnectionOn();

    /** This slot is called by the internal timer to signal a timeout.
     *  This timeout occurs if no valid position fix has been received since
     *  the last fix for the given time. The cause can be bad receiver conditions
     *  view to the sky is not clear a.s.o.
     */
    void _slotTimeoutFix();

  signals: // Signals
    /**
     * This signal is emitted if the position has been changed.
     */
    void newPosition( QPoint &newPosition );

    /**
     * This signal is emitted if the altitude has been changed.
     */
    void newAltitude( Altitude& user, Altitude& std, Altitude& gnns );

    /**
     * This signal is emitted if a new speed fix has been established.
     */
    void newSpeed( Speed& newSpeed );

    /**
     * This signal is emitted if a new heading has been established.
     */
    void newHeading( const double& newHeading );

    /**
     * This signal is emitted if a new wind (speed, direction)
     * has been established.
     */
    void newWind( const Speed&, const short );

    /**
     * This signal is emitted if a new TAS value has been established.
     */
    void newTas( const Speed& );
    /**
     * This signal is emitted if a new variometer value has been established.
     */
    void newVario( const Speed& );

    /**
     * This signal is emitted if a new MacCready value has been established.
     */
    void newMc( const Speed& );

    /**
     * This signal is send if a new satellite constellation
     * has been detected (that is, the satellites used to
     * make a fix have changed).
     */
    void newSatConstellation( SatInfo& newConstellation );

    /**
     * This signal is send if a new satellite count
     * has been detected.
     */
    void newSatCount( SatInfo& satInfo );

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
    void newFix( const QTime& newFixTime );

    /**
     * This signal is send to indicate that new satellite in view
     * info is available.
     */
    void newSatInViewInfo( QList<SIVInfo>& siv);

    /**
     * Relay GpsCon newSentence signal
     */
    void newSentence(const QString&);

#ifdef FLARM

    /**
     * This signal is send to indicate that the number of received
     * Flarms has been changed.
     */
    void newFlarmCount( int newCount );

#endif

  private:

    /** Resets all data objects to their initial values. This is called
     *  at startup, at restart and if the GPS fix has been lost. */
    void resetDataObjects();

    /** write configuration data to allow restore of last fix */
    void writeConfig();

    /** Extracts GPRMC sentence. */
    void __ExtractGprmc( const QStringList& slst );
    /** Extracts GPGLL sentence. */
    void __ExtractGpgll( const QStringList& slst );
    /** Extracts GPGGA sentence. */
    void __ExtractGpgga( const QStringList& slst );
    /** Extracts PGRMZ sentence. */
    void __ExtractPgrmz( const QStringList& slst );
    /** Extracts PCAID sentence. */
    void __ExtractPcaid( const QStringList& slst );
    /** Extracts PGCS sentence. */
    void __ExtractPgcs( const QStringList& slst );
    /** Extracts GPDTM sentence. */
    void __ExtractGpdtm( const QStringList& slst );

#ifdef FLARM
    /** Extracts PFLAU sentence. */
    void __ExtractPflau( const QStringList& slst );
#endif

    /** This function return a QTime from the time encoded in a MNEA sentence. */
    QTime __ExtractTime(const QString& timestring);
    /** This function return a QDate from the date encoded in a MNEA sentence. */
    QDate __ExtractDate(const QString& datestring);
    /** This function return a Speed from the speed encoded in knots */
    Speed __ExtractKnotSpeed(const QString& speedstring);
    /** This function converts the coordinate data from the NMEA sentence to the internal QPoint coordinate format. */
    QPoint __ExtractCoord(const QString& slat, const QString& slatNS, const QString& slon, const QString& slonEW);
    /** Extract the heading from the NMEA sentence. */
    double __ExtractHeading(const QString& headingstring);
    /** Extracts the altitude from a NMEA GGA or Gramin/Flarm PGRMZ sentence */
    Altitude __ExtractAltitude(const QString& altitude, const QString& unit);
    /** Extracts the constellation from the NMEA sentence. */
    QString __ExtractConstellation(const QStringList& sentence);
    /** Extracts the satellites in view from the NMEA sentence. */
    bool __ExtractSatsInView(const QString& satcount);
    /** Extracts satellites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QStringList& sentence);
    /** Extracts satellites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QString&, const QString&, const QString&, const QString&);
    /** Extracts wind, QNH and vario data from Cambridge's !w sentence. */
    void __ExtractCambridgeW(const QStringList& stringList);
    /**
     * Extracts speed, altitude, vario, heading, wind data from LX Navigation $LXWP0
     * sentence.
     */
    void __ExtractLxwp0(const QStringList& stringList);
    /**
     * Extracts McCready data from LX Navigation $LXWP2 sentence.
     */
    void __ExtractLxwp2(const QStringList& stringList);

#ifdef MAEMO
    /**
     * Extract proprietary sentence $MAEMO0.
     */
    void __ExtractMaemo0(const QStringList& stringList);
    /**
     * Extract proprietary sentence $MAEMO1.
     */
    void __ExtractMaemo1(const QStringList& stringList);
#endif

    /** This function is called to indicate that good data has been received.
     *  It resets the TimeOut timer and if necessary changes the connected status.
     */
    void dataOK();
    /** This function is called to indicate that a good fix has been received. */

  public:

    void fixOK( const char* who );
    /** This function is called to indicate that a negative fix has been received. */
    void fixNOK( const char* who );

  private:

    /** This function calculates the checksum in the sentence. */
    static uint calcCheckSum (int pos, const QString& sentence);
    /** This function checks if the checksum in the sentence matches the sentence.
     *  It return true if it matches, and false otherwise. */
    static bool checkCheckSum(int pos, const QString& sentence);

    /** This function calculates the STD altitude from the passed MSL altitude. */
    void calcStdAltitude(const Altitude& altitude);
    /** This function calculates the MSL altitude from the passed STD altitude. */
    void calcMslAltitude(const Altitude& altitude);

    /** Set system date/time. Input is UTC related. */
    void setSystemClock( const QDateTime& utcDt );
    /** create a GPS connection */
    void createGpsConnection();

  private: // Private attributes

    /** contains the time of the last RMC fix */
    QTime _lastRmcTime;
    /** contains the time of the last fix */
    QTime _lastTime;
    /** contains the date of the last fix. */
    QDate _lastDate;
    /** contains the last utc date/time of the last fix */
    QDateTime _lastUtc;
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
    /** Force one altitude report after reset, regardless of change or not */
    bool _reportAltitude;
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
    /** Contains the last MacCready setting */
    Speed _lastMc;
    /** Contains the last TAS setting */
    Speed _lastTas;

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

#ifndef ANDROID
    /** The reference to the used serial connection */
    GpsCon* serial;
#else
    QObject* serial;
#endif

    /** Flag to enable/disable the GPS data processing. */
    bool _enableGpsDataProcessing;

    /** NMEA log file */
    QFile* nmeaLogFile;
    /** Flag to indicate receive of GPRMC. */
    bool _gprmcSeen;

#ifdef FLARM

    /** Flag to control begin and end of receiving PFLAA sentences. */
    bool pflaaIsReceiving;

#endif

    // number of created class instances
    static short instances;

    // Dictionary with known sentence keywords
    static QHash<QString, short> gpsHash;

    // Set with reported unknown GPS keys
    QSet<QString> reportedUnknownKeys;

    /** Mutex for thread synchronization. */
    static QMutex mutex;

  public:

    // make class object for all available
    static GpsNmea *gps;
  };

#endif
