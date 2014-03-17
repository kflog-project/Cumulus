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
 * \brief Configuration settings for airfield data loading from Welt2000 and
 * OpenAIP.
 *
 * Configuration settings for airfield data loading from Welt2000 and
 * OpenAIP.
 *
 * \date 2008-2014
 *
 * \version $Id$
 *
 */

#ifndef SETTINGS_PAGE_AIRFIELDS_H
#define SETTINGS_PAGE_AIRFIELDS_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
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
   * Checks if the configuration of the OpenAIP has been changed
   */
  bool checkIsOpenAipChanged();

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
   * Called, if the source selection is changed.
   */
  void slot_sourceChanged( int index );

  /**
   * Called if the text of the filter has been changed
   */
  void slot_filterChanged( const QString& text );

  /**
   * Called to open the airfield file load selection dialog.
   */
  void slot_openLoadDialog();

  /**
   * Called if the help button is clicked.
   */
  void slot_openHelp();

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

  /**
   * Called, if download button of openAIP is clicked.
   */
  void slot_downloadOpenAip();

  signals:

  /**
   * This signal is emitted when a Welt2000 file shall be downloaded from
   * the Internet. The filename must be the name of the web page without
   * any path prefixes.
   *
   * \param welt2000FileName The Welt2000 file name to be downloaded.
   */
  void downloadWelt2000( const QString& welt2000FileName );

  /**
   * This signal is emitted when openAIP airfield country files shall be
   * downloaded from the Internet.
   *
   * \param openAipCountryList The list of countries to be downloaded.
   */
  void downloadOpenAipAirfields( const QStringList& openAipCountryList );

#endif

  /**
   * This signal is emitted, if OpenAIP items have been changed to trigger
   * a reload of all data files.
   */
  void reloadOpenAip();

  /**
   * This signal is emitted, if Welt2000 items have been changed to trigger
   * a reload of the data file.
   */
  void reloadWelt2000();

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  private:

  /**
   * Creates a home radius widget.
   */
  NumberEditor* createHomeRadiusWidget( QWidget* parent=0 );

  /**
   * Creates a runway length filter widget.
   */
  NumberEditor* createRwyLenthFilterWidget( QWidget* parent=0 );

  /**
   * Checks if the list contains valid two letter country code entries.
   * Allowed letters are a...z and A...Z. All other is rejected with false,
   * also an empty list.
   *
   * \return true in case of success otherwise false
   */
  bool checkCountryList( QStringList& clist );

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  bool save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  /** Source selector */
  QComboBox* m_sourceBox;

  /** OpenAIP group box widget */
  QGroupBox* m_oaipGroup;

  /** Welt2000 group box widget */
  QGroupBox* m_weltGroup;

  /** Country selector for Welt2000 data file */
  QLineEdit* m_countriesW2000;

  /** Country selector for openAIP downloads*/
  QLineEdit* m_countriesOaip4Download;

  /** Radius around home position for Welt2000 data file */

  /** Home radius OpenAIP. */
  NumberEditor* m_homeRadiusOaip;

  /** Home radius Welt2000. */
  NumberEditor* m_homeRadiusW2000;

  /** Filter for minimum runway length. If set to zero, the filter is inactive.*/
  NumberEditor* m_minRwyLengthW2000;

  /** Filter for minimum runway length. If set to zero, the filter is inactive.*/
  NumberEditor* m_minRwyLengthOaip;

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

  /** The home radius initial value in the selected distance unit. */
  float m_homeRadiusInitValue;

  /** The runway filter initial value in the selected altitude unit. */
  float m_runwayFilterInitValue;

#ifdef INTERNET

  /** Download and install Welt2000 file. */
  QPushButton* m_installWelt2000;

  /** Line editor for Welt2000 filename input. The file name on the web page
   *  is extended by a date string. */
  QLineEdit* m_welt2000FileName;

  /** Download and install openAIP country files. */
  QPushButton* m_downloadOpenAip;

#endif

};

#endif
