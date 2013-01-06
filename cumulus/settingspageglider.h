/***********************************************************************
**
**   settingspageglider.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
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
 * \date 2002-2013
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

public slots:

  /**
   * called to initiate saving to the configuration file
   */
  void slot_save();

  /**
   * Called to initiate loading of the configuration file.
   */
  void slot_load();

  /**
   * Called to ask is confirmation on the close is needed.
   */
  void slot_query_close(bool& warn, QStringList& warnings);

protected:

  void showEvent( QShowEvent* event );

private slots:

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

private:

  GliderListWidget* m_list;
  QBoxLayout *m_buttonrow;
  int m_added;
};

#endif
