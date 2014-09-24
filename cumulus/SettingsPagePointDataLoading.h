/***********************************************************************
**
**   SettingsPagePointDataLoading.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPagePointDataLoading_h
#define SettingsPagePointDataLoading_h

#include <QPushButton>
#include <QWidget>
#include <QTableWidget>

/**
 * \class SettingsPagePointDataLoading
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for OpenAIP point data loading.
 *
 * This widget provides a file selection table, where the user can
 * choose, which available point data files shall be loaded or not loaded.
 *
 * \date 2013-2014
 *
 * \version $Id$
 *
 */
class SettingsPagePointDataLoading : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPagePointDataLoading )

public:

  SettingsPagePointDataLoading( QWidget *parent=0 );

  virtual ~SettingsPagePointDataLoading();

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

#endif /* SettingsPagePointDataLoading_h */
