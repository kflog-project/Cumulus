/***********************************************************************
**
**   settingspageairfieldloading.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirfieldLoading_h
#define SettingsPageAirfieldLoading_h

#include <QPushButton>
#include <QWidget>
#include <QTableWidget>

/**
 * \class SettingsPageAirfieldLoading
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for OpenAIP airfield loading.
 *
 * This widget provides an airfield file selection table, where the user can
 * choose, which available airfield files shall be loaded or not loaded.
 *
 * \date 2013
 *
 * \version $Id$
 *
 */
class SettingsPageAirfieldLoading : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirfieldLoading )

public:

  SettingsPageAirfieldLoading( QWidget *parent=0 );

  virtual ~SettingsPageAirfieldLoading();

signals:

  /**
   * Emitted, if the file list has been changed.
   */
  void fileListChanged();

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

private:

  /** Table containing loadable airfield files. */
  QTableWidget* m_fileTable;

  QPushButton *m_delButton;
};

#endif /* SettingsPageAirfiledLoading_h */
