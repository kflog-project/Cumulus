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

#include "projectionbase.h"
#include "distance.h"

#ifdef USE_NUM_PAD

class NumberEditor;

#include "coordeditnumpad.h"

#else

class QSpinBox;

#include "coordedit.h"

#endif


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

 protected:

  void showEvent(QShowEvent *);

 signals:

#ifdef INTERNET

  void downloadMapArea( const QPoint&, const Distance& );

#endif

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

#ifdef INTERNET

  /**
   * Called if the install maps button is pressed
   */
  void slot_installMaps();

#endif

 private:

  QPushButton *mapSelection;
  QLineEdit   *mapDirectory;
  QCheckBox   *chkUnloadUnneeded;
  QCheckBox   *chkProjectionFollowHome;
  QComboBox   *cmbProjection;
  QLabel      *edtLat2Label;
  QLabel      *edtLonLabel;

#ifdef USE_NUM_PAD
  LatEditNumPad   *edtLat1;
  LatEditNumPad   *edtLat2;
  LongEditNumPad  *edtLon;
#else
  LatEdit     *edtLat1;
  LatEdit     *edtLat2;
  LongEdit    *edtLon;
#endif


#ifdef INTERNET

  QCheckBox   *chkDownloadMissingMaps;
  QPushButton *installMaps;

#ifdef USE_NUM_PAD

  NumberEditor    *installRadius;
  LatEditNumPad   *edtCenterLat;
  LongEditNumPad  *edtCenterLon;

#else

  QSpinBox    *installRadius;
  LatEdit     *edtCenterLat;
  LongEdit    *edtCenterLon;

#endif
#endif

  int m_cylinPar;
  int m_lambertV1;
  int m_lambertV2;
  int m_lambertOrigin;

  // variable m_currentProjType is an enumeration ProjectionBase::ProjectionType
  int m_currentProjType;
};

#endif
