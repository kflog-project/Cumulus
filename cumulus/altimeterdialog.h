/***********************************************************************
**
**   altimeterdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Voellm
**                   2008-2020 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class AltimeterDialog
 *
 * \author Eckhard Völlm, Axel Pauli
 *
 * \brief Dialog for altimeter user interaction.
 *
 * This dialog is the user interface for the altimeter settings.
 *
 * \date 2004-2021
 *
 */

#ifndef ALTIMETER_DIALOG_H
#define ALTIMETER_DIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QTimer>
#include <QRadioButton>
#include <QSpinBox>

class Altitude;

class AltimeterDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( AltimeterDialog )

public:

  AltimeterDialog( QWidget *parent );
  virtual ~AltimeterDialog();

  static QString mode2String();
  static int mode();

  /**
   * @return Returns the current number of instances.
   */
  static int getNrOfInstances()
  {
    return noOfInstances;
  };

  /**
   * Calculate the QNH from the passed altitude.
   *
   * \param altitude Altitude in meters
   *
   * \return QNH in hPa
   */
  static int getQNH( const Altitude& altitude );

protected:

  /** User has pressed the ok button. */
  void accept();

  /** User has pressed cancel button or timeout has occurred. */
  void reject();

private:

  /** Gets the initial data for all widgets which needs a start configuration. */
  void load();

  /** Starts resp. restarts the inactively timer. */
  void startTimer();

  /** Check for configuration changes. */
  bool changesDone();

  /** inactively timer control */
  QTimer* m_timeout;

  /** Altitude references */
  QRadioButton* m_msl;
  QRadioButton* m_agl;
  QRadioButton* m_std;
  QRadioButton* m_ahl;

  /** Altitude modes */
  int m_mode; // 0: MSL,  1: STD,  2: AGL, 3: AHL

  /** Altitude units */
  int m_unit;     // 0: Meter,  1: Feet,  2: FL
  QRadioButton* m_meter;
  QRadioButton* m_feet;

  /** Altitude reference. */
  int _ref; // 0: Gps, 1: Baro
  QRadioButton* m_gps;
  QRadioButton* m_baro;

  /** selection list of devices, which delivers a pressure altitude. */
  QComboBox* m_devicesList;

  /** Altitude display */
  QSpinBox* m_altitudeDisplay;

  /** Altitude gain display */
  QLineEdit* altitudeGainDisplay;

  /** Display for altitude correction factor */
  QLabel* levelingDisplay;

  /** Spin box for QNH setting */
  QSpinBox* spinQnh;

  /** Setup buttons */
  QPushButton *plus;
  QPushButton *pplus;
  QPushButton *minus;
  QPushButton *mminus;
  QPushButton *reset;

  QPushButton *setAltitudeGain;

  /** Save the initial values here. They are needed in the reject case. */
  int m_saveMode;
  int m_saveUnit;
  int m_saveRef;
  int m_saveQnh;
  int m_saveLeveling;
  QString m_savePressureDevice;

  /** Auto sip flag storage. */
  bool m_autoSip;

  /** contains the current number of class instances */
  static int noOfInstances;

public slots:

  /** This slot is being called if the altitude value has been changed. */
  void slotAltitudeChanged(const Altitude& altitude );

  /** This slot is being called if the altitude gain value has been changed. */
  void slotAltitudeGain(const Altitude& altitudeGain );

private slots:

  /**
   * This slot is called if the altitude reference has been changed. It
   * restarts too the close timer.
   */
  void slotModeChanged( int mode );

  /**
   * This slot is called if the altitude unit has been changed. It
   * restarts too the close timer.
   */
  void slotUnitChanged( int unit );

  /**
   * This slot is called if the altitude reference has been changed. It
   * restarts too the close timer.
   */
  void slotReferenceChanged( int reference );

  /**
   * This slot is called if a button is pressed to change the content of the
   * related spin box which has the current focus.
   */
  void slotChangeSpinValue();

  /**
   * This slot is called if the pressure device is changed.
   */
  void slotPressureDeviceChanged( const QString& device );

  /**
   * This slot is called if the S button is pressed. It resets the gained
   * altitude display and informs the calculator about that.
   */
  void slotResetGainedAltitude();

signals:

  /** Emitted, if the altimeter mode has been changed. */
  void newAltimeterMode();

  /** Emitted, if the altimeter QNH resp. leveling have been changed. */
  void newAltimeterSettings();

  /** Emitted, if the pressure device is changed. */
  void newPressureDevice( const QString& device );

  /**
   * This signal is emitted, when the dialog is closed
   */
  void closingWidget();
};

#endif
