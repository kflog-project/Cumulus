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
 * \brief Configuration settings for a FlarmNet Database window
 *
 * \date 2023-2026
 *
 * \version 1.1
 *
 */

#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "DownloadManager.h"

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
   * Called, if the FlarmNet Download button is pressed.
   */
  void slotDownloadFN();

  /**
   * Called, if the OGN Download button is pressed.
   */
  void slotDownloadOGN();

  /**
   * Slot for download manager.
   */
  void slotDownloadsFinished( int, int );

  /**
   * Slot for download manager.
   */
  void slotNetworkError();

  /**
   * Slot for download manager.
   */
  void slotFileDownloaded(QString& );

  /**
   * Slot to count filter items.
   */
  void slotCount();

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /**
   * Set the loaded FlarmNet records.
   */
  void setLoadedRecords();

  QCheckBox*   useFlarmDB;
  QLineEdit*   editFnFile;
  QLineEdit*   editOGNFile;
  QLineEdit*   editDBFilter;
  QPushButton* buttonDownloadFN;
  QPushButton* buttonDownloadOGN;
  QPushButton* buttonReset;
  QPushButton *cancel;
  QPushButton *ok;
  QPushButton *count;

  QLabel*      info;
  QLabel*      dbFilterLabel;

  /** save start values for change check. */
  bool fnUseStart;
  QString fnFileStart;
  QString fnFilterStart;

  /** Manager to handle downloads of METAR-TAF data. */
  DownloadManager* m_downloadManger;

  /** Flag to mark a running update action. */
  bool m_downloadIsRunning;

  /** Flag to mark a database download. */
  bool m_downloadDone;

  /** first entry flag */
  bool m_first;
};
