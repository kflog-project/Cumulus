/***********************************************************************
**
**   settingspageinformation.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2003-2012 by Axel Pauli (axel@kflog.org)
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
 * \date 2003-2012
 *
 * \version $Id$
 *
 */

#ifndef  SettingsPageInformation_H
#define  SettingsPageInformation_H

#include <QWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>

class SettingsPageInformation : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageInformation )

  public:

  SettingsPageInformation(QWidget *parent=0);

  virtual ~SettingsPageInformation();

  protected:

  virtual void showEvent( QShowEvent *event );

  virtual void hideEvent( QHideEvent *event );

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

#ifndef ANDROID
  QLineEdit*   soundTool;
#endif

  QSpinBox*    spinAirfield;
  QSpinBox*    spinAirspace;
  QSpinBox*    spinWaypoint;
  QSpinBox*    spinWarning;
  QSpinBox*    spinInfo;
  QSpinBox*    spinSuppress;
  QCheckBox*   checkAlarmSound;
  QCheckBox*   calculateNearestSites;
  QPushButton* buttonReset;

  bool loadConfig; // control loading of config data
};

#endif // SettingsPageInformation_h
