/***********************************************************************
**
**   altimetermodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Voellm, 2008-2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ALTIMETER_MODE_DIALOG_H
#define ALTIMETER_MODE_DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QRadioButton>

/**
  *@author Eckhard Voellm 
  */

class AltimeterModeDialog : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AltimeterModeDialog )

public:

  AltimeterModeDialog(QWidget *parent);
  ~AltimeterModeDialog();
  void work();
  static QString mode2String();
  static int mode();

protected:

  void load ();
  void accept ();
  void save (int mode); // 0: MSL,  1: GND,  2: STD

private:

  QTimer* timeout;
  int _time;
  int _mode;  	 // 0: MSL,  1: GND,  2: STD
  bool _toggling_mode;   // 1: On
  QRadioButton * _msl;
  QRadioButton * _gnd;
  QRadioButton * _std;

private slots:

  void setTimer();
  void change_mode (int);

signals:

  void settingsChanged();
  void newAltimeterMode();
};

#endif
