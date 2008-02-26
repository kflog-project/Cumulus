/***********************************************************************
**
**   airfieldlistview.h
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

#ifndef AIRFIELDISTVIEW_H
#define AIRFIELDISTVIEW_H

#include <QWidget>
#include <Q3ListView>
#include <Q3ListViewItem>
#include <QPushButton>
#include <QBoxLayout>
#include <QMainWindow>

#include "waypoint.h"
#include "listviewfilter.h"
#include "mapcontents.h"

/**
 * This widget provides a list of waypoints and a means to select one.
 * @author André Somers
 */

class AirfieldListView : public QWidget
{
    Q_OBJECT

public:

    AirfieldListView(QMainWindow *parent=0);
    ~AirfieldListView();

    /**
     * @returns a pointer to the currently highlighted waypoint.
     */
    wayPoint *getSelectedAirfield(Q3ListView *list = 0);

    enum MapContents::MapContentsListID itemList[3];

    /**
     * Called when the selected waypoint should be deleted from the catalog
     */
    void fillWpList(Q3ListView *list = 0, ListViewFilter *filter = 0);

private:

    Q3ListView* list;
    QMainWindow *par;
    QBoxLayout *buttonrow;
    ListViewFilter *filter;
    wayPoint *wp;
    bool listFilled;
    QPushButton *cmdSelect;

protected:

    void showEvent(QShowEvent *);

private: // Private methods

public slots: // Public slots
    /**
     * This signal is called to indicate that a selection has been made.
     */
    void slot_Select();
    /**
     * This slot is called if the info button has been clicked, or the user pressed 'i'
     */
    void slot_Info();
    /**
     * Called when the listview should be closed without selection
     */
    void slot_Close ();

    /**
     * Called to set a waypoint as homesite
     */
    void slot_setHome();

    void slot_Selected();

signals: // Signals
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint*, bool);

    /**
     * This signal is send if the selection is done, and the screen can be closed.
     */
    void done();

    /**
     * Emitted if the user clicks the Info button.
     */
    void info(wayPoint*);

    /**
     * Emitted if a new home position is selected
     */
    void newHomePosition(const QPoint*);

private:

class _AirfieldItem : public Q3ListViewItem
    {
    public:
        _AirfieldItem(Q3ListView*, Airport*);
        Airport* airport;

    };

};

#endif
