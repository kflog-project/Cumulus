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
#include <Q3ListView>
#include <Q3CheckListItem>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <QGridLayout>

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
    void enabledToggled(bool enabled);

  protected:
    /**
     * saves current altitude unit during construction of object
     */
    Altitude::altitude altUnit;

    QPushButton* cmdWarning;
    QPushButton* cmdFilling;

    Q3ListView*      lvLoadOptions;
    Q3CheckListItem* drawAirspaceA;
    Q3CheckListItem* drawAirspaceB;
    Q3CheckListItem* drawAirspaceC;
    Q3CheckListItem* drawControlC;
    Q3CheckListItem* drawAirspaceD;
    Q3CheckListItem* drawControlD;
    Q3CheckListItem* drawAirspaceElow;
    Q3CheckListItem* drawAirspaceEhigh;
    Q3CheckListItem* drawAirspaceF;
    Q3CheckListItem* drawRestricted;
    Q3CheckListItem* drawDanger;
    Q3CheckListItem* drawLowFlight;
    Q3CheckListItem* drawTMZ;
    Q3CheckListItem* drawSuSector;

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
    friend class SettingsPageAirspace;

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

  protected:
    /**
     * Reimplemented from QDialog
     * Reload settings from configuration file so that changes won't
     * be written to the configuration.
     */
    void reject();

    QCheckBox* m_enableFilling;
    QWidget*  m_separations;

    QSpinBox* m_basicFilling;

    QSpinBox* m_verticalNotNear;
    QSpinBox* m_verticalNear;
    QSpinBox* m_verticalVeryNear;
    QSpinBox* m_verticalInside;
    QSpinBox* m_lateralNotNear;
    QSpinBox* m_lateralNear;
    QSpinBox* m_lateralVeryNear;
    QSpinBox* m_lateralInside;


  private slots:
    /**
     * Invoked if m_enableFilling changes value
     * @param enabled true if filling is enabled
     */
    void enabledToggled(bool enabled);

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
    friend class SettingsPageAirspace;

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
    QWidget*   separations;

    QSpinBox* horiWarnDist;
    QSpinBox* horiWarnDistVN;
    QSpinBox* aboveWarnDist;
    QSpinBox* aboveWarnDistVN;
    QSpinBox* belowWarnDist;
    QSpinBox* belowWarnDistVN;

    // here are the fetched config items stored to have control about
    // changed done by the user
    int horiWarnDistValue;
    int horiWarnDistVNValue;
    int aboveWarnDistValue;
    int aboveWarnDistVNValue;
    int belowWarnDistValue;
    int belowWarnDistVNValue;

  private slots:
    /**
     * Invoked if m_enableWarning changes value
     * @param enabled true if warning is enabled
     */
    void enabledToggled(bool enabled);

  };

#endif
