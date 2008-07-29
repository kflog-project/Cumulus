/***********************************************************************
**
**   gliderlist.h
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

#ifndef GLIDERLIST_H
#define GLIDERLIST_H

#include <QWidget>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QStringList>

#include "glider.h"


/**
 * This widget provides a list of gliders and a means to select one.
 * @author André Somers
 */


class GliderList : public QTreeWidget
{
    Q_OBJECT

public:

    GliderList(QWidget *parent=0);
    ~GliderList();

    /**
     * @returns the id of the currently highlighted glider.
     */
    Glider * getSelectedGlider(bool take=false);

    /**
     * Retreives the gliders from the configfile, and fills the list.
     */
    void fillList();

    void save();

    QList<Glider*> Gliders;

    bool has_changed()
    {
        return _changed;
    };

    /**
     * Retrieves the glider object for the glider that was last stored
     * as the selected glider.
     * 
     * @returns a @ref Glider object representing the stored glider,
     * or 0 if an error occured or there was no stored selection.
     */
    static Glider * getStoredSelection();

    /**
     * Stores a reference in the configurationfile that this glider was
     * the last selected glider. This is used to restore the selection
     * after a restart of Cumulus.
     */
    static void setStoredSelection(Glider*);

    /**
     * Sets the selection to the item with this registration string.
     */
    void selectItemFromReg(const QString& registration);

public slots:
    /**
     * Called if a glider has been edited.
     */
    void slot_Edited(Glider * glider);

    /**
     * Called if a glider has been added.
     */
    void slot_Added(Glider * glider);

    /**
     * Called if a glider has been deleted.
     */
    void slot_Deleted(Glider * glider);

    /**
     * Called when a glider has been selected from the listview
     */

private:
    QStandardItemModel *data;
    int _added;
    bool _changed;
};


#endif
