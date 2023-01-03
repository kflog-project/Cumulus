/***********************************************************************
**
**   preflightwaypointpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreFlightWaypointPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight waypoint management.
 *
 * This widget can import waypoint data from a file by using filter options.
 * The supported waypoint formats are the Seeyou cup format and the KFLog
 * XML waypoint format.
 *
 * \date 2011-2023
 *
 * \version 1.3
 *
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QHash>
#include <QLabel>
#include <QWidget>
#include <QRadioButton>
#include <QString>

#include "coordeditnumpad.h"
#include "singlepoint.h"

class NumberEditor;

class PreFlightWaypointPage : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( PreFlightWaypointPage )

public:

  PreFlightWaypointPage(QWidget *parent=0);

  virtual ~PreFlightWaypointPage();

  void load();

  void save();

private slots:

  /**
   * Called, if the help button is clicked.
   */
  void slotHelp();

  /**
   * Sets the center reference to be used for waypoint data import.
   */
  void slotSelectCenterReference( int reference );

  /**
   * Called to open the airfield selection dialog.
   */
  void slotOpenAirfieldDialog();

  /**
   * Called, if a new home shall be set.
   */
  void slotNewHome( const SinglePoint* singlePoint )  ;

  /**
   * Called to toggle the filter.
   */
  void slotToggleFilter( int toggle );

  /**
   * Imports a new waypoint file.
   */
  void slotImportFile();

  /**
   * Save the waypoint data as file at another place.
   */
  void slotSaveAs();

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

signals:

  /**
   * Signal is emitted, when new waypoints have been added to the global list.
   */
  void waypointsAdded();

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

public:

  /**
   * Possible center references.
   */
  enum CenterReference { Position, Home, Airfield };

private:

  QHash<QString, SinglePoint*> m_airfieldDict;

  enum CenterReference m_centerRef;

  LatEditNumPad   *m_centerLat;
  LongEditNumPad  *m_centerLon;
  NumberEditor    *m_wpRadiusBox;

  QComboBox*   m_wpTypesBox;
  QPushButton* m_airfieldSelection;
  QComboBox*   m_wpPriorityBox;
  QComboBox*   m_wpFileFormatBox;

  QRadioButton* m_positionRB;
  QRadioButton* m_homeRB;
  QRadioButton* m_airfieldRB;

  QPushButton* m_saveAs;

  QLabel* m_centerLatLabel;
  QLabel* m_centerLonLabel;
  QLabel* m_homeLabel;

  QCheckBox* m_filterToggle;
  QGroupBox* m_selectGroup;
  QGroupBox* m_centerPointGroup;

  /** Saved waypoint file format after load. */
  int m_waypointFileFormat;

  /** Flag to remember of a file load. */
  bool m_fileLoaded;
};

