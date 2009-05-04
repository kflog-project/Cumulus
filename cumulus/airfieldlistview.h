/***********************************************************************
**
**   airfieldlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2009 by Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef AIRFIELD_LIST_VIEW_H
#define AIRFIELD_LIST_VIEW_H

#include <QPushButton>
#include <QBoxLayout>
#include <QMainWindow>
#include <QVector>

#include "airfieldlistwidget.h"
#include "waypoint.h"
#include "mapcontents.h"

/**
 * This widget provides a list of waypoints and a means to select one.
 * @author André Somers
 */

class AirfieldListView : public QWidget
{
    Q_OBJECT

public:

    AirfieldListView( QVector<enum MapContents::MapContentsListID> &itemList,
                      QMainWindow *parent=0);
                      
    ~AirfieldListView();

    /**
     * @returns a pointer to the currently high lighted waypoint.
     */
    wayPoint *getSelectedAirfield(QTreeWidget *list=0);

    AirfieldListWidget* listWidget() {
      return listw;
    };

    wayPoint* getSelectedWaypoint() {
      return listw->getSelectedWaypoint();
    };

private:

    AirfieldListWidget* listw;
    QMainWindow *par;
    QBoxLayout *buttonrow;
    QPushButton *cmdSelect;
    QPushButton *cmdHome;

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
     * Called to set a point as home site
     */
    void slot_Home();

    void slot_Selected();

    /**
     * Called to reload the airfield item list
     */
    void slot_reloadList()
    {
      listw->refillWpList();
    };

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
    void newHomePosition(const QPoint&);

};

#endif
