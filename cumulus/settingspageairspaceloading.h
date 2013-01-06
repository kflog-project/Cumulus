/***********************************************************************
**
**   settingspageairspaceloading.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SettingsPageAirSpaceLoading_h
#define SettingsPageAirSpaceLoading_h

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

private:

  /** Table containing loadable airspace files. */
  QTableWidget* fileTable;

};

#endif /* SettingsPageAirSpaceLoading_h */
