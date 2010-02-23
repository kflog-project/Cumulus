/***********************************************************************
**
**   settingspageairspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * The three widgets SettingsPageAirspace, SettingsPageAirspaceFilling and
 * SettingsPageAirspaceWarnings in this file provide all options for airspace
 * configuration.
 *
 * @author Eggert Ehmke
 *
 */

#ifndef SettingsPageAirSpace_H
#define SettingsPageAirSpace_H

#include <QWidget>
#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QSpinBox>
#include <QRadioButton>

#include "altitude.h"

class SettingsPageAirspaceWarnings;
class SettingsPageAirspaceFilling;

class SettingsPageAirspace : public QWidget
  {
    Q_OBJECT

  private:

  Q_DISABLE_COPY ( SettingsPageAirspace )

  public:

    SettingsPageAirspace(QWidget *parent=0);
    virtual ~SettingsPageAirspace();

  protected:

    void showEvent(QShowEvent *);

  public slots: // Public slots
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

  signals: // Signals

  /**
   * Emitted if the airspace colors have been updated
   */
  void airspaceColorsUpdated();

  protected:
    /**
     * saves current altitude unit during construction of object
     */
    Altitude::altitude altUnit;

    QPushButton* cmdWarning;
    QPushButton* cmdFilling;
    QPushButton* cmdColorDefaults;

    QTableWidget* drawOptions;

    // enable/disable drawing of airspaces
    QTableWidgetItem* drawAirspaceA;
    QTableWidgetItem* drawAirspaceB;
    QTableWidgetItem* drawAirspaceC;
    QTableWidgetItem* drawControlC;
    QTableWidgetItem* drawAirspaceD;
    QTableWidgetItem* drawControlD;
    QTableWidgetItem* drawAirspaceE;
    QTableWidgetItem* drawAirspaceF;
    QTableWidgetItem* drawRestricted;
    QTableWidgetItem* drawDanger;
    QTableWidgetItem* drawLowFlight;
    QTableWidgetItem* drawTMZ;
    QTableWidgetItem* drawWaveWindow;
    QTableWidgetItem* drawGliderSector;

    // border colors of airspaces
    QWidget* borderColorAirspaceA;
    QWidget* borderColorAirspaceB;
    QWidget* borderColorAirspaceC;
    QWidget* borderColorControlC;
    QWidget* borderColorAirspaceD;
    QWidget* borderColorControlD;
    QWidget* borderColorAirspaceE;
    QWidget* borderColorAirspaceF;
    QWidget* borderColorRestricted;
    QWidget* borderColorDanger;
    QWidget* borderColorLowFlight;
    QWidget* borderColorTMZ;
    QWidget* borderColorWaveWindow;
    QWidget* borderColorGliderSector;

    // fill (brush) colors of airspaces
    QWidget* fillColorAirspaceA;
    QWidget* fillColorAirspaceB;
    QWidget* fillColorAirspaceC;
    QWidget* fillColorControlC;
    QWidget* fillColorAirspaceD;
    QWidget* fillColorControlD;
    QWidget* fillColorAirspaceE;
    QWidget* fillColorAirspaceF;
    QWidget* fillColorRestricted;
    QWidget* fillColorDanger;
    QWidget* fillColorLowFlight;
    QWidget* fillColorTMZ;
    QWidget* fillColorWaveWindow;
    QWidget* fillColorGliderSector;

    QCheckBox*      enableForceDrawing;
    QSpinBox*       spinForceMargin;

    // value of spin box after load
    int spinForceMarginValue;

    SettingsPageAirspaceFilling*  m_fillingDlg;
    SettingsPageAirspaceWarnings* m_warningsDlg;
  };

//-------------------------------------------------------------------------

class SettingsPageAirspaceFilling: public QDialog
  {
    Q_OBJECT
  public:

    SettingsPageAirspaceFilling( QWidget *parent=0 );
    ~SettingsPageAirspaceFilling();

  public slots: // Public slots
    /**
     * Called to initiate saving to the configuration file.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configuration file
     */
    void slot_load();

    /**
     * Called to set all spinboxes to the default value
     */
    void slot_defaults();

    /**
     * Called to reset all spinboxes to zero
     */
    void slot_reset();

    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

  private slots:

    /**
     * Invoked if enableFilling changes value
     * @param enabled true if filling is enabled
     */
    void slot_enabledToggled(bool enabled);

    /**
     * Called to change the step width of the spin boxes
     */
    void slot_change(int newStep);

  protected:
    /**
     * Reimplemented from QDialog
     * Reload settings from configuration file so that changes won't
     * be written to the configuration.
     */
    void reject();

    QCheckBox* enableFilling;

    QRadioButton* s1;
    QRadioButton* s2;
    QRadioButton* s3;
    QRadioButton* s4;

    QWidget*   separations;

    QSpinBox*  verticalNotNear;
    QSpinBox*  verticalNear;
    QSpinBox*  verticalVeryNear;
    QSpinBox*  verticalInside;
    QSpinBox*  lateralNotNear;
    QSpinBox*  lateralNear;
    QSpinBox*  lateralVeryNear;
    QSpinBox*  lateralInside;
  };

//-------------------------------------------------------------------------

class SettingsPageAirspaceWarnings : public QDialog
  {
    Q_OBJECT
  public:

    SettingsPageAirspaceWarnings( QWidget *parent=0 );
    ~SettingsPageAirspaceWarnings();

  public slots: // Public slots
    /**
     * Called to initiate saving to the configuration file.
     */
    void slot_save();

    /**
     * Called to initiate loading of the configuration file
     */
    void slot_load();

    /**
     * Called to set all spinboxes to the default value
     */
    void slot_defaults();

    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

  private slots:
    /**
     * Invoked if enableWarning changes value
     * @param enabled true if warning is enabled
     */
    void slot_enabledToggled(bool enabled);

    /**
     * Called to change the step width of the spin boxes
     */
    void slot_change(int newStep);

  protected:
    /**
     * Reimplemented from QDialog
     * Reload settings from configuration file so that changes won't
     * be written to the configuration.
     */
    void reject();

    /**
     * saves current altitude unit during construction of object
     */
    Altitude::altitude altUnit;

    QCheckBox* enableWarning;

    QRadioButton* s1;
    QRadioButton* s2;
    QRadioButton* s3;
    QRadioButton* s4;

    QWidget*   separations;

    QSpinBox*  horiWarnDist;
    QSpinBox*  horiWarnDistVN;
    QSpinBox*  aboveWarnDist;
    QSpinBox*  aboveWarnDistVN;
    QSpinBox*  belowWarnDist;
    QSpinBox*  belowWarnDistVN;

    // here are the fetched configuration items stored to have control about
    // changes done by the user
    int horiWarnDistValue;
    int horiWarnDistVNValue;
    int aboveWarnDistValue;
    int aboveWarnDistVNValue;
    int belowWarnDistValue;
    int belowWarnDistVNValue;
  };

#endif
