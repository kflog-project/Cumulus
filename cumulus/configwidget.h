/***********************************************************************
**
**   configwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2007-2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class ConfigWidget
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration widget of Cumulus
 *
 * This is the general configuration widget for Cumulus.
 *
 * \date 2002-2012
 *
 * \version $Id$
 */

#ifndef _ConfigWidget_h
#define _ConfigWidget_h

#include <QWidget>
#include <QStringList>

#include "settingspagepersonal.h"

#ifdef ANDROID
#include "settingspagegps4a.h"
#else
#include "settingspagegps.h"
#endif

#include "settingspageunits.h"
#include "settingspagemapobjects.h"
#include "settingspagemapsettings.h"
#include "settingspageairfields.h"
#include "settingspageairspace.h"
#include "settingspageinformation.h"
#include "settingspagetask.h"
#include "settingspageglider.h"
#include "settingspagelooknfeel.h"
#include "settingspageterraincolors.h"

class QSize;
class QTabWidget;

class ConfigWidget : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( ConfigWidget )

public:

  /**
   * Constructor
   */
  ConfigWidget(QWidget *parent=0);

  /**
   * Destructor
   */
  virtual ~ConfigWidget();

protected:

  void keyReleaseEvent( QKeyEvent* event );

public slots:

  /**
   * Called if OK button is pressed
   */
  void accept();

  /**
   * Called if Cancel button is pressed
   */
  void reject();

private slots:

  /**
   * This slot is called just before showing the dialog, and loads the current settings.
   */
  void slot_LoadCurrent();

signals:

  /**
   * Signal emitted to indicate the settings should be saved to the configuration file
   */
  void save();

  /**
   * Emitted to indicate that the settings should be (re-) loaded from the configuration file.
   */
  void load();

  /**
   * This signal is emitted after a save procedure has occurred.
   * It gives connected objects the chance to adjust to new settings.
   */
  void settingsChanged();

  /**
   * This signal is emitted after a save procedure has occurred and
   * the configuration of Welt2000 has been changed.
   */
  void welt2000ConfigChanged();

  /**
   * This signal is emitted when the dialog is canceled. It gives connected objects
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

  /**
   * Requests a move to the home position.
   */
  void gotoHomePosition();

private:

  // Overall widget
  QTabWidget* m_tabWidget;

  // Single configuration widgets
  SettingsPagePersonal*      spp;
  SettingsPageGlider*        spgl;
  SettingsPageTask*          spt;

#ifdef ANDROID
  SettingsPageGPS4A*         spg;
#else
  SettingsPageGPS*           spg;
#endif

  SettingsPageMapSettings*   spms;
  SettingsPageTerrainColors* sptc;
  SettingsPageMapObjects*    spmo;
  SettingsPageAirfields*     spaf;
  SettingsPageAirspace*      spa;
  SettingsPageUnits*         spu;
  SettingsPageInformation*   spi;
  SettingsPageLookNFeel*     splnf;

  bool loadConfig; // controls loading of configuration data

#ifdef ANDROID
  QSize fullSize;
#endif

};

#endif
