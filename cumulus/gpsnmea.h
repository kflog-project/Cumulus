/***************************************************************************
                          gpsnmea.h  - NMEA sentence decoding
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2008 by Axel Pauli
    email                : axel@kflog.org

    $Id$

***************************************************************************/

/***************************************************************************
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
 * This class provided the interprettation of the NMEA sentences, and
 * provides access to the last know data.
 *
 *@author André Somers
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
    int id;        //satelite id
    int db;        //signal to noice ratio (0-99), or negative for not tracking
    int azimuth;   //0-359
    int elevation; //0-90
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

class GPSNMEA : public QObject
  {
    Q_OBJECT

  public:

    /**
     * defines for altitude bases delivered by gps unit
     */
    enum DeliveredAltitude{MSL=0, HAE=1, USER=2};
    
    enum connectedStatus{notConnected=0, noFix=1, validFix=2};

  public:

    GPSNMEA(QObject* parent);
    
    virtual ~GPSNMEA();

    /**
     * @Starts the GPS receiver client process and activates the receiver.
     */
     void startGpsReceiver();

    /**
     * @Returns the current GPS connection status. True if connected, false if not.
     */
    bool getConnected();

    /**
     * @Returns the last known speed.
     */
    Speed getLastSpeed();

    /**
     * @Returns the date of the last fix.
     */
    QDate getLastDate();

    /**
     * @Returns the time of the last fix.
     */
    QTime getLastTime();

    /**
     * @Returns the last known coordinate in KFLog format (x=lat, y=lon).
     */
    QPoint getLastCoord();

    /**
     * @Returns the last know heading.
     */
    double getLastHeading();

    /**
     * @Returns the last know standard pressure altitude
     */
    Altitude getLastStdAltitude();

    /**
     * @Returns the last know pressure altitude above sea level
     */
    Altitude getLastPressureAltitude();

    /**
     * @Returns the last know gps altitude above sea level
     */
    Altitude getLastAltitude();

    /**
     * @Returns the last know altitude above the WGS84 ellipsoidl
     */
    Altitude getLastGNSSAltitude();

    /**
     * @Returns the last know satellite constellation string.
     */
    SatInfo getLastSatInfo();

    /**
     * force a reset of the serial connection after a resume
     */
    void forceReset();

    /**
     * set altitude reference delivered by the GPS unit
     */
    void setDeliveredAltitude( const GPSNMEA::DeliveredAltitude newAltRef )
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
     * @return selected altitude reference deliverd by the GPS unit
     */
    const GPSNMEA::DeliveredAltitude getDeliveredAltitude() const
      {
        return _deliveredAltitude;
      };

    QList<SIVInfo>& getSivInfo()
    {
      return sivInfo;
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

  signals: // Signals
    /**
     * This signal signifies a new position fix.
     */
    void newPosition();

    /**
     * This signal is emitted if the altitude has changed.
     */
    void newAltitude();

    /**
     * This signal is emitted if a new speed fix has been
     * established.
     */
    void newSpeed();

    /**
     * This signal is emitted if a new heading has been
     * estabished.
     */
    void newHeading();

    /**
     * This signal is send if a new sataliteconstellation
     * has been detected (that is, the satalites used to
     * make a fix have changed).
     */
    void newSatConstellation();

    /**
     * This signal is send to indicate a change in the
     * connected status. The new status is send as bool
     * (connected=true).
     */
    void connectedChange(bool);

    /**
     * This signal is send to indicate a change in status.
     * It supersedes the old connecedChange(bool) signal.
     */
    void statusChange(GPSNMEA::connectedStatus);

    /**
     * This signal is send to indicate that there is a new fix.
     * Data send after this fix belongs to the new fix!
     */
    void newFix();

    /**
     * This signal is send to indicate that new satelite-in-view
     * info is available.
     */
    void newSatInViewInfo();

    /**
     * Relay GPSCon newSentence signal
     */
    void newSentence(const QString&);

  private: // Private methods

    /** write config data to allow restore of last fix */
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
    /** Extracts the satcount from the NMEA sentence. */
    void __ExtractSatcount(const QString& satcount);
    /** Extract Satelites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QStringList& sentence);
    /** Extract Satelites In View (SIV) info from a NMEA sentence. */
    void __ExtractSatsInView(const QString&, const QString&, const QString&, const QString&);

    /** This function is called to indicate that good dat has been received. It resets the TimeOut timer and if necesairy changes the connected status. */
    void dataOK();
    /** This function is called to indicate that a good fix has been received. */
    void fixOK();
    /** This function calculates the checksum in the sentence. */
    static uint calcCheckSum (int pos, const QString& sentence);
    /** This function checks if the checksum in the sentence matches the sentence. It retuns true if it matches, and false otherwise. */
    static bool checkCheckSum(int pos, const QString& sentence);
    /** This function sends the data of last valid fix to the gps receiver. */
    void sendLastFix (bool hard, bool soft);
    /** Set system date/time. Input is utc related. */
    void setSystemClock( const QDateTime& utcDt );
    /** create a gps connection */
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
    /** This timer fires if a timeout on the datareception occurs. The connection is then probably (temporary?) lost. */
    QTimer * timeOut;
    /** This timer fires if a timeout on the fix occurs. The satelite reception is then probably (temporary?) lost. */
    QTimer * timeOutFix;
    /** Indicates the current connection status */
    connectedStatus _status;
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
    /** selected GPS device */
    QString gpsDevice;
    /** reference to the normal serial connection */
    GPSCon * serial;
    
#ifdef MAEMO
    /** reference to the Maemo gpsd connection */
    GpsMaemo * gpsdConnection;
#endif  

  private slots: // Private slots

    /** This slot is called by the internal timer to signal a timeout. If this timeout occurs, the connection is set to false, i.e., the connection has been lost. */
    void _slotTimeout();
    /** This slot is called by the internal timer to signal a timeout. If this timeout occurs, the fix is set to false. */
    void _slotTimeoutFix();
  };

extern GPSNMEA *gps;

#endif
