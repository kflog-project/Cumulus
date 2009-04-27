/***********************************************************************
**
**   settingspageairfields.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 *
 * contains airfield related data settings
 *
 * @author Axel Pauli, axel@kflog.org
 *
 */

#ifndef SETTINGS_PAGE_AIRFIELDS_H
#define SETTINGS_PAGE_AIRFIELDS_H

#include <QWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QStringList>
#include <QCheckBox>

#include "distance.h"

class SettingsPageAirfields : public QWidget
{
  Q_OBJECT

    public:

  /**
   * Constructor
   */
  SettingsPageAirfields(QWidget *parent=0);

  /**
   * Destructor
   */
  ~SettingsPageAirfields();

  /**
   * Checks if the configuration of the Welt2000 has been changed
   */
  bool checkIsWelt2000Changed();

  /**
   * Checks if the configuration of list display has been changed
   */
  bool checkIsListDisplayChanged();

  public slots: // Public slots
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configurationfile
   */
  void slot_load();

  /**
   * Called to ask is confirmation on the close is needed.
   */
  void slot_query_close(bool& warn, QStringList& warnings);

  private slots: // Private slots

  /**
   * Called if the text of the filter has been changed
   */
  void slot_filterChanged( const QString& text );

 private:

  /** Country filter for Welt2000 data file */
  QLineEdit* countryFilter;

  /** Radius around home position for Welt2000 data file */
  QSpinBox* homeRadius;

  /** Check box to load outlandings or not. */
  QCheckBox* loadOutlandings;

  /** Initial state of the loadOutlandings checkbox for change control */
  bool olInitState;

  /** Number of page entries in airfield/waypoint lists. 0 disables */
  QSpinBox* pageSize;

  /** Pixels to add to the row height in airfield/waypoint lists
   *  (for easy finger selection)
   */
  QSpinBox* afMargin;

  /** Pixels to add to the row height in emergency (reachable points) list
   *  (for easy finger selection)
   */
  QSpinBox* rpMargin;

  /** stores distance unit set during construction of object */
  Distance::distanceUnit distUnit;

};

#endif
