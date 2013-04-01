/***********************************************************************
**
**   settingspagemapsettings.h
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
************************************************************************/

/**
 * \class SettingsPageMapSettings
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for map projection.
 *
 * \date 2002-2013
 *
 * \version $Id$
 *
 */

#ifndef SettingsPageMapSettings_H
#define SettingsPageMapSettings_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QStringList>
#include <QPushButton>
#include <QLabel>
#include <QPoint>

#include "coordeditnumpad.h"
#include "distance.h"
#include "projectionbase.h"

class NumberEditor;

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
  virtual ~SettingsPageMapSettings();

  /**
   * Checks, if the configuration of the projection has been changed
   */
  bool checkIsProjectionChanged();

 signals:

   /**
    * Emitted, if settings have been changed.
    */
   void settingsChanged();

#ifdef INTERNET

  void downloadMapArea( const QPoint&, const Distance& );

#endif

 private slots:

  /**
   * Called if the Ok button is pressed.
   */
  void slotAccept();

  /**
   * Called if the Cancel button is pressed.
   */
  void slotReject();

  void slot_selectProjection(int);

  /**
   * Called if the map selection button is pressed
   */
  void slot_openFileDialog();

#ifdef INTERNET

  /**
   * Called if the install maps button is pressed
   */
  void slot_installMaps();

#endif

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  QLineEdit   *mapDirectory;
  QCheckBox   *chkUnloadUnneeded;
  QCheckBox   *chkProjectionFollowHome;
  QComboBox   *cmbProjection;
  QLabel      *edtLat2Label;
  QLabel      *edtLonLabel;

  LatEditNumPad   *edtLat1;
  LatEditNumPad   *edtLat2;
  LongEditNumPad  *edtLon;

#ifdef INTERNET

  QCheckBox   *chkDownloadMissingMaps;
  QPushButton *installMaps;

  NumberEditor    *installRadius;
  LatEditNumPad   *edtCenterLat;
  LongEditNumPad  *edtCenterLon;

#endif

  int m_cylinPar;
  int m_lambertV1;
  int m_lambertV2;
  int m_lambertOrigin;

  // variable m_currentProjType is an enumeration ProjectionBase::ProjectionType
  int m_currentProjType;
};

#endif
