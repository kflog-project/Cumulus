/***********************************************************************
**
**   altimetermodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by Eckhard Voellm
**                   2008-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ALTIMETER_MODE_DIALOG_H
#define ALTIMETER_MODE_DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QRadioButton>
#include <QSpinBox>
/**
  * \author Eckhard Voellm, Axel Pauli
  *
  * \brief Dialog for altimeter user interaction.
  *
  * This dialog is the user interface for the altimeter settings.
  *
  */

class AltimeterModeDialog : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AltimeterModeDialog )

public:

  AltimeterModeDialog(QWidget *parent);
  virtual ~AltimeterModeDialog();

  static QString mode2String();
  static int mode();

protected:

  void load();
  void accept();
  void save( int mode ); // 0: MSL,  1: STD,  2: AGL, 3: AHL

private:

  QTimer* timeout;
  int _time;
  int _mode;  	 // 0: MSL,  1: STD,  2: AGL, 3: AHL
  bool _toggling_mode;   // 1: On

  // Altitude references
  QRadioButton* _msl;
  QRadioButton* _agl;
  QRadioButton* _std;
  QRadioButton* _ahl;

  // Altitude units
  int _unit;     // 0: Meter,  1: Feet,  2: FL
  QRadioButton* _meter;
  QRadioButton* _feet;
  QRadioButton* _fl;

  // Spin box for altitude correction
  QSpinBox* spinUserCorrection;

  // Setup buttons
  QPushButton *plus;
  QPushButton *pplus;
  QPushButton *minus;
  QPushButton *mminus;

private slots:

  void slotSetTimer();
  void slotChangeMode( int mode );
  void slotChangeUnit( int unit );
  void slotChangeValue( int unit );

signals:

  void settingsChanged();
  void newAltimeterMode();
  void newAltimeterSetting();
};

#endif
