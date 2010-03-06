/***********************************************************************
**
**   settingspagemapsettings.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2010 by Axel pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
#include <QSpinBox>
#include <QPoint>

#include "coordedit.h"
#include "projectionbase.h"
#include "distance.h"

class SettingsPageMapSettings : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageMapSettings )

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

 protected:

  void showEvent(QShowEvent *);

 signals:

  void downloadMapArea( const QPoint&, const Distance& );

 public slots:
  /**
   * Called to initiate saving to the configuration file.
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file
   */
  void slot_load();

  /**
   * Called to ask is confirmation on the close is needed.
   */
  void slot_query_close(bool& warn, QStringList& warnings);

 private slots:

  void slot_selectProjection(int);

  /**
   * Called if the map selection button is pressed
   */
  void slot_openFileDialog();

  /**
   * Called if the install maps button is pressed
   */
  void slot_installMaps();

  /**
   * Opens proxy dialog on user request.
   */
  void slot_editProxy();

 private:

  QPushButton *mapSelection;
  QLineEdit   *mapDirectory;
  QCheckBox   *chkUnloadUnneeded;
  QCheckBox   *chkProjectionFollowHome;
  QComboBox   *cmbProjection;
  QCheckBox   *chkDownloadMissingMaps;
  LatEdit     *edtLat1;
  QLabel      *edtLat2Label;
  LatEdit     *edtLat2;
  QLabel      *edtLonLabel;
  LongEdit    *edtLon;
  QPushButton *installMaps;
  QSpinBox    *installRadius;
  LatEdit     *edtCenterLat;
  LongEdit    *edtCenterLon;

  /** Dialog editor for proxy host and port input. */
  QPushButton *editProxy;

  /** Label to show the current proxy settings. */
  QLabel *proxyDisplay;

  int cylinPar;
  int lambertV1;
  int lambertV2;
  int lambertOrigin;

  // variable currentProjType is an enumeration ProjectionBase::ProjectionType
  int currentProjType;
};

#endif
