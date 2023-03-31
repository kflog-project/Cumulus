/***********************************************************************
**
**   flarmbase.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class FlarmBase
 *
 * \author Axel Pauli
 *
 * \brief Flarm base class.
 *
 * This is the base Flarm class containing static methods and data definitions.
 *
 * \date 2010-2023
 *
 * \version 1.13
 */

#pragma once

#include <QDateTime>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

class QPoint;
class QStringList;
class QTimer;

class FlarmBase
{
 public:

  FlarmBase();

  virtual ~FlarmBase();

  /**
   * FLARM Alarm Level definitions.
   */
  enum AlarmLevel
  {
    No=0,
    Low=1,
    Important=2,
    Urgent=3
  };

  /**
   * FLARM GPS fix definitions.
   */
  enum GpsStatus
  {
    NoFix=0,
    GroundFix=1,
    MovingFix=2
  };

  /**
   * Flarm protocol mode. Can be text or binary.
   */
  enum ProtocolMode
  {
    text,
    binary
  };

  /**
   * \struct FlarmStatus
   *
   * \author Axel Pauli
   *
   * \brief FLARM status structure.
   *
   * FLARM status structure. It contains the last data of the PFLAU sentence.
   *
   * \date 2010-2018
   */
  struct FlarmStatus
  {
    bool    valid; // true displays a valid filled structure
    short   RX;
    short   TX;
    enum    GpsStatus Gps;
    short   Power;
    enum AlarmLevel Alarm;
    QString RelativeBearing;  // can be empty
    short   AlarmType;
    QString RelativeVertical; // can be empty
    QString RelativeDistance; // can be empty
    QString ID;               // can be empty

    FlarmStatus() :
      valid(false),
      RX(0),
      TX(0),
      Gps(NoFix),
      Power(0),
      Alarm(No),
      AlarmType(0)
    {};

    void reset()
    {
      valid = false;
      RX = 0;
      TX = 0;
      Gps = NoFix;
      Power = 0;
      Alarm = No;
      RelativeBearing.clear();
      AlarmType = 0;
      RelativeVertical.clear();
      RelativeDistance.clear();
      ID.clear();
    }
    /**
     * This flag handles the validity of the structure data.
     *
     * \return state of valid flag. Can be true or false.
     */
    bool isValid() const
    {
      return valid;
    }

  };

  /**
   * \struct FlarmData
   *
   * \author Axel Pauli
   *
   * \brief FLARM configuration data structure.
   *
   * FLARM configuration data  structure. It contains the configuration data
   * reported by the Flarm device. The data members are identical to the Flarm
   * key words.
   *
   * \date 2012-2021
   */
  struct FlarmData
  {
    QString acft;
    QString audioout;
    QString audiovolume;
    QString batterytype;
    QString brightness;
    QString baud;
    QString baud1;
    QString baud2;
    QString build;
    QString cap;
    QString cflags;
    QString compid;
    QString compclass;
    QString copil;
    QString deviceid;
    QString devtype;
    QString flarmver;
    QString gliderid;
    QString glidertype;
    QString hwver;
    QString id;
    QString igcser;
    QString logint;
    QString nmeaout;
    QString nmeaout1;
    QString nmeaout2;
    QString notrack;

    struct Obstdb
    {
      QString version;
      QString status;
      QString name;
      QString date;
    } obstdb;

    QString obstexp;
    QString pilot;
    QString priv;
    QString radioid;
    QString range;
    QString region;
    QString ser;
    QString swexp;
    QString swver;
    QString task;
    QString thre;
    QString ui;
    QString vol;
    QString vrange;

    void reset()
      {
        acft.clear();
        audioout.clear();
        audiovolume.clear();
        batterytype.clear();
        brightness.clear();
        baud.clear();
        baud1.clear();
        baud2.clear();
        build.clear();
        cap.clear();
        cflags.clear();
        compid.clear();
        compclass.clear();
        copil.clear();
        deviceid.clear();
        devtype.clear();
        flarmver.clear();
        gliderid.clear();
        glidertype.clear();
        hwver.clear();
        id.clear();
        igcser.clear();
        logint.clear();
        nmeaout.clear();
        nmeaout1.clear();
        nmeaout2.clear();
        notrack.clear();

        obstdb.status.clear();
        obstdb.status.clear();
        obstdb.name.clear();
        obstdb.date.clear();

        obstexp.clear();
        pilot.clear();
        priv.clear();
        radioid.clear();
        range.clear();
        region.clear();
        ser.clear();
        swexp.clear();
        swver.clear();
        task.clear();
        thre.clear();
        ui.clear();
        vol.clear();
        vrange.clear();
      };
  };

  /**
   * \struct FlarmError
   *
   * \author Axel Pauli
   *
   * \brief FLARM error structure.
   *
   * FLARM error structure. It contains the last reported error status by the Flarm.
   *
   * \date 2012-2015
   */
  struct FlarmError
  {
    QString severity;
    QString errorCode;
    QString errorText;

    void reset()
    {
      severity.clear();
      errorCode.clear();
      errorText.clear();
    };
   };

  /**
   * \struct FlarmAcft
   *
   * \author Axel Pauli
   *
   * \brief FLARM aircraft data structure.
   *
   * FLARM aircraft data structure. It contains the data of a PFLAA sentence.
   *
   * \date 2010
   */
  struct FlarmAcft
  {
    QTime   TimeStamp;  // Creation time of this structure
    enum AlarmLevel Alarm;
    int     RelativeNorth;
    int     RelativeEast;
    int     RelativeVertical;
    short   IdType;
    QString ID;
    int     Track;       // 0-359 or INT_MIN in stealth mode
    double  TurnRate;    // degrees per second or INT_MIN in stealth mode
    double  GroundSpeed; // meters per second or INT_MIN in stealth mode
    double  ClimbRate;   // meters per second or INT_MIN in stealth mode
    short   AcftType;
  };

