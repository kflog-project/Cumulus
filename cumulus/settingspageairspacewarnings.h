/***********************************************************************
**
**   settingspageairspacewarnings.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirSpaceWarnings_h
#define SettingsPageAirSpaceWarnings_h

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>

#include "altitude.h"

/**
 * \class SettingsPageAirspaceWarnings
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Configuration settings for airspace warnings.
 *
 * \date 2002-2013
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceWarnings : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirspaceWarnings )

public:

  SettingsPageAirspaceWarnings( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceWarnings();

 public slots:
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file
   */
  void slot_load();

  /**
   * Called to set all spin boxes to the default value
   */
  void slot_defaults();

 private slots:
  /**
   * Invoked if enableWarning changes value
   * @param enabled true if warning is enabled
   */
  void slot_enabledToggled(bool enabled);

  /**
   * Called to change the step width of the spin boxes
   *
   * \param newStep The new value to be used.
   */
  void slot_change(int newStep);

  /**
  * This slot increments the value in the spin box which has the current focus.
  */
  void slotIncrementBox();

  /**
  * This slot decrements the value in the spin box which has the current focus.
  */
  void slotDecrementBox();

 private:

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit altUnit;

  QCheckBox* enableWarning;

  QRadioButton* s1;
  QRadioButton* s2;
  QRadioButton* s3;
  QRadioButton* s4;

#ifndef MAEMO5
  QGroupBox* separations;
#else
  QWidget* separations;
#endif

  QSpinBox*  horiWarnDist;
  QSpinBox*  horiWarnDistVN;
  QSpinBox*  aboveWarnDist;
  QSpinBox*  aboveWarnDistVN;
  QSpinBox*  belowWarnDist;
  QSpinBox*  belowWarnDistVN;

  QPushButton *defaults;
  QPushButton *plus;
  QPushButton *minus;

  // here are the fetched configuration items stored to have control about
  // changes done by the user
  int horiWarnDistValue;
  int horiWarnDistVNValue;
  int aboveWarnDistValue;
  int aboveWarnDistVNValue;
  int belowWarnDistValue;
  int belowWarnDistVNValue;

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif /* SettingsPageAirSpaceWarnings_h */
