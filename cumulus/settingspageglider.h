/***********************************************************************
**
**   settingspagegliderlist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGSPAGEGLIDER_H
#define SETTINGSPAGEGLIDER_H

#include <QWidget>
#include <QBoxLayout>
#include <QListView>
#include <QStringList>

//#include "glider.h"
#include "gliderlistwidget.h"


/**
 * This widget provides an interface to add, edit and delete gliders
 * from the gliderlist.
 *
 * @author André Somers
 */

class SettingsPageGlider : public QWidget
{
  Q_OBJECT

public:

    SettingsPageGlider(QWidget *parent=0);
    ~SettingsPageGlider();

public slots: // Public slots
    /**
     * called to initiate saving to the configurationfile
     */
    void slot_save();

    /**
     * Called to initiate loading of the configurationfile.
     */
    void slot_load();

    /**
     * Called to ask is confirmation on the close is needed.
     */
    void slot_query_close(bool& warn, QStringList& warnings);

protected:

    void showEvent(QShowEvent *);

private slots: // Private slots
    /**
     * Called when the selected glider should be deleted from the list
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
    GliderListWidget* list;
    QBoxLayout *buttonrow;
    int _added;

};

#endif
