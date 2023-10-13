/***********************************************************************
**
**   SettingsPageFlarmDB.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2023 by Axel Pauli (kflog.cumulus@gmail.com)
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageFlarmDB
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for popup window of Flarm Database
 *
 * \date 2023
 *
 * \version 1.0
 *
 */

#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>

class SettingsPageFlarmDB : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageFlarmDB )

  public:

  SettingsPageFlarmDB(QWidget *parent=0);

  virtual ~SettingsPageFlarmDB();

  signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  private slots:

  /**
   * Called to restore the factory settings
   */
  void slotSetFactoryDefault();

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

  /**
   * Called, if the Download button is pressed.
   */
  void slotDownload();

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  QCheckBox*   useFlarmDB;
  QLineEdit*   editDbFile;
  QPushButton* buttonDownload;
  QPushButton* buttonReset;

  /** save start values for change check. */
  bool useFlarmDBStart;
  QString dbFileStart;
};

