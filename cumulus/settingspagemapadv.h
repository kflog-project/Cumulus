/***********************************************************************
**
**   settingspagemapadv.h
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
***********************************************************************/

/**
 *
 * contains map-related data settings
 *
 * @author André Somers
 *
 */

#ifndef SETTINGSPAGEMAPADV_H
#define SETTINGSPAGEMAPADV_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QStringList>
#include <QLineEdit>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "coordedit.h"
#include "projectionbase.h"
#include "distance.h"

class SettingsPageMapAdv : public QWidget
{
  Q_OBJECT

    public:

  /**
   * Constructor
   */
  SettingsPageMapAdv(QWidget *parent=0);

  /**
   * Destructor
   */
  ~SettingsPageMapAdv();

  /**
   * Checks, if the configuration of the projection has been changed
   */
  bool checkIsProjectionChanged();

  /**
   * Checks, if the configuration of the welt 2000 has been changed
   */
  bool checkIsWelt2000Changed();

  // SettingsPageMap takes care of our loading and saving,
  // so it needs access to our internals
  friend class SettingsPageMap; 

  public slots: // Public slots
  /**
   * Called to initiate saving to the configurationfile.
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
   * Called if the text of the filter has been changed
   */
  void slot_filterChanged( const QString& text );


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
  LatEdit * edtLat1;
  LatEdit * edtLat2;
  LongEdit * edtLon;

  int cylinPar;
  int lambertV1;
  int lambertV2;
  int lambertOrigin;

  // variable currentProjType is an enum ProjectionBase::ProjectionType
  int currentProjType;

  /** saves distance unit set during construction of object */
  Distance::distanceUnit distUnit;

  // Country filter for welt 2000 data file
  QLineEdit* countryFilter;

  // Radius around home position for welt 2000 data file
  QSpinBox* homeRadius;

  protected slots:

  void slotSelectProjection(int);
};

#endif
