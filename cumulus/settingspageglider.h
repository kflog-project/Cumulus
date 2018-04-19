/***********************************************************************
**
**   settingspageglider.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2018 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SettingsPageGlider
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Configuration settings for gliders.
 *
 * This widget provides an interface to add, edit and delete gliders
 * from the glider list.
 *
 * \date 2002-2018
 *
 * \version 1.1
 *
 */

#ifndef SETTINGS_PAGE_GLIDER_H
#define SETTINGS_PAGE_GLIDER_H

#include <QWidget>
#include <QBoxLayout>
#include <QStringList>

#include "gliderlistwidget.h"

class SettingsPageGlider : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( SettingsPageGlider )

 public:

  SettingsPageGlider(QWidget *parent=0);

  virtual ~SettingsPageGlider();

 protected:

  void showEvent( QShowEvent* event );

 private slots:

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

  /**
   * Called when the selected glider should be deleted from the m_list
   */
  void slot_delete();

  /**
   * Called when the selected glider needs must be opened in the editor
   */
  void slot_edit();

  /**
   * Called when a new glider needs to be made.
   */
  void slot_new();

 signals:

  /**
   * Emitted, if settings have been changed.
   */
  void settingsChanged();

 private:

  /** Called to load the configuration file data. */
  void load();

  /** Called to save the configuration file data.*/
  void save();

  GliderListWidget* m_list;
  QBoxLayout *m_buttonrow;
  int m_added;
};

#endif
