/***********************************************************************
**
**   tasklistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
**   Displays all points of a task as list with different buttons for
**   actions. Can be used in two modes, as display only, buttons for
**   actions are not visible or with command buttons.
**
***********************************************************************/

#ifndef TASK_LIST_VIEW_H
#define TASK_LIST_VIEW_H

#include <QWidget>
#include <QTreeWidget>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>

#include "rowdelegate.h"
#include "waypoint.h"
#include "flighttask.h"

class MainWindow;

/**
 * Presents a view that holds a list of the waypoints in the currently selected task.
 * @author André Somers
 */
class TaskListView : public QWidget
{
    Q_OBJECT
public:
  TaskListView( QWidget *parent=0, bool showButtons=true );

    ~TaskListView();

    /**
     * @Returns a pointer to the currently highlighted waypoint.
     */
    wayPoint *getSelectedWaypoint();

    /** clears all data */
    void clear();

public slots: // Public slots
    /**
     * This signal is called to indicate that a selection has
     * been made.
     */
    void slot_Select();
    /**
     * This slot is called if the info button has been clicked,
     * or the user pressed 'i'
     */
    void slot_Info();
    /**
     * Called when the listview should be closed without selection
     */
    void slot_Close ();
    /**
     * Retreives the waypoints from the task, and fills the list.
     */
    void slot_setTask(const FlightTask *);

    /**
     * Updates the internal task data. Will be called after
     * configuration changes of task sector items
     */
    void slot_updateTask();

signals: // Signals
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint*, bool);
    /**
     * This signal is send if the selection is done, and the
     * screen can be closed.
     */
    void done();
    /**
     * Emitted if the user clicks the Info button.
     */
    void info(wayPoint*);


protected:
    void showEvent(QShowEvent *);

private:
    QTreeWidget*  list;
    MainWindow  *par;
    QBoxLayout  *buttonrow;
    bool        _outlandShow;
    QPushButton *cmdShowOl;
    QPushButton *cmdHideOl;
    QPushButton *cmdSelect;
    QLabel      *distTotal;
    QLabel      *speedTotal;
    QLabel      *timeTotal;
    QPixmap     _arrows;
    FlightTask* _task;
    wayPoint *  _selectedWp;
    QTreeWidgetItem * _currSelectedTp;
    QTreeWidgetItem * _newSelectedTp;
    QString _selectText, _unselectText;

    // flag for showing buttons (or not)
    bool _showButtons;

    RowDelegate* rowDelegate;


private slots:
    /**
     * This slot is called if the user selects a waypoint in the task
     */
    void slot_Selected();


private:

    class _TaskPoint : public QTreeWidgetItem
    {
    public:
//        _TaskPoint(QTreeWidget* parent, const QStringList& strings, int type=0, waypoint* wp ):
//                  QTreeWidgetItem( parent, strings, type );
        _TaskPoint(QTreeWidget* wpList, wayPoint* point);
        wayPoint *wp;
    };
};

#endif
