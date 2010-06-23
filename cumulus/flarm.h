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
    QString RelativeBearing;
    short   AlarmType;
    QString RelativeVertical;
    QString RelativeDistance;
    QString ID;
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
    int     Id;
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
   * @Returns the Flarm status structure with the last parsed data
   */
  FlarmStatus& getFlarmStatus()
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
   * Extracts the $PFLAU sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAU as string list
   * @return true is a valid value exists otherwise false
   */
  bool extractPflau(const QStringList& stringList);

  /**
   * Extracts the $PFLAA sentence sent by the Flarm device.
   *
   * @param stringList Flarm sentence $PFLAA as string list
   * @param aircraft extracted aircraft data from sentence
   * @return true is a valid value exists otherwise false
   */
  bool extractPflaa( const QStringList& stringList, FlarmAcft& aircraft );

private:

  /** Flarm status */
  FlarmStatus flarmStatus;
};

#endif /* FLARM_H_ */
