/***********************************************************************
**
**   settingspageinformation.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003-2013 by Axel Pauli (axel@kflog.org)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageInformation
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for popup window display times and alarm sound.
 *
 * \date 2003-2013
 *
 * \version $Id$
 *
 */

#ifndef  SettingsPageInformation_H
#define  SettingsPageInformation_H

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>

#ifdef USE_NUM_PAD
class NumberEditor;
#else
class QSpinBox;
#endif

class SettingsPageInformation : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageInformation )

  public:

  SettingsPageInformation(QWidget *parent=0);

  virtual ~SettingsPageInformation();

  public slots: // Public slots
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file
   */
  void slot_load();

  private slots:
  /**
   * Called to restore the factory settings
   */
  void slot_setFactoryDefault();

#ifndef ANDROID

  /**
   * Called to open a selection file dialog for the sound tool
   */
  void slot_openToolDialog();

#endif

 private:

  /**
   * Creates an default NumberEditor instance.
   *
   * @return an NumberEditor instance
   */
  NumberEditor* createNumEd( QWidget* parent=0 );

#ifndef ANDROID
  QLineEdit*   soundTool;
#endif

#ifdef USE_NUM_PAD
  NumberEditor* spinAirfield;
  NumberEditor* spinAirspace;
  NumberEditor* spinWaypoint;
  NumberEditor* spinWarning;
  NumberEditor* spinInfo;
  NumberEditor* spinSuppress;
#else
  QSpinBox*    spinAirfield;
  QSpinBox*    spinAirspace;
  QSpinBox*    spinWaypoint;
  QSpinBox*    spinWarning;
  QSpinBox*    spinInfo;
  QSpinBox*    spinSuppress;
#endif

  QCheckBox*   checkAlarmSound;
  QCheckBox*   checkFlarmAlarms;
  QCheckBox*   calculateNearestSites;
  QPushButton* buttonReset;

  bool m_loadConfig; // control loading of config data
};

#endif // SettingsPageInformation_h
