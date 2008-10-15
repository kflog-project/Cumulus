/***********************************************************************
**
**   airfieldlistwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef AIRFIELD_LISTWIDGET_H
#define AIRFIELD_LISTWIDGET_H

#include <QWidget>

#include "waypoint.h"
#include "wplistwidgetparent.h"
#include "mapcontents.h"

/**
 * This widget provides a list of airfields and a means to select one.
 * @author André Somers
 */

class AirfieldListWidget : public WpListWidgetParent
{
    Q_OBJECT

public:

    AirfieldListWidget(QWidget *parent=0);
    ~AirfieldListWidget();

    /**
     * @returns a pointer to the currently highlighted waypoint.
     */
    wayPoint *getSelectedWaypoint();

    enum MapContents::MapContentsListID itemList[2];

    /**
     * Called to fill the display list
     */
    void fillWpList();

private:

    wayPoint *wp;

private:

class _AirfieldItem : public QTreeWidgetItem
    {
    public:
        _AirfieldItem(QTreeWidget*, Airport*, int type = 1000);
        Airport* airport;
    };
};

#endif
