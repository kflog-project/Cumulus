/***********************************************************************
**
**   vario.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class Vario
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Variometer calculations.
 *
 * This class executes the variometer calculations.
 *
 *\date 2002-2013
 */

#ifndef VARIO_H
#define VARIO_H

#include <QObject>
#include <QTimer>

#include "altitude.h"
#include "limitedlist.h"
#include "speed.h"

/** Default integration time in seconds for variometer calculation. */
#define INT_TIME 3

class Vario: public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( Vario )

public:

  Vario( QObject* object );

  virtual ~Vario();

  /**
   * Called to signal that a new altitude value is available. That triggers the
   * variometer calculation.
   */
  void newAltitude();

  /**
   * Called to signal that a new pressure altitude value is available.
   *  That triggers the variometer calculation.
   */
  void newPressureAltitude( const Altitude& altitude, const Speed& tas );

public slots:

  /**
   * This slot is called, if the integration time has been changed in the UI.
   *
   * @param newTime new time value in seconds
   */
  void slotNewVarioTime(int newTime);

  /**
   * This slot is called, if the TEK mode has been changed in the UI.
   *
   * @param newMode new mode value used to switch on/off TEK calculation
   */
  void slotNewTEKMode(bool newMode);

  /**
   * This slot is called, if the TEK adjust value has been changed in the UI.
   *
   * @param newAdjust new adjust value in percent
   */
  void slotNewTEKAdjust(int newAdjust);

signals:

  /**
   * This signal is emitted when a new variometer value is available.
   *
   * @param newLift new available lift value
   */
  void newVario(const Speed& newLift);

private:

  QTimer  m_timeOut; // calling supervision timer
  qint64  m_intTime; // integration time in ms
  bool    m_TEKOn;   // TEK compensated Mode
  double  m_energyAlt; // v*v/2g
  double  m_TekAdjust; // adjust TEK Compensation

  class AltSample
  {
   public:

    AltSample() :
      altitude(0.0),
      tas(0.0),
      timeStamp(0)
    {};

    double altitude;  // unit is meters
    double tas;       // unit is meters per second
    qint64 timeStamp; // unit is milliseconds since the epoch
  };

  // List of altitude samples
  LimitedList<AltSample> m_sampleList;

private slots:

  /**
   * This slot is called by the internal timer to signal a
   * timeout. It resets the variometer to the initial settings.
   */
  void slotTimeout();
};

#endif
