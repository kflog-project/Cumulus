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
 */

#ifndef SETTINGS_PAGE_AIRFIELDS_H
#define SETTINGS_PAGE_AIRFIELDS_H

#include <QWidget>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QCheckBox>
#include <QPushButton>

#include "distance.h"

#ifdef USE_NUM_PAD
class NumberEditor;
#endif

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

  protected:

  void showEvent( QShowEvent *event );

  public slots: // Public slots
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file
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

#endif

  private:

  /** Country filter for Welt2000 data file */
  QLineEdit* m_countryFilter;

  /** Radius around home position for Welt2000 data file */

#ifdef USE_NUM_PAD
  NumberEditor* m_homeRadius;
#else
  QSpinBox* m_homeRadius;
#endif

  /** Check box to load outlandings or not. */
  QCheckBox* m_loadOutlandings;

  /** Pixels to add to the row height in airfield/waypoint lists
   *  (for easy finger selection)
   */
  QSpinBox* m_afMargin;

  /** Pixels to add to the row height in emergency (reachable points) list
   *  (for easy finger selection)
   */
  QSpinBox* m_rpMargin;

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
