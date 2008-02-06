/***********************************************************************
**
**   altimetermodedialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Eckhard Voellm, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef ALTIMETERMODEDIALOG_H
#define ALTIMETERMODEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QTimer>
#include <Q3ButtonGroup>

/**
  *@author Eckhard Voellm 
  */

class AltimeterModeDialog : public QDialog
{
  Q_OBJECT
public:
  AltimeterModeDialog(QWidget *parent);
  ~AltimeterModeDialog();
  void work ();
  QString Pretext();
  static int mode() { return _mode; };
protected:
  void load ();
  void accept ();
  void save (int mode); // 0: MSL,  1: GND,  2: STD
private:
  Q3ButtonGroup * altMode;
  QPushButton* buttonOK;
  QPushButton* buttonCancel;
  QTimer* timeout;
  int _time;
  static int _mode;  	 // 0: MSL,  1: GND,  2: STD
  bool _toggling_mode;   // 1: On
private slots:
  void setTimer();
signals:
  void settingsChanged();
  void newAltimeterMode();
};

#endif
