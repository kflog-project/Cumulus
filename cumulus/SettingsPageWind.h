/***********************************************************************
**
**   SettingsPageWind.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2021 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageWind
 *
 * \author Axel Pauli
 *
 * \brief A widget for wind settings.
 *
 * \date 2021
 *
 * \version 1.0
 *
 */

#pragma once

#include <QCheckBox>
#include <QWidget>

#include "speed.h"

class DoubleNumberEditor;
class NumberEditor;

class SettingsPageWind : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageWind )

 public:

  SettingsPageWind(QWidget *parent=0);

  virtual ~SettingsPageWind();

  void load();

  void save();

 private slots:

  /**
  * Called, if the help button is clicked.
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

 signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  QCheckBox*          m_enableWindInSF;
  DoubleNumberEditor* m_minimumAirSpeed;
  DoubleNumberEditor* m_speedTolerance;
  NumberEditor*       m_headingTolerance; // in degrees 0...359
  NumberEditor*       m_windAfter; // wind after x seconds

  double m_loadedSpeed;

  /** saves horizontal speed unit during construction of object */
  Speed::speedUnit m_speedUnit;
};

