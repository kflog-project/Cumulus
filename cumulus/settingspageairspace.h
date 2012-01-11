/***********************************************************************
**
**   settingspageairspace.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2009-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

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
class SettingsPageAirspaceLoading;

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
 * \date 2002-2012
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

    void showEvent(QShowEvent *);

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

    QCheckBox* enableForceDrawing;
    QSpinBox*  spinForceMargin;
    QSpinBox*  spinAsLineWidth;

    // values of spin boxes after load
    int spinForceMarginValue;
    int spinAsLineWidthValue;

    SettingsPageAirspaceFilling*  m_fillingDlg;
  };

//----------------------------SettingsPageAirspaceFilling-----------------------

/**
 * \class SettingsPageAirspaceFilling
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Configuration settings for airspace fillings.
 *
 * \date 2002-2012
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceFilling: public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageAirspaceFilling )

 public:

  SettingsPageAirspaceFilling( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceFilling();

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
   * Called to set all spinboxes to the default value
   */
  void slot_defaults();

  /**
   * Called to reset all spinboxes to zero
   */
  void slot_reset();

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

  /**
  * This slot increments the value in the spin box which has the current focus.
  */
  void slotIncrementBox();

  /**
  * This slot decrements the value in the spin box which has the current focus.
  */
  void slotDecrementBox();

 private:

  QCheckBox* enableFilling;

  QRadioButton* s1;
  QRadioButton* s2;
  QRadioButton* s3;
  QRadioButton* s4;

  QGroupBox*   separations;

  QSpinBox*  verticalNotNear;
  QSpinBox*  verticalNear;
  QSpinBox*  verticalVeryNear;
  QSpinBox*  verticalInside;
  QSpinBox*  lateralNotNear;
  QSpinBox*  lateralNear;
  QSpinBox*  lateralVeryNear;
  QSpinBox*  lateralInside;

  QPushButton *plus;
  QPushButton *minus;
  QPushButton *reset;
  QPushButton *defaults;
};

//-------------------SettingsPageAirspaceWarnings-------------------------------

/**
 * \class SettingsPageAirspaceWarnings
 *
 * \author Eggert Ehmke, Axel Pauli
 *
 * \brief Configuration settings for airspace warnings.
 *
 * \date 2002-2012
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceWarnings : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirspaceWarnings )

public:

  SettingsPageAirspaceWarnings( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceWarnings();

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
   * Called to set all spin boxes to the default value
   */
  void slot_defaults();

 private slots:
  /**
   * Invoked if enableWarning changes value
   * @param enabled true if warning is enabled
   */
  void slot_enabledToggled(bool enabled);

  /**
   * Called to change the step width of the spin boxes
   *
   * \param newStep The new value to be used.
   */
  void slot_change(int newStep);

  /**
  * This slot increments the value in the spin box which has the current focus.
  */
  void slotIncrementBox();

  /**
  * This slot decrements the value in the spin box which has the current focus.
  */
  void slotDecrementBox();

 private:

  /**
   * saves current altitude unit during construction of object
   */
  Altitude::altitudeUnit altUnit;

  QCheckBox* enableWarning;

  QRadioButton* s1;
  QRadioButton* s2;
  QRadioButton* s3;
  QRadioButton* s4;

  QGroupBox* separations;

  QSpinBox*  horiWarnDist;
  QSpinBox*  horiWarnDistVN;
  QSpinBox*  aboveWarnDist;
  QSpinBox*  aboveWarnDistVN;
  QSpinBox*  belowWarnDist;
  QSpinBox*  belowWarnDistVN;

  QPushButton *defaults;
  QPushButton *plus;
  QPushButton *minus;

  // here are the fetched configuration items stored to have control about
  // changes done by the user
  int horiWarnDistValue;
  int horiWarnDistVNValue;
  int aboveWarnDistValue;
  int aboveWarnDistVNValue;
  int belowWarnDistValue;
  int belowWarnDistVNValue;
};

//-------------------SettingsPageAirspaceLoading-------------------------------

/**
 * \class SettingsPageAirspaceLoading
 *
 * \author Axel Pauli
 *
 * \brief Configuration settings for airspace loading.
 *
 * This widget provides an airspace file selection table, where the user can
 * choose, which available airspace files shall be loaded or not loaded.
 *
 * \date 2011
 *
 * \version $Id$
 *
 */
class SettingsPageAirspaceLoading : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( SettingsPageAirspaceLoading )

public:

  SettingsPageAirspaceLoading( QWidget *parent=0 );

  virtual ~SettingsPageAirspaceLoading();

signals:

  /**
   * Emitted, if the airspace file list has been changed.
   */
  void airspaceFileListChanged();

private slots:

  /**
   * Called to save data to the configuration file.
   */
  void slot_save();

  /**
   * Called to toggle the check box of the clicked table cell.
   */
  void slot_toggleCheckBox( int row, int column );

private:

  /** Table containing loadable airspace files. */
  QTableWidget* fileTable;

};

#endif
