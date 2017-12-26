/***********************************************************************
**
**   preflightflarmpage.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012-2017 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class PreflightFlarmPage
 *
 * \author Axel Pauli
 *
 * \brief A widget for pre-flight Flarm IGC settings.
 *
 * A widget for pre-flight Flarm IGC settings.
 *
 * \date 2012-2017
 *
 * \version 1.6
 */

#ifndef PREFLIGHT_FLARM_PAGE_H_
#define PREFLIGHT_FLARM_PAGE_H_

#include <QWidget>
#include <QMessageBox>

#include "flarm.h"

class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStringList;
class QTimer;

class CuLabel;
class FlightTask;
class NumberEditor;

class PreFlightFlarmPage : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( PreFlightFlarmPage )

 public:

  PreFlightFlarmPage( QWidget *parent=0 );

  virtual ~PreFlightFlarmPage();

  /** Creates a task definition file in Flarm format. */
  static bool createFlarmTaskList( FlightTask* flightTask );

 private slots:

  /** Requests the Flarm data from the device.*/
  void slotRequestFlarmData();

  /** Sends all IGC data to the Flarm. */
  void slotWriteFlarmData();

  /** Sets all IGC data as defined in Cumulus. */
  void slotSetIgcData();

  /** Clears all IGC data in the editable widgets. */
  void slotClearIgcData();

  /** Called to update version info. */
  void slotUpdateVersions( const Flarm::FlarmData& info );

  /** Called to update error info. */
  void slotUpdateErrors( const Flarm::FlarmError& info );

  /** Called to receive and report an error info. */
  void slotReportError( QStringList& info );

  /** Called to update configuration info. */
  void slotUpdateConfiguration( QStringList& info );

  /** Called if the connection timer has expired. */
  void slotTimeout();

  /** Called if the widget is closed. */
  void slotClose();

  /** Called if the priv button is pressed. */
  void slotChangePrivMode();

  /** Called if the notrack button is pressed. */
  void slotChangeNotrackMode();

  /**
   * Called to show the Flarm error as text, when the related label is pressed.
   */
  void slotShowErrorText();

  signals:

  /**
   * Emitted, if the task selection has been changed.
   */
  void newTaskSelected();

  /**
   * Emitted, if the widget is closed.
   */
  void closingWidget();

 private:

  /** Loads the available Flarm data into the label displays. */
  void loadFlarmData();

  /** Called, to close a running Flarm data transfer. */
  void closeFlarmDataTransfer();

  /** Toggles operation of buttons. */
  void enableButtons( const bool toggle );

  /** Clears all user input fields. */
  void clearUserInputFields();

  /** Shows a popup message box to the user. */
  void messageBox( QMessageBox::Icon icon, QString message, QString title="" );

  /**
   * Ask the user via a message box, if the Flarm shall be rebooted.
   * If he answers yes the boot is initiated.
   */
  void ask4RebootFlarm();

  /** Send next command to Flarm from the command list. */
  void nextFlarmCommand();

  QGroupBox*   dataBox;
  QLabel*      deviceType;
  QLabel*      swVersion;
  QLabel*      swExp;
  QLabel*      igcVersion;
  QLabel*      region;
  QLabel*      serial;
  QLabel*      radioLabel;
  QLabel*      radioId;
  QLabel*      errSeverity;
  CuLabel*     errCode;
  QSpinBox*    logInt;
  QPushButton* priv;
  QPushButton* notrack;
  QLineEdit*   pilot;
  QLineEdit*   copil;
  QLineEdit*   gliderId;
  QLineEdit*   gliderType;
  QLineEdit*   compId;
  QLineEdit*   compClass;
  QLabel*      flarmTask;
  QComboBox*   taskBox;
  QPushButton* readButton;
  QPushButton* writeButton;
  QPushButton* setButton;
  QPushButton* clearButton;

  NumberEditor* hRange;
  NumberEditor* vRange;

  QTimer* m_timer;

  // List with commands to be sent to Flarm step by step.
  QStringList m_cmdList;

  // List index pointing to the next command to be sent.
  int m_cmdIdx;

  // Error report counter
  int m_errorReportCounter;

  // Flag to store a started Flarm task upload
  bool m_taskUploadRunning;

  // Flag to recognize that the first task record is sent
  bool m_firstTaskRecord;
};

#endif