  /**
   * \struct FlarmAlertZone
   *
   * \author Axel Pauli
   *
   * \brief FLARM alert zone data object.
   *
   * FLARM alert zone data object. It contains the data of a PFLAO sentence.
   *
   * \date 2015
   */
  struct FlarmAlertZone
  {
    bool    valid;         // valid flag of structure
    QTime   TimeStamp;     // Creation time of this structure
    enum AlarmLevel Alarmlevel;
    bool    Inside;        // 1=active and inside, 0=otherwise
    long    Latitude;      // in KFLog degree format
    long    Longitude;     // in KFLog degree format
    int     Radius;        // 0...2000m
    int     Bottom;        // -1000...6000m
    int     Top;           // 0...6000m
    ulong   ActivityLimit; // 0...4294967295
    short   ZoneType;      // 0x10 ... 0xFF
    QString ID;            // Flarm Identifier
    short   IdType;        // ID-Type
    QString Key;           // A key built from ID and IDType

    FlarmAlertZone() :
      valid(false),
      TimeStamp(0, 0, 0),
      Alarmlevel(No),
      Inside(0),
      Latitude(0),
      Longitude(0),
      Radius(0),
      Bottom(0),
      Top(0),
      ActivityLimit(0),
      ZoneType(0),
      IdType(-1)
      {};

    /**
     * This flag handles the validity of the structure data.
     *
     * \return state of valid flag. Can be true or false.
     */
    bool isValid() const
    {
      return valid;
    }

    /**
     * Check activity limit, if it has expired.
     *
     * \return true when active otherwise false
     */
    bool isActive()
    {
      quint64 secondsUtc = static_cast<quint64> (QDateTime::currentMSecsSinceEpoch() / 1000);

      if( ActivityLimit > 0 && ActivityLimit < secondsUtc )
        {
          return false;
        }

      return true;
    }
  };

  /**
   * @param flag true or false to switch on/off PFLAA data collection.
   */
  static void setCollectPflaa( bool flag )
  {
    m_collectPflaa = flag;
  };

  /**
   * @return flag return current state of PFLAA data collection flag.
   */
  static bool getCollectPflaa()
  {
    return m_collectPflaa;
  };

  /**
   * @return the Flarm status structure with the last parsed data
   */
  static const FlarmStatus& getFlarmStatus()
  {
    return m_flarmStatus;
  };

  /**
   * @return the Flarm data structure
   */
  static FlarmData& getFlarmData()
  {
    return m_flarmData;
  };

  /**
   * @return the Flarm error structure
   */
  static const FlarmError& getFlarmError()
  {
    return m_flarmError;
  };

  /**
   * Creates a hash key by using the passed parameters.
   *
   * @param idType 'ID-Type' tag of Flarm sentence $PFLAA
   * @param id 6-digit 'ID' hex value of Flarm sentence $PFLAA
   * @return A hash key generated from the input.
   */
  static QString createHashKey( int idType, const QString& id )
  {
    Q_UNUSED(idType)
    // return QString("%1-%2").arg(idType).arg(id);
    // idType is not more considered due to alias selection
    return QString(id);
  };

  /**
   * @return the m_pflaaHash to the caller.
   */
  static const QHash<QString, FlarmAcft>& getPflaaHash()
  {
    return m_pflaaHash;
  };

  /**
   * Resets the internal stored Flarm data.
   */
  static void reset()
  {
    m_pflaaHash.clear();
    m_flarmStatus.reset();
    m_flarmData.reset();
    m_flarmError.reset();
  };

  /**
   * Returns a flag, if a Flarm decive has been seen.
   */
  static bool isFlarmAvailable()
  {
    return m_flarmStatus.valid;
  };

  /**
   * Returns the read Flarm device type.
   */
  static QString getDeviceType()
  {
    return m_flarmData.devtype;
  }

  static enum ProtocolMode getProtocolMode()
  {
    QMutexLocker ml(&m_mutex);
    return m_protocolMode;
  };

  static void setProtocolMode( enum ProtocolMode pm )
  {
    QMutexLocker ml(&m_mutex);
    m_protocolMode = pm;
  };

  /**
   * Replace all German umlauts in a string by two ASCIII characters.
   *
   * @param string String to be processed as latin-1 encoding.
   *
   * @return QByteArray with replaced umlauts.
   */
  static QByteArray replaceUmlauts( QString string );

  /**
   * Translates a hexadecimal alarm type into a readable string.
   *
   * \param hexType alarm type as byte value
   *
   * \return Alarm type as string
   */
  static QString translateAlarmType( const short hexType );

  /**
   * Translates a hexadecimal aircraft type into a readable string.
   *
   * \param acftType aircraft type as byte value
   *
   * \return Aircraft type as string
   */
  static QString translateAcftType( const short acftType );

 protected:

  /**
   * Flarm status structure. Contains the data of the last parsed PFLAU
   * sentence.
   */
  static FlarmStatus m_flarmStatus;

  /**
   * Flarm information data.
   */
  static FlarmData m_flarmData;

  /**
   * Flarm error data.
   */
  static struct FlarmError m_flarmError;

  /** Flag to switch on the collecting of PFLAA data. */
  static bool m_collectPflaa;

  /**
   * Hash map with collected PFLAA records. The key is a concatenation of
   * the Flarm tags 'ID-Type' and 'ID'.
   */
  static QHash<QString, FlarmAcft> m_pflaaHash;

  /** Flarm protocol mode.  */
  static enum ProtocolMode m_protocolMode;

  static QMutex m_mutex;
};

