/***********************************************************************
**
**   settingspageairspacefillingnumpad.h
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

#ifndef SettingsPageAirSpaceFillingNumPad_h
#define SettingsPageAirSpaceFillingNumPad_h

#include <QCheckBox>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>

class NumberEditor;

/**
 * \class SettingsPageAirspaceFillingNumPad
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for airspace fillings.
 *
 * Configuration settings for airspace fillings. For the number edit
 * an own number pad is opened as popup on mouse request.
 *
 * \date 2013
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceFillingNumPad: public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageAirspaceFillingNumPad )

 public:

  SettingsPageAirspaceFillingNumPad( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceFillingNumPad();

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
   * Called to set all input fields to the default value
   */
  void slot_defaults();

  /**
   * Called to reset all input fields to zero
   */
  void slot_reset();

 private slots:

  /**
   * Invoked if enableFilling changes value
   *
   * @param enabled true if filling is enabled
   */
  void slot_enabledToggled(bool enabled);

 private:

  /**
   * Creates an default NumberEditor instance.
   *
   * @return an NumberEditor instance
   */
  NumberEditor* createNumEd( QWidget* parent=0 );

  QCheckBox* m_enableFilling;
  QGroupBox* m_distanceGroup;

  NumberEditor*  m_verticalNotNear;
  NumberEditor*  m_verticalNear;
  NumberEditor*  m_verticalVeryNear;
  NumberEditor*  m_verticalInside;
  NumberEditor*  m_lateralNotNear;
  NumberEditor*  m_lateralNear;
  NumberEditor*  m_lateralVeryNear;
  NumberEditor*  m_lateralInside;

  QPushButton* m_reset;
  QPushButton* m_defaults;
};

#endif /* SettingsPageAirSpaceFillingNumPad_h */
