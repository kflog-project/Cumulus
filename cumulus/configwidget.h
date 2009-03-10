/***********************************************************************
**
**   configwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2009 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef _ConfigWidget_h
#define _ConfigWidget_h

#include <QStringList>

#include "settingspagepersonal.h"
#include "settingspagegps.h"
#include "settingspageunits.h"
#include "settingspagemapobjects.h"
#include "settingspagemapsettings.h"
#include "settingspageairfields.h"
#include "settingspageairspace.h"
#include "settingspageinformation.h"
#include "settingspagesector.h"
#include "settingspageglider.h"
#include "settingspagelooknfeel.h"
#include "settingspageterraincolors.h"

/**
  * @short Configuration widget of cumulus
  *
  * This is the general configuration widget for Cumulus.
  *
  * @author André Somers
  *
  */
class ConfigWidget : public QWidget
  {
    Q_OBJECT

  public:
    /**
     * Constructor
     */
    ConfigWidget(QWidget *parent=0);

    /**
     * Destructor
     */
    ~ConfigWidget();

  public slots:
    /**
     * Called if OK button is pressed
     */
    void accept();

    /**
     * Called if Cancel button is pressed
     */
    void reject();

  private slots: // Private slots
    /**
     * This slot is called just before showing the dialog, and loads the current settings.
     */
    void slot_LoadCurrent();

  signals: // Signals
    /**
     * Signal emitted to indicate the settings should be saved to the configurationfile
     */
    void save();

    /**
     * Emitted to indicate that the settings should be (re-) loaded from the configurationfile.
     */
    void load();

    /**
     * This signal is emitted after a save procedure has occured.
     * It gives connected objects the chance to adjust to new settings.
     */
    void settingsChanged();

    /**
     * This signal is emitted after a save procedure has occured and
     * the configuration of welt 2000 has been changed.
     */
    void welt2000ConfigChanged();

    /**
     * This signal is emitted when the dialog is cancelled. It gives connected objects
     * the chance to restore to old settings.
     */
    void reload();

    /**
     * This signal is emitted when the "dialog" should close. MainWindow will subsequently
     * delete it
     */
    void closeConfig();
    /**
     * This signal is emitted to the settings pages to ask them if they want to display a
     * warning when closing without saving. If so, the boolean flag warn must be set by in the
     * slot. In response, the configuration dialog object will display a dialog box to confirm
     * the closing and offering to save the changes instead.
     */
    void query_close(bool& warn, QStringList& warnings);

  private:

    SettingsPagePersonal* spp;
    SettingsPageGlider* spgl;
    SettingsPageSector* sps;
    SettingsPageGPS* spg;
    SettingsPageMapSettings* spms;
    SettingsPageTerrainColors* sptc;
    SettingsPageMapObjects* spmo;
    SettingsPageAirfields* spaf;
    SettingsPageAirspace* spa;
    SettingsPageUnits* spu;
    SettingsPageInformation* spi;
    SettingsPageLookNFeel* splnf;

    bool loadConfig; // control loading of configuration data
  };

#endif
