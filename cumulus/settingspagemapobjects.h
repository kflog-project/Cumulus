/***********************************************************************
**
**   settingspagemapobjects.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
*************************************************************************
**
** Contains the map objects related to load/draw settings.
**
** @author André Somers
**
************************************************************************/

#ifndef SettingsPageMapObjects_H
#define SettingsPageMapObjects_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

class SettingsPageMapObjects : public QWidget
{
  Q_OBJECT

    public:

  SettingsPageMapObjects(QWidget *parent=0);
  ~SettingsPageMapObjects();

  public slots: // Public slots
  /**
   * Called to initiate saving to the configurationfile.
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

private slots:
    /**
     * Called to toggle the check box of the clicked table cell.
     */
  void slot_toggleCheckBox( int row, int column );
  

protected:

  QTableWidget * loadOptions;

  // list items in listview
  QTableWidgetItem * liIsolines;
  QTableWidgetItem * liIsolineBorders;
  QTableWidgetItem * liWpLabels;
  QTableWidgetItem * liWpLabelsExtraInfo;
  QTableWidgetItem * liRoads;
  QTableWidgetItem * liHighways;
  QTableWidgetItem * liRailroads;
  QTableWidgetItem * liCities;
  QTableWidgetItem * liWaterways;
  QTableWidgetItem * liForests;  //forests and ice
  QTableWidgetItem * liTargetLine;

 private: // Private methods
  /**
   * Fills the list with load options
   */
  void fillLoadOptionList();

};

#endif
