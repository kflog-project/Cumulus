/***********************************************************************
**
**   settingspagesector.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007, 2008 Axel Pauli, axel@kflog.org
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef __SettingsPageSector__h
#define __SettingsPageSector__h

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <Q3ButtonGroup>
#include <Q3GroupBox>
#include <QDoubleSpinBox>

#include "altitude.h"

/**
 * @short Configuration settings for start, turn and end points of a
 * task
 *
 * @author Axel Pauli
 */

class SettingsPageSector : public QWidget
{
  Q_OBJECT

public:

  SettingsPageSector( QWidget *parent=0);
  
  virtual ~SettingsPageSector();

public slots:

  void slot_load();
  void slot_save();

private slots:

  // radio button of cs scheme was pressed
  void slot_buttonPressedCS( int newScheme );

  // radio button of nt scheme was pressed
  void slot_buttonPressedNT( int newScheme );

  // value inner spin box changed
  void slot_outerSBChanged( double value );

private:

  // active scheme
  Q3ButtonGroup* csScheme; // cylinder-sector scheme
  Q3ButtonGroup* ntScheme; // nearst-touched scheme

  // Cylinder widgets
  Q3GroupBox*     cylinderGroup;
  QDoubleSpinBox* cylinderRadius; // Radius of cylinder task point in meter or feet

  // Sector widgets
  Q3GroupBox*     sectorGroup;
  QDoubleSpinBox* innerSectorRadius; // inner sector radius of task point in meter or feet
  QDoubleSpinBox* outerSectorRadius; // outer sector radius of task point in meter or feet
  QSpinBox*       sectorAngle;       // 0-180 degrees
    
  // Drawing options
  Q3GroupBox*    shapeGroup;
  QCheckBox*     drawShape;
  QCheckBox*     fillShape;

  /** saves distance unit set during construction of object */
  Distance::distanceUnit distUnit;

  // store selected cs scheme button
  int selectedCSScheme;
  // store selected nt scheme button
  int selectedNTScheme;
  // store cylinder radius after load
  double loadedCylinderRadius;
  // store inner sector radius after load
  double loadedInnerSectorRadius;
  // store outer sector radius after load
  double loadedOuterSectorRadius;
};

#endif
