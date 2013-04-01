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

class NumberEditor;

class SettingsPageInformation : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageInformation )

  public:

  SettingsPageInformation(QWidget *parent=0);

  virtual ~SettingsPageInformation();

  signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  private slots:

  /**
   * Called to restore the factory settings
   */
  void slot_setFactoryDefault();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

#ifndef ANDROID

  /**
   * Called to open a selection file dialog for the sound tool
   */
  void slot_openToolDialog();

#endif

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /**
   * Creates an default NumberEditor instance.
   *
   * @return an NumberEditor instance
   */
  NumberEditor* createNumEd( QWidget* parent=0 );

#ifndef ANDROID
  QLineEdit*   soundTool;
#endif

  NumberEditor* spinAirfield;
  NumberEditor* spinAirspace;
  NumberEditor* spinWaypoint;
  NumberEditor* spinWarning;
  NumberEditor* spinInfo;
  NumberEditor* spinSuppress;

  QCheckBox*   checkAlarmSound;
  QCheckBox*   checkFlarmAlarms;
  QCheckBox*   calculateNearestSites;
  QPushButton* buttonReset;
};

#endif // SettingsPageInformation_h
