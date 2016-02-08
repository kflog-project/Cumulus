/***********************************************************************
**
**   taskpointeditor.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2013-2016 by Eggert Ehmke, Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class TaskPointEditor
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Editor for configuration settings of task points.
 *
 * \date 2013-2016
 *
 * \version 1.3
 *
 */

#ifndef __TaskPointEditor__h
#define __TaskPointEditor__h

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>

#include "coordeditnumpad.h"
#include "taskpoint.h"

class DoubleNumberEditor;
class NumberEditor;

class TaskPointEditor : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( TaskPointEditor )

public:

  /**
   * Class constructor.
   *
   * \param parent Instance of parent wigdet.
   *
   * \param tp The task point to be edited.
   */
  TaskPointEditor( QWidget *parent, TaskPoint* tp );

  virtual ~TaskPointEditor();

protected:

  virtual void showEvent(QShowEvent *event);

signals:

  /**
   * Emitted, if the widget is closed and the task point has been edited.
   *
   * \param tp edited task point
   */
  void taskPointEdited( TaskPoint* tp );

private slots:

  void slotAccept ();

  void slotReject ();

  // radio button of csl scheme was pressed
  void slot_buttonPressedCSL( int newScheme );

  // value of outer m_sector radius changed
  void slot_outerSectorRadiusChanged( double value );

  // value of outer m_sector radius changed
  void slot_outerSectorRadiusChanged( const QString& value );

  /**
   * Called, if the default button is pressed. Resets all schema values of
   * the widget to the default values.
   */
  void slot_schemaConfigurationDefaults();

private:

  void load();

  void loadSchema();

  void save();

  QLabel*         m_pointShortLabel;
  QLineEdit*      m_pointShortNameEditor;
  QLineEdit*      m_pointLongNameEditor;
  LatEditNumPad*  m_latEditor;
  LongEditNumPad* m_lonEditor;
  NumberEditor*   m_elevetionEditor;

  // active scheme
  QButtonGroup* m_cslScheme; // m_circle-m_sector-m_line scheme

  QRadioButton* m_circle;
  QRadioButton* m_sector;
  QRadioButton* m_line;

  // Circle widgets
  QGroupBox* m_circleGroup;

  /** Radius of m_circle task point in meter or feet. */
  DoubleNumberEditor* m_circleRadius;

  // Sector widgets
  QGroupBox*  m_sectorGroup;

  DoubleNumberEditor* m_innerSectorRadius; // inner m_sector radius of task point in meter or feet
  DoubleNumberEditor* m_outerSectorRadius; // outer m_sector radius of task point in meter or feet
  NumberEditor*       m_sectorAngle;       // 1-360 degrees
  DoubleNumberEditor* m_lineLength;

  // Line widgets
  QGroupBox* m_lineGroup;

  QCheckBox* m_autoZoom;

  // store selected csl scheme button
  int m_selectedCSLScheme;

  // store loaded csl scheme
  int m_loadedCSLScheme;
  // store m_circle radius after load
  double m_loadedCircleRadius;
  // store inner m_sector radius after load
  double m_loadedInnerSectorRadius;
  // store outer m_sector radius after load
  double m_loadedOuterSectorRadius;
  // store m_line length after load
  double m_loadedLineLength;
  // store m_sector angle after load
  int m_loadedSectorAngle;
  // loaded auto zoom flag
  bool m_loadedAutoZoom;

  /** The passed task point during class creation. */
  TaskPoint* m_taskPoint;

  /** The working task point. */
  TaskPoint m_workTp;
};

#endif
