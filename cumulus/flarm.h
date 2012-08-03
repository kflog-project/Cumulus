/***********************************************************************
**
**   flarm.h
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
 * \class Flarm
 *
 * \author Axel Pauli
 *
 * \brief Flarm data parser and manager class.
 *
 * This class parses Flarm sentences and provides the results to the caller.
 *
 * \date 2010-2012
 */

#ifndef FLARM_H
#define FLARM_H

#include <QObject>
#include <QString>
#include <QTime>
#include <QHash>
#include <QMutex>

class QPoint;
class QStringList;
class QTimer;

class Flarm : public QObject
{
  Q_OBJECT

private:

  /**
   * Constructor is private because this is a singleton class.
   */
  Flarm( QObject* parent=0 );

  Q_DISABLE_COPY ( Flarm )

public:

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

  virtual ~Flarm();

  /**
   * @return the single instance of the class.
   */
  static Flarm* instance()
  {
    static Flarm instance;

    return &instance;
  };

  /**
   * @param flag true or false to switch on/off PFLAA data collection.
   */
  static void setCollectPflaa( bool flag )
  {
    collectPflaa = flag;
  };

  /**
   * @return flag return current state of PFLAA data collection flag.
   */
  static bool getCollectPflaa()
  {
    return collectPflaa;
  };

  /**
   * @return the Flarm status structure with the last parsed data
   */
  static const FlarmStatus& getFlarmStatus()
  {
    return flarmStatus;
  };

  /**
   * @return the Flarm version structure
   */
  static FlarmVersion& getFlarmVersion()
  {
    return flarmVersion;
  };

  /**
   * @return the Flarm error structure
   */
  static const FlarmError& getFlarmError()
  {
    return flarmError;
  };

  /**
   * @param relativeBearing returns the relative bearing in degree from the
   * own position as integer -180...+180.
   * @return true if a valid value exists otherwise false
   */
  bool getFlarmRelativeBearing( int &relativeBearing );

  /**
   * @param relativeVertical Returns the relative vertical separation in
   * meters from the own position as integer. Plus means above own position
   * minus means below own position.
   * @return true if a valid value exists otherwise false
   */
  bool getFlarmRelativeVertical( int &relativeVertical );

  /**
   * @param relativeDistance returns the relative horizontal distance in
   * meters from the own position as integer.
   * @return true if a valid value exists otherwise false
   */
  bool getFlarmRelativeDistance( int &relativeDistance );

  /**
   * Extracts all items from the $PFLAU sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAU as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflau(const QStringList& stringList);

  /**
   * Extracts all items from the $PFLAA sentence sent by the Flarm device.
   *
   * @param stringList Flarm sentence $PFLAA as string list
   * @param aircraft extracted aircraft data from sentence
   * @return true if a valid value exists otherwise false
   */
  bool extractPflaa( const QStringList& stringList, FlarmAcft& aircraft );

  /**
   * Extracts all items from the $PFLAV sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAV as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflav(const QStringList& stringList);

  /**
   * Extracts all items from the $PFLAE sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAV as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflae(const QStringList& stringList);

  /**
   * Extracts all items from the $PFLAC sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAV as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflac(QStringList& stringList);

  /**
   * PFLAA data collection is finished.
   */
  void collectPflaaFinished();

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
   * @return the pflaaHash to the caller.
   */
  static const QHash<QString, FlarmAcft>& getPflaaHash()
  {
    return pflaaHash;
  };

  /**
   * Resets the internal stored Flarm data.
   */
  static void reset()
  {
    pflaaHash.clear();
    flarmStatus.valid = false;
    flarmVersion.reset();
    flarmError.reset();
  };

  static enum ProtocolMode getProtocolMode();

  static void setPotocolMode( enum ProtocolMode pm );

private:

  /**
   * Creates a traffic message in HTML format and emits this message as signal.
   */
  void createTrafficMessage();

signals:

  /**
   * This signal is emitted if a complete sequence of PFLAA sentences has been
   * received.
   */
  void newFlarmPflaaData();

  /**
   * This signal is emitted if a new Flarm traffic info is available.
   */
  void flarmTrafficInfo( QString& info );

  /**
   * This signal is emitted, if no new Flarm data are received and the
   * data expecting timeout has expired.
   */
  void flarmPflaaDataTimeout();

  /**
   * This signal is emitted if a new Flarm error info is available.
   */
  void flarmErrorInfo( const Flarm::FlarmError& info );

  /**
   * This signal is emitted if a new Flarm version info is available.
   */
  void flarmVersionInfo( const Flarm::FlarmVersion& info );

  /**
   * This signal is emitted if a new Flarm configuration info is available.
   */
  void flarmConfigurationInfo( QStringList& info );

private slots:

  /** Called if m_timer has expired. Used for Flarm data clearing. */
  void slotTimeout();

private:

  /**
   * Flarm status structure. Contains the data of the last parsed PFLAU
   * sentence.
   */
  static FlarmStatus flarmStatus;

  /**
   * Flarm version data.
   */
  static FlarmVersion flarmVersion;

  /**
   * Flarm error data.
   */
  static struct FlarmError flarmError;

  /** Flag to switch on the collecting of PFLAA data. */
  static bool collectPflaa;

  /**
   * Hash map with collected PFLAA records. The key is a concatenation of
   * the Flarm tags 'ID-Type' and 'ID'.
   */
  static QHash<QString, FlarmAcft> pflaaHash;

  /** Timer for data clearing. */
  QTimer* m_timer;

  /** Flarm protocol mode.  */
  static enum ProtocolMode m_protocolMode;

  static QMutex m_mutex;
};

#endif /* FLARM_H_ */
