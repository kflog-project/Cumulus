/***********************************************************************
**
**   settingspageairspacewarningsnumpad.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirSpaceWarningsNumPad_h
#define SettingsPageAirSpaceWarningsNumPad_h

#include <QCheckBox>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>

#include "altitude.h"

class NumberEditor;

/**
 * \class SettingsPageAirspaceWarningsNumPad
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for airspace warnings.
 *
 * \date 2013
 *
 * Configuration settings for airspace warnings. For the number edit
 * an own number pad is opened as popup on mouse request.
 *
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceWarningsNumPad : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirspaceWarningsNumPad )

public:

  SettingsPageAirspaceWarningsNumPad( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceWarningsNumPad();

 public slots:
  /**
   * Called to initiate saving to the configuration file with widget closing.
   */
  void slot_save();

  /**
   * Called to close the widget without saving to the configuration file.
   */
  void slot_reject();

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
   *
   * @param enabled true if warning is enabled
   */
  void slot_enabledToggled(bool enabled);

 private:

  /**
   * Creates an default NumberEditor instance.
   *
   * @return an NumberEditor instance
   */
  NumberEditor* createNumEd( QWidget* parent=0 );

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit m_altUnit;

  QCheckBox* m_enableWarning;
  QGroupBox* m_distanceGroup;

  NumberEditor*  m_horiWarnDist;
  NumberEditor*  m_horiWarnDistVN;
  NumberEditor*  m_aboveWarnDist;
  NumberEditor*  m_aboveWarnDistVN;
  NumberEditor*  m_belowWarnDist;
  NumberEditor*  m_belowWarnDistVN;

  QPushButton *m_defaults;

  // here are the fetched configuration items stored to have control about
  // changes done by the user
  int m_horiWarnDistValue;
  int m_horiWarnDistVNValue;
  int m_aboveWarnDistValue;
  int m_aboveWarnDistVNValue;
  int m_belowWarnDistValue;
  int m_belowWarnDistVNValue;
};

#endif /* SettingsPageAirSpaceWarningsNumPad_h */
