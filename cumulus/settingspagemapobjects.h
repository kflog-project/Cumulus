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

#ifdef USE_NUM_PAD
class NumberEditor;
#else
class QSpinBox;
#endif

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

  virtual void hideEvent(QHideEvent *);

public slots:
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

#ifdef USE_NUM_PAD
  NumberEditor *m_wpLowScaleLimit;
  NumberEditor *m_wpNormalScaleLimit;
  NumberEditor *m_wpHighScaleLimit;
#else
  // Spin boxes with scale limits according to their priority.
  QSpinBox *m_wpLowScaleLimit;
  QSpinBox *m_wpNormalScaleLimit;
  QSpinBox *m_wpHighScaleLimit;
#endif

  /** Auto sip flag storage. */
  bool m_autoSip;
};

#endif
