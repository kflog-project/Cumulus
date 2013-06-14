/***********************************************************************
**
**   settingspageairspaceloading.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011-2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirSpaceLoading_h
#define SettingsPageAirSpaceLoading_h

#include <QPushButton>
#include <QWidget>
#include <QTableWidget>

/**
 * \class SettingsPageAirspaceLoading
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for airspace loading.
 *
 * This widget provides an airspace file selection table, where the user can
 * choose, which available airspace files shall be loaded or not loaded.
 *
 * \date 2011-2013
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceLoading : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirspaceLoading )

public:

  SettingsPageAirspaceLoading( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceLoading();

signals:

  /**
   * Emitted, if the airspace file list has been changed.
   */
  void airspaceFileListChanged();

private slots:

  /**
   * Called to save data to the configuration file.
   */
  void slot_save();

  /**
   * Called to toggle the check box of the clicked table cell.
   */
  void slot_toggleCheckBox( int row, int column );

  /** Removes all selected rows from the table. */
  void slot_DeleteRows();

  /** Called, if the item selection in the table has changed. */
  void slot_itemSelectionChanged();

  /**
   * Called if the help button is clicked.
   */
  void slot_openHelp();

private:

  /** Table containing loadable airspace files. */
  QTableWidget* m_fileTable;

  QPushButton *m_delButton;
};

#endif /* SettingsPageAirSpaceLoading_h */
