/***********************************************************************
**
**   vario.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2010 by Axel Pauli
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
 *\date 2002-2010
 */

#ifndef VARIO_H
#define VARIO_H

#include <QObject>
#include <QTimer>

#include "speed.h"

/** Default integration time in s for variometer calculation. */
#define INT_TIME 5

class Vario: public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( Vario )

public:

  Vario(QObject*);

  virtual ~Vario();

  /**
   * Called to signal that a new altitude value is available. That triggers the
   * variometer calculation.
   */
  void newAltitude();

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

  QTimer  _timeOut; // calling supervision timer
  int     _intTime; // integration time
  bool    _TEKOn;   // TEK compensated Mode
  double  _energyAlt; // v*v/2g
  double  _TekAdjust; // adjust TEK Compensation

private slots:

  /**
   * This slot is called by the internal timer to signal a
   * timeout. It resets the variometer to the initial settings.
   */
  void _slotTimeout();
};

#endif
