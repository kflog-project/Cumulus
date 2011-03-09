/***********************************************************************
**
**   preflightwaypointpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
 * \date 2011
 *
 * \version $Id$
 *
 */

#ifndef PREFLIGHT_WAYPOINT_PAGE_H
#define PREFLIGHT_WAYPOINT_PAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QWidget>
#include <QRadioButton>
#include <QString>

#include "coordedit.h"
#include "singlepoint.h"

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
   * Sets the center reference to be used for waypoint data import.
   */
  void slotSelectCenterReference( int reference );

  /**
   * Called to toggle the filter.
   */
  void slotToggleFilter( int toggle );

  /**
   * Imports a new waypoint file.
   */
  void slotImportFile();

signals:

  /**
   * Signal is emitted, when new waypoints have been added to the global list.
   */
  void waypointsAdded();

private:

  /**
   * Loads the available airfields into the airfield combo box.
   */
  void loadAirfieldComboBox();

public:

  /**
   * Possible center references.
   */
  enum CenterReference { Position, Home, Airfield };

private:

  QHash<QString, SinglePoint*> airfieldDict;

  enum CenterReference centerRef;

  LatEdit*  centerLat;
  LongEdit* centerLon;

  QComboBox* wpTypesBox;
  QComboBox* wpRadiusBox;
  QComboBox* airfieldBox;
  QComboBox* wpFileFormatBox;

  QRadioButton* positionRB;
  QRadioButton* homeRB;
  QRadioButton* airfieldRB;

  QLabel* homeLabel;

  QCheckBox* filterToggle;
  QGroupBox* selectGroup;
  QGroupBox* centerPointGroup;

  /** Saved waypoint file format after load. */
  int _waypointFileFormat;
};

#endif
