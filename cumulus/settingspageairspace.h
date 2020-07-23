/***********************************************************************
**
**   settingspageairspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef SettingsPageAirSpace_H
#define SettingsPageAirSpace_H

#include <QCheckBox>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QSpinBox>

#include "altitude.h"

class NumberEditor;

/**
 * \class SettingsPageAirspace
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Configuration settings for airspace drawing, colors, filling, warnings
 * and loading.
 *
 * Configuration settings for airspace drawing, colors, filling, warnings and
 * loading. Filling, warnings and loading configuration are realized as separate
 * widgets, callable via buttons.
 *
 * \date 2002-2020
 *
 * \version 1.4
 *
 */
class SettingsPageAirspace : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageAirspace )

 public:

  SettingsPageAirspace(QWidget *parent=0);

  virtual ~SettingsPageAirspace();

 protected:

  virtual void showEvent(QShowEvent *);

 private slots:
  /**
   * Invoked if enableForceWarning changes value
   * @param enabled true if warning is enabled
   */
  void slot_enabledToggled(bool enabled);

  /**
   * Called to toggle the check box of the clicked table cell.
   */
  void slot_toggleCheckBox( int row, int column );

  /**
   * Called to set all colors to their default value.
   */
  void slot_setColorDefaults();

  /**
   * Called to open the airspace fill dialog.
   */
  void slot_openFillDialog();

  /**
   * Called to open the airspace warning dialog.
   */
  void slot_openWarningDialog();

  /**
   * Called to open the airspace load selection dialog.
   */
  void slot_openLoadDialog();

  /**
   * Called to open the airspace filters editor.
   */
  void slot_editAsFilters();

#ifdef INTERNET

  /**
   * Called to request the download of openAIP airspace files.
   */
  void slot_downloadAirspaces();

   /**
    * Called, if the help button is clicked.
    */
   void slotHelp();

   /**
    * Called if the Ok button is pressed.
    */
   void slotAccept();

   /**
    * Called if the Cancel button is pressed.
    */
   void slotReject();

#endif

 signals:

  /**
   * Emitted if the airspace colors have been updated
   */
  void airspaceColorsUpdated();

  /**
   * Emitted if openAIP airspaces shall be downloaded
   */
  void downloadAirspaces( const QStringList& );

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  /** Called to check, if something has been changed by the user. */
  bool checkChanges();

  QTableWidget* drawOptions;
  QPushButton* cmdWarning;
  QPushButton* cmdFilling;
  QPushButton* cmdLoading;
  QPushButton* cmdAirspaceFilters;
  QPushButton* cmdColorDefaults;

#ifdef INTERNET
  QPushButton* cmdInstall;
#endif

  // enable/disable drawing of airspaces
  QTableWidgetItem* drawAirspaceA;
  QTableWidgetItem* drawAirspaceB;
  QTableWidgetItem* drawAirspaceC;
  QTableWidgetItem* drawAirspaceD;
  QTableWidgetItem* drawAirspaceE;
  QTableWidgetItem* drawAirspaceF;
  QTableWidgetItem* drawAirspaceFir;
  QTableWidgetItem* drawAirspaceFlarm;
  QTableWidgetItem* drawAirspaceG;
  QTableWidgetItem* drawControl;
  QTableWidgetItem* drawRestricted;
  QTableWidgetItem* drawDanger;
  QTableWidgetItem* drawProhibited;
  QTableWidgetItem* drawLowFlight;
  QTableWidgetItem* drawTMZ;
  QTableWidgetItem* drawWaveWindow;
  QTableWidgetItem* drawGliderSector;
  QTableWidgetItem* drawRMZ;

  // border colors of airspaces
  QWidget* borderColorAirspaceA;
  QWidget* borderColorAirspaceB;
  QWidget* borderColorAirspaceC;
  QWidget* borderColorAirspaceD;
  QWidget* borderColorAirspaceE;
  QWidget* borderColorAirspaceF;
  QWidget* borderColorAirspaceFir;
  QWidget* borderColorAirspaceFlarm;
  QWidget* borderColorAirspaceG;
  QWidget* borderColorControl;
  QWidget* borderColorRestricted;
  QWidget* borderColorDanger;
  QWidget* borderColorProhibited;
  QWidget* borderColorLowFlight;
  QWidget* borderColorTMZ;
  QWidget* borderColorWaveWindow;
  QWidget* borderColorGliderSector;
  QWidget* borderColorRMZ;

  // fill (brush) colors of airspaces
  QWidget* fillColorAirspaceA;
  QWidget* fillColorAirspaceB;
  QWidget* fillColorAirspaceC;
  QWidget* fillColorAirspaceD;
  QWidget* fillColorAirspaceE;
  QWidget* fillColorAirspaceF;
  QWidget* fillColorAirspaceFlarm;
  QWidget* fillColorAirspaceG;
  QWidget* fillColorControl;
  QWidget* fillColorRestricted;
  QWidget* fillColorDanger;
  QWidget* fillColorProhibited;
  QWidget* fillColorLowFlight;
  QWidget* fillColorTMZ;
  QWidget* fillColorWaveWindow;
  QWidget* fillColorGliderSector;
  QWidget* fillColorRMZ;
  QTableWidgetItem* fillColorAirspaceFir;

  QSpinBox*  m_spinAsLineWidth;
  QCheckBox* m_enableBorderDrawing;

  NumberEditor* m_borderDrawingValue;
};

#endif
