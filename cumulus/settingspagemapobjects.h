/***********************************************************************
**
**   settingspagemapobjects.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
**                   2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
*************************************************************************
**
** The widget provides all options related to load and draw map items.
**
** @author André Somers, Axel Pauli
**
************************************************************************/

#ifndef SettingsPageMapObjects_H
#define SettingsPageMapObjects_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSpinBox>

class SettingsPageMapObjects : public QWidget
{
  Q_OBJECT

  public:

  SettingsPageMapObjects(QWidget *parent=0);
  virtual ~SettingsPageMapObjects();

  protected:

  /**
   * Called before widget is displayed. The content of the option table
   * is aligned to the window size.
   */
  void showEvent(QShowEvent *);

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
  void slot_query_close(bool& warn, QStringList& warningWaypoints);

private slots:

  /**
   * Called to toggle the check box of the clicked table cell.
   */
  void slot_toggleCheckBox( int row, int column );

  /**
   * Called, if the value in the spin box is changed.
   */
  void slot_wpLowScaleLimitChanged( int newValue );

  /**
   * Called, if the value in the spin box is changed.
   */
  void slot_wpNormalScaleLimitChanged( int newValue );

  /**
   * Called, if the value in the spin box is changed.
   */
  void slot_wpHighScaleLimitChanged( int newValue );

private:

  QTableWidget *loadOptions;

  // list items in listview
  QTableWidgetItem *liIsolines;
  QTableWidgetItem *liIsolineBorders;
  QTableWidgetItem *liRoads;
  QTableWidgetItem *liHighways;
  QTableWidgetItem *liRailways;
  QTableWidgetItem *liCities;
  QTableWidgetItem *liWaterways;
  QTableWidgetItem *liForests;
  QTableWidgetItem *liTargetLine;
  QTableWidgetItem *liWpLabels;
  QTableWidgetItem *liAfLabels;
  QTableWidgetItem *liTpLabels;
  QTableWidgetItem *liOlLabels;
  QTableWidgetItem *liLabelsInfo;

  // Spin boxes with scale limits according to their importance.
  QSpinBox *wpLowScaleLimitSpinBox;
  QSpinBox *wpNormalScaleLimitSpinBox;
  QSpinBox *wpHighScaleLimitSpinBox;

  /**
   * Fills the list with load options
   */
  void fillLoadOptionList();

};

#endif
