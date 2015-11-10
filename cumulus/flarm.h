/***********************************************************************
**
**   flarm.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2015 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2010-2015
 *
 * \version 1.2
 */

#ifndef FLARM_H
#define FLARM_H

#include <QObject>
#include <QString>
#include <QTime>

#include "flarmbase.h"

class QPoint;
class QStringList;
class QTimer;

class Flarm : public QObject, public FlarmBase
{
  Q_OBJECT

 private:

  /**
   * Constructor is private because this is a singleton class.
   */
  Flarm( QObject* parent=0 );

  Q_DISABLE_COPY ( Flarm )

 public:

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
   * Extracts all items from the $PFLAR sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAR as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflar(QStringList& stringList);

  /**
   * Extracts all items from the $PFLAI sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAR as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflai(QStringList& stringList);

  /**
   * Extracts all items from the $PFLAO sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAR as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractPflao(QStringList& stringList);

  /**
   * Extracts all items from the $ERROR sentence sent by the Flarm device.
   * @param stringList Flarm sentence $PFLAV as string list
   * @return true if a valid value exists otherwise false
   */
  bool extractError(QStringList& stringList);

  /**
   * PFLAA data collection is finished.
   */
  void collectPflaaFinished();

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
   * This signal is emitted if a new Flarm error is available.
   */
  void flarmError( QStringList& info );

  /**
   * This signal is emitted if a new Flarm version info is available.
   */
  void flarmVersionInfo( const Flarm::FlarmVersion& info );

  /**
   * This signal is emitted if a Flarm reset command response is received.
   */
  void flarmResetResponse( QStringList& info );

  /**
   * This signal is emitted if a Flarm IGC command response is received.
   */
  void flarmIgcResponse( QStringList& info );

  /**
   * This signal is emitted if a new Flarm configuration info is available.
   */
  void flarmConfigurationInfo( QStringList& info );

  /**
   * This signal is emitted, if a new/updated alert zone is available.
   */
  void flarmAlertZoneInfo( FlarmBase::FlarmAlertZone& info );

 private slots:

  /** Called if m_timer has expired. Used for Flarm data clearing. */
  void slotTimeout();

 private:

  /** Timer for data clearing. */
  QTimer* m_timer;
};

#endif /* FLARM_H */
