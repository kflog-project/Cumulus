/***********************************************************************
**
**   settingspageairspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Eggert Ehmke, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGEAIRSPACE_H
#define SETTINGSPAGEAIRSPACE_H

#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <QGridLayout>
#include <QRadioButton>

#include "altitude.h"

class SettingsPageAirspaceWarnings;
class SettingsPageAirspaceFilling;

/**
 * @author Eggert Ehmke
 */
class SettingsPageAirspace : public QWidget
  {
    Q_OBJECT

  public:

    SettingsPageAirspace(QWidget *parent=0);
    ~SettingsPageAirspace();

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

  protected:
    /**
     * saves current altitude unit during construction of object
     */
    Altitude::altitude altUnit;

    QPushButton* cmdWarning;
    QPushButton* cmdFilling;

    QTableWidget*     drawOptions;
    QTableWidgetItem* drawAirspaceA;
    QTableWidgetItem* drawAirspaceB;
    QTableWidgetItem* drawAirspaceC;
    QTableWidgetItem* drawControlC;
    QTableWidgetItem* drawAirspaceD;
    QTableWidgetItem* drawControlD;
    QTableWidgetItem* drawAirspaceElow;
    QTableWidgetItem* drawAirspaceEhigh;
    QTableWidgetItem* drawAirspaceF;
    QTableWidgetItem* drawRestricted;
    QTableWidgetItem* drawDanger;
    QTableWidgetItem* drawLowFlight;
    QTableWidgetItem* drawTMZ;
    QTableWidgetItem* drawSuSector;

    QCheckBox*      enableForceDrawing;
    QSpinBox*       spinForceMargin;

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

    //SettingsPageAirspace takes care of our loading and saving,
    //so it needs access to our internals
    // friend class SettingsPageAirspace;

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

    // SettingsPageWarnings takes care of our loading and saving, so
    // it needs access to our internals
    // friend class SettingsPageAirspace;

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

    // here are the fetched config items stored to have control about
    // changes done by the user
    int horiWarnDistValue;
    int horiWarnDistVNValue;
    int aboveWarnDistValue;
    int aboveWarnDistVNValue;
    int belowWarnDistValue;
    int belowWarnDistVNValue;
  };

#endif
