/***********************************************************************
**
**   preflightmiscpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers
**                   2008-2012 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class PreFlightMiscPage
 *
 * \author André Somers, Axel Pauli
 *
 * \brief A widget for preflight miscellaneous settings.
 *
 * \date 2004-2012
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_MISCPAGE_H
#define PREFLIGHT_MISCPAGE_H

#include <QWidget>

#include "altitude.h"
#include "speed.h"

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;

class PreFlightMiscPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightMiscPage )

 public:

  PreFlightMiscPage(QWidget *parent=0);

  virtual ~PreFlightMiscPage();

  void load();

  void save();

 private slots:

  /**
   * Called to open the flight logbook dialog.
   */
  void slotOpenLogbook();

#ifdef FLARM

  /**
   * Called to open the Flarm download flight dialog.
   */
  void slotOpenFlarmFlights();

#endif

 private:

  QCheckBox*      m_chkLogAutoStart;
  QDoubleSpinBox* m_logAutoStartSpeed;
  QSpinBox*       m_edtMinimalArrival;
  QComboBox*      m_edtArrivalAltitude;
  QSpinBox*       m_edtQNH;
  QSpinBox*       m_bRecordInterval; // B-Record logging interval in seconds
  QSpinBox*       m_kRecordInterval; // K-Record logging interval in seconds

  double m_loadedSpeed;

  /** saves horizontal speed unit during construction of object */
  Speed::speedUnit m_speedUnit;

  /** saves altitude unit set during construction of object */
  Altitude::altitudeUnit m_altUnit;
};

#endif
