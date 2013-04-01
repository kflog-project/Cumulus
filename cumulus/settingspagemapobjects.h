/***********************************************************************
**
**   settingspagemapobjects.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
*************************************************************************/

/**
 * \class SettingsPageMapObjects
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for map loading and drawing..
 *
 * \date 2002-2013
 *
 * \version $Id$
 *
 */

#ifndef SettingsPageMapObjects_H
#define SettingsPageMapObjects_H

#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>

class NumberEditor;

class SettingsPageMapObjects : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageMapObjects )

public:

  SettingsPageMapObjects(QWidget *parent=0);

  virtual ~SettingsPageMapObjects();

protected:

  /**
   * Called before widget is displayed. The content of the option table
   * is aligned to the window size.
   */
  virtual void showEvent(QShowEvent *);

signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

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

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  /**
   * Fills the list with load options
   */
  void fillLoadOptionList();

  QTableWidget *loadOptions;

  // list items in listview
  QTableWidgetItem *liIsolines;
  QTableWidgetItem *liIsolineBorders;
  QTableWidgetItem *liRoads;
  QTableWidgetItem *liMotorways;
  QTableWidgetItem *liRailways;
  QTableWidgetItem *liCities;
  QTableWidgetItem *liWaterways;
  QTableWidgetItem *liForests;
  QTableWidgetItem *liWpLabels;
  QTableWidgetItem *liAfLabels;
  QTableWidgetItem *liTpLabels;
  QTableWidgetItem *liOlLabels;
  QTableWidgetItem *liLabelsInfo;
  QTableWidgetItem *liRelBearingInfo;
  QTableWidgetItem *liFlightTrail;

  NumberEditor *m_wpLowScaleLimit;
  NumberEditor *m_wpNormalScaleLimit;
  NumberEditor *m_wpHighScaleLimit;

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif
