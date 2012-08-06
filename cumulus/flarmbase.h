/***********************************************************************
**
**   flarmbase.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
 * \date 2010-2012
 *
 * \version $Id$
 */

#ifndef FLARM_BASE_H
#define FLARM_BASE_H

#include <QString>
#include <QHash>
#include <QMutex>

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
   * \date 2010
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
  };

  /**
   * \struct FlarmVersion
   *
   * \author Axel Pauli
   *
   * \brief FLARM version structure.
   *
   * FLARM version structure. It contains the version data reported by the Flarm.
   *
   * \date 2012
   */
  struct FlarmVersion
  {
    QString hwVersion;
    QString swVersion;
    QString obstVersion;
    QString igcVersion;
    QString serial;
    QString radioId;

    void reset()
    {
      hwVersion.clear();
      swVersion.clear();
      obstVersion.clear();
      igcVersion.clear();
      serial.clear();
      radioId.clear();
    };
  };

  /**
   * \struct FlarmError
   *
   * \author Axel Pauli
   *
   * \brief FLARM error structure.
   *
   * FLARM error structure. It contains the error status reported by the Flarm.
   *
   * \date 2012
   */
  struct FlarmError
  {
    QString severity;
    QString errorCode;

    void reset()
    {
      severity.clear();
      errorCode.clear();
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
    int     IdType;
    QString ID;
    int     Track;       // 0-359 or INT_MIN in stealth mode
    double  TurnRate;    // degrees per second or INT_MIN in stealth mode
    double  GroundSpeed; // meters per second or INT_MIN in stealth mode
    double  ClimbRate;   // meters per second or INT_MIN in stealth mode
    short   AcftType;
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
   * @return the Flarm version structure
   */
  static FlarmVersion& getFlarmVersion()
  {
    return m_flarmVersion;
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
    m_flarmStatus.valid = false;
    m_flarmVersion.reset();
    m_flarmError.reset();
  };

  static enum ProtocolMode getProtocolMode()
  {
    QMutexLocker ml(&m_mutex);
    return m_protocolMode;
  };

  static void setPotocolMode( enum ProtocolMode pm )
  {
    QMutexLocker ml(&m_mutex);
    m_protocolMode = pm;
  };

 protected:

  /**
   * Flarm status structure. Contains the data of the last parsed PFLAU
   * sentence.
   */
  static FlarmStatus m_flarmStatus;

  /**
   * Flarm version data.
   */
  static FlarmVersion m_flarmVersion;

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

#endif /* FLARM_BASE_H */
