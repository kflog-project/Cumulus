/***********************************************************************
**
**   settingspageairfields.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2013 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageAirfields
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for airfield loading from Welt2000.
 *
 * \date 2008-2013
 *
 * \version $Id$
 *
 */

#ifndef SETTINGS_PAGE_AIRFIELDS_H
#define SETTINGS_PAGE_AIRFIELDS_H

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QPushButton>

#include "distance.h"

class NumberEditor;

class SettingsPageAirfields : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageAirfields )

  public:

  /**
   * Constructor
   */
  SettingsPageAirfields(QWidget *parent=0);

  /**
   * Destructor
   */
  virtual ~SettingsPageAirfields();

  /**
   * Checks if the configuration of the Welt2000 has been changed
   */
  bool checkIsWelt2000Changed();

  /**
   * Checks if the configuration of list display has been changed
   */
  bool checkIsListDisplayChanged();

  private slots: // Private slots

  /**
   * Called if the text of the filter has been changed
   */
  void slot_filterChanged( const QString& text );

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

#ifdef INTERNET

  /**
   * Called, if install button of Welt2000 is clicked.
   */
  void slot_installWelt2000();

  signals:

  /**
   * This signal is emitted when a Welt2000 file shall be downloaded from
   * the internet. The filename must be the name of the web page without
   * any path prefixes.
   */
  void downloadWelt2000( const QString& welt2000FileName );

  /**
   * This signal is emitted, if Welt2000 items have been changed to trigger
   * a reload of the data file.
   */
  void reloadWelt2000();

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

#endif

  private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  bool save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();


  /** Country filter for Welt2000 data file */
  QLineEdit* m_countryFilter;

  /** Radius around home position for Welt2000 data file */

  /** Home radius. */
  NumberEditor* m_homeRadius;

  /** Pixels to add to the row height in airfield/waypoint lists
   *  (for easy finger selection)
   */
  NumberEditor* m_afMargin;

  /** Pixels to add to the row height in emergency (reachable points) list
   *  (for easy finger selection)
   */
  NumberEditor* m_rpMargin;

  /** Check box to load outlandings or not. */
  QCheckBox* m_loadOutlandings;

  /** stores distance unit set during construction of object */
  Distance::distanceUnit m_distUnit;

#ifdef INTERNET

  /** Download and install Welt2000 file. */
  QPushButton* m_installWelt2000;

  /** Line editor for Welt2000 filename input. The file name on the web page
   *  is extended by a date string. */
  QLineEdit* m_welt2000FileName;

#endif

};

#endif
