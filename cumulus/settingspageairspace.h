/***********************************************************************
**
**   settingspageairspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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

#ifdef USE_NUM_PAD
class NumberEditor;
#endif

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
 * \date 2002-2013
 *
 * \version $Id$
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

    virtual void hideEvent(QHideEvent *);

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

#ifdef INTERNET

    /**
     * Called to request the download of an airspace file.
     */
    void slot_installAirspace();

    /**
     * Called to start a download of an airspace file.
     */
     void slot_startDownload( QString &url );

#endif

  signals:

    /**
     * Emitted if the airspace colors have been updated
     */
    void airspaceColorsUpdated();

    /**
     * Emitted if an airspace shall be installed
     */
    void downloadAirspace( QString& url );

  protected:

    /**
     * saves current altitude unit during construction of object
     */
    Altitude::altitudeUnit altUnit;

    QTableWidget* drawOptions;
    QPushButton* cmdWarning;
    QPushButton* cmdFilling;
    QPushButton* cmdLoading;
    QPushButton* cmdColorDefaults;

#ifdef INTERNET
    QPushButton* cmdInstall;
#endif

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
    QTableWidgetItem* drawProhibited;
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
    QWidget* borderColorProhibited;
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
    QWidget* fillColorProhibited;
    QWidget* fillColorLowFlight;
    QWidget* fillColorTMZ;
    QWidget* fillColorWaveWindow;
    QWidget* fillColorGliderSector;

    QSpinBox*  m_spinAsLineWidth;
    QCheckBox* m_enableBorderDrawing;

#ifdef USE_NUM_PAD
  NumberEditor* m_borderDrawingValue;
#else
    QSpinBox*  m_borderDrawingValue;
#endif

    // values of spin boxes after load
    int spinAsLineWidthValue;

    /** Auto sip flag storage. */
    bool m_autoSip;
  };

#endif
