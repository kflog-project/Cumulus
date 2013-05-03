/***********************************************************************
**
**   settingspagetask.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2007-2013 by Axel Pauli
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
 * \date 2007-2013
 *
 * \version $Id$
 *
 */

#ifndef __SettingsPageTask__h
#define __SettingsPageTask__h

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>
#include <QButtonGroup>
#include <QGroupBox>
#include <QPushButton>

#include "distance.h"

class DoubleNumberEditor;
class NumberEditor;

class SettingsPageTask : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageTask )

public:

  SettingsPageTask( QWidget *parent=0);

  virtual ~SettingsPageTask();

signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

private slots:

  // radio button of Nearest/Touch scheme was pressed
  void slot_buttonPressedNT( int newScheme );

  // value of outer sector box changed
  void slot_outerSectorStartChanged( const QString& value );

  // value of outer sector box changed
  void slot_outerSectorFinishChanged( const QString& value );

  // value of outer sector box changed
  void slot_outerSectorObsChanged( const QString& value );

  // fill shape checkbox state changed
  void slot_fillShapeStateChanged ( int state );

  // Start Scheme changed
  void slot_buttonPressedSS(int);

  // Finish Scheme changed
  void slot_buttonPressedFS(int);

  // Observer Scheme changed
  void slot_buttonPressedOS(int);

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

private:

  /**
   * Creates a default DoubleNumberEditor instance.
   *
   * @return a DoubleNumberEditor instance
   */
  DoubleNumberEditor* createDNE( QWidget* parent=0 );

  /**
   * Creates a default NumberEditor instance.
   *
   * @return a NumberEditor instance
   */
  NumberEditor* createNE( QWidget* parent=0 );

  /**
   * Sets the distance in the destination according to source value and selected
   * user distance unit.
   *
   * @param dest Distance to be checked and set
   *
   * @param src Distance value from double editor used as check input
   */
  void setDistanceValue( Distance& dest, DoubleNumberEditor* src);

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  // Drawing options
  QGroupBox* m_shapeGroup;
  QCheckBox* m_drawShape;
  QCheckBox* m_fillShape;

  // Zoom option
  QCheckBox* m_autoZoom;

  // report switch option
  QCheckBox* m_reportSwitch;

  QButtonGroup* ntScheme;       // nearest-touch schema
  QButtonGroup* startScheme;    // circle-sector scheme
  QButtonGroup* finishScheme;   // circle-sector scheme
  QButtonGroup* obsScheme;      // circle-sector scheme

  NumberEditor* m_transShape; // 0...100%

  // Start options
  DoubleNumberEditor* m_startLine; // length in  distance unit
  DoubleNumberEditor* m_startRing; // Radius in distance unit
  DoubleNumberEditor* m_startSectorInnerRadius; // Radius in distance unit
  DoubleNumberEditor* m_startSectorOuterRadius; // Radius in distance unit
  NumberEditor*       m_startSectorAngle; // 1-360 degrees

  // Finish options
  DoubleNumberEditor* m_finishLine; // length in distance unit
  DoubleNumberEditor* m_finishRing; // Radius in distance unit
  DoubleNumberEditor* m_finishSectorInnerRadius; // Radius in distance unit
  DoubleNumberEditor* m_finishSectorOuterRadius; // Radius in distance unit
  NumberEditor*       m_finishSectorAngle; // 1-360 degrees

  // Observation Zone options
  DoubleNumberEditor* m_obsCircleRadius; // Radius in distance unit
  DoubleNumberEditor* m_obsSectorInnerRadius; // Radius in distance unit
  DoubleNumberEditor* m_obsSectorOuterRadius; // Radius in distance unit
  NumberEditor*       m_obsSectorAngle; // 1-360 degrees

  // Store selected NT scheme button
  int m_selectedSwitchScheme;
  int m_selectedStartScheme;
  int m_selectedFinishScheme;
  int m_selectedObsScheme;

  /** saves distance unit set during construction of object */
  Distance::distanceUnit m_distUnit;

  /** The loaded configuration values are saved in those variables. */
  Distance m_startLineValue;
  Distance m_startRingValue;
  Distance m_startSectorInnerRadiusValue;
  Distance m_startSectorOuterRadiusValue;
  int      m_startSectorAngleValue;

  Distance m_finishLineValue;
  Distance m_finishRingValue;
  Distance m_finishSectorInnerRadiusValue;
  Distance m_finishSectorOuterRadiusValue;
  int      m_finishSectorAngleValue;

  Distance m_obsCircleRadiusValue;
  Distance m_obsSectorInnerRadiusValue;
  Distance m_obsSectorOuterRadiusValue;
  int      m_obsSectorAngleValue;
};

#endif
