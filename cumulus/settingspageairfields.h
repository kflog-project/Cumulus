/***********************************************************************
**
**   settingspageairfields.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008 Axel Pauli
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

#ifndef SETTINGSPAGEAIRFIELDS_H
#define SETTINGSPAGEAIRFIELDS_H

#include <QWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QStringList>

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
   * Checks if the configuration of the welt 2000 has been changed
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

  /** Country filter for welt 2000 data file */
  QLineEdit* countryFilter;

  /** Radius around home position for welt 2000 data file */
  QSpinBox* homeRadius;

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
