/***********************************************************************
**
**   settingspagemapsettings.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
************************************************************************
**
** Contains the map projection related settings
**
** @author André Somers
**
***********************************************************************/

#ifndef SettingsPageMapSettings_H
#define SettingsPageMapSettings_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QStringList>
#include <QPushButton>
#include <QLabel>

#include "coordedit.h"
#include "projectionbase.h"

class SettingsPageMapSettings : public QWidget
{
  Q_OBJECT

    public:

  /**
   * Constructor
   */
  SettingsPageMapSettings(QWidget *parent=0);

  /**
   * Destructor
   */
  ~SettingsPageMapSettings();

  /**
   * Checks, if the configuration of the projection has been changed
   */
  bool checkIsProjectionChanged();

  public slots: // Public slots
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configurationfile
   */
  void slot_load();

  /**
   * Called to ask is confirmation on the close is needed.
   */
  void slot_query_close(bool& warn, QStringList& warnings);

  private slots: // Private slots

  /**
   * Called if the map selection button is pressed
   */
  void slot_openFileDialog();

 protected:

  QPushButton * mapSelection;
  QLineEdit * mapDirectory;
  QCheckBox * chkDeleteAfterCompile;
  QCheckBox * chkUnloadUnneeded;
  QComboBox * cmbProjection;
  LatEdit   * edtLat1;
  QLabel    * edtLat2Label;
  LatEdit   * edtLat2;
  QLabel    * edtLonLabel;
  LongEdit  * edtLon;

  int cylinPar;
  int lambertV1;
  int lambertV2;
  int lambertOrigin;

  // variable currentProjType is an enum ProjectionBase::ProjectionType
  int currentProjType;

  protected slots:

  void slotSelectProjection(int);
};

#endif
