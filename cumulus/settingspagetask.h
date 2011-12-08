/***********************************************************************
**
**   settingspagetask.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPageTask
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for flight tasks.
 *
 * \date 2007-2011
 *
 * \version $Id$
 *
 */

#ifndef __SettingsPageTask__h
#define __SettingsPageTask__h

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QButtonGroup>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>

#include "altitude.h"

class SettingsPageTask : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageTask )

public:

  SettingsPageTask( QWidget *parent=0);

  virtual ~SettingsPageTask();

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

  /** Opens the color chooser dialog for the target line */
  void slot_editTlColor();

  /** Opens the color chooser dialog for the track line */
  void slot_editTrColor();

private:

  // active scheme
  QButtonGroup* csScheme; // cylinder-sector scheme
  QButtonGroup* ntScheme; // nearst-touched scheme

  // Cylinder widgets
  QGroupBox*      cylinderGroup;
  QDoubleSpinBox* cylinderRadius; // Radius of cylinder task point in meter or feet

  // Sector widgets
  QGroupBox*      sectorGroup;
  QDoubleSpinBox* innerSectorRadius; // inner sector radius of task point in meter or feet
  QDoubleSpinBox* outerSectorRadius; // outer sector radius of task point in meter or feet
  QSpinBox*       sectorAngle;       // 0-180 degrees

  // Drawing options
  QGroupBox* shapeGroup;
  QCheckBox* drawShape;
  QCheckBox* fillShape;

  // target line options
  QGroupBox*   tlGroup;
  QSpinBox*    tlWidth;
  QPushButton* tlColorButton;
  QCheckBox*   tlCheckBox;

  // temporary storage of target line color
  QColor tlColor;
  QColor selectedTlColor;

  // store selected target line width
  int seletedTlWidth;

  // track line options
  QGroupBox*   trGroup;
  QSpinBox*    trWidth;
  QPushButton* trColorButton;
  QCheckBox*   trCheckBox;

  // temporary storage of track line color
  QColor trColor;
  QColor selectedTrColor;

  // store selected track line width
  int seletedTrWidth;

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
