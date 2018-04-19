/***********************************************************************
**
**   SettingsPagePointData.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2008-2017 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPagePointData
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for point data loading from OpenAIP.
 *
 * Configuration settings for point data loading from OpenAIP.
 *
 * \date 2008-2018
 *
 * \version 1.2
 *
 */

#ifndef SETTINGS_PAGE_POINT_DATA_H
#define SETTINGS_PAGE_POINT_DATA_H

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

class SettingsPagePointData : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPagePointData )

  public:

  /**
   * Constructor
   */
  SettingsPagePointData(QWidget *parent=0);

  /**
   * Destructor
   */
  virtual ~SettingsPagePointData();

  /**
   * Checks if the configuration of the OpenAIP has been changed
   */
  bool checkIsOpenAipChanged();

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
  void slotHelp();

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
   * Called, if download button of openAIP is clicked.
   */
  void slot_downloadOpenAip();

  signals:

  /**
   * This signal is emitted when openAIP airfield and nav aids country files
   * shall be downloaded from the Internet.
   *
   * \param openAipCountryList The list of countries to be downloaded.
   */
  void downloadOpenAipPois( const QStringList& openAipCountryList );

#endif

  /**
   * This signal is emitted, if OpenAIP items have been changed to trigger
   * a reload of all data files.
   */
  void reloadOpenAip();

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

 /** Country selector for openAIP downloads*/
  QLineEdit* m_countriesOaip4Download;

  /** Radius around home position for Welt2000 data file */

  /** Home radius OpenAIP. */
  NumberEditor* m_homeRadiusOaip;

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

  /** The home radius initial value in the selected distance unit. */
  float m_homeRadiusInitValue;

  /** The runway filter initial value in the selected altitude unit. */
  float m_runwayFilterInitValue;

#ifdef INTERNET

  /** Download and install openAIP country files. */
  QPushButton* m_downloadOpenAip;

#endif

};

#endif
