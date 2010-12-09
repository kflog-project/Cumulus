/***********************************************************************
**
**   settingspageunits.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageUnits
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for personal units.
 *
 * \date 2002-2010
 *
 */

#ifndef SETTINGS_PAGE_UNITS_H
#define SETTINGS_PAGE_UNITS_H

#include <QWidget>
#include <QComboBox>
#include <QStringList>

class SettingsPageUnits : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageUnits )

public:

  SettingsPageUnits(QWidget *parent=0);

  virtual ~SettingsPageUnits();

public slots:

  /**
   * Called to initiate loading of the configuration file
   */
  void slot_load();

  /**
   * called to initiate saving to the configuration file.
   */
  void slot_save();

  /** Called to ask is confirmation on the close is needed. */
  void slot_query_close(bool& warn, QStringList& warnings);

private:
  /**
   * This function returns the location of the value in the array.
   */
  int searchItem(int* p, int value, int max);

  QComboBox *UnitAlt;
  QComboBox *UnitSpeed;
  QComboBox *UnitDistance;
  QComboBox *UnitVario;
  QComboBox *UnitWind;
  QComboBox *UnitPosition;
  QComboBox *UnitTime;

  // we use seven arrays to store mappings of item locations in the combo boxes
  // to the enumeration values of the units they do represent
  int altitudes[2];
  int speeds[4];
  int distances[3];
  int varios[3];
  int winds[5];
  int positions[3];
  int times[2];
};

#endif
