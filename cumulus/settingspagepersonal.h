/***********************************************************************
**
**   settingspagepersonal.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class SettingsPagePersonal
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for personal settings.
 *
 * Configuration settings for personal settings.
 *
 * \date 2002-2013
 *
 * \version $Id$
 */

#ifndef SETTINGS_PAGE_PERSONAL_H
#define SETTINGS_PAGE_PERSONAL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

#include "altitude.h"

#ifdef USE_NUM_PAD

class NumberEditor;

#include "coordeditnumpad.h"

#else

class QSpinBox;

#include "coordedit.h"

#endif

class SettingsPagePersonal : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPagePersonal )

public:

  SettingsPagePersonal(QWidget *parent=0);

  virtual ~SettingsPagePersonal();

  /** Checks if the home position has been changed */
  bool checkIsHomePositionChanged();

  /** Checks if the home latitude has been changed */
  bool checkIsHomeLatitudeChanged();

  /** Checks if the home longitude has been changed */
  bool checkIsHomeLongitudeChanged();

public slots:

  /** Called to initiate saving to the configuration file */
  void slot_save();

  /** Called to initiate loading of the configuration file. */
  void slot_load();

  /** Called to ask is confirmation on the close is needed. */
  void slot_query_close(bool& warn, QStringList& warnings);

private slots:

  /** Called to open the directory selection dialog */
  void slot_openDirectoryDialog();

  /** Called, if something has entered in edtHomeCountry. */
  void slot_textEditedCountry( const QString& input );

#ifdef INTERNET

  /** Called, if proxy button was pressed. */
  void slot_editProxy();

#endif

private:

  QLineEdit *edtName;
  QComboBox *langBox;
  QLineEdit *edtHomeCountry;
  QLineEdit *edtHomeName;
  QLineEdit *userDataDir;

#ifdef USE_NUM_PAD
  NumberEditor    *edtHomeElevation;
  LatEditNumPad   *edtHomeLat;
  LongEditNumPad  *edtHomeLong;
#else
  QSpinBox *edtHomeElevation;
  LatEdit  *edtHomeLat;
  LongEdit *edtHomeLong;
#endif

#ifdef INTERNET
  QLabel* proxyDisplay;
#endif

  /**
   * Initial elevation value after load from configuration.
   */
  int m_initalHomeElevationValue;

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit m_altUnit;
};

#endif
