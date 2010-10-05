/***********************************************************************
**
**   flarm.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief This class parses Flarm sentences and provides the results to the caller.
 *
 */

#ifndef FLARM_H
#define FLARM_H

#include <QObject>
#include <QString>
#include <QTime>
#include <QHash>
#include <QPoint>

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
   * FLARM status structure. It contains the last data of the sentence
   * PFLAU.
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
   * FLARM aircraft data structure. It contains the data of the
   * sentence PFLAA.
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
   * @returns the single instance of the class.
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
   * @Returns the Flarm status structure with the last parsed data
   */
  static const FlarmStatus& getFlarmStatus()
  {
    return flarmStatus;
  };

  /**
   * @param relativeBearing returns the relative bearing in degree from the
   * own position as integer -180...+180.
   * @return true is a valid value exists otherwise false
   */
  bool getFlarmRelativeBearing( int &relativeBearing );

  /**
   * @param relativeVertical Returns the relative vertical separation in
   * meters from the own position as integer. Plus means above own position
   * minus means below own position.
   * @return true is a valid value exists otherwise false
   */
  bool getFlarmRelativeVertical( int &relativeVertical );

  /**
   * @param relativeDistance returns the relative horizontal distance in
   * meters from the own position as integer.
   * @return true is a valid value exists otherwise false
   */
  bool getFlarmRelativeDistance( int &relativeDistance );

  /**
   * Extracts all items from the $PFLAU sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAU as string list
   * @return true is a valid value exists otherwise false
   */
  bool extractPflau(const QStringList& stringList);

  /**
   * Extracts all items from the $PFLAA sentence sent by the Flarm device.
   *
   * @param stringList Flarm sentence $PFLAA as string list
   * @param aircraft extracted aircraft data from sentence
   * @return true is a valid value exists otherwise false
   */
  bool extractPflaa( const QStringList& stringList, FlarmAcft& aircraft );

  /**
   * Creates a hash key by using the passed parameters.
   *
   * @param idType <ID-Type> tag of Flarm sentence $PFLAA
   * @param id <ID> 6-digit hex value of Flarm sentence $PFLAA
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
  };

  /**
   * PFLAA data collection is finished.
   */
  void collectPflaaFinished();

signals:

  /**
   * This signal is emitted if a complete sequence of PFLAA sentences has been
   * received.
   */
  void newFlarmPflaaData();

  /**
   * This signal is emitted, if no new Flarm data are received and the
   * data expecting timeout has expired.
   */
  void flarmPflaaDataTimeout();

private slots:

  /** Called if timer has expired. Used for Flarm data clearing. */
  void slotTimeout();

private:

  /**
   * Flarm status structure. Contains the data of the last parsed PFLAU
   * sentence.
   */
  static FlarmStatus flarmStatus;

  /** Flag to switch on the collecting of PFLAA data. */
  static bool collectPflaa;

  /** Hash map with collected PFLAA records. The key is a concatenation of
   *  the Flarm tags <ID-Type> and <ID>.*/
  static QHash<QString, FlarmAcft> pflaaHash;

  /** Timer for data clearing. */
  QTimer *timer;
};

#endif /* FLARM_H_ */
